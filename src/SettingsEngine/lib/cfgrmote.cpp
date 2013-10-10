//-------------------------------------------------------------------------------------------------
// <copyright file="cfgrmote.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    Settings engine API (functions for connecting to non-local databases)
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

static HRESULT RemoteDatabaseInitialize(
    __in LPCWSTR wzPath,
    __in BOOL fSyncByDefault,
    __deref_out_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE *pcdHandle
    );
static BOOL FindPathInOpenDatabasesList(
    __in CFGDB_STRUCT *pcdbLocal,
    __in_z LPCWSTR wzPath,
    __out DWORD *pdwIndex
    );
// Converts the string in-place from a mounted drive path to a UNC path
// This is a last-ditch effort to fix mounted drives that are remembered but not disconnected at the moment
// In some cases, windows will reconnect the drive based on remembered credentials if the user double-clicks the drive in explorer
// However, programs that automatically try to reference files on the mounted drive are left out in the cold.
// This works around the issue by falling back to accessing the UNC path directly.
static HRESULT ConvertToUNCPath(
    __in LPWSTR *psczPath
    );

HRESULT EnsureRemoteSummaryDataTable(
    __in CFGDB_STRUCT *pcdb
    )
{
    HRESULT hr = S_OK;
    BOOL fInSceTransaction = FALSE;
    RPC_STATUS rs = RPC_S_OK;
    BOOL fEmpty = FALSE;
    UUID guid = { };
    DWORD_PTR cchGuid = 39;
    SCE_ROW_HANDLE sceRow = NULL;

    hr = SceGetFirstRow(pcdb->psceDb, SUMMARY_DATA_TABLE, &sceRow);
    if (E_NOTFOUND == hr)
    {
        fEmpty = TRUE;
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to get first row of summary data table");

    if (fEmpty)
    {
        hr = StrAlloc(&pcdb->sczGuid, cchGuid);
        ExitOnFailure(hr, "Failed to allocate space for guid");

        // Create the unique endpoint name.
        rs = ::UuidCreate(&guid);
        hr = HRESULT_FROM_RPC(rs);
        ExitOnFailure(hr, "Failed to create endpoint guid.");

        if (!::StringFromGUID2(guid, pcdb->sczGuid, cchGuid))
        {
            hr = E_OUTOFMEMORY;
            ExitOnRootFailure(hr, "Failed to convert endpoint guid into string.");
        }

        hr = SceBeginTransaction(pcdb->psceDb);
        ExitOnFailure(hr, "Failed to begin transaction");
        fInSceTransaction = TRUE;

        hr = ScePrepareInsert(pcdb->psceDb, SUMMARY_DATA_TABLE, &sceRow);
        ExitOnFailure(hr, "Failed to prepare for insert");

        hr = SceSetColumnString(sceRow, SUMMARY_GUID, pcdb->sczGuid);
        ExitOnFailure(hr, "Failed to set column string of summary data table guid");

        hr = SceFinishUpdate(sceRow);
        ExitOnFailure(hr, "Failed to finish insert into summary data table");

        hr = SceCommitTransaction(pcdb->psceDb);
        ExitOnFailure(hr, "Failed to commit transaction");
        fInSceTransaction = FALSE;

        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to move to first row in SummaryData table");

    hr = SceGetColumnString(sceRow, SUMMARY_GUID, &pcdb->sczGuid);
    ExitOnFailure(hr, "Failed to get GUID from summary data table");

LExit:
    ReleaseSceRow(sceRow);
    if (fInSceTransaction)
    {
        SceRollbackTransaction(pcdb->psceDb);
    }
    if (FAILED(hr))
    {
        ReleaseNullStr(pcdb->sczGuid);
    }

    return hr;
}

extern "C" HRESULT CFGAPI CfgCreateRemoteDatabase(
    __in LPCWSTR wzPath,
    __deref_out_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE *pcdHandle
    )
{
    HRESULT hr = S_OK;

    if (FileExistsEx(wzPath, NULL))
    {
        hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        ExitOnFailure1(hr, "Tried to create remote database %ls, but it already exists!", wzPath);
    }

    hr = RemoteDatabaseInitialize(wzPath, FALSE, pcdHandle);
    ExitOnFailure1(hr, "Failed to create remote database at path: %ls", wzPath);

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgOpenRemoteDatabase(
    __in LPCWSTR wzPath,
    __deref_out_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE *pcdHandle
    )
{
    HRESULT hr = S_OK;

    if (!FileExistsEx(wzPath, NULL))
    {
        hr = E_NOTFOUND;
        ExitOnFailure1(hr, "Tried to open remote database %ls, but it doesn't exist!", wzPath);
    }

    hr = RemoteDatabaseInitialize(wzPath, FALSE, pcdHandle);
    ExitOnFailure1(hr, "Failed to open remote database at path: %ls", wzPath);

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgRememberDatabase(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdLocalHandle,
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdRemoteHandle,
    __in LPCWSTR wzFriendlyName,
    __in BOOL fSyncByDefault
    )
{
    HRESULT hr = S_OK;
    BOOL fInSceTransaction = FALSE;
    SCE_ROW_HANDLE sceRow = NULL;
    CFGDB_STRUCT *pcdbLocal = static_cast<CFGDB_STRUCT *>(cdLocalHandle);
    CFGDB_STRUCT *pcdbRemote = static_cast<CFGDB_STRUCT *>(cdRemoteHandle);

    ExitOnNull(pcdbLocal, hr, E_INVALIDARG, "Local database handle input pointer must not be NULL");
    ExitOnNull(pcdbRemote, hr, E_INVALIDARG, "Remote database handle input pointer must not be NULL");
    ExitOnNull(wzFriendlyName, hr, E_INVALIDARG, "Friendly name must not be NULL");

    hr = DatabaseListFind(pcdbLocal, wzFriendlyName, &sceRow);
    if (E_NOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to find database index row for friendly name '%ls'", wzFriendlyName);

        hr = SceBeginTransaction(pcdbLocal->psceDb);
        ExitOnFailure(hr, "Failed to begin transaction");
        fInSceTransaction = TRUE;

        hr = SceSetColumnBool(sceRow, DATABASE_INDEX_SYNC_BY_DEFAULT, fSyncByDefault);
        ExitOnFailure(hr, "Failed to update 'sync by default' column for existing database in list");

        if (pcdbRemote->fSyncByDefault && !fSyncByDefault)
        {
            hr = BackgroundRemoveRemote(pcdbLocal, pcdbRemote->sczDbPath);
            ExitOnFailure1(hr, "Failed to remove remote path to background thread for automatic synchronization: %ls", pcdbRemote->sczDbPath);
        }
        else if (!pcdbRemote->fSyncByDefault && fSyncByDefault)
        {
            hr = BackgroundAddRemote(pcdbLocal, pcdbRemote->sczDbPath);
            ExitOnFailure1(hr, "Failed to add remote path to background thread for automatic synchronization: %ls", pcdbRemote->sczDbPath);
        }

        hr = SceFinishUpdate(sceRow);
        ExitOnFailure(hr, "Failed to finish update while updating existing database in list");

        hr = SceCommitTransaction(pcdbLocal->psceDb);
        ExitOnFailure(hr, "Failed to commit transaction");
        fInSceTransaction = FALSE;
    }
    else
    {
        hr = DatabaseListInsert(pcdbLocal, wzFriendlyName, fSyncByDefault, pcdbRemote->sczDbPath);
        ExitOnFailure1(hr, "Failed to remember database '%ls' in database list", wzFriendlyName);
    }

    pcdbRemote->fSyncByDefault = fSyncByDefault;

LExit:
    ReleaseSceRow(sceRow);
    if (fInSceTransaction)
    {
        SceRollbackTransaction(pcdbLocal->psceDb);
    }

    return hr;
}

extern "C" HRESULT CFGAPI CfgOpenKnownRemoteDatabase(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdLocalHandle,
    __in LPCWSTR wzFriendlyName,
    __deref_out_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE *pcdHandle
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdbLocal = static_cast<CFGDB_STRUCT *>(cdLocalHandle);
    SCE_ROW_HANDLE sceRow = NULL;
    LPWSTR sczPath = NULL;
    BOOL fSyncByDefault = FALSE;
    BOOL fLocked = FALSE;

    ExitOnNull(pcdbLocal, hr, E_INVALIDARG, "Local database handle input pointer must not be NULL");
    ExitOnNull(wzFriendlyName, hr, E_INVALIDARG, "Friendly name must not be NULL");
    ExitOnNull(pcdHandle, hr, E_INVALIDARG, "Output database handle input pointer must not be NULL");

    hr = HandleLock(pcdbLocal);
    ExitOnFailure(hr, "Failed to lock handle while opening known remote database");
    fLocked = TRUE;

    hr = DatabaseListFind(pcdbLocal, wzFriendlyName, &sceRow);
    ExitOnFailure1(hr, "Failed to find database index row for friendly name '%ls'", wzFriendlyName);

    hr = SceGetColumnString(sceRow, DATABASE_INDEX_PATH, &sczPath);
    ExitOnFailure1(hr, "Failed to get path from database list for database '%ls'", wzFriendlyName);

    hr = SceGetColumnBool(sceRow, DATABASE_INDEX_SYNC_BY_DEFAULT, &fSyncByDefault);
    ExitOnFailure(hr, "Failed to get 'sync by default' column for existing database in list");

    hr = RemoteDatabaseInitialize(sczPath, fSyncByDefault, pcdHandle);
    ExitOnFailure2(hr, "Failed to open known database '%ls' at path '%ls'", wzFriendlyName, sczPath);

LExit:
    ReleaseSceRow(sceRow);
    if (fLocked)
    {
        HandleUnlock(pcdbLocal);
    }
    ReleaseStr(sczPath);

    return hr;
}

extern "C" HRESULT CFGAPI CfgForgetDatabase(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdLocalHandle,
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdRemoteHandle,
    __in LPCWSTR wzFriendlyName
    )
{
    HRESULT hr = S_OK;
    BOOL fLocked = FALSE;
    CFGDB_STRUCT *pcdbLocal = static_cast<CFGDB_STRUCT *>(cdLocalHandle);
    CFGDB_STRUCT *pcdbRemote = static_cast<CFGDB_STRUCT *>(cdRemoteHandle);

    ExitOnNull(pcdbLocal, hr, E_INVALIDARG, "Local database handle input pointer must not be NULL");
    ExitOnNull(wzFriendlyName, hr, E_INVALIDARG, "Friendly name must not be NULL");

    hr = HandleLock(pcdbLocal);
    ExitOnFailure(hr, "Failed to lock handle while forgetting database");
    fLocked = TRUE;

    if (pcdbRemote->fSyncByDefault)
    {
        hr = BackgroundRemoveRemote(pcdbLocal, pcdbRemote->sczDbPath);
        ExitOnFailure1(hr, "Failed to remove remote path to background thread for automatic synchronization: %ls", pcdbRemote->sczDbPath);

        pcdbRemote->fSyncByDefault = FALSE;
    }

    hr = DatabaseListDelete(pcdbLocal, wzFriendlyName);
    ExitOnFailure1(hr, "Failed to delete database '%ls' from database list", wzFriendlyName);

LExit:
    if (fLocked)
    {
        HandleUnlock(pcdbLocal);
    }

    return hr;
}

extern "C" HRESULT CFGAPI CfgRemoteDisconnect(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle
    )
{
    HRESULT hr = S_OK;
    DWORD dwIndex = DWORD_MAX;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    CFGDB_STRUCT *pcdbLocal = pcdb->pcdbLocal;
    BOOL fLocked = FALSE;

    hr = HandleLock(pcdbLocal);
    ExitOnFailure(hr, "Failed to lock handle while disconnecting from remote");
    fLocked = TRUE;

    if (pcdb->fSyncByDefault)
    {
        hr = BackgroundRemoveRemote(pcdb->pcdbLocal, pcdb->sczDbPath);
        ExitOnFailure(hr, "Failed to remove remote database from automatic sync list");
    }

    if (FindPathInOpenDatabasesList(pcdb->pcdbLocal, pcdb->sczDbPath, &dwIndex))
    {
        MemRemoveFromArray(reinterpret_cast<void *>(pcdb->pcdbLocal->rgpcdbOpenDatabases), dwIndex, 1, pcdb->pcdbLocal->cOpenDatabases, sizeof(CFGDB_STRUCT *), TRUE);
        --pcdb->pcdbLocal->cOpenDatabases;
    }

    pcdb->dwAppID = DWORD_MAX;
    pcdb->fProductSet = FALSE;
    ReleaseStr(pcdb->sczGuid);
    ReleaseStr(pcdb->sczDbPath);
    ReleaseStr(pcdb->sczDbDir);
    ReleaseStr(pcdb->sczStreamsDir);

    hr = SceCloseDatabase(pcdb->psceDb);
    ExitOnFailure(hr, "Failed to close remote database");

    DatabaseReleaseSceSchema(&pcdb->dsSceDb);

    hr = CfgUninitialize(pcdb->pcdbLocal);
    ExitOnFailure(hr, "Failed to uninitialize Cfg Db");

    ::DeleteCriticalSection(&pcdb->cs);

    ReleaseMem(pcdb);

LExit:
    if (fLocked)
    {
        HandleUnlock(pcdbLocal);
    }

    return hr;
}

// Static functions
static HRESULT RemoteDatabaseInitialize(
    __in LPCWSTR wzPath,
    __in BOOL fSyncByDefault,
    __deref_out_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE *pcdHandle
    )
{
    HRESULT hr = S_OK;
    DWORD dwIndex = DWORD_MAX;
    CFGDB_STRUCT *pcdbLocal = NULL;
    CFGDB_STRUCT *pcdb = NULL;
    BOOL fLocked = FALSE;

    hr = CfgInitialize(reinterpret_cast<CFGDB_HANDLE *>(&pcdbLocal), NULL, NULL, NULL);
    ExitOnFailure(hr, "Failed to initialize Cfg Db");

    hr = HandleLock(pcdbLocal);
    ExitOnFailure(hr, "Failed to lock handle while initializing remote database");
    fLocked = TRUE;

    // See if we already exist in the open databases list - if we do, return existing pointer and don't grow the list
    if (FindPathInOpenDatabasesList(pcdbLocal, wzPath, &dwIndex))
    {
        *pcdHandle = static_cast<CFGDB_HANDLE>(pcdbLocal->rgpcdbOpenDatabases[dwIndex]);
        ExitFunction1(hr = S_OK);
    }

    pcdb = static_cast<CFGDB_STRUCT *>(MemAlloc(sizeof(CFGDB_STRUCT), TRUE));
    ExitOnNull(pcdb, hr, E_OUTOFMEMORY, "Failed to allocate memory for cfg db struct");

    ::InitializeCriticalSection(&pcdb->cs);
    pcdb->dwAppID = DWORD_MAX;
    pcdb->fRemote = TRUE;

    hr = StrAllocString(&pcdb->sczDbPath, wzPath, 0);
    ExitOnFailure(hr, "Failed to copy database path string");

    hr = PathGetDirectory(pcdb->sczDbPath, &pcdb->sczDbDir);
    ExitOnFailure(hr, "Failed to copy remote database directory");

    hr = DirEnsureExists(pcdb->sczDbDir, NULL);
    if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
    {
        // If path isn't found, it *might* be a mounted drive that is currently disconnected. Fall back to raw UNC path
        hr = S_OK;

        hr = ConvertToUNCPath(&pcdb->sczDbPath);
        ExitOnFailure(hr, "Failed to convert to UNC path");

        hr = PathGetDirectory(pcdb->sczDbPath, &pcdb->sczDbDir);
        ExitOnFailure(hr, "Failed to copy remote database directory after UNC conversion");

        hr = DirEnsureExists(pcdb->sczDbDir, NULL);
        ExitOnFailure(hr, "Failed to ensure remote database directory exists after UNC conversion");
    }
    ExitOnFailure(hr, "Failed to ensure remote database directory exists");

    hr = PathConcat(pcdb->sczDbDir, L"LAST_REAL_CHANGE", &pcdb->sczDbChangesPath);
    ExitOnFailure(hr, "Failed to get db changes path");

    if (!FileExistsEx(pcdb->sczDbChangesPath, NULL))
    {
        hr = FileWrite(pcdb->sczDbChangesPath, FILE_ATTRIBUTE_HIDDEN, NULL, 0, NULL);
        ExitOnFailure1(hr, "Failed to write new db changes file: %ls", pcdb->sczDbChangesPath);
    }

    // Setup expected schema in memory
    hr = DatabaseSetupUserSchema(SHARED_TABLES_NUMBER, &pcdb->dsSceDb);
    ExitOnFailure(hr, "Failed to setup user database schema structure in memory");

    // Open the database (or create if it doesn't exist)
    hr = SceEnsureDatabase(pcdb->sczDbPath, wzSqlCeDllPath, L"CfgRemote", 1, &pcdb->dsSceDb, &pcdb->psceDb);
    ExitOnFailure(hr, "Failed to create SQL CE database");

    hr = EnsureRemoteSummaryDataTable(pcdb);
    ExitOnFailure(hr, "Failed to ensure remote database summary data");

    hr = ProductSet(pcdb, wzCfgProductId, wzCfgVersion, wzCfgPublicKey, FALSE, NULL);
    ExitOnFailure(hr, "Failed to set product to cfg product id");

    pcdb->dwCfgAppID = pcdb->dwAppID;

    hr = PathConcat(pcdb->sczDbDir, L"Streams", &pcdb->sczStreamsDir);
    ExitOnFailure(hr, "Failed to get path to streams directory");

    hr = MemEnsureArraySize(reinterpret_cast<void **>(&pcdbLocal->rgpcdbOpenDatabases), pcdbLocal->cOpenDatabases + 1, sizeof(CFGDB_STRUCT *), 0);
    ExitOnFailure(hr, "Failed to ensure open database list array size");
    pcdbLocal->rgpcdbOpenDatabases[pcdbLocal->cOpenDatabases] = pcdb;
    ++pcdbLocal->cOpenDatabases;

    if (fSyncByDefault)
    {
        hr = BackgroundAddRemote(pcdbLocal, pcdb->sczDbDir);
        ExitOnFailure1(hr, "Failed to add remote path to background thread for automatic synchronization: %ls", pcdb->sczDbDir);
    }
    pcdb->pcdbLocal = pcdbLocal;
    pcdb->fSyncByDefault = fSyncByDefault;

    hr = SceCloseDatabase(pcdb->psceDb);
    ExitOnFailure(hr, "Failed to close remote database - ignoring");
    pcdb->psceDb = NULL;

    *pcdHandle = pcdb;

LExit:
    if (NULL != pcdbLocal && FAILED(hr))
    {
        CfgUninitialize(pcdbLocal);
    }
    if (fLocked)
    {
        HandleUnlock(pcdbLocal);
    }

    return hr;
}

static BOOL FindPathInOpenDatabasesList(
    __in CFGDB_STRUCT *pcdbLocal,
    __in_z LPCWSTR wzPath,
    __out DWORD *pdwIndex
    )
{
    for (DWORD i = 0; i < pcdbLocal->cOpenDatabases; ++i)
    {
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pcdbLocal->rgpcdbOpenDatabases[i]->sczDbPath, -1, wzPath, -1))
        {
            *pdwIndex = i;
            return TRUE;
        }
    }

    return FALSE;
}

static HRESULT ConvertToUNCPath(
    __in LPWSTR *psczPath
    )
{
    HRESULT hr = S_OK;
    DWORD dwLength = 0;
    DWORD er = ERROR_SUCCESS;
    LPWSTR sczDrive = NULL;
    LPWSTR sczPath = NULL;

    // Backup entire string
    hr = StrAllocString(&sczPath, *psczPath, 0);
    ExitOnFailure(hr, "Failed to copy drive");

    // Only copy drive letter and colon
    hr = StrAllocString(&sczDrive, *psczPath, 2);
    ExitOnFailure(hr, "Failed to copy drive");

    /* ERROR_NOT_CONNECTED means it's not a mapped drive */
    er = ::WNetGetConnectionW(sczDrive, NULL, &dwLength);
    if (ERROR_MORE_DATA == er)
    {
        er = ERROR_SUCCESS;

        hr = StrAlloc(psczPath, dwLength);
        ExitOnFailure1(hr, "Failed to allocate string to get raw UNC path %u", dwLength);

        er = ::WNetGetConnectionW(sczDrive, *psczPath, &dwLength);
        if (ERROR_CONNECTION_UNAVAIL == er)
        {
            // This means the drive is remembered but not currently connected, exactly what this code was designed to fix
            er = ERROR_SUCCESS;
        }
        ExitOnWin32Error1(er, hr, "::WNetGetConnectionW() failed with buffer provided on drive %ls", sczDrive);

        // Skip drive letter and colon
        hr = StrAllocConcat(psczPath, sczPath + 2, 0);
        ExitOnFailure(hr, "Failed to copy rest of database path");
    }
    else
    {
        if (ERROR_SUCCESS == er)
        {
            er = ERROR_NO_DATA;
        }

        ExitOnWin32Error1(er, hr, "::WNetGetConnectionW() failed on drive %ls", sczDrive);
    }

LExit:
    ReleaseStr(sczDrive);
    ReleaseStr(sczPath);

    return hr;
}
