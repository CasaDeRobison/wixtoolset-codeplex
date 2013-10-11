//-------------------------------------------------------------------------------------------------
// <copyright file="handle.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
// Internal utility functions related to interacting with settings engine handles
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

const DWORD STALE_WRITETIME_RETRY = 100;

HRESULT HandleLock(
    __inout CFGDB_STRUCT *pcdb
    )
{
    HRESULT hr = S_OK;

    ::EnterCriticalSection(&pcdb->cs);
    ++pcdb->dwLockRefCount;

    if (1 < pcdb->dwLockRefCount)
    {
        ExitFunction1(hr = S_OK);
    }

    // Connect to database, if it's a remote database
    if (pcdb->fRemote)
    {
        // By default, we'll update the cached FILETIME on db disconnect. This bool can be overridden when necessary.
        pcdb->fUpdateLastModified = TRUE;
        hr = SceEnsureDatabase(pcdb->sczDbPath, wzSqlCeDllPath, L"CfgRemote", 1, &pcdb->dsSceDb, &pcdb->psceDb);
        ExitOnFailure1(hr, "Failed to ensure SQL CE database at %ls exists", pcdb->sczDbPath);
    }

LExit:
    if (FAILED(hr))
    {
        --pcdb->dwLockRefCount;
        ::LeaveCriticalSection(&pcdb->cs);
    }

    return hr;
}

void HandleUnlock(
    __inout CFGDB_STRUCT *pcdb
    )
{
    HRESULT hr = S_OK;
    FILETIME ftOriginalLastModified = { };
    FILETIME ftNewLastModified = { };
    LONG lCompareResult = 0;

    if (1 < pcdb->dwLockRefCount)
    {
        ExitFunction1(hr = S_OK);
    }

    Assert(0 < pcdb->dwLockRefCount);

    // Disconnect from database, if it's a remote database
    if (pcdb->fRemote && NULL != pcdb->psceDb)
    {
        if (SceDatabaseChanged(pcdb->psceDb))
        {
            hr = FileGetTime(pcdb->sczDbChangesPath, NULL, NULL, &ftOriginalLastModified);
            ExitOnFailure1(hr, "Failed to get file time of remote db changes path: %ls", pcdb->sczDbChangesPath);

            // Check if timestamp is recognized by filesystem as new. If not, keep trying again and sleeping until it is.
            // Last written timestamp granularity can vary by filesystem
            do
            {
                hr = FileWrite(pcdb->sczDbChangesPath, FILE_ATTRIBUTE_HIDDEN, NULL, 0, NULL);
                ExitOnFailure1(hr, "Failed to write new db changes file: %ls", pcdb->sczDbChangesPath);

                hr = FileGetTime(pcdb->sczDbChangesPath, NULL, NULL, &ftNewLastModified);
                ExitOnFailure1(hr, "Failed to re-get file time of remote db changes path: %ls", pcdb->sczDbChangesPath);

                lCompareResult = ::CompareFileTime(&ftOriginalLastModified, &ftNewLastModified);

                if (0 == lCompareResult)
                {
                    ::Sleep(STALE_WRITETIME_RETRY);
                }
            } while (0 == lCompareResult);
        }

        if (pcdb->fUpdateLastModified)
        {
            hr = FileGetTime(pcdb->sczDbChangesPath, NULL, NULL, &pcdb->ftLastModified);
            if (E_FILENOTFOUND == hr || E_NOTFOUND == hr)
            {
                hr = S_OK;
            }
            ExitOnFailure1(hr, "Failed to get file time of remote db: %ls", pcdb->sczDbPath);
        }

        hr = SceCloseDatabase(pcdb->psceDb);
        ExitOnFailure(hr, "Failed to close remote database");
        pcdb->psceDb = NULL;
    }

LExit:
    --pcdb->dwLockRefCount;
    ::LeaveCriticalSection(&pcdb->cs);

    return;
}
