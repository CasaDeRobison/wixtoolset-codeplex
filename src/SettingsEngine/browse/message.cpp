//-------------------------------------------------------------------------------------------------
// <copyright file="message.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Main window message functionality
// </summary>
//-------------------------------------------------------------------------------------------------


#include "precomp.h"

HRESULT SendDwordString(
    __in DWORD dwThreadId,
    __in DWORD dwMessageId,
    __in DWORD dwDatabaseIndex,
    __in DWORD dwDword1,
    __in_z LPCWSTR wzString1
    )
{
    HRESULT hr = S_OK;
    DWORD_STRING *pDwordString = NULL;

    pDwordString = static_cast<DWORD_STRING *>(MemAlloc(sizeof(DWORD_STRING), TRUE));

    pDwordString->dwDword1 = dwDword1;
    
    if (NULL != wzString1)
    {
        hr = StrAllocString(&pDwordString->sczString1, wzString1, 0);
        ExitOnFailure1(hr, "Failed to allocate copy of string1: %ls", wzString1);
    }

    if (!::PostThreadMessageW(dwThreadId, dwMessageId, dwDatabaseIndex, reinterpret_cast<LPARAM>(pDwordString)))
    {
        ExitWithLastError1(hr, "Failed to send message %u to worker thread", dwMessageId);
    }

    pDwordString = NULL;

LExit:
    ReleaseMem(pDwordString);

    return hr;
}

HRESULT SendQwordString(
    __in DWORD dwThreadId,
    __in DWORD dwMessageId,
    __in DWORD dwDatabaseIndex,
    __in DWORD64 qwQword1,
    __in_z LPCWSTR wzString1
    )
{
    HRESULT hr = S_OK;
    QWORD_STRING *pQwordString = NULL;

    pQwordString = static_cast<QWORD_STRING *>(MemAlloc(sizeof(QWORD_STRING), TRUE));

    pQwordString->qwQword1 = qwQword1;
    
    if (NULL != wzString1)
    {
        hr = StrAllocString(&pQwordString->sczString1, wzString1, 0);
        ExitOnFailure1(hr, "Failed to allocate copy of string1: %ls", wzString1);
    }

    if (!::PostThreadMessageW(dwThreadId, dwMessageId, dwDatabaseIndex, reinterpret_cast<LPARAM>(pQwordString)))
    {
        ExitWithLastError1(hr, "Failed to send message %u to worker thread", dwMessageId);
    }

    pQwordString = NULL;

LExit:
    ReleaseMem(pQwordString);

    return hr;
}

HRESULT SendStringPair(
    __in DWORD dwThreadId,
    __in DWORD dwMessageId,
    __in DWORD dwDatabaseIndex,
    __in_z LPCWSTR wzString1,
    __in_z LPCWSTR wzString2
    )
{
    HRESULT hr = S_OK;
    STRING_PAIR *pStringPair = NULL;

    pStringPair = static_cast<STRING_PAIR *>(MemAlloc(sizeof(STRING_PAIR), TRUE));

    if (NULL != wzString1)
    {
        hr = StrAllocString(&pStringPair->sczString1, wzString1, 0);
        ExitOnFailure1(hr, "Failed to allocate copy of string1: %ls", wzString1);
    }

    if (NULL != wzString2)
    {
        hr = StrAllocString(&pStringPair->sczString2, wzString2, 0);
        ExitOnFailure1(hr, "Failed to allocate copy of string2: %ls", wzString2);
    }

    if (!::PostThreadMessageW(dwThreadId, dwMessageId, dwDatabaseIndex, reinterpret_cast<LPARAM>(pStringPair)))
    {
        ExitWithLastError1(hr, "Failed to send message %u to worker thread", dwMessageId);
    }

    pStringPair = NULL;

LExit:
    ReleaseMem(pStringPair);

    return hr;
}

HRESULT SendStringTriplet(
    __in DWORD dwThreadId,
    __in DWORD dwMessageId,
    __in DWORD dwDatabaseIndex,
    __in_z LPCWSTR wzString1,
    __in_z LPCWSTR wzString2,
    __in_z LPCWSTR wzString3
    )
{
    HRESULT hr = S_OK;
    STRING_TRIPLET *pStringTriplet = NULL;

    pStringTriplet = static_cast<STRING_TRIPLET *>(MemAlloc(sizeof(STRING_TRIPLET), TRUE));

    if (NULL != wzString1)
    {
        hr = StrAllocString(&pStringTriplet->sczString1, wzString1, 0);
        ExitOnFailure1(hr, "Failed to allocate copy of string1: %ls", wzString1);
    }

    if (NULL != wzString2)
    {
        hr = StrAllocString(&pStringTriplet->sczString2, wzString2, 0);
        ExitOnFailure1(hr, "Failed to allocate copy of string2: %ls", wzString2);
    }

    if (NULL != wzString3)
    {
        hr = StrAllocString(&pStringTriplet->sczString3, wzString3, 0);
        ExitOnFailure1(hr, "Failed to allocate copy of string3: %ls", wzString3);
    }

    if (!::PostThreadMessageW(dwThreadId, dwMessageId, dwDatabaseIndex, reinterpret_cast<LPARAM>(pStringTriplet)))
    {
        ExitWithLastError1(hr, "Failed to send message %u to worker thread", dwMessageId);
    }

    pStringTriplet = NULL;

LExit:
    ReleaseMem(pStringTriplet);

    return hr;
}
