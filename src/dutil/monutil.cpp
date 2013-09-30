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

const int MON_THREAD_GROWTH = 5;
const int MON_ARRAY_GROWTH = 40;
const int MON_MAX_MONITORS_PER_THREAD = 63;
const int MON_THREAD_INIT_RETRIES = 1000;
const int MON_THREAD_INIT_RETRY_PERIOD_IN_MS = 10;

enum MON_MESSAGE
{
    MON_MESSAGE_ADD = WM_APP + 1,
    MON_MESSAGE_REMOVE,
    MON_MESSAGE_REMOVED, // Sent by waiter thread back to coordinator thread to indicate a remove occurred
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
    DWORD dwMaxSilencePeriodInMs;

    BOOL fRecursive;
    void *pvContext;

    DWORD dwPathHierarchyIndex;
    LPWSTR *rgsczPathHierarchy;
    DWORD cPathHierarchy;

    // If the notify fires, fPendingFire gets set to TRUE, and we wait to see if other writes are occurring, and only after the configured silence period do we notify of changes
    // after notification, we set fPendingFire back to FALSE
    BOOL fPendingFire;
    BOOL fSkipDeltaAdd;
    DWORD dwSilencePeriodInMs;

    union
    {
        struct
        {
        } directory;
        struct
        {
            HKEY hkRoot;
            HKEY hkSubKey;
            REG_KEY_BITNESS kbKeyBitness; // Only used to pass on 32-bit, 64-bit, or default parameter
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
    BOOL fRecursive;

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
            REG_KEY_BITNESS kbKeyBitness;
        } regkey;
    };
};

struct MON_STRUCT
{
    HANDLE hCoordinatorThread;
    DWORD dwCoordinatorThreadId;
    BOOL fCoordinatorThreadMessageQueueInitialized;

    // Callbacks
    PFN_MONGENERAL vpfMonGeneral;
    PFN_MONDIRECTORY vpfMonDirectory;
    PFN_MONREGKEY vpfMonRegKey;

    // Context for callbacks
    LPVOID pvContext;
};

struct MON_WAITER_CONTEXT
{
    DWORD dwCoordinatorThreadId;

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

// Info stored about each waiter by the coordinator
struct MON_WAITER_INFO
{
    DWORD cMonitorCount;

    MON_WAITER_CONTEXT *pWaiterContext;
};

const int MON_HANDLE_BYTES = sizeof(MON_STRUCT);

static DWORD WINAPI CoordinatorThread(
    __in_bcount(sizeof(MON_STRUCT)) LPVOID pvContext
    );
// Initiates (or if *pHandle is non-null, continues) wait on the directory or subkey
// if the directory or subkey doesn't exist, instead calls it on the first existing parent directory or subkey
// writes to pRequest->dwPathHierarchyIndex with the array index that was waited on
static HRESULT InitiateWait(
    __inout MON_REQUEST *pRequest,
    __inout HANDLE *pHandle
    );
static DWORD WINAPI WaiterThread(
    __in_bcount(sizeof(MON_WAITER_CONTEXT)) LPVOID pvContext
    );
static void Notify(
    __in HRESULT hr,
    __in MON_WAITER_CONTEXT *pWaiterContext,
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
    __in MON_WAITER_CONTEXT *pWaiterContext,
    __in MON_REMOVE_MESSAGE *pMessage,
    __out DWORD *pdwIndex
    );
static void RemoveRequest(
    __inout MON_WAITER_CONTEXT *pWaiterContext,
    __in DWORD dwRequestIndex
    );
static void RemoveFromPendingRequestArray(
    __inout MON_WAITER_CONTEXT *pWaiterContext,
    __in DWORD dwRequestIndex,
    __in BOOL fDeletingRequest
    );
static REGSAM GetRegKeyBitness(
    __in MON_REQUEST *pRequest
    );
static HRESULT DuplicateRemoveMessage(
    __in MON_REMOVE_MESSAGE *pMessage,
    __out MON_REMOVE_MESSAGE **ppMessage
    );

extern "C" HRESULT DAPI MonCreate(
    __out_bcount(MON_HANDLE_BYTES) MON_HANDLE *pHandle,
    __in PFN_MONGENERAL vpfMonGeneral,
    __in_opt PFN_MONDIRECTORY vpfMonDirectory,
    __in_opt PFN_MONREGKEY vpfMonRegKey,
    __in_opt LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    DWORD dwRetries = MON_THREAD_INIT_RETRIES;

    ExitOnNull(pHandle, hr, E_INVALIDARG, "Pointer to handle not specified while creating monitor");

    // Allocate the struct
    *pHandle = static_cast<MON_HANDLE>(MemAlloc(sizeof(MON_STRUCT), TRUE));
    ExitOnNull(*pHandle, hr, E_OUTOFMEMORY, "Failed to allocate monitor object");

    MON_STRUCT *pm = static_cast<MON_STRUCT *>(*pHandle);

    pm->vpfMonGeneral = vpfMonGeneral;
    pm->vpfMonDirectory = vpfMonDirectory;
    pm->vpfMonRegKey = vpfMonRegKey;
    pm->pvContext = pvContext;

    pm->hCoordinatorThread = ::CreateThread(NULL, 0, CoordinatorThread, pm, 0, &pm->dwCoordinatorThreadId);
    if (!pm->hCoordinatorThread)
    {
        ExitWithLastError(hr, "Failed to create waiter thread.");
    }

    // Ensure the created thread initializes its message queue. It does this first thing, so if it doesn't within 10 seconds, there must be a huge problem.
    while (!pm->fCoordinatorThreadMessageQueueInitialized && 0 < dwRetries)
    {
        ::Sleep(MON_THREAD_INIT_RETRY_PERIOD_IN_MS);
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
    __in DWORD dwSilencePeriodInMs,
    __in_opt LPVOID pvDirectoryContext
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    LPWSTR sczDirectory = NULL;
    MON_ADD_MESSAGE *pMessage = NULL;

    hr = StrAllocString(&sczDirectory, wzDirectory, 0);
    ExitOnFailure(hr, "Failed to copy directory string");

    hr = PathBackslashTerminate(&sczDirectory);
    ExitOnFailure(hr, "Failed to ensure directory ends in backslash");

    pMessage = reinterpret_cast<MON_ADD_MESSAGE *>(MemAlloc(sizeof(MON_ADD_MESSAGE), TRUE));
    ExitOnNull(pMessage, hr, E_OUTOFMEMORY, "Failed to allocate memory for message");

    pMessage->handle = INVALID_HANDLE_VALUE;
    pMessage->request.type = MON_DIRECTORY;
    pMessage->request.fRecursive = fRecursive;
    pMessage->request.dwMaxSilencePeriodInMs = dwSilencePeriodInMs,
    pMessage->request.pvContext = pvDirectoryContext;

    hr = PathGetHierarchyArray(sczDirectory, &pMessage->request.rgsczPathHierarchy, reinterpret_cast<LPUINT>(&pMessage->request.cPathHierarchy));
    ExitOnFailure1(hr, "Failed to get hierarchy array for path %ls", sczDirectory);

    hr = InitiateWait(&pMessage->request, &pMessage->handle);
    ExitOnFailure(hr, "Failed to initiate wait");

    if (!::PostThreadMessageW(pm->dwCoordinatorThreadId, MON_MESSAGE_ADD, reinterpret_cast<WPARAM>(pMessage), 0))
    {
        ExitWithLastError1(hr, "Failed to send message to worker thread to add directory wait for path %ls", sczDirectory);
    }
    pMessage = NULL;

LExit:
    ReleaseStr(sczDirectory);
    MonAddMessageDestroy(pMessage);

    return hr;
}

extern "C" HRESULT DAPI MonAddRegKey(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in REG_KEY_BITNESS kbKeyBitness,
    __in BOOL fRecursive,
    __in DWORD dwSilencePeriodInMs,
    __in_opt LPVOID pvRegKeyContext
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    LPWSTR sczSubKey = NULL;
    MON_ADD_MESSAGE *pMessage = NULL;

    hr = StrAllocString(&sczSubKey, wzSubKey, 0);
    ExitOnFailure(hr, "Failed to copy subkey string");

    hr = PathBackslashTerminate(&sczSubKey);
    ExitOnFailure(hr, "Failed to ensure subkey path ends in backslash");

    pMessage = reinterpret_cast<MON_ADD_MESSAGE *>(MemAlloc(sizeof(MON_ADD_MESSAGE), TRUE));
    ExitOnNull(pMessage, hr, E_OUTOFMEMORY, "Failed to allocate memory for message");

    pMessage->handle = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    if (NULL == pMessage->handle) // Yes, unfortunately NULL is the invalid handle value for regkey waits, INVALID_HANDLE_VALUE is for directory waits. yuck.
    {
        ExitWithLastError(hr, "Failed to create anonymous event for regkey monitor");
    }
    pMessage->request.type = MON_REGKEY;
    pMessage->request.regkey.hkRoot = hkRoot;
    pMessage->request.regkey.kbKeyBitness = kbKeyBitness;
    pMessage->request.fRecursive = fRecursive;
    pMessage->request.dwMaxSilencePeriodInMs = dwSilencePeriodInMs,
    pMessage->request.pvContext = pvRegKeyContext;

    hr = PathGetHierarchyArray(sczSubKey, &pMessage->request.rgsczPathHierarchy, reinterpret_cast<LPUINT>(&pMessage->request.cPathHierarchy));
    ExitOnFailure1(hr, "Failed to get hierarchy array for subkey %ls", sczSubKey);

    hr = InitiateWait(&pMessage->request, &pMessage->handle);
    ExitOnFailure(hr, "Failed to initiate wait");

    if (!::PostThreadMessageW(pm->dwCoordinatorThreadId, MON_MESSAGE_ADD, reinterpret_cast<WPARAM>(pMessage), 0))
    {
        ExitWithLastError1(hr, "Failed to send message to worker thread to add directory wait for regkey %ls", sczSubKey);
    }
    pMessage = NULL;

LExit:
    ReleaseStr(sczSubKey);
    MonAddMessageDestroy(pMessage);

    return hr;
}

extern "C" HRESULT DAPI MonRemoveDirectory(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in_z LPCWSTR wzDirectory,
    __in BOOL fRecursive
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    LPWSTR sczDirectory = NULL;
    MON_REMOVE_MESSAGE *pMessage = NULL;

    hr = StrAllocString(&sczDirectory, wzDirectory, 0);
    ExitOnFailure(hr, "Failed to copy directory string");

    hr = PathBackslashTerminate(&sczDirectory);
    ExitOnFailure(hr, "Failed to ensure directory ends in backslash");

    pMessage = reinterpret_cast<MON_REMOVE_MESSAGE *>(MemAlloc(sizeof(MON_REMOVE_MESSAGE), TRUE));
    ExitOnNull(pMessage, hr, E_OUTOFMEMORY, "Failed to allocate memory for message");

    pMessage->type = MON_DIRECTORY;
    pMessage->fRecursive = fRecursive;

    hr = StrAllocString(&pMessage->directory.sczDirectory, sczDirectory, 0);
    ExitOnFailure(hr, "Failed to allocate copy of directory string");

    if (!::PostThreadMessageW(pm->dwCoordinatorThreadId, MON_MESSAGE_REMOVE, reinterpret_cast<WPARAM>(pMessage), 0))
    {
        ExitWithLastError1(hr, "Failed to send message to worker thread to add directory wait for path %ls", sczDirectory);
    }
    pMessage = NULL;

LExit:
    MonRemoveMessageDestroy(pMessage);

    return hr;
}

extern "C" HRESULT DAPI MonRemoveRegKey(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in REG_KEY_BITNESS kbKeyBitness,
    __in BOOL fRecursive
    )
{
    HRESULT hr = S_OK;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);
    LPWSTR sczSubKey = NULL;
    MON_REMOVE_MESSAGE *pMessage = NULL;

    hr = StrAllocString(&sczSubKey, wzSubKey, 0);
    ExitOnFailure(hr, "Failed to copy subkey string");

    hr = PathBackslashTerminate(&sczSubKey);
    ExitOnFailure(hr, "Failed to ensure subkey path ends in backslash");

    pMessage = reinterpret_cast<MON_REMOVE_MESSAGE *>(MemAlloc(sizeof(MON_REMOVE_MESSAGE), TRUE));
    ExitOnNull(pMessage, hr, E_OUTOFMEMORY, "Failed to allocate memory for message");

    pMessage->type = MON_REGKEY;
    pMessage->regkey.hkRoot = hkRoot;
    pMessage->regkey.kbKeyBitness = kbKeyBitness;
    pMessage->fRecursive = fRecursive;

    hr = StrAllocString(&pMessage->regkey.sczSubKey, sczSubKey, 0);
    ExitOnFailure(hr, "Failed to allocate copy of directory string");

    if (!::PostThreadMessageW(pm->dwCoordinatorThreadId, MON_MESSAGE_REMOVE, reinterpret_cast<WPARAM>(pMessage), 0))
    {
        ExitWithLastError1(hr, "Failed to send message to worker thread to add directory wait for path %ls", sczSubKey);
    }
    pMessage = NULL;

LExit:
    ReleaseStr(sczSubKey);
    MonRemoveMessageDestroy(pMessage);

    return hr;
}

extern "C" void DAPI MonDestroy(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    MON_STRUCT *pm = static_cast<MON_STRUCT *>(handle);

    if (!::PostThreadMessageW(pm->dwCoordinatorThreadId, MON_MESSAGE_STOP, 0, 0))
    {
        er = ::GetLastError();
        if (ERROR_INVALID_THREAD_ID == er)
        {
            // It already halted, or doesn't exist for some other reason, so let's just ignore it and clean up
            er = ERROR_SUCCESS;
        }
        ExitOnWin32Error(er, hr, "Failed to send message to background thread to halt");
    }

    if (pm->hCoordinatorThread)
    {
        ::WaitForSingleObject(pm->hCoordinatorThread, INFINITE);
        ::CloseHandle(pm->hCoordinatorThread);
    }

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
        if (MON_DIRECTORY == pMessage->request.type && INVALID_HANDLE_VALUE != pMessage->handle)
        {
            ::FindCloseChangeNotification(pMessage->handle);
        }
        else if (MON_REGKEY == pMessage->request.type)
        {
            ReleaseHandle(pMessage->handle);
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

static DWORD WINAPI CoordinatorThread(
    __in_bcount(sizeof(MON_STRUCT)) LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    MSG msg = { };
    DWORD dwThreadIndex = DWORD_MAX;
    DWORD dwRetries;
    MON_WAITER_INFO *rgWaiterThreads = NULL;
    DWORD cWaiterThreads = 0;
    MON_WAITER_CONTEXT *pWaiterContext = NULL;
    MON_REMOVE_MESSAGE *pRemoveMessage = NULL;
    MON_REMOVE_MESSAGE *pTempRemoveMessage = NULL;
    MON_STRUCT *pm = reinterpret_cast<MON_STRUCT*>(pvContext);
    BOOL fRet = FALSE;

    // Ensure the thread has a message queue
    ::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    pm->fCoordinatorThreadMessageQueueInitialized = TRUE;

    while (0 != (fRet = ::GetMessageW(&msg, NULL, 0, 0)))
    {
        if (-1 == fRet)
        {
            hr = E_UNEXPECTED;
            ExitOnRootFailure(hr, "Unexpected return value from message pump.");
        }
        else
        {
            switch (msg.message)
            {
            case MON_MESSAGE_ADD:
                dwThreadIndex = DWORD_MAX;
                for (DWORD i = 0; i < cWaiterThreads; ++i)
                {
                    if (rgWaiterThreads[i].cMonitorCount < MON_MAX_MONITORS_PER_THREAD)
                    {
                        dwThreadIndex = i;
                        break;
                    }
                }

                if (dwThreadIndex < cWaiterThreads)
                {
                    pWaiterContext = rgWaiterThreads[dwThreadIndex].pWaiterContext;
                }
                else
                {
                    hr = MemEnsureArraySize(reinterpret_cast<void **>(&rgWaiterThreads), cWaiterThreads + 1, sizeof(MON_WAITER_INFO), MON_THREAD_GROWTH);
                    ExitOnFailure(hr, "Failed to grow waiter thread array size");
                    ++cWaiterThreads;

                    dwThreadIndex = cWaiterThreads - 1;
                    rgWaiterThreads[dwThreadIndex].pWaiterContext = reinterpret_cast<MON_WAITER_CONTEXT*>(MemAlloc(sizeof(MON_WAITER_CONTEXT), TRUE));
                    ExitOnNull(rgWaiterThreads[dwThreadIndex].pWaiterContext, hr, E_OUTOFMEMORY, "Failed to allocate waiter context struct");
                    pWaiterContext = rgWaiterThreads[dwThreadIndex].pWaiterContext;
                    pWaiterContext->dwCoordinatorThreadId = ::GetCurrentThreadId();
                    pWaiterContext->vpfMonGeneral = pm->vpfMonGeneral;
                    pWaiterContext->vpfMonDirectory = pm->vpfMonDirectory;
                    pWaiterContext->vpfMonRegKey = pm->vpfMonRegKey;
                    pWaiterContext->pvContext = pm->pvContext;

                    hr = MemEnsureArraySize(reinterpret_cast<void **>(&pWaiterContext->rgHandles), pWaiterContext->cHandles + 1, sizeof(HANDLE), MON_ARRAY_GROWTH);
                    ExitOnFailure(hr, "Failed to allocate first handle");
                    ++pWaiterContext->cHandles;

                    pWaiterContext->rgHandles[0] = ::CreateEventW(NULL, FALSE, FALSE, NULL);
                    ExitOnNullWithLastError(pWaiterContext->rgHandles[0], hr, "Failed to create general event");

                    pWaiterContext->hWaiterThread = ::CreateThread(NULL, 0, WaiterThread, pWaiterContext, 0, &pWaiterContext->dwWaiterThreadId);
                    if (!pWaiterContext->hWaiterThread)
                    {
                        ExitWithLastError(hr, "Failed to create waiter thread.");
                    }

                    dwRetries = MON_THREAD_INIT_RETRIES;
                    while (!pWaiterContext->fWaiterThreadMessageQueueInitialized && 0 < dwRetries)
                    {
                        ::Sleep(MON_THREAD_INIT_RETRY_PERIOD_IN_MS);
                        --dwRetries;
                    }

                    if (dwRetries == 0)
                    {
                        hr = E_UNEXPECTED;
                        ExitOnFailure(hr, "Waiter thread apparently never initialized its message queue.");
                    }
                }

                ++rgWaiterThreads[dwThreadIndex].cMonitorCount;
                if (!::PostThreadMessageW(pWaiterContext->dwWaiterThreadId, MON_MESSAGE_ADD, msg.wParam, 0))
                {
                    ExitWithLastError(hr, "Failed to send message to waiter thread to add monitor");
                }

                if (!::SetEvent(pWaiterContext->rgHandles[0]))
                {
                    ExitWithLastError(hr, "Failed to set event to notify waiter thread of incoming message");
                }
                break;

            case MON_MESSAGE_REMOVE:
                // Send remove to all waiter threads. They'll ignore it if they don't have that monitor.
                // If they do have that monitor, they'll remove it from their list, and tell coordinator they have another
                // empty slot via MON_MESSAGE_REMOVED message
                for (DWORD i = 0; i < cWaiterThreads; ++i)
                {
                    pWaiterContext = rgWaiterThreads[i].pWaiterContext;
                    pRemoveMessage = reinterpret_cast<MON_REMOVE_MESSAGE *>(msg.wParam);

                    hr = DuplicateRemoveMessage(pRemoveMessage, &pTempRemoveMessage);
                    ExitOnFailure(hr, "Failed to duplicate remove message");

                    if (!::PostThreadMessageW(pWaiterContext->dwWaiterThreadId, MON_MESSAGE_REMOVE, reinterpret_cast<WPARAM>(pTempRemoveMessage), msg.lParam))
                    {
                        ExitWithLastError(hr, "Failed to send message to waiter thread to add monitor");
                    }
                    pTempRemoveMessage = NULL;

                    if (!::SetEvent(pWaiterContext->rgHandles[0]))
                    {
                        ExitWithLastError(hr, "Failed to set event to notify waiter thread of incoming message");
                    }
                }
                MonRemoveMessageDestroy(pRemoveMessage);
                pRemoveMessage = NULL;
                break;

            case MON_MESSAGE_REMOVED:
                for (DWORD i = 0; i < cWaiterThreads; ++i)
                {
                    if (rgWaiterThreads[i].pWaiterContext->dwWaiterThreadId == static_cast<DWORD>(msg.wParam))
                    {
                        Assert(rgWaiterThreads[i].cMonitorCount > 0);
                        --rgWaiterThreads[i].cMonitorCount;
                        if (0 == rgWaiterThreads[i].cMonitorCount)
                        {
                            if (!::PostThreadMessageW(rgWaiterThreads[i].pWaiterContext->dwWaiterThreadId, MON_MESSAGE_STOP, msg.wParam, msg.lParam))
                            {
                                ExitWithLastError(hr, "Failed to send message to waiter thread to stop");
                            }
                            MemRemoveFromArray(reinterpret_cast<LPVOID>(rgWaiterThreads), i, 1, cWaiterThreads, sizeof(MON_WAITER_INFO), TRUE);
                            --cWaiterThreads;
                            --i; // reprocess this index in the for loop, which will now contain the item after the one we removed
                        }
                    }
                }
                break;

            case MON_MESSAGE_STOP:
                ExitFunction1(hr = S_OK);

            default:
                Assert(false);
                break;
            }
        }
    }

LExit:
    // First tell all waiter threads to shutdown
    for (DWORD i = 0; i < cWaiterThreads; ++i)
    {
        pWaiterContext = rgWaiterThreads[i].pWaiterContext;
        if (!::PostThreadMessageW(pWaiterContext->dwWaiterThreadId, MON_MESSAGE_STOP, msg.wParam, msg.lParam))
        {
            TraceError(hr, "Failed to send message to waiter thread to stop");
        }

        if (!::SetEvent(pWaiterContext->rgHandles[0]))
        {
            TraceError(hr, "Failed to set event to notify waiter thread of incoming message");
        }
    }
    // Now confirm they're actually shut down before returning
    for (DWORD i = 0; i < cWaiterThreads; ++i)
    {
        pWaiterContext = rgWaiterThreads[i].pWaiterContext;
        ::WaitForSingleObject(pWaiterContext->hWaiterThread, INFINITE);
        ::CloseHandle(pWaiterContext->hWaiterThread);
    }

    if (FAILED(hr))
    {
        // If coordinator thread fails, notify general callback of an error
        Assert(pWaiterContext->vpfMonGeneral);
        pWaiterContext->vpfMonGeneral(hr, pWaiterContext->pvContext);
    }
    MonRemoveMessageDestroy(pRemoveMessage);
    MonRemoveMessageDestroy(pTempRemoveMessage);

    return hr;
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

                *pHandle = ::FindFirstChangeNotificationW(pRequest->rgsczPathHierarchy[dwIndex], GetRecursiveFlag(pRequest, dwIndex), FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SECURITY);
                if (INVALID_HANDLE_VALUE == *pHandle)
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());
                    if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr || E_ACCESSDENIED == hr)
                    {
                        continue;
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
                hr = RegOpen(pRequest->regkey.hkRoot, pRequest->rgsczPathHierarchy[dwIndex], KEY_NOTIFY | GetRegKeyBitness(pRequest), &pRequest->regkey.hkSubKey);
                if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr)
                {
                    continue;
                }

                er = ::RegNotifyChangeKeyValue(pRequest->regkey.hkSubKey, GetRecursiveFlag(pRequest, dwIndex), REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_SECURITY, *pHandle, TRUE);
                ReleaseRegKey(hk);
                hr = HRESULT_FROM_WIN32(er);
                if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr || HRESULT_FROM_WIN32(ERROR_KEY_DELETED) == hr)
                {
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
                    hrTemp = RegOpen(pRequest->regkey.hkRoot, pRequest->rgsczPathHierarchy[dwIndex + 1], KEY_NOTIFY | GetRegKeyBitness(pRequest), &hk);
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
    __in_bcount(sizeof(MON_WAITER_CONTEXT)) LPVOID pvContext
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
    MON_WAITER_CONTEXT *pWaiterContext = reinterpret_cast<MON_WAITER_CONTEXT *>(pvContext);
    DWORD dwRequestIndex;
    // If we have one or more requests pending notification, this is the period we intend to wait for multiple objects (shortest amount of time to next potential notify)
    DWORD dwWait = 0;
    DWORD uCurrentTime = 0;
    DWORD uLastTimeInMs = ::GetTickCount();
    DWORD uDeltaInMs = 0;

    // Ensure the thread has a message queue
    ::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    pWaiterContext->fWaiterThreadMessageQueueInitialized = TRUE;

    do
    {
        dwRet = ::WaitForMultipleObjects(pWaiterContext->cHandles, pWaiterContext->rgHandles, FALSE, pWaiterContext->cRequestsPending > 0 ? dwWait : INFINITE);

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

                            hr = MemEnsureArraySize(reinterpret_cast<void **>(&pWaiterContext->rgHandles), pWaiterContext->cHandles + 1, sizeof(HANDLE), MON_ARRAY_GROWTH);
                            ExitOnFailure(hr, "Failed to allocate additional handle for request");
                            ++pWaiterContext->cHandles;

                            hr = MemEnsureArraySize(reinterpret_cast<void **>(&pWaiterContext->rgRequests), pWaiterContext->cRequests + 1, sizeof(MON_REQUEST), MON_ARRAY_GROWTH);
                            ExitOnFailure(hr, "Failed to allocate additional request struct");
                            ++pWaiterContext->cRequests;

                            pWaiterContext->rgRequests[pWaiterContext->cRequests - 1] = pAddMessage->request;
                            pWaiterContext->rgHandles[pWaiterContext->cHandles - 1] = pAddMessage->handle;

                            ReleaseNullMem(pAddMessage);
                            break;
                        case MON_MESSAGE_REMOVE:
                            pRemoveMessage = reinterpret_cast<MON_REMOVE_MESSAGE *>(msg.wParam);

                            // Find the request to remove
                            hr = FindRequestIndex(pWaiterContext, pRemoveMessage, &dwRequestIndex);
                            if (E_NOTFOUND == hr)
                            {
                                // Coordinator sends removes blindly to all waiter threads, so maybe this one wasn't intended for us
                                hr = S_OK;
                            }
                            else
                            {
                                ExitOnFailure(hr, "Failed to find request index for remove message");

                                RemoveRequest(pWaiterContext, dwRequestIndex);
                                if (!::PostThreadMessageW(pWaiterContext->dwCoordinatorThreadId, MON_MESSAGE_REMOVED, static_cast<WPARAM>(::GetCurrentThreadId()), 0))
                                {
                                    ExitWithLastError(hr, "Failed to send message to coordinator thread to confirm directory was removed");
                                }
                            }

                            MonRemoveMessageDestroy(pRemoveMessage);
                            pRemoveMessage = NULL;
                            break;
                        case MON_MESSAGE_STOP:
                            // Stop requested, so abort the whole thread
                            Trace(REPORT_DEBUG, "MonUtil thread was told to stop");
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
        else if (dwRet >= WAIT_OBJECT_0 && dwRet - WAIT_OBJECT_0 < pWaiterContext->cHandles)
        {
            // OK a handle fired - only notify if it's the actual target, and not just some parent waiting for the target child to exist
            dwRequestIndex = dwRet - WAIT_OBJECT_0 - 1;
            fNotify = (pWaiterContext->rgRequests[dwRequestIndex].dwPathHierarchyIndex == pWaiterContext->rgRequests[dwRequestIndex].cPathHierarchy - 1);

            // Initiate re-waits before we notify callback, to ensure we don't miss a single update
            hr = InitiateWait(pWaiterContext->rgRequests + dwRequestIndex, pWaiterContext->rgHandles + dwRequestIndex + 1);
            if (FAILED(hr))
            {
                // If there's an error re-waiting, we immediately notify of the error and remove this notification from the queue, because something went wrong
                Notify(hr, pWaiterContext, pWaiterContext->rgRequests + dwRequestIndex);
                RemoveRequest(pWaiterContext, dwRequestIndex);
            }
            // If there were no errors and we were already waiting on the right target, or if we weren't yet but are able to now, it's a successful notify
            else if (fNotify || (pWaiterContext->rgRequests[dwRequestIndex].dwPathHierarchyIndex == pWaiterContext->rgRequests[dwRequestIndex].cPathHierarchy - 1))
            {
                Trace1(REPORT_DEBUG, "Changes detected, waiting for silence period index %u", dwRequestIndex);

                if (0 < pWaiterContext->rgRequests[dwRequestIndex].dwMaxSilencePeriodInMs)
                {
                    pWaiterContext->rgRequests[dwRequestIndex].dwSilencePeriodInMs = 0;
                    pWaiterContext->rgRequests[dwRequestIndex].fSkipDeltaAdd = TRUE;

                    if (!pWaiterContext->rgRequests[dwRequestIndex].fPendingFire)
                    {
                        pWaiterContext->rgRequests[dwRequestIndex].fPendingFire = TRUE;

                        // Append this pending request to the pending request array
                        hr = MemEnsureArraySize(reinterpret_cast<void **>(&pWaiterContext->rgRequestsPending), pWaiterContext->cRequestsPending + 1, sizeof(DWORD), 10);
                        ExitOnFailure(hr, "Failed to increase size of pending requests array");
                        ++pWaiterContext->cRequestsPending;

                        pWaiterContext->rgRequestsPending[pWaiterContext->cRequestsPending - 1] = dwRequestIndex;
                    }
                }
                else
                {
                    // If no silence period, notify immediately
                    Notify(S_OK, pWaiterContext, pWaiterContext->rgRequests + dwRequestIndex);
                }
            }
        }
        else if (WAIT_TIMEOUT != dwRet)
        {
            ExitWithLastError1(hr, "Failed to wait for multiple objects with return code %u", dwRet);
        }

        // OK, now that we've checked all triggered handles (resetting silence period timers appropriately), check for any pending notifications that we can finally fire
        // And set dwWait appropriately so we awaken at the right time to fire the next pending notification (in case no further writes occur during that time)
        if (0 < pWaiterContext->cRequestsPending)
        {
            dwWait = 0;

            for (DWORD i = 0; i < pWaiterContext->cRequestsPending; ++i)
            {
                dwRequestIndex = pWaiterContext->rgRequestsPending[i];
                if (pWaiterContext->rgRequests[dwRequestIndex].fPendingFire)
                {
                    if (pWaiterContext->rgRequests[dwRequestIndex].fSkipDeltaAdd)
                    {
                        pWaiterContext->rgRequests[dwRequestIndex].fSkipDeltaAdd = FALSE;
                    }
                    else
                    {
                        pWaiterContext->rgRequests[dwRequestIndex].dwSilencePeriodInMs += uDeltaInMs;

                        // silence period has elapsed without further notifications, so reset pending-related variables, and finally fire a notify!
                        if (pWaiterContext->rgRequests[dwRequestIndex].dwSilencePeriodInMs >= pWaiterContext->rgRequests[dwRequestIndex].dwMaxSilencePeriodInMs)
                        {
                            Trace1(REPORT_DEBUG, "Silence period surpassed, notifying %u ms late", pWaiterContext->rgRequests[dwRequestIndex].dwSilencePeriodInMs - pWaiterContext->rgRequests[dwRequestIndex].dwMaxSilencePeriodInMs);
                            Notify(S_OK, pWaiterContext, pWaiterContext->rgRequests + dwRequestIndex);
                            RemoveFromPendingRequestArray(pWaiterContext, dwRequestIndex, FALSE);
                            pWaiterContext->rgRequests[dwRequestIndex].fPendingFire = FALSE;
                            pWaiterContext->rgRequests[dwRequestIndex].fSkipDeltaAdd = FALSE;
                            pWaiterContext->rgRequests[dwRequestIndex].dwSilencePeriodInMs = 0;
                        }
                        else
                        {
                            // set dwWait to the shortest interval period so that if no changes occur, WaitForMultipleObjects
                            // wakes the thread back up when it's time to fire the next pending notification
                            if (dwWait > pWaiterContext->rgRequests[dwRequestIndex].dwMaxSilencePeriodInMs - pWaiterContext->rgRequests[dwRequestIndex].dwSilencePeriodInMs)
                            {
                                dwWait = pWaiterContext->rgRequests[dwRequestIndex].dwMaxSilencePeriodInMs - pWaiterContext->rgRequests[dwRequestIndex].dwSilencePeriodInMs;
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

    for (DWORD i = 0; i < pWaiterContext->cRequests; ++i)
    {
        switch (pWaiterContext->rgRequests[i].type)
        {
        case MON_DIRECTORY:
            if (INVALID_HANDLE_VALUE != pWaiterContext->rgHandles[i + 1])
            {
                ::FindCloseChangeNotification(pWaiterContext->rgHandles[i + 1]);
            }
            break;
        case MON_REGKEY:
            ReleaseHandle(pWaiterContext->rgHandles[i + 1]);
            ReleaseRegKey(pWaiterContext->rgRequests[i].regkey.hkSubKey);
            break;
        default:
            Assert(false);
        }

        MonRequestDestroy(pWaiterContext->rgRequests + i);
    }
    ReleaseMem(pWaiterContext->rgRequestsPending);

    if (FAILED(hr))
    {
        // If waiter thread fails, notify general callback of an error
        Assert(pWaiterContext->vpfMonGeneral);
        pWaiterContext->vpfMonGeneral(hr, pWaiterContext->pvContext);
    }

    return hr;
}

static void Notify(
    __in HRESULT hr,
    __in MON_WAITER_CONTEXT *pWaiterContext,
    __in MON_REQUEST *pRequest
    )
{
    switch (pRequest->type)
    {
    case MON_DIRECTORY:
        Assert(pWaiterContext->vpfMonDirectory);
        pWaiterContext->vpfMonDirectory(hr, pRequest->rgsczPathHierarchy[pRequest->cPathHierarchy - 1], pRequest->fRecursive, pWaiterContext->pvContext, pRequest->pvContext);
        break;
    case MON_REGKEY:
        Assert(pWaiterContext->vpfMonRegKey);
        pWaiterContext->vpfMonRegKey(hr, pRequest->regkey.hkRoot, pRequest->rgsczPathHierarchy[pRequest->cPathHierarchy - 1], pRequest->regkey.kbKeyBitness, pRequest->fRecursive, pWaiterContext->pvContext, pRequest->pvContext);
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
    __in MON_WAITER_CONTEXT *pWaiterContext,
    __in MON_REMOVE_MESSAGE *pMessage,
    __out DWORD *pdwIndex
    )
{
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < pWaiterContext->cRequests; ++i)
    {
        if (pWaiterContext->rgRequests[i].type == pMessage->type)
        {
            switch (pWaiterContext->rgRequests[i].type)
            {
            case MON_DIRECTORY:
                if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pWaiterContext->rgRequests[i].rgsczPathHierarchy[pWaiterContext->rgRequests[i].cPathHierarchy - 1], -1, pMessage->directory.sczDirectory, -1) && pWaiterContext->rgRequests[i].fRecursive == pMessage->fRecursive)
                {
                    *pdwIndex = i;
                    ExitFunction1(hr = S_OK);
                }
                break;
            case MON_REGKEY:
                if (reinterpret_cast<DWORD_PTR>(pMessage->regkey.hkRoot) == reinterpret_cast<DWORD_PTR>(pWaiterContext->rgRequests[i].regkey.hkRoot) && CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pWaiterContext->rgRequests[i].rgsczPathHierarchy[pWaiterContext->rgRequests[i].cPathHierarchy - 1], -1, pMessage->regkey.sczSubKey, -1) && pWaiterContext->rgRequests[i].fRecursive == pMessage->fRecursive && pWaiterContext->rgRequests[i].regkey.kbKeyBitness == pMessage->regkey.kbKeyBitness)
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
    __inout MON_WAITER_CONTEXT *pWaiterContext,
    __in DWORD dwRequestIndex
    )
{
    switch (pWaiterContext->rgRequests[dwRequestIndex].type)
    {
    case MON_DIRECTORY:
        if (pWaiterContext->rgHandles[dwRequestIndex + 1] != INVALID_HANDLE_VALUE)
        {
            ::FindCloseChangeNotification(pWaiterContext->rgHandles[dwRequestIndex + 1]);
        }
        break;
    case MON_REGKEY:
        ReleaseHandle(pWaiterContext->rgHandles[dwRequestIndex + 1]);
        ReleaseRegKey(pWaiterContext->rgRequests[dwRequestIndex].regkey.hkSubKey);
        break;
    default:
        Assert(false);
    }

    if (pWaiterContext->rgRequests[dwRequestIndex].fPendingFire)
    {
        RemoveFromPendingRequestArray(pWaiterContext, dwRequestIndex, TRUE);
    }

    MemRemoveFromArray(reinterpret_cast<void *>(pWaiterContext->rgHandles), dwRequestIndex + 1, 1, pWaiterContext->cHandles, sizeof(HANDLE), TRUE);
    --pWaiterContext->cHandles;
    MemRemoveFromArray(reinterpret_cast<void *>(pWaiterContext->rgRequests), dwRequestIndex, 1, pWaiterContext->cRequests, sizeof(MON_REQUEST), TRUE);
    --pWaiterContext->cRequests;
}

static void RemoveFromPendingRequestArray(
    __inout MON_WAITER_CONTEXT *pWaiterContext,
    __in DWORD dwRequestIndex,
    __in BOOL fDeletingRequest
    )
{
    for (DWORD i = 0; i < pWaiterContext->cRequestsPending; ++i)
    {
        // If we find the actual index, we need to remove it
        if (pWaiterContext->rgRequestsPending[i] == dwRequestIndex)
        {
            MemRemoveFromArray(reinterpret_cast<void *>(pWaiterContext->rgRequestsPending), i, 1, pWaiterContext->cRequestsPending, sizeof(DWORD), TRUE);
            --pWaiterContext->cRequestsPending;
            // We just removed this item, so hit the same index again
            --i;
        }
        // We're about to remove this request from the array, which means every index into the array after that point needs to be decremented
        else if (fDeletingRequest && pWaiterContext->rgRequestsPending[i] > dwRequestIndex)
        {
            --pWaiterContext->rgRequestsPending[i];
        }
    }
}

static REGSAM GetRegKeyBitness(
    __in MON_REQUEST *pRequest
    )
{
    if (REG_KEY_32BIT == pRequest->regkey.kbKeyBitness)
    {
        return KEY_WOW64_32KEY;
    }
    else if (REG_KEY_64BIT == pRequest->regkey.kbKeyBitness)
    {
        return KEY_WOW64_64KEY;
    }
    else
    {
        return 0;
    }
}

static HRESULT DuplicateRemoveMessage(
    __in MON_REMOVE_MESSAGE *pMessage,
    __out MON_REMOVE_MESSAGE **ppMessage
    )
{
    HRESULT hr = S_OK;

    *ppMessage = reinterpret_cast<MON_REMOVE_MESSAGE *>(MemAlloc(sizeof(MON_REMOVE_MESSAGE), TRUE));
    ExitOnNull(*ppMessage, hr, E_OUTOFMEMORY, "Failed to allocate copy of remove message");

    (*ppMessage)->type = pMessage->type;
    (*ppMessage)->fRecursive = pMessage->fRecursive;

    switch (pMessage->type)
    {
    case MON_DIRECTORY:
        hr = StrAllocString(&(*ppMessage)->directory.sczDirectory, pMessage->directory.sczDirectory, 0);
        ExitOnFailure(hr, "Failed to copy directory");
        break;
    case MON_REGKEY:
        (*ppMessage)->regkey.hkRoot = pMessage->regkey.hkRoot;
        (*ppMessage)->regkey.kbKeyBitness = pMessage->regkey.kbKeyBitness;
        hr = StrAllocString(&(*ppMessage)->regkey.sczSubKey, pMessage->regkey.sczSubKey, 0);
        ExitOnFailure(hr, "Failed to copy subkey");
        break;
    default:
        Assert(false);
        break;
    }

LExit:
    return hr;
}
