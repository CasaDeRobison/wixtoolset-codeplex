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

static HRESULT CFGAPI RemoteDatabaseInitialize(
    __in LPCWSTR wzPath,
    __deref_out_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE *pcdHandle
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

    hr = RemoteDatabaseInitialize(wzPath, pcdHandle);
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

    hr = RemoteDatabaseInitialize(wzPath, pcdHandle);
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

    ExitOnNull(pcdbLocal, hr, E_INVALIDARG, "Local database handle input pointer must not be NULL");
    ExitOnNull(wzFriendlyName, hr, E_INVALIDARG, "Friendly name must not be NULL");
    ExitOnNull(pcdHandle, hr, E_INVALIDARG, "Output database handle input pointer must not be NULL");

    hr = DatabaseListFind(pcdbLocal, wzFriendlyName, &sceRow);
    ExitOnFailure1(hr, "Failed to find database index row for friendly name '%ls'", wzFriendlyName);

    hr = SceGetColumnString(sceRow, DATABASE_INDEX_PATH, &sczPath);
    ExitOnFailure1(hr, "Failed to get path from database list for database '%ls'", wzFriendlyName);

    hr = RemoteDatabaseInitialize(sczPath, pcdHandle);
    ExitOnFailure2(hr, "Failed to open known database '%ls' at path '%ls'", wzFriendlyName, sczPath);

LExit:
    ReleaseSceRow(sceRow);
    ReleaseStr(sczPath);

    return hr;
}

extern "C" HRESULT CFGAPI CfgForgetDatabase(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdLocalHandle,
    __in LPCWSTR wzFriendlyName
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdbLocal = static_cast<CFGDB_STRUCT *>(cdLocalHandle);

    ExitOnNull(pcdbLocal, hr, E_INVALIDARG, "Local database handle input pointer must not be NULL");
    ExitOnNull(wzFriendlyName, hr, E_INVALIDARG, "Friendly name must not be NULL");

    hr = DatabaseListDelete(pcdbLocal, wzFriendlyName);
    ExitOnFailure1(hr, "Failed to delete database '%ls' from database list", wzFriendlyName);

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgRemoteDisconnect(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);

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
    return hr;
}

// Static functions
static HRESULT CFGAPI RemoteDatabaseInitialize(
    __in LPCWSTR wzPath,
    __deref_out_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE *pcdHandle
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb;

    pcdb = static_cast<CFGDB_STRUCT *>(MemAlloc(sizeof(CFGDB_STRUCT), TRUE));
    ExitOnNull(pcdb, hr, E_OUTOFMEMORY, "Failed to allocate memory for cfg db struct");

    hr = CfgInitialize(reinterpret_cast<CFGDB_HANDLE *>(&pcdb->pcdbLocal));
    ExitOnFailure(hr, "Failed to initialize Cfg Db");

    ::InitializeCriticalSection(&pcdb->cs);
    pcdb->dwAppID = DWORD_MAX;
    pcdb->fRemote = TRUE;

    hr = StrAllocString(&pcdb->sczDbPath, wzPath, 0);
    ExitOnFailure(hr, "Failed to copy database path string");

    hr = PathGetDirectory(pcdb->sczDbPath, &pcdb->sczDbDir);
    ExitOnFailure(hr, "Failed to copy remote database directory");

    // Setup expected schema in memory
    hr = DatabaseSetupUserSchema(SHARED_TABLES_NUMBER, &pcdb->dsSceDb);
    ExitOnFailure(hr, "Failed to setup user database schema structure in memory");

    // Open the database (or create if it doesn't exist)
    hr = SceEnsureDatabase(wzPath, wzSqlCeDllPath, L"CfgRemote", 1, &pcdb->dsSceDb, &pcdb->psceDb);
    ExitOnFailure(hr, "Failed to create SQL CE database");

    hr = EnsureRemoteSummaryDataTable(pcdb);
    ExitOnFailure(hr, "Failed to ensure remote database summary data");

    hr = ProductSet(pcdb, wzCfgProductId, wzCfgVersion, wzCfgPublicKey, FALSE, NULL);
    ExitOnFailure(hr, "Failed to set product to cfg product id");

    pcdb->dwCfgAppID = pcdb->dwAppID;

    hr = PathConcat(pcdb->sczDbDir, L"Streams", &pcdb->sczStreamsDir);
    ExitOnFailure(hr, "Failed to get path to streams directory");

    ::EnterCriticalSection(&cfgState.cs);
    hr = MemEnsureArraySize(reinterpret_cast<void **>(&cfgState.rgpcdbOpenDatabases), cfgState.cOpenDatabases + 1, sizeof(CFGDB_STRUCT *), 0);
    ExitOnFailure(hr, "Failed to ensure open database list array size");
    cfgState.rgpcdbOpenDatabases[cfgState.cOpenDatabases] = pcdb;
    ++cfgState.cOpenDatabases;
    ::LeaveCriticalSection(&cfgState.cs);

    *pcdHandle = pcdb;

LExit:
    return hr;
}
