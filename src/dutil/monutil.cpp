//-------------------------------------------------------------------------------------------------
// <copyright file="monutil.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Filesystem / registry monitor helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

const int MON_ARRAY_GROWTH = 40;

enum MON_MESSAGE
{
    MON_MESSAGE_ADD = WM_APP + 1,
    MON_MESSAGE_REMOVE,
    MON_MESSAGE_STOP
};

enum MON_TYPE
{
    MON_NONE = 0,
    MON_DIRECTORY = 1,
    MON_REGKEY = 2
};

struct MON_REQUEST
{
    MON_TYPE type;

    BOOL fRecursive;
    void *pvContext;

    DWORD dwPathHierarchyIndex;
    LPWSTR *rgsczPathHierarchy;
    DWORD cPathHierarchy;

    // If the notify fires, fPendingFire gets set to TRUE, and we wait to see if other writes are occurring, and only after the configured silence period do we notify of changes
    // after notification, we set fPendingFire back to FALSE
    BOOL fPendingFire;
    BOOL fSkipDeltaAdd;
    DWORD dwSilencePeriod;

    union
    {
        struct
        {
        } directory;
        struct
        {
            HKEY hkRoot;
            HKEY hkSubKey;
        } regkey;
    };
};

struct MON_ADD_MESSAGE
{
    MON_REQUEST request;
    HANDLE handle;
};

struct MON_REMOVE_MESSAGE
{
    MON_TYPE type;

    union
    {
        struct
        {
            LPWSTR sczDirectory;
        } directory;
        struct
        {
            HKEY hkRoot;
            LPWSTR sczSubKey;
        } regkey;
    };
};

struct MON_STRUCT
{
    DWORD dwSilencePeriodInMs;

    HANDLE hWaiterThread;
    DWORD dwWaiterThreadId;
    BOOL fWaiterThreadMessageQueueInitialized;

    // Callbacks
    PFN_MONGENERAL vpfMonGeneral;
    PFN_MONDIRECTORY vpfMonDirectory;
    PFN_MONREGKEY vpfMonRegKey;

    // Context for callbacks
    LPVOID pvContext;

    // HANDLEs are in their own array for easy use with WaitForMultipleObjects()
    // After initialization, the very first handle is just to wake the listener thread to have it re-wait on a new list
    HANDLE *rgHandles;
    DWORD cHandles;

    // Requested things to monitor
    MON_REQUEST *rgRequests;
    DWORD cRequests;

    // Array of pending notifications (each item is an index into requests array)
    // Pending notification happens when changes have been detected, but the silence period has not yet surpassed
    DWORD *rgRequestsPending;
    DWORD cRequestsPending;
};

const int MON_HANDLE_BYTES = sizeof(MON_STRUCT);

// Initiates (or if *pHandle is non-null, continues) wait on the directory or subkey
// if the directory or subkey doesn't exist, instead calls it on the first existing parent directory or subkey
// writes to pRequest->dwPathHierarchyIndex with the array index that was waited on
static HRESULT InitiateWait(
    __inout MON_REQUEST *pRequest,
    __inout HANDLE *pHandle
    );
static DWORD WINAPI WaiterThread(
    __in_bcount(sizeof(MON_STRUCT)) LPVOID pvContext
    );
static void Notify(
    __in HRESULT hr,
    __in MON_STRUCT *pm,
    __in MON_REQUEST *pRequest
    );
static void MonRequestDestroy(
    __in MON_REQUEST *pRequest
    );
static void MonAddMessageDestroy(
    __in MON_ADD_MESSAGE *pMessage
    );
static void MonRemoveMessageDestroy(
    __in MON_REMOVE_MESSAGE *pMessage
    );
static BOOL GetRecursiveFlag(
    __in MON_REQUEST *pRequest,
    __in DWORD dwIndex
    );
static HRESULT FindRequestIndex(
    __in MON_STRUCT *pm,
    __in MON_REMOVE_MESSAGE *pMessage,
    __out DWORD *pdwIndex
    );
static void RemoveRequest(
    __inout MON_STRUCT *pm,
    __in DWORD dwRequestIndex
    );
static void RemoveFromPendingRequestArray(
    __inout MON_STRUCT *pm,
    __in DWORD dwRequestIndex
    );

extern "C" HRESULT DAPI MonCreate(
    __out_bcount(MON_HANDLE_BYTES) MON_HANDLE *pHandle,
    __in PFN_MONGENERAL vpfMonGeneral,
    __in_opt PFN_MONDIRECTORY vpfMonDirectory,
    __in_opt PFN_MONREGKEY vpfMonRegKey,
    __in_opt LPVOID pvContext,
    __in DWORD dwSilencePeriodInMs
    )
{
    HRESULT hr = S_OK;
    DWORD dwRetries = 1000;
    const DWORD dwRetryPeriodInMs = 10;

    ExitOnNull(pHandle, hr, E_INVALIDARG, "Pointer to handle not specified while creating monitor");

    // Allocate the struct
    *pHandle = static_cast<MON_HANDLE>(MemAlloc(sizeof(MON_STRUCT), TRUE));
    ExitOnNull(*pHandle, hr, E_OUTOFMEMORY, "Failed to allocate monitor object");

    MON_STRUCT *pm = static_cast<MON_STRUCT *>(*pHandle);

    hr = MemEnsureArraySize(reinterpret_cast<void **>(&pm->rgHandles), pm->cHandles + 1, sizeof(HANDLE), MON_ARRAY_GROWTH);
    ExitOnFailure(hr, "Failed to allocate first handle");
    ++pm->cHandles;

    pm->rgHandles[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
    ExitOnNullWithLastError(pm->rgHandles[0], hr, "Failed to create general event");

    pm->vpfMonGeneral = vpfMonGeneral;
    pm->vpfMonDirectory = vpfMonDirectory;
    pm->vpfMonRegKey = vpfMonRegKey;
    pm->pvContext = pvContext;
    pm->dwSilencePeriodInMs = dwSilencePeriodInMs;

    pm->hWaiterThread = ::CreateThread(NULL, 0, WaiterThread, pm, 0, &pm->dwWaiterThreadId);
    if (!pm->hWaiterThread)
    {
        ExitWithLastError(hr, "Failed to create UI thread.");
    }

    // Ensure the created thread initializes its message queue. It does this first thing, so if it doesn't within 10 seconds, there must be a huge problem.
    while (!pm->fWaiterThreadMessageQueueInitialized && 0 < dwRetries)
    {
        ::Sleep(dwRetryPeriodInMs);
        --dwRetries;
    }

    if (dwRetries == 0)
    {
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Waiter thread apparently never initialized its message queue.");
    }

LExit:
    return hr;
}

extern "C" HRESULT DAPI MonAddDirectory(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in_z LPCWSTR wzDirectory,
    __in BOOL fRecursive,
    __in_opt LPVOID pvDirectoryContext
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    MON_ADD_MESSAGE *pMessage = NULL;
    BOOL fRet = FALSE;

    if (wzDirectory[lstrlenW(wzDirectory) - 1] != L'\\')
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "File path %ls must end in a backslash", wzDirectory);
    }

    pMessage = reinterpret_cast<MON_ADD_MESSAGE *>(MemAlloc(sizeof(MON_ADD_MESSAGE), TRUE));
    ExitOnNull(pMessage, hr, E_OUTOFMEMORY, "Failed to allocate memory for message");

    pMessage->handle = INVALID_HANDLE_VALUE;
    pMessage->request.type = MON_DIRECTORY;
    pMessage->request.fRecursive = fRecursive;
    pMessage->request.pvContext = pvDirectoryContext;

    hr = PathGetHierarchyArray(wzDirectory, &pMessage->request.rgsczPathHierarchy, reinterpret_cast<LPUINT>(&pMessage->request.cPathHierarchy));
    ExitOnFailure1(hr, "Failed to get hierarchy array for path %ls", wzDirectory);

    hr = InitiateWait(&pMessage->request, &pMessage->handle);
    ExitOnFailure(hr, "Failed to initiate wait");

    if (!::PostThreadMessageW(pm->dwWaiterThreadId, MON_MESSAGE_ADD, reinterpret_cast<WPARAM>(pMessage), 0))
    {
        ExitWithLastError1(hr, "Failed to send message to worker thread to add directory wait for path %ls", wzDirectory);
    }
    pMessage = NULL;

    fRet = ::SetEvent(pm->rgHandles[0]);
    if (!fRet)
    {
        ExitWithLastError(hr, "Failed to set event to notify worker thread of incoming message");
    }

LExit:
    MonAddMessageDestroy(pMessage);

    return hr;
}

extern "C" HRESULT DAPI MonAddRegKey(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in BOOL fRecursive,
    __in_opt LPVOID pvRegKeyContext
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    MON_ADD_MESSAGE *pMessage = NULL;
    BOOL fRet = FALSE;

    if (wzSubKey[lstrlenW(wzSubKey) - 1] != L'\\')
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Registry subkey %ls must end in a backslash", wzSubKey);
    }

    pMessage = reinterpret_cast<MON_ADD_MESSAGE *>(MemAlloc(sizeof(MON_ADD_MESSAGE), TRUE));
    ExitOnNull(pMessage, hr, E_OUTOFMEMORY, "Failed to allocate memory for message");

    pMessage->handle = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == pMessage->handle) // Yes, unfortunately NULL is the invalid handle value for regkey waits, INVALID_HANDLE_VALUE is for directory waits. yuck.
    {
        ExitWithLastError(hr, "Failed to create anonymous event for regkey monitor");
    }
    pMessage->request.type = MON_REGKEY;
    pMessage->request.regkey.hkRoot = hkRoot;
    pMessage->request.fRecursive = fRecursive;
    pMessage->request.pvContext = pvRegKeyContext;

    hr = PathGetHierarchyArray(wzSubKey, &pMessage->request.rgsczPathHierarchy, reinterpret_cast<LPUINT>(&pMessage->request.cPathHierarchy));
    ExitOnFailure1(hr, "Failed to get hierarchy array for subkey %ls", wzSubKey);

    hr = InitiateWait(&pMessage->request, &pMessage->handle);
    ExitOnFailure(hr, "Failed to initiate wait");

    if (!::PostThreadMessageW(pm->dwWaiterThreadId, MON_MESSAGE_ADD, reinterpret_cast<WPARAM>(pMessage), 0))
    {
        ExitWithLastError1(hr, "Failed to send message to worker thread to add directory wait for regkey %ls", wzSubKey);
    }
    pMessage = NULL;

    fRet = ::SetEvent(pm->rgHandles[0]);
    if (!fRet)
    {
        ExitWithLastError(hr, "Failed to set event to notify worker thread of incoming message");
    }

LExit:
    MonAddMessageDestroy(pMessage);

    return hr;
}

extern "C" HRESULT DAPI MonRemoveDirectory(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in_z LPCWSTR wzDirectory
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    MON_REMOVE_MESSAGE *pMessage = NULL;
    BOOL fRet = FALSE;

    if (wzDirectory[lstrlenW(wzDirectory) - 1] != L'\\')
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "File path %ls must end in a backslash", wzDirectory);
    }

    pMessage = reinterpret_cast<MON_REMOVE_MESSAGE *>(MemAlloc(sizeof(MON_REMOVE_MESSAGE), TRUE));
    ExitOnNull(pMessage, hr, E_OUTOFMEMORY, "Failed to allocate memory for message");

    pMessage->type = MON_DIRECTORY;

    hr = StrAllocString(&pMessage->directory.sczDirectory, wzDirectory, 0);
    ExitOnFailure(hr, "Failed to allocate copy of directory string");

    if (!::PostThreadMessageW(pm->dwWaiterThreadId, MON_MESSAGE_REMOVE, reinterpret_cast<WPARAM>(pMessage), 0))
    {
        ExitWithLastError1(hr, "Failed to send message to worker thread to add directory wait for path %ls", wzDirectory);
    }
    pMessage = NULL;

    fRet = ::SetEvent(pm->rgHandles[0]);
    if (!fRet)
    {
        ExitWithLastError(hr, "Failed to set event to notify worker thread of incoming message");
    }

LExit:
    MonRemoveMessageDestroy(pMessage);

    return hr;
}

extern "C" HRESULT DAPI MonRemoveRegKey(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    MON_REMOVE_MESSAGE *pMessage = NULL;
    BOOL fRet = FALSE;

    if (wzSubKey[lstrlenW(wzSubKey) - 1] != L'\\')
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Subkey %ls must end in a backslash", wzSubKey);
    }

    pMessage = reinterpret_cast<MON_REMOVE_MESSAGE *>(MemAlloc(sizeof(MON_REMOVE_MESSAGE), TRUE));
    ExitOnNull(pMessage, hr, E_OUTOFMEMORY, "Failed to allocate memory for message");

    pMessage->type = MON_REGKEY;
    pMessage->regkey.hkRoot = hkRoot;

    hr = StrAllocString(&pMessage->regkey.sczSubKey, wzSubKey, 0);
    ExitOnFailure(hr, "Failed to allocate copy of directory string");

    if (!::PostThreadMessageW(pm->dwWaiterThreadId, MON_MESSAGE_REMOVE, reinterpret_cast<WPARAM>(pMessage), 0))
    {
        ExitWithLastError1(hr, "Failed to send message to worker thread to add directory wait for path %ls", wzSubKey);
    }
    pMessage = NULL;

    fRet = ::SetEvent(pm->rgHandles[0]);
    if (!fRet)
    {
        ExitWithLastError(hr, "Failed to set event to notify worker thread of incoming message");
    }

LExit:
    MonRemoveMessageDestroy(pMessage);

    return hr;
}

extern "C" void DAPI MonDestroy(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    BOOL fRet = FALSE;

    if (!::PostThreadMessageW(pm->dwWaiterThreadId, MON_MESSAGE_STOP, 0, 0))
    {
        ExitWithLastError(hr, "Failed to send message to worker thread to stop monitoring");
    }

    fRet = ::SetEvent(pm->rgHandles[0]);
    if (!fRet)
    {
        ExitWithLastError(hr, "Failed to set event to notify worker thread of incoming message");
    }

    if (pm->hWaiterThread)
    {
        ::WaitForSingleObject(pm->hWaiterThread, INFINITE);
        ::CloseHandle(pm->hWaiterThread);
    }

    for (DWORD i = 0; i < pm->cRequests; ++i)
    {
        switch (pm->rgRequests[i].type)
        {
        case MON_DIRECTORY:
            if (INVALID_HANDLE_VALUE != pm->rgHandles[i + 1])
            {
                ::FindCloseChangeNotification(pm->rgHandles[i + 1]);
            }
            break;
        case MON_REGKEY:
            if (NULL != pm->rgHandles[i + 1])
            {
                ::CloseHandle(pm->rgHandles[i + 1]);
                pm->rgHandles[i + 1] = NULL;
            }
            ReleaseRegKey(pm->rgRequests[i].regkey.hkSubKey);
            break;
        default:
            Assert(false);
        }

        MonRequestDestroy(pm->rgRequests + i);
    }

    ReleaseMem(pm->rgRequestsPending);

LExit:
    return;
}

static void MonRequestDestroy(
    __in MON_REQUEST *pRequest
    )
{
    if (NULL != pRequest)
    {
        ReleaseStrArray(pRequest->rgsczPathHierarchy, pRequest->cPathHierarchy);
        if (MON_REGKEY == pRequest->type)
        {
            ReleaseRegKey(pRequest->regkey.hkSubKey);
        }
    }
}

static void MonAddMessageDestroy(
    __in MON_ADD_MESSAGE *pMessage
    )
{
    if (NULL != pMessage)
    {
        if (pMessage->handle != INVALID_HANDLE_VALUE)
        {
            switch (pMessage->request.type)
            {
            case MON_DIRECTORY:
                ::FindCloseChangeNotification(pMessage->handle);
                break;
            case MON_REGKEY:
                ReleaseFile(pMessage->handle);
                break;
            default:
                Assert(false);
            }
        }
        MonRequestDestroy(&pMessage->request);

        ReleaseMem(pMessage);
    }
}

static void MonRemoveMessageDestroy(
    __in MON_REMOVE_MESSAGE *pMessage
    )
{
    if (NULL != pMessage)
    {
        switch (pMessage->type)
        {
        case MON_DIRECTORY:
            ReleaseStr(pMessage->directory.sczDirectory);
            break;
        case MON_REGKEY:
            ReleaseStr(pMessage->regkey.sczSubKey);
            break;
        default:
            Assert(false);
        }

        ReleaseMem(pMessage);
    }
}

static HRESULT InitiateWait(
    __inout MON_REQUEST *pRequest,
    __inout HANDLE *pHandle
    )
{
    HRESULT hr = S_OK;
    HRESULT hrTemp = S_OK;
    BOOL fRedo = FALSE;
    BOOL fHandleFound;
    DWORD er = ERROR_SUCCESS;
    DWORD dwIndex = 0;
    DWORD dwRetryCount = 0;
    // Wait up to 5 seconds for the path to be available for watching
    const DWORD c_dwMaxRetryCount = 250;
    const DWORD c_dwRetryIntervalInMs = 20;
    HKEY hk = NULL;
    HANDLE hTemp = INVALID_HANDLE_VALUE;

    do
    {
        fRedo = FALSE;
        fHandleFound = FALSE;

        for (DWORD i = 0; i < pRequest->cPathHierarchy && !fHandleFound && !fRedo; ++i)
        {
            dwIndex = pRequest->cPathHierarchy - i - 1;
            switch (pRequest->type)
            {
            case MON_DIRECTORY:
                if (INVALID_HANDLE_VALUE != *pHandle)
                {
                    ::FindCloseChangeNotification(*pHandle);
                    *pHandle = INVALID_HANDLE_VALUE;
                }

                *pHandle = ::FindFirstChangeNotificationW(pRequest->rgsczPathHierarchy[dwIndex], GetRecursiveFlag(pRequest, dwIndex), FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);
                if (INVALID_HANDLE_VALUE == *pHandle)
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr)
                    {
                        hr = S_OK;
                        continue;
                    }
                    else if (E_ACCESSDENIED == hr)
                    {
                        ++dwRetryCount;
                        if (dwRetryCount > c_dwMaxRetryCount)
                        {
                            ExitOnFailure3(hr, "Repeated access denied errors on path %ls, even after max %u retries of %u ms each", pRequest->rgsczPathHierarchy[dwIndex], c_dwMaxRetryCount, c_dwRetryIntervalInMs);
                        }
                        else
                        {
                            // Unfortunately, access denied can occur in situations where the directory is in the process of being deleted, so retry a few times before giving up
                            ::Sleep(c_dwRetryIntervalInMs);
                            fRedo = TRUE;
                            break;
                        }
                    }
                    ExitOnWin32Error1(er, hr, "Failed to wait on path %ls", pRequest->rgsczPathHierarchy[dwIndex]);
                }
                else
                {
                    fHandleFound = TRUE;
                    hr = S_OK;
                }
                break;
            case MON_REGKEY:
                ReleaseRegKey(pRequest->regkey.hkSubKey);
                hr = RegOpen(pRequest->regkey.hkRoot, pRequest->rgsczPathHierarchy[dwIndex], KEY_NOTIFY, &pRequest->regkey.hkSubKey);
                if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr)
                {
                    hr = S_OK;
                    continue;
                }

                er = ::RegNotifyChangeKeyValue(pRequest->regkey.hkSubKey, GetRecursiveFlag(pRequest, dwIndex), REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, *pHandle, TRUE);
                ReleaseRegKey(hk);
                hr = HRESULT_FROM_WIN32(er);
                if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr || HRESULT_FROM_WIN32(ERROR_KEY_DELETED) == hr)
                {
                    hr = S_OK;
                    continue;
                }
                else
                {
                    ExitOnWin32Error1(er, hr, "Failed to wait on subkey %ls", pRequest->rgsczPathHierarchy[dwIndex]);

                    fHandleFound = TRUE;
                }

                break;
            default:
                return E_INVALIDARG;
            }
        }

        if (!fRedo)
        {
            pRequest->dwPathHierarchyIndex = dwIndex;

            // If we're monitoring a parent instead of the real path because the real path didn't exist, double-check the child hasn't been created since.
            // If it has, restart the whole loop
            if (dwIndex < pRequest->cPathHierarchy - 1)
            {
                switch (pRequest->type)
                {
                case MON_DIRECTORY:
                    hTemp = ::FindFirstChangeNotificationW(pRequest->rgsczPathHierarchy[dwIndex + 1], GetRecursiveFlag(pRequest, dwIndex + 1), FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);
                    if (INVALID_HANDLE_VALUE != hTemp)
                    {
                        ::FindCloseChangeNotification(hTemp);
                        fRedo = TRUE;
                    }
                    break;
                case MON_REGKEY:
                    hrTemp = RegOpen(pRequest->regkey.hkRoot, pRequest->rgsczPathHierarchy[dwIndex + 1], KEY_NOTIFY, &hk);
                    ReleaseRegKey(hk);
                    fRedo = SUCCEEDED(hrTemp);
                    break;
                default:
                    Assert(false);
                }
            }
        }
    } while (fRedo);

    ExitOnFailure1(hr, "Didn't get a successful wait after looping through all available options %ls", pRequest->rgsczPathHierarchy[0]);

LExit:
    ReleaseRegKey(hk);

    return hr;
}

static DWORD WINAPI WaiterThread(
    __in_bcount(sizeof(MON_STRUCT)) LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    DWORD dwRet = 0;
    BOOL fAgain = FALSE;
    BOOL fContinue = TRUE;
    BOOL fNotify = FALSE;
    BOOL fRet = FALSE;
    MSG msg = { };
    MON_ADD_MESSAGE *pAddMessage = NULL;
    MON_REMOVE_MESSAGE *pRemoveMessage = NULL;
    MON_STRUCT *pm = reinterpret_cast<MON_STRUCT *>(pvContext);
    DWORD dwRequestIndex;
    // If we have one or more requests pending notification, this is the period we intend to wait for multiple objects (shortest amount of time to next potential notify)
    DWORD dwWait = 0;
    DWORD uCurrentTime = 0;
    DWORD uLastTimeInMs = ::GetTickCount();
    DWORD uDeltaInMs = 0;

    // Ensure the thread has a message queue
    ::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    pm->fWaiterThreadMessageQueueInitialized = TRUE;

    do
    {
        dwRet = ::WaitForMultipleObjects(pm->cHandles, pm->rgHandles, FALSE, pm->cRequestsPending > 0 ? dwWait : INFINITE);

        uCurrentTime = ::GetTickCount();
        uDeltaInMs = uCurrentTime - uLastTimeInMs;
        uLastTimeInMs = uCurrentTime;

        if (WAIT_OBJECT_0 == dwRet)
        {
            do
            {
                fRet = ::PeekMessage(&msg, reinterpret_cast<HWND>(-1), 0, 0, PM_REMOVE);
                fAgain = fRet;
                if (fRet)
                {
                    switch (msg.message)
                    {
                        case MON_MESSAGE_ADD:
                            pAddMessage = reinterpret_cast<MON_ADD_MESSAGE *>(msg.wParam);

                            hr = MemEnsureArraySize(reinterpret_cast<void **>(&pm->rgHandles), pm->cHandles + 1, sizeof(HANDLE), MON_ARRAY_GROWTH);
                            ExitOnFailure(hr, "Failed to allocate additional handle for request");
                            ++pm->cHandles;

                            hr = MemEnsureArraySize(reinterpret_cast<void **>(&pm->rgRequests), pm->cRequests + 1, sizeof(MON_REQUEST), MON_ARRAY_GROWTH);
                            ExitOnFailure(hr, "Failed to allocate additional request struct");
                            ++pm->cRequests;

                            pm->rgRequests[pm->cRequests - 1] = pAddMessage->request;
                            pm->rgHandles[pm->cHandles - 1] = pAddMessage->handle;

                            ReleaseNullMem(pAddMessage);
                            break;
                        case MON_MESSAGE_REMOVE:
                            pRemoveMessage = reinterpret_cast<MON_REMOVE_MESSAGE *>(msg.wParam);

                            // Find the request to remove
                            hr = FindRequestIndex(pm, pRemoveMessage, &dwRequestIndex);
                            ExitOnFailure(hr, "Failed to find request index for remove message");

                            RemoveRequest(pm, dwRequestIndex);

                            MonRemoveMessageDestroy(pRemoveMessage);
                            pRemoveMessage = NULL;
                            break;
                        case MON_MESSAGE_STOP:
                            // Stop requested, so abort the whole thread
                            fAgain = FALSE;
                            fContinue = FALSE;
                            break;
                        default:
                            Assert(false);
                            break;
                    }
                }
            } while (fAgain);
        }
        else if (dwRet >= WAIT_OBJECT_0 && dwRet - WAIT_OBJECT_0 < pm->cHandles)
        {
            // OK a handle fired - only notify if it's the actual target, and not just some parent waiting for the target child to exist
            dwRequestIndex = dwRet - WAIT_OBJECT_0 - 1;
            fNotify = (pm->rgRequests[dwRequestIndex].dwPathHierarchyIndex == pm->rgRequests[dwRequestIndex].cPathHierarchy - 1);

            // Initiate re-waits before we notify callback, to ensure we don't miss a single update
            hr = InitiateWait(pm->rgRequests + dwRequestIndex, pm->rgHandles + dwRequestIndex + 1);
            if (FAILED(hr))
            {
                // If there's an error re-waiting, we immediately notify of the error and remove this notification from the queue, because something went wrong
                Notify(hr, pm, pm->rgRequests + dwRequestIndex);
                RemoveRequest(pm, dwRequestIndex);
            }
            // If there were no errors and we were already waiting on the right target, or if we weren't yet but are able to now, it's a successful notify
            else if (fNotify || (pm->rgRequests[dwRequestIndex].dwPathHierarchyIndex == pm->rgRequests[dwRequestIndex].cPathHierarchy - 1))
            {
                Trace1(REPORT_DEBUG, "Changes detected, waiting for silence period index %u", dwRequestIndex);

                if (0 < pm->dwSilencePeriodInMs)
                {
                    pm->rgRequests[dwRequestIndex].dwSilencePeriod = 0;
                    pm->rgRequests[dwRequestIndex].fSkipDeltaAdd = TRUE;

                    if (!pm->rgRequests[dwRequestIndex].fPendingFire)
                    {
                        pm->rgRequests[dwRequestIndex].fPendingFire = TRUE;

                        // Append this pending request to the pending request array
                        hr = MemEnsureArraySize(reinterpret_cast<void **>(&pm->rgRequestsPending), pm->cRequestsPending + 1, sizeof(DWORD), 10);
                        ExitOnFailure(hr, "Failed to increase size of pending requests array");
                        ++pm->cRequestsPending;

                        pm->rgRequestsPending[pm->cRequestsPending - 1] = dwRequestIndex;
                    }
                }
                else
                {
                    // If no silence period, notify immediately
                    Notify(S_OK, pm, pm->rgRequests + dwRequestIndex);
                }
            }
        }
        else if (WAIT_TIMEOUT != dwRet)
        {
            ExitWithLastError1(hr, "Failed to wait for multiple objects with return code %u", dwRet);
        }

        // OK, now that we've checked all triggered handles (resetting silence period timers appropriately), check for any pending notifications that we can finally fire
        // And set dwWait appropriately so we awaken at the right time to fire the next pending notification (in case no further writes occur during that time)
        if (0 < pm->cRequestsPending)
        {
            dwWait = pm->dwSilencePeriodInMs;

            for (DWORD i = 0; i < pm->cRequestsPending; ++i)
            {
                dwRequestIndex = pm->rgRequestsPending[i];
                if (pm->rgRequests[dwRequestIndex].fPendingFire)
                {
                    if (pm->rgRequests[dwRequestIndex].fSkipDeltaAdd)
                    {
                        pm->rgRequests[dwRequestIndex].fSkipDeltaAdd = FALSE;
                    }
                    else
                    {
                        pm->rgRequests[dwRequestIndex].dwSilencePeriod += uDeltaInMs;

                        // silence period has elapsed without further notifications, so reset pending-related variables, and finally fire a notify!
                        if (pm->rgRequests[dwRequestIndex].dwSilencePeriod >= pm->dwSilencePeriodInMs)
                        {
                            Trace1(REPORT_DEBUG, "Silence period surpassed, notifying %u ms late", pm->rgRequests[dwRequestIndex].dwSilencePeriod - pm->dwSilencePeriodInMs);
                            Notify(S_OK, pm, pm->rgRequests + dwRequestIndex);
                            RemoveFromPendingRequestArray(pm, dwRequestIndex);
                            pm->rgRequests[dwRequestIndex].fPendingFire = FALSE;
                            pm->rgRequests[dwRequestIndex].fSkipDeltaAdd = FALSE;
                            pm->rgRequests[dwRequestIndex].dwSilencePeriod = 0;
                        }
                        else
                        {
                            // set dwWait to the shortest interval period so that if no changes occur, WaitForMultipleObjects
                            // wakes the thread back up when it's time to fire the next pending notification
                            if (dwWait > pm->dwSilencePeriodInMs - pm->rgRequests[dwRequestIndex].dwSilencePeriod)
                            {
                                dwWait = pm->dwSilencePeriodInMs - pm->rgRequests[dwRequestIndex].dwSilencePeriod;
                            }
                        }
                    }
                }
            }
        }
    } while (fContinue);

    // Don't bother firing pending notifications. We were told to stop monitoring, so client doesn't care.

LExit:
    MonAddMessageDestroy(pAddMessage);
    MonRemoveMessageDestroy(pRemoveMessage);
    if (FAILED(hr))
    {
        // If worker thread fails, notify general callback of an error
        Assert(pm->vpfMonGeneral);
        pm->vpfMonGeneral(hr, pm->pvContext);
    }

    return hr;
}

static void Notify(
    __in HRESULT hr,
    __in MON_STRUCT *pm,
    __in MON_REQUEST *pRequest
    )
{
    switch (pRequest->type)
    {
    case MON_DIRECTORY:
        Assert(pm->vpfMonDirectory);
        pm->vpfMonDirectory(hr, pRequest->rgsczPathHierarchy[pRequest->cPathHierarchy - 1], pm->pvContext, pRequest->pvContext);
        break;
    case MON_REGKEY:
        Assert(pm->vpfMonRegKey);
        pm->vpfMonRegKey(hr, pRequest->regkey.hkRoot, pRequest->rgsczPathHierarchy[pRequest->cPathHierarchy - 1], pm->pvContext, pRequest->pvContext);
        break;
    default:
        Assert(false);
    }
}

static BOOL GetRecursiveFlag(
    __in MON_REQUEST *pRequest,
    __in DWORD dwIndex
    )
{
    if (dwIndex == pRequest->cPathHierarchy - 1)
    {
        return pRequest->fRecursive;
    }
    else
    {
        return FALSE;
    }
}

static HRESULT FindRequestIndex(
    __in MON_STRUCT *pm,
    __in MON_REMOVE_MESSAGE *pMessage,
    __out DWORD *pdwIndex
    )
{
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < pm->cRequests; ++i)
    {
        if (pm->rgRequests[i].type == pMessage->type)
        {
            switch (pm->rgRequests[i].type)
            {
            case MON_DIRECTORY:
                if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pm->rgRequests[i].rgsczPathHierarchy[pm->rgRequests[i].cPathHierarchy - 1], -1, pMessage->directory.sczDirectory, -1))
                {
                    *pdwIndex = i;
                    ExitFunction1(hr = S_OK);
                }
                break;
            case MON_REGKEY:
                if (reinterpret_cast<DWORD_PTR>(pMessage->regkey.hkRoot) == reinterpret_cast<DWORD_PTR>(pm->rgRequests[i].regkey.hkRoot) && CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pm->rgRequests[i].rgsczPathHierarchy[pm->rgRequests[i].cPathHierarchy - 1], -1, pMessage->regkey.sczSubKey, -1))
                {
                    *pdwIndex = i;
                    ExitFunction1(hr = S_OK);
                }
                break;
            default:
                Assert(false);
            }
        }
    }

    hr = E_NOTFOUND;

LExit:
    return hr;
}

static void RemoveRequest(
    __inout MON_STRUCT *pm,
    __in DWORD dwRequestIndex
    )
{
    switch (pm->rgRequests[dwRequestIndex].type)
    {
    case MON_DIRECTORY:
        if (pm->rgHandles[dwRequestIndex + 1] != INVALID_HANDLE_VALUE)
        {
            ::FindCloseChangeNotification(pm->rgHandles[dwRequestIndex + 1]);
        }
        break;
    case MON_REGKEY:
        if (pm->rgHandles[dwRequestIndex + 1] != NULL)
        {
            ::CloseHandle(pm->rgHandles[dwRequestIndex + 1]);
        }
        ReleaseRegKey(pm->rgRequests[dwRequestIndex].regkey.hkSubKey);
        break;
    default:
        Assert(false);
    }

    if (pm->rgRequests[dwRequestIndex].fPendingFire)
    {
        RemoveFromPendingRequestArray(pm, dwRequestIndex);

        --pm->cRequestsPending;
    }

    MemRemoveFromArray(reinterpret_cast<void *>(pm->rgHandles), dwRequestIndex + 1, 1, pm->cHandles, sizeof(HANDLE), TRUE);
    --pm->cHandles;
    MemRemoveFromArray(reinterpret_cast<void *>(pm->rgRequests), dwRequestIndex, 1, pm->cRequests, sizeof(MON_REQUEST), TRUE);
    --pm->cRequests;
}

static void RemoveFromPendingRequestArray(
    __inout MON_STRUCT *pm,
    __in DWORD dwRequestIndex
    )
{
    for (DWORD i = 0; i < pm->cRequestsPending; ++i)
    {
        // We're about to remove this request from the array, which means every index into the array after that point needs to be decremented
        if (pm->rgRequestsPending[i] > dwRequestIndex)
        {
            --pm->rgRequestsPending[i];
        }
        // And if we find the actual index, we need to remove it
        else if (pm->rgRequestsPending[i] == dwRequestIndex)
        {
            MemRemoveFromArray(reinterpret_cast<void *>(pm->rgRequestsPending), i, 1, pm->cRequestsPending, sizeof(DWORD), TRUE);
            --pm->cRequestsPending;
            // We just removed this item, so hit the same index again
            --i;
        }
    }
}
