//-------------------------------------------------------------------------------------------------
// <copyright file="cfgapi.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
// Core settings engine API (functions for apps related to per-user state)
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

volatile static DWORD s_dwRefCount = 0;
static BOOL s_fXmlInitialized = FALSE;
static CFGDB_STRUCT s_cdb = { };
volatile static BOOL vfComInitialized = FALSE;

const int CFGDB_HANDLE_BYTES = sizeof(CFGDB_STRUCT);
const int CFG_ENUMERATION_HANDLE_BYTES = sizeof(CFG_ENUMERATION);

CFG_GLOBAL_STATE cfgState = { };

static HRESULT InitializeImpersonationToken(
    __inout CFGDB_STRUCT *pcdb
    );

HRESULT EnsureSummaryDataTable(
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

extern "C" HRESULT CFGAPI CfgInitialize(
    __deref_out_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE *pcdHandle
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDbFilePath = NULL;
    CFGDB_STRUCT *pcdb = &s_cdb;

    ExitOnNull(pcdHandle, hr, E_INVALIDARG, "Database handle output pointer must not be NULL");

    *pcdHandle = pcdb;

    ::InterlockedIncrement(&s_dwRefCount);

    if (1 == s_dwRefCount)
    {
        LogInitialize(NULL);

        hr = LogOpen(NULL, L"CfgAPI", NULL, L".log", FALSE, TRUE, NULL);
        ExitOnFailure(hr, "Failed to initialize log");

        ::InitializeCriticalSection(&cfgState.cs);

        hr = ::CoInitialize(0);
        ExitOnFailure(hr, "Failed to initialize COM");
        vfComInitialized = TRUE;

        ZeroMemory(&s_cdb, sizeof(s_cdb));
        ::InitializeCriticalSection(&pcdb->cs);
        pcdb->dwAppID = DWORD_MAX;

        hr = CfgAdminInitialize(reinterpret_cast<CFGDB_HANDLE *>(&pcdb->pcdbAdmin), FALSE);
        ExitOnFailure(hr, "Failed to initialize Cfg Admin Db");

        hr = InitializeImpersonationToken(pcdb);
        ExitOnFailure(hr, "Failed to initialize impersonation token");

        hr = DatabaseGetUserDir(&pcdb->sczDbDir);
        ExitOnFailure(hr, "Failed to get user database directory");

        hr = XmlInitialize();
        ExitOnFailure(hr, "Failed to initialize MSXML");
        s_fXmlInitialized = TRUE;

        // Setup expected schema in memory
        hr = DatabaseSetupUserSchema(USER_TABLES_NUMBER, &pcdb->dsSceDb);
        ExitOnFailure(hr, "Failed to setup user database schema structure in memory");

        // Get the path to the exact file
        hr = DatabaseGetUserPath(&sczDbFilePath);
        ExitOnFailure(hr, "Failed to get user database path");

        // Create the database
        hr = SceEnsureDatabase(sczDbFilePath, wzSqlCeDllPath, L"CfgUser", 1, &pcdb->dsSceDb, &pcdb->psceDb);
        ExitOnFailure(hr, "Failed to create SQL CE database");

        hr = EnsureSummaryDataTable(pcdb);
        ExitOnFailure(hr, "Failed to ensure summary data");

        hr = PathConcat(pcdb->sczDbDir, L"Streams", &pcdb->sczStreamsDir);
        ExitOnFailure(hr, "Failed to get path to streams directory");
    }

    hr = ProductSet(pcdb, wzCfgProductId, wzCfgVersion, wzCfgPublicKey, FALSE, NULL);
    ExitOnFailure(hr, "Failed to set product to cfg product id");

    pcdb->dwCfgAppID = pcdb->dwAppID;

    hr = CfgRegisterProduct(pcdb, wzCfgProductId, wzCfgVersion, wzCfgPublicKey);
    ExitOnFailure(hr, "Failed to set product to cfg product id");

LExit:
    ReleaseStr(sczDbFilePath);

    return hr;
}

extern "C" HRESULT CFGAPI CfgUninitialize(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");

    ::InterlockedDecrement(&s_dwRefCount);

    if (0 == s_dwRefCount)
    {
        pcdb->dwAppID = DWORD_MAX;
        pcdb->fProductSet = FALSE;
        ReleaseNullStr(pcdb->sczGuid);
        ReleaseNullStr(pcdb->sczDbDir);
        ReleaseNullStr(pcdb->sczStreamsDir);

        if (s_fXmlInitialized)
        {
            s_fXmlInitialized = FALSE;
            XmlUninitialize();
        }

        hr = CfgAdminUninitialize(pcdb->pcdbAdmin);
        ExitOnFailure(hr, "Failed to uninitialize Cfg Admin Db");

        DatabaseReleaseSceSchema(&pcdb->dsSceDb);

        hr = SceCloseDatabase(pcdb->psceDb);
        ExitOnFailure(hr, "Failed to close user database");

        if (vfComInitialized)
        {
            ::CoUninitialize();
            vfComInitialized = FALSE;
        }

        if (pcdb->hToken)
        {
            ::CloseHandle(pcdb->hToken);
        }

        LogUninitialize(TRUE);

        ::DeleteCriticalSection(&pcdb->cs);
        ReleaseNullMem(cfgState.rgpcdbOpenDatabases);
        cfgState.cOpenDatabases = 0;
        ::DeleteCriticalSection(&cfgState.cs);
    }

LExit:
    return hr;
}

extern "C" HRESULT CfgGetEndpointGuid(
    __in_bcount(CFGDB_HANDLE_BYTES) C_CFGDB_HANDLE cdHandle,
    __out_z LPWSTR *psczGuid
    )
{
    HRESULT hr = S_OK;
    const CFGDB_STRUCT *pcdb = static_cast<const CFGDB_STRUCT *>(cdHandle);

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(psczGuid, hr, E_INVALIDARG, "Guid must not be NULL");

    if (NULL == pcdb->sczGuid)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    hr = StrAllocString(psczGuid, pcdb->sczGuid, 0);
    ExitOnFailure(hr, "Failed to allocate copy of guid string");

LExit:
    return hr;
}

// Generally this is the first call after CfgInitialize() an app will make - it tells the settings engine your application's unique identifier,
// and is roughly equivalent to setting the namespace for all your configuration data which will be stored
extern "C" HRESULT CfgSetProduct(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzProductName,
    __in_z LPCWSTR wzVersion,
    __in_z LPCWSTR wzPublicKey
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LPWSTR sczLowPublicKey = NULL;
    BOOL fLegacyProduct = (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, wzPublicKey, -1, wzLegacyPublicKey, -1));

    // Immediately unset the previously set product in case caller ignores a
    // failed return value and starts writing to the previously set product
    pcdb->fProductSet = FALSE;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzProductName, hr, E_INVALIDARG, "Product Name must not be NULL");
    ExitOnNull(wzVersion, hr, E_INVALIDARG, "Version must not be NULL");
    ExitOnNull(wzPublicKey, hr, E_INVALIDARG, "Public Key must not be NULL for non-legacy databases");

    hr = StrAllocString(&sczLowPublicKey, wzPublicKey, 0);
    ExitOnFailure(hr, "Failed to allocate 2nd public key buffer");

    // Convert to lower case characters
    StrStringToLower(sczLowPublicKey);

    hr = ProductValidateName(wzProductName);
    ExitOnFailure1(hr, "Failed to validate ProductName: %ls", wzProductName);

    hr = ProductValidateVersion(wzVersion);
    ExitOnFailure1(hr, "Failed to validate Version while setting product: %ls", wzVersion);

    hr = ProductValidatePublicKey(sczLowPublicKey);
    ExitOnFailure1(hr, "Failed to validate Public Key: %ls", sczLowPublicKey);

    // Don't allow creating legacy products from here, because they won't have manifests and thus won't be sync-able
    hr = ProductSet(pcdb, wzProductName, wzVersion, sczLowPublicKey, fLegacyProduct, NULL);
    ExitOnFailure(hr, "Failed to call internal set product function");

LExit:
    ReleaseStr(sczLowPublicKey);

    return hr;
}

// Get / set DWORD values
extern "C" HRESULT CfgSetDword(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __in DWORD dwValue
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LEGACY_SYNC_SESSION syncSession = { };
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncInitializeSession(TRUE, &syncSession);
        ExitOnFailure(hr, "Failed to initialize legacy sync session");

        hr = LegacySyncSetProduct(pcdb, &syncSession, pcdb->sczProductName);
        ExitOnFailure(hr, "Failed to set product in legacy sync session");

        hr = LegacyProductMachineToDb(pcdb, &syncSession.syncProductSession);
        ExitOnFailure(hr, "Failed to read data from local machine and write into settings database for app");
    }

    hr = ValueSetDword(dwValue, NULL, pcdb->sczGuid, &cvValue);
    ExitOnFailure(hr, "Failed to set dword value in memory");

    hr = ValueWrite(pcdb, pcdb->dwAppID, wzName, &cvValue, TRUE);
    ExitOnFailure1(hr, "Failed to set DWORD value: %u", dwValue);

    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncFinalizeProduct(pcdb, &syncSession);
        ExitOnFailure(hr, "Failed to finalize product in legacy sync session");
    }

LExit:
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        LegacySyncUninitializeSession(pcdb, &syncSession);
    }
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CfgGetDword(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __out DWORD *pdwValue
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    SCE_ROW_HANDLE sceRow = NULL;
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzName, hr, E_INVALIDARG, "Name of value must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    hr = ValueFindRow(pcdb, VALUE_INDEX_TABLE, pcdb->dwAppID, wzName, &sceRow);
    ExitOnFailure2(hr, "Failed to find config value for AppID: %u, Config Value named: %ls", pcdb->dwAppID, wzName);

    hr = ValueRead(pcdb, sceRow, &cvValue);
    ExitOnFailure(hr, "Failed to retrieve deleted column");

    if (VALUE_DELETED == cvValue.cvType)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    if (VALUE_DWORD != cvValue.cvType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        ExitOnFailure1(hr, "Tried to retrieve value as dword, but it's of type: %d", cvValue.cvType);
    }

    *pdwValue = cvValue.dword.dwValue;

LExit:
    ReleaseSceRow(sceRow);
    ReleaseCfgValue(cvValue);

    return hr;
}

// Get / set QWORD values
extern "C" HRESULT CfgSetQword(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __in DWORD64 qwValue
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LEGACY_SYNC_SESSION syncSession = { };
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncInitializeSession(TRUE, &syncSession);
        ExitOnFailure(hr, "Failed to initialize legacy sync session");

        hr = LegacySyncSetProduct(pcdb, &syncSession, pcdb->sczProductName);
        ExitOnFailure(hr, "Failed to set product in legacy sync session");

        hr = LegacyProductMachineToDb(pcdb, &syncSession.syncProductSession);
        ExitOnFailure(hr, "Failed to read data from local machine and write into settings database for app");
    }

    hr = ValueSetQword(qwValue, NULL, pcdb->sczGuid, &cvValue);
    ExitOnFailure(hr, "Failed to set qword value in memory");

    hr = ValueWrite(pcdb, pcdb->dwAppID, wzName, &cvValue, TRUE);
    ExitOnFailure1(hr, "Failed to set QWORD value: %I64u", qwValue);

    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncFinalizeProduct(pcdb, &syncSession);
        ExitOnFailure(hr, "Failed to finalize product in legacy sync session");
    }

LExit:
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        LegacySyncUninitializeSession(pcdb, &syncSession);
    }
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CfgGetQword(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __out DWORD64 *pqwValue
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    SCE_ROW_HANDLE sceRow = NULL;
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzName, hr, E_INVALIDARG, "Name of value must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    hr = ValueFindRow(pcdb, VALUE_INDEX_TABLE, pcdb->dwAppID, wzName, &sceRow);
    ExitOnFailure2(hr, "Failed to find config value for AppID: %u, Config Value named: %ls", pcdb->dwAppID, wzName);

    hr = ValueRead(pcdb, sceRow, &cvValue);
    ExitOnFailure(hr, "Failed to retrieve deleted column");

    if (VALUE_DELETED == cvValue.cvType)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    if (VALUE_QWORD != cvValue.cvType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        ExitOnFailure1(hr, "Tried to retrieve value as dword, but it's of type: %d", cvValue.cvType);
    }

    *pqwValue = cvValue.qword.qwValue;

LExit:
    ReleaseSceRow(sceRow);
    ReleaseCfgValue(cvValue);

    return hr;
}

// Get / set string values
extern "C" HRESULT CfgSetString(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __in_z LPCWSTR wzValue
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LEGACY_SYNC_SESSION syncSession = { };
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    // TODO: Optimize this better (only write the value changed)
    // Also TODO: better transactionality - catch the situation when the data on local machine has
    // changed since last read, and return error in this case
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncInitializeSession(TRUE, &syncSession);
        ExitOnFailure(hr, "Failed to initialize legacy sync session");

        hr = LegacySyncSetProduct(pcdb, &syncSession, pcdb->sczProductName);
        ExitOnFailure(hr, "Failed to set product in legacy sync session");

        hr = LegacyProductMachineToDb(pcdb, &syncSession.syncProductSession);
        ExitOnFailure(hr, "Failed to read data from local machine and write into settings database for app");
    }

    hr = ValueSetString(wzValue, FALSE, NULL, pcdb->sczGuid, &cvValue);
    ExitOnFailure(hr, "Failed to set string value in memory");

    hr = ValueWrite(pcdb, pcdb->dwAppID, wzName, &cvValue, TRUE);
    ExitOnFailure2(hr, "Failed to set string value '%ls' to '%ls'", wzName, wzValue);

    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncFinalizeProduct(pcdb, &syncSession);
        ExitOnFailure(hr, "Failed to finalize product in legacy sync session");
    }

LExit:
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        LegacySyncUninitializeSession(pcdb, &syncSession);
    }
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CfgGetString(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __out LPWSTR *psczValue
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    SCE_ROW_HANDLE sceRow = NULL;
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzName, hr, E_INVALIDARG, "Name of value must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    hr = ValueFindRow(pcdb, VALUE_INDEX_TABLE, pcdb->dwAppID, wzName, &sceRow);
    ExitOnFailure2(hr, "Failed to find config value for AppID: %u, Config Value named: %ls", pcdb->dwAppID, wzName);

    hr = ValueRead(pcdb, sceRow, &cvValue);
    ExitOnFailure(hr, "Failed to retrieve deleted column");

    if (VALUE_DELETED == cvValue.cvType)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    if (VALUE_STRING != cvValue.cvType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        ExitOnFailure1(hr, "Tried to retrieve value as string, but it's of type: %d", cvValue.cvType);
    }

    *psczValue = cvValue.string.sczValue;
    cvValue.string.sczValue = NULL;

LExit:
    ReleaseSceRow(sceRow);
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CFGAPI CfgSetBool(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __in BOOL fValue
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LEGACY_SYNC_SESSION syncSession = { };
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    // TODO: Optimize this better (only write the value changed)
    // Also TODO: better transactionality - catch the situation when the data on local machine has
    // changed since last read, and return error in this case
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncInitializeSession(TRUE, &syncSession);
        ExitOnFailure(hr, "Failed to initialize legacy sync session");

        hr = LegacySyncSetProduct(pcdb, &syncSession, pcdb->sczProductName);
        ExitOnFailure(hr, "Failed to set product in legacy sync session");

        hr = LegacyProductMachineToDb(pcdb, &syncSession.syncProductSession);
        ExitOnFailure(hr, "Failed to read data from local machine and write into settings database for app");
    }

    hr = ValueSetBool(fValue, NULL, pcdb->sczGuid, &cvValue);
    ExitOnFailure(hr, "Failed to set bool value in memory");

    hr = ValueWrite(pcdb, pcdb->dwAppID, wzName, &cvValue, TRUE);
    ExitOnFailure1(hr, "Failed to set BOOL value named: %ls", wzName);

    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncFinalizeProduct(pcdb, &syncSession);
        ExitOnFailure(hr, "Failed to finalize product in legacy sync session");
    }

LExit:
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        LegacySyncUninitializeSession(pcdb, &syncSession);
    }
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CFGAPI CfgGetBool(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __out BOOL *pfValue
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    SCE_ROW_HANDLE sceRow = NULL;
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzName, hr, E_INVALIDARG, "Name of value must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    hr = ValueFindRow(pcdb, VALUE_INDEX_TABLE, pcdb->dwAppID, wzName, &sceRow);
    ExitOnFailure2(hr, "Failed to find config value for AppID: %u, Config Value named: %ls", pcdb->dwAppID, wzName);

    hr = ValueRead(pcdb, sceRow, &cvValue);
    ExitOnFailure(hr, "Failed to retrieve deleted column");

    if (VALUE_DELETED == cvValue.cvType)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    if (VALUE_BOOL != cvValue.cvType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        ExitOnFailure1(hr, "Tried to retrieve value as bool, but it's of type: %d", cvValue.cvType);
    }

    *pfValue = cvValue.boolean.fValue;

LExit:
    ReleaseSceRow(sceRow);
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CfgDeleteValue(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LEGACY_SYNC_SESSION syncSession = { };
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzName, hr, E_INVALIDARG, "Name of value must not be NULL");

    // TODO: Optimize this better (only write the value changed)
    // Also TODO: better transactionality - catch the situation when the data on local machine has
    // changed since last read, and return error in this case
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncInitializeSession(TRUE, &syncSession);
        ExitOnFailure(hr, "Failed to initialize legacy sync session");

        hr = LegacySyncSetProduct(pcdb, &syncSession, pcdb->sczProductName);
        ExitOnFailure(hr, "Failed to set product in legacy sync session");

        hr = LegacyProductMachineToDb(pcdb, &syncSession.syncProductSession);
        ExitOnFailure(hr, "Failed to read data from local machine and write into settings database for app");
    }

    hr = ValueSetDelete(NULL, pcdb->sczGuid, &cvValue);
    ExitOnFailure(hr, "Failed to set delete value in memory");

    hr = ValueWrite(pcdb, pcdb->dwAppID, wzName, &cvValue, TRUE);
    ExitOnFailure1(hr, "Failed to delete value: %ls", wzName);

    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncFinalizeProduct(pcdb, &syncSession);
        ExitOnFailure(hr, "Failed to finalize product in legacy sync session");
    }

LExit:
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        LegacySyncUninitializeSession(pcdb, &syncSession);
    }
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CFGAPI CfgSetBlob(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __in_bcount(cbBuffer) const BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LEGACY_SYNC_SESSION syncSession = { };
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzName, hr, E_INVALIDARG, "Name of file must not be NULL");

    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    if (cbBuffer > 0)
    {
        ExitOnNull(pbBuffer, hr, E_INVALIDARG, "Size of file was more than zero, but no content was provided!");
    }
    if (!pcdb->fProductSet)
    {
        ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME));
    }

    // TODO: Optimize this better (only write the value changed)
    // Also TODO: better transactionality - catch the situation when the data on local machine has
    // changed since last read, and return error in this case
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncInitializeSession(TRUE, &syncSession);
        ExitOnFailure(hr, "Failed to initialize legacy sync session");

        hr = LegacySyncSetProduct(pcdb, &syncSession, pcdb->sczProductName);
        ExitOnFailure(hr, "Failed to set product in legacy sync session");

        hr = LegacyProductMachineToDb(pcdb, &syncSession.syncProductSession);
        ExitOnFailure(hr, "Failed to read data from local machine and write into settings database for app");
    }

    hr = ValueSetBlob(pbBuffer, cbBuffer, FALSE, NULL, pcdb->sczGuid, &cvValue);
    ExitOnFailure(hr, "Failed to set delete value in memory");

    hr = ValueWrite(pcdb, pcdb->dwAppID, wzName, &cvValue, TRUE);
    ExitOnFailure1(hr, "Failed to set blob: %ls", wzName);

    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        hr = LegacySyncFinalizeProduct(pcdb, &syncSession);
        ExitOnFailure(hr, "Failed to finalize product in legacy sync session");
    }

LExit:
    if (!pcdb->fRemote && pcdb->fProductIsLegacy)
    {
        LegacySyncUninitializeSession(pcdb, &syncSession);
    }
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CFGAPI CfgGetBlob(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __deref_opt_out_bcount_opt(*piBuffer) BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    DWORD dwContentID = DWORD_MAX;
    SCE_ROW_HANDLE sceRow = NULL;
    CONFIG_VALUE cvValue = { };

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");

    hr = ValueFindRow(pcdb, VALUE_INDEX_TABLE, pcdb->dwAppID, wzName, &sceRow);
    if (E_NOTFOUND == hr)
    {
        ExitFunction();
    }
    ExitOnFailure2(hr, "Failed to find blob named: %ls for AppID: %u", wzName, pcdb->dwAppID);

    hr = ValueRead(pcdb, sceRow, &cvValue);
    ExitOnFailure(hr, "Failed to retrieve deleted column");

    if (VALUE_DELETED == cvValue.cvType)
    {
        ExitFunction1(hr = E_NOTFOUND);
    }

    if (VALUE_BLOB != cvValue.cvType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        ExitOnFailure1(hr, "Tried to retrieve value as blob, but it's of type: %d", cvValue.cvType);
    }

    if (CFG_BLOB_DB_STREAM != cvValue.blob.cbType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_DATATYPE_MISMATCH);
        ExitOnFailure1(hr, "Tried to retrieve value as db stream blob, but it's of type: %d", cvValue.blob.cbType);
    }

    hr = StreamRead(pcdb, cvValue.blob.dbstream.dwContentID, NULL, ppbBuffer, piBuffer);
    ExitOnFailure2(hr, "Failed to get binary content of blob named: %ls, with content ID: %u", wzName, dwContentID);

LExit:
    ReleaseSceRow(sceRow);
    ReleaseCfgValue(cvValue);

    return hr;
}

extern "C" HRESULT CfgEnumerateValues(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in CONFIG_VALUETYPE cvType,
    __deref_out_bcount(CFG_ENUMERATION_HANDLE_BYTES) CFG_ENUMERATION_HANDLE *ppvHandle,
    __out_opt DWORD *pcCount
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(ppvHandle, hr, E_INVALIDARG, "Output handle must not be NULL");

    hr = EnumValues(pcdb, cvType, reinterpret_cast<CFG_ENUMERATION **>(ppvHandle), pcCount);
    ExitOnFailure(hr, "Failed to enumerate values");

LExit:
    return hr;
}

extern "C" HRESULT CfgEnumerateProducts(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z_opt LPCWSTR wzPublicKey,
    __deref_out_bcount(CFG_ENUMERATION_HANDLE_BYTES) CFG_ENUMERATION_HANDLE *ppvHandle,
    __out_opt DWORD *pcCount
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LPWSTR sczLowPublicKey = NULL;
    SCE_ROW_HANDLE sceRow = NULL;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(ppvHandle, hr, E_INVALIDARG, "Must pass in pointer to output handle to CfgEnumerateProducts()");

    // Allocate the Enumeration struct and its members
    CFG_ENUMERATION *pcesEnum = static_cast<CFG_ENUMERATION *>(MemAlloc(sizeof(CFG_ENUMERATION), TRUE));
    ExitOnNull(pcesEnum, hr, E_OUTOFMEMORY, "Failed to allocate Cfg Enumeration Struct");

    pcesEnum->enumType = ENUMERATION_PRODUCTS;

    hr = EnumResize(pcesEnum, 64);
    ExitOnFailure(hr, "Failed to resize enumeration struct immediately after its creation");

    if (NULL != wzPublicKey)
    {
        hr = StrAllocString(&sczLowPublicKey, wzPublicKey, 0);
        ExitOnFailure(hr, "Failed to allocate 2nd public key buffer");

        // Convert to lower case characters
        StrStringToLower(sczLowPublicKey);
    }

    hr = SceGetFirstRow(pcdb->psceDb, PRODUCT_INDEX_TABLE, &sceRow);
    while (E_NOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to get row from table: %u", PRODUCT_INDEX_TABLE);

        // If they specified a public key and it doesn't equal the one we're looking at, skip it
        if (NULL == sczLowPublicKey || 0 == lstrcmpW(pcesEnum->products.rgsczPublicKey[pcesEnum->dwNumValues], sczLowPublicKey))
        {
            if (pcesEnum->dwNumValues >= pcesEnum->dwMaxValues)
            {
                DWORD dwNewSize = pcesEnum->dwMaxValues * 2;

                hr = EnumResize(pcesEnum, dwNewSize);
                ExitOnFailure(hr, "Failed to resize enumeration struct");
            }

            hr = SceGetColumnString(sceRow, PRODUCT_NAME, &(pcesEnum->products.rgsczName[pcesEnum->dwNumValues]));
            ExitOnFailure(hr, "Failed to retrieve product name while enumerating products");

            hr = SceGetColumnString(sceRow, PRODUCT_VERSION, &(pcesEnum->products.rgsczVersion[pcesEnum->dwNumValues]));
            ExitOnFailure(hr, "Failed to retrieve version while enumerating products");

            hr = SceGetColumnString(sceRow, PRODUCT_PUBLICKEY, &(pcesEnum->products.rgsczPublicKey[pcesEnum->dwNumValues]));
            ExitOnFailure(hr, "Failed to retrieve public key while enumerating products");

            hr = SceGetColumnBool(sceRow, PRODUCT_REGISTERED, &(pcesEnum->products.rgfRegistered[pcesEnum->dwNumValues]));
            ExitOnFailure(hr, "Failed to retrieve registered flag while enumerating products");

            ++pcesEnum->dwNumValues;
        }

        ReleaseNullSceRow(sceRow);
        hr = SceGetNextRow(pcdb->psceDb, PRODUCT_INDEX_TABLE, &sceRow);
    }

    if (E_NOTFOUND == hr)
    {
        hr = S_OK;
    }

    if (NULL != pcCount)
    {
        *pcCount = pcesEnum->dwNumValues;
    }

    if (0 == pcesEnum->dwNumValues)
    {
        EnumFree(pcesEnum);
        pcesEnum = NULL;
    }
    else if (pcesEnum->dwNumValues < pcesEnum->dwMaxValues)
    {
        // Now that we'll no longer be adding to the struct, free any unneeded space we allocated
        hr = EnumResize(pcesEnum, pcesEnum->dwNumValues);
        ExitOnFailure(hr, "Failed to free unneeded memory from enumeration struct");
    }

    *ppvHandle = (static_cast<CFG_ENUMERATION_HANDLE>(pcesEnum));

LExit:
    ReleaseSceRow(sceRow);
    ReleaseStr(sczLowPublicKey);

    return hr;
}

extern "C" HRESULT CfgEnumPastValues(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzName,
    __deref_opt_out_bcount(CFG_ENUMERATION_HANDLE_BYTES) CFG_ENUMERATION_HANDLE *ppvHandle,
    __out DWORD *pcCount
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzName, hr, E_INVALIDARG, "Value name must not be NULL");

    hr = EnumPastValues(pcdb, wzName, reinterpret_cast<CFG_ENUMERATION **>(ppvHandle), pcCount);
    ExitOnFailure1(hr, "Failed to call internal enumerate past values function on value named: %ls", wzName);

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumDatabaseList(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __deref_opt_out_bcount(CFG_ENUMERATION_HANDLE_BYTES) CFG_ENUMERATION_HANDLE *ppvHandle,
    __out DWORD *pcCount
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");

    hr = EnumDatabaseList(pcdb, reinterpret_cast<CFG_ENUMERATION **>(ppvHandle), pcCount);
    ExitOnFailure(hr, "Failed to call internal enumerate database list function");

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumReadDataType(
    __in_bcount(CFG_ENUMERATION_HANDLE_BYTES) C_CFG_ENUMERATION_HANDLE cehHandle,
    __in DWORD dwIndex,
    __in CFG_ENUM_DATA cedData,
    __out_opt CONFIG_VALUETYPE *pcvType
    )
{
    HRESULT hr = S_OK;
    const CFG_ENUMERATION *pcesEnum = static_cast<const CFG_ENUMERATION *>(cehHandle);

    ExitOnNull(pcesEnum, hr, E_INVALIDARG, "CfgEnumReadDataType() requires an enumeration handle");
    ExitOnNull(cedData, hr, E_INVALIDARG, "CfgEnumReadDataType()'s CFG_ENUM_DATA parameter must not be ENUM_DATA_NULL");
    ExitOnNull(pcvType, hr, E_INVALIDARG, "CfgEnumReadDataType()'s output parameter must not be NULL");

    // Index out of bounds
    if (dwIndex >= pcesEnum->dwNumValues)
    {
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Index %u out of bounds (max value: %u)", dwIndex, pcesEnum->dwNumValues);
    }

    switch (pcesEnum->enumType)
    {
    case ENUMERATION_VALUES:
        switch (cedData)
        {
        case ENUM_DATA_VALUETYPE:
            ExitOnNull(pcvType, hr, E_INVALIDARG, "CfgEnumReadDataType() must not be sent NULL for pdwDword parameter when requesting ENUM_DATA_VALUEDWORD");

            *pcvType = pcesEnum->values.rgcValues[dwIndex].cvType;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for datatype. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    case ENUMERATION_VALUE_HISTORY:
        switch (cedData)
        {
        case ENUM_DATA_VALUETYPE:
            ExitOnNull(pcvType, hr, E_INVALIDARG, "CfgEnumReadDataType() must not be sent NULL for pdwDword parameter when requesting ENUM_DATA_VALUEDWORD");

            *pcvType = pcesEnum->valueHistory.rgcValues[dwIndex].cvType;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for datatype. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Unsupported request for datatype. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;
    }

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumReadString(
    __in_bcount(CFG_ENUMERATION_HANDLE_BYTES) C_CFG_ENUMERATION_HANDLE cehHandle,
    __in DWORD dwIndex,
    __in CFG_ENUM_DATA cedData,
    __deref_opt_out_z LPCWSTR *psczString
    )
{
    HRESULT hr = S_OK;
    const CFG_ENUMERATION *pcesEnum = static_cast<const CFG_ENUMERATION *>(cehHandle);

    ExitOnNull(pcesEnum, hr, E_INVALIDARG, "CfgEnumReadString() requires an enumeration handle");
    ExitOnNull(cedData, hr, E_INVALIDARG, "CfgEnumReadString()'s CFG_ENUM_DATA parameter must not be ENUM_DATA_NULL");
    ExitOnNull(psczString, hr, E_INVALIDARG, "CfgEnumReadString() must not be sent NULL for string output parameter");

    // Index out of bounds
    if (dwIndex >= pcesEnum->dwNumValues)
    {
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Index %u out of bounds (max value: %u)", dwIndex, pcesEnum->dwNumValues);
    }

    switch (pcesEnum->enumType)
    {
    case ENUMERATION_VALUES:
        switch (cedData)
        {
        case ENUM_DATA_VALUENAME:
            *psczString = pcesEnum->values.rgsczName[dwIndex];
            break;

        case ENUM_DATA_VALUESTRING:
            *psczString = pcesEnum->values.rgcValues[dwIndex].string.sczValue;
            break;

        case ENUM_DATA_BY:
            *psczString = pcesEnum->valueHistory.rgcValues[dwIndex].sczBy;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for string. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    case ENUMERATION_PRODUCTS:
        switch (cedData)
        {
        case ENUM_DATA_PRODUCTNAME:
            *psczString = pcesEnum->products.rgsczName[dwIndex];
            break;

        case ENUM_DATA_VERSION:
            *psczString = pcesEnum->products.rgsczVersion[dwIndex];
            break;

        case ENUM_DATA_PUBLICKEY:
            *psczString = pcesEnum->products.rgsczPublicKey[dwIndex];
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for string. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }

        break;

    case ENUMERATION_VALUE_HISTORY:
        switch (cedData)
        {
        case ENUM_DATA_VALUENAME:
            *psczString = pcesEnum->valueHistory.sczName;
            break;

        case ENUM_DATA_VALUESTRING:
            *psczString = pcesEnum->valueHistory.rgcValues[dwIndex].string.sczValue;
            break;

        case ENUM_DATA_BY:
            *psczString = pcesEnum->valueHistory.rgcValues[dwIndex].sczBy;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for string. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        
        break;

    case ENUMERATION_DATABASE_LIST:
        switch (cedData)
        {
        case ENUM_DATA_FRIENDLY_NAME:
            *psczString = pcesEnum->databaseList.rgsczFriendlyName[dwIndex];
            break;

        case ENUM_DATA_PATH:
            *psczString = pcesEnum->databaseList.rgsczPath[dwIndex];
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for string. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    default:
        hr = E_INVALIDARG;
          ExitOnFailure2(hr, "Unsupported request for string. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;
    }

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumReadDword(
    __in_bcount(CFG_ENUMERATION_HANDLE_BYTES) C_CFG_ENUMERATION_HANDLE cehHandle,
    __in DWORD dwIndex,
    __in CFG_ENUM_DATA cedData,
    __out_opt DWORD *pdwDword
    )
{
    HRESULT hr = S_OK;
    const CFG_ENUMERATION *pcesEnum = static_cast<const CFG_ENUMERATION *>(cehHandle);

    ExitOnNull(pcesEnum, hr, E_INVALIDARG, "CfgEnumReadDword() requires an enumeration handle");
    ExitOnNull(cedData, hr, E_INVALIDARG, "CfgEnumReadDword()'s CFG_ENUM_DATA parameter must not be ENUM_DATA_NULL");
    ExitOnNull(pdwDword, hr, E_INVALIDARG, "CfgEnumReadDword() must not be sent NULL for pdwDword parameter");

    if (ENUM_DATA_COUNT == cedData)
    {
        *pdwDword = *(reinterpret_cast<const DWORD *>(&(pcesEnum->dwNumValues)));
        ExitFunction1(hr = S_OK);
    }

    // Index out of bounds
    if (dwIndex >= pcesEnum->dwNumValues)
    {
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Index %u out of bounds (max value: %u)", dwIndex, pcesEnum->dwNumValues);
    }

    switch (pcesEnum->enumType)
    {
    case ENUMERATION_VALUES:
        switch (cedData)
        {
        case ENUM_DATA_VALUEDWORD:
            *pdwDword = pcesEnum->values.rgcValues[dwIndex].dword.dwValue;
            break;

        case ENUM_DATA_BLOBSIZE:
            *pdwDword = pcesEnum->values.rgcValues[dwIndex].blob.cbValue;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for dword. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    case ENUMERATION_PRODUCTS:
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Unsupported request for dword. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;

    case ENUMERATION_VALUE_HISTORY:
        switch (cedData)
        {
        case ENUM_DATA_VALUEDWORD:
            *pdwDword = pcesEnum->valueHistory.rgcValues[dwIndex].dword.dwValue;
            break;

        case ENUM_DATA_BLOBSIZE:
            *pdwDword = pcesEnum->valueHistory.rgcValues[dwIndex].blob.cbValue;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for dword. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        
        break;

    default:
        hr = E_INVALIDARG;
          ExitOnFailure2(hr, "Unsupported request for dword. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;
    }

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumReadQword(
    __in_bcount(CFG_ENUMERATION_HANDLE_BYTES) C_CFG_ENUMERATION_HANDLE cehHandle,
    __in DWORD dwIndex,
    __in CFG_ENUM_DATA cedData,
    __out_opt DWORD64 *pqwQword
    )
{
    HRESULT hr = S_OK;
    const CFG_ENUMERATION *pcesEnum = static_cast<const CFG_ENUMERATION *>(cehHandle);

    ExitOnNull(pcesEnum, hr, E_INVALIDARG, "CfgEnumReadQword() requires an enumeration handle");
    ExitOnNull(cedData, hr, E_INVALIDARG, "CfgEnumReadQword()'s CFG_ENUM_DATA parameter must not be ENUM_DATA_NULL");
    ExitOnNull(pqwQword, hr, E_INVALIDARG, "CfgEnumReadQword() must not be sent NULL for pqwQword parameter");

    // Index out of bounds
    if (dwIndex >= pcesEnum->dwNumValues)
    {
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Index %u out of bounds (max value: %u)", dwIndex, pcesEnum->dwNumValues);
    }

    switch (pcesEnum->enumType)
    {
    case ENUMERATION_VALUES:
        switch (cedData)
        {
        case ENUM_DATA_VALUEQWORD:
            *pqwQword = pcesEnum->values.rgcValues[dwIndex].qword.qwValue;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for qword. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    case ENUMERATION_VALUE_HISTORY:
        switch (cedData)
        {
        case ENUM_DATA_VALUEQWORD:
            *pqwQword = pcesEnum->valueHistory.rgcValues[dwIndex].qword.qwValue;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for qword. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Unsupported request for qword. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;
    }

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumReadBool(
    __in_bcount(CFG_ENUMERATION_HANDLE_BYTES) C_CFG_ENUMERATION_HANDLE cehHandle,
    __in DWORD dwIndex,
    __in CFG_ENUM_DATA cedData,
    __out_opt BOOL *pfBool
    )
{
    HRESULT hr = S_OK;
    const CFG_ENUMERATION *pcesEnum = static_cast<const CFG_ENUMERATION *>(cehHandle);

    ExitOnNull(pcesEnum, hr, E_INVALIDARG, "CfgEnumReadBool() requires an enumeration handle");
    ExitOnNull(cedData, hr, E_INVALIDARG, "CfgEnumReadBool()'s CFG_ENUM_DATA parameter must not be ENUM_DATA_NULL");
    ExitOnNull(pfBool, hr, E_INVALIDARG, "CfgEnumReadBool() must not be sent NULL for bool output parameter");

    // Index out of bounds
    if (dwIndex >= pcesEnum->dwNumValues)
    {
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Index %u out of bounds (max value: %u)", dwIndex, pcesEnum->dwNumValues);
    }

    switch (pcesEnum->enumType)
    {
    case ENUMERATION_VALUES:
        switch (cedData)
        {
        case ENUM_DATA_VALUEBOOL:
            *pfBool = pcesEnum->values.rgcValues[dwIndex].boolean.fValue;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for bool. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    case ENUMERATION_PRODUCTS:
        switch (cedData)
        {
        case ENUM_DATA_REGISTERED:
            *pfBool = pcesEnum->products.rgfRegistered[dwIndex];
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for bool. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    case ENUMERATION_VALUE_HISTORY:
        switch (cedData)
        {
        case ENUM_DATA_VALUEBOOL:
            *pfBool = pcesEnum->valueHistory.rgcValues[dwIndex].boolean.fValue;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for bool. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        
        break;

    case ENUMERATION_DATABASE_LIST:
        switch (cedData)
        {
        case ENUM_DATA_SYNC_BY_DEFAULT:
            *pfBool = pcesEnum->databaseList.rgfSyncByDefault[dwIndex];
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for bool. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }

        break;

    default:
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Unsupported request for bool. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;
    }

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumReadHash(
    __in_bcount(CFG_ENUMERATION_HANDLE_BYTES) C_CFG_ENUMERATION_HANDLE cehHandle,
    __in DWORD dwIndex,
    __in CFG_ENUM_DATA cedData,
    __deref_out_bcount(CFG_ENUM_HASH_LEN) BYTE** ppbBuffer
    )
{
    HRESULT hr = S_OK;
    const CFG_ENUMERATION *pcesEnum = static_cast<const CFG_ENUMERATION *>(cehHandle);

    ExitOnNull(pcesEnum, hr, E_INVALIDARG, "CfgEnumReadHash() requires an enumeration handle");
    ExitOnNull(cedData, hr, E_INVALIDARG, "CfgEnumReadHash()'s CFG_ENUM_DATA parameter must not be ENUM_DATA_NULL");
    ExitOnNull(ppbBuffer, hr, E_INVALIDARG, "CfgEnumReadHash() must not be sent NULL for BYTE * output parameter");

    // Index out of bounds
    if (dwIndex >= pcesEnum->dwNumValues)
    {
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Index %u out of bounds (max value: %u)", dwIndex, pcesEnum->dwNumValues);
    }

    switch (pcesEnum->enumType)
    {
    case ENUMERATION_VALUES:
        switch (cedData)
        {
        case ENUM_DATA_BLOBHASH:
            *ppbBuffer = pcesEnum->values.rgcValues[dwIndex].blob.rgbHash;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for hash. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    case ENUMERATION_VALUE_HISTORY:
        switch (cedData)
        {
        case ENUM_DATA_BLOBHASH:
            *ppbBuffer = pcesEnum->valueHistory.rgcValues[dwIndex].blob.rgbHash;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for hash. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Unsupported request for hash. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;
    }

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumReadSystemTime(
    __in_bcount(CFG_ENUMERATION_HANDLE_BYTES) C_CFG_ENUMERATION_HANDLE cehHandle,
    __in DWORD dwIndex,
    __in CFG_ENUM_DATA cedData,
    __out SYSTEMTIME *pst
    )
{
    HRESULT hr = S_OK;
    const CFG_ENUMERATION *pcesEnum = static_cast<const CFG_ENUMERATION *>(cehHandle);

    ExitOnNull(pcesEnum, hr, E_INVALIDARG, "CfgEnumReadSystemTime() requires an enumeration handle");
    ExitOnNull(cedData, hr, E_INVALIDARG, "CfgEnumReadSystemTime()'s CFG_ENUM_DATA parameter must not be ENUM_DATA_NULL");
    ExitOnNull(pst, hr, E_INVALIDARG, "CfgEnumReadSystemTime() must not be sent NULL for pst parameter");

    // Index out of bounds
    if (dwIndex >= pcesEnum->dwNumValues)
    {
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Index %u out of bounds (max value: %u)", dwIndex, pcesEnum->dwNumValues);
    }

    switch (pcesEnum->enumType)
    {
    case ENUMERATION_VALUES:
        switch (cedData)
        {
        case ENUM_DATA_WHEN:
            *pst = pcesEnum->valueHistory.rgcValues[dwIndex].stWhen;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for systemtime. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        
        break;

    case ENUMERATION_VALUE_HISTORY:
        switch (cedData)
        {
        case ENUM_DATA_WHEN:
            *pst = pcesEnum->valueHistory.rgcValues[dwIndex].stWhen;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for systemtime. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Unsupported request for systemtime. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;
    }

LExit:
    return hr;
}

extern "C" HRESULT CFGAPI CfgEnumReadBinary(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_bcount(CFG_ENUMERATION_HANDLE_BYTES) C_CFG_ENUMERATION_HANDLE cehHandle,
    __in DWORD dwIndex,
    __in CFG_ENUM_DATA cedData,
    __deref_out_bcount(*piBuffer) BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    const CFG_ENUMERATION *pcesEnum = static_cast<const CFG_ENUMERATION *>(cehHandle);

    ExitOnNull(pcdb, hr, E_INVALIDARG, "CfgEnumReadBinary() requires a database handle");
    ExitOnNull(pcesEnum, hr, E_INVALIDARG, "CfgEnumReadBinary() requires an enumeration handle");
    ExitOnNull(cedData, hr, E_INVALIDARG, "CfgEnumReadBinary()'s CFG_ENUM_DATA parameter must not be ENUM_DATA_NULL");
    ExitOnNull(ppbBuffer, hr, E_INVALIDARG, "CfgEnumReadBinary() must not be sent NULL for BYTE * output parameter");
    ExitOnNull(piBuffer, hr, E_INVALIDARG, "CfgEnumReadBinary() must not be sent NULL for SIZE_T output parameter");

    // Index out of bounds
    if (dwIndex >= pcesEnum->dwNumValues)
    {
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Index %u out of bounds (max value: %u)", dwIndex, pcesEnum->dwNumValues);
    }

    switch (pcesEnum->enumType)
    {
    case ENUMERATION_VALUES:
        switch (cedData)
        {
        case ENUM_DATA_BLOBCONTENT:
            hr = StreamRead(pcdb, pcesEnum->values.rgcValues[dwIndex].blob.dbstream.dwContentID, NULL, ppbBuffer, piBuffer);
            ExitOnFailure1(hr, "Failed to get blob content with ID: %u", pcesEnum->values.rgcValues[dwIndex].blob.dbstream.dwContentID);
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for blob. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }
        break;

    case ENUMERATION_VALUE_HISTORY:
        switch (cedData)
        {
        case ENUM_DATA_BLOBCONTENT:
            hr = StreamRead(pcdb, pcesEnum->valueHistory.rgcValues[dwIndex].blob.dbstream.dwContentID, NULL, ppbBuffer, piBuffer);
            ExitOnFailure1(hr, "Failed to get binary content with ID: %u", pcesEnum->valueHistory.rgcValues[dwIndex].blob.dbstream.dwContentID);
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure2(hr, "Unsupported request for blob. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
            break;
        }

        break;

    default:
        hr = E_INVALIDARG;
        ExitOnFailure2(hr, "Unsupported request for blob. Enumeration type: %d, request type: %d", pcesEnum->enumType, cedData);
        break;
    }

LExit:
    return hr;
}

extern "C" void CfgReleaseEnumeration(
    __in_bcount_opt(CFG_ENUMERATION_HANDLE_BYTES) CFG_ENUMERATION_HANDLE cehHandle
    )
{
    CFG_ENUMERATION *pcesEnum = static_cast<CFG_ENUMERATION *>(cehHandle);

    EnumFree(pcesEnum);
}

extern "C" HRESULT CfgSync(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __deref_out_ecount_opt(*pcProduct) CONFLICT_PRODUCT **prgcpProduct,
    __out DWORD *pcProduct
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    BOOL fRevertProducts = FALSE;
    DWORD dwOriginalAppIDLocal = 0;
    DWORD dwOriginalAppIDRemote = 0;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "CfgSync cannot be sent NULL for its first parameter");
    ExitOnNull(pcdb->pcdbLocal, hr, E_INVALIDARG, "CfgSync must be sent a remote database for its first parameter");
    ExitOnNull(prgcpProduct, hr, E_INVALIDARG, "CfgSync cannot be sent NULL for its second parameter");
    ExitOnNull(pcProduct, hr, E_INVALIDARG, "CfgSync cannot be sent NULL for its third parameter");

    if (*prgcpProduct != NULL)
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, "Must release conflict array, or resolve conflicts before syncing again");
    }

    fRevertProducts = TRUE;
    dwOriginalAppIDLocal = pcdb->pcdbLocal->dwAppID;
    dwOriginalAppIDRemote = pcdb->dwAppID;

    hr = UtilSyncAllProducts(pcdb, prgcpProduct, pcProduct);
    ExitOnFailure(hr, "Failed to sync with remote database");

LExit:
    // Restore the previous AppIDs that were set before syncing began
    if (fRevertProducts)
    {
        if (NULL != pcdb)
        {
            pcdb->dwAppID = dwOriginalAppIDRemote;
            if (NULL != pcdb->pcdbLocal)
            {
                pcdb->pcdbLocal->dwAppID = dwOriginalAppIDLocal;
            }
        }
    }

    return hr;
}

extern "C" void CfgReleaseConflictProductArray(
    __in_ecount_opt(cProduct) CONFLICT_PRODUCT *rgcpProduct,
    __in DWORD cProduct
    )
{
    DWORD i;

    if (NULL == rgcpProduct)
    {
        return;
    }

    for (i = 0; i < cProduct; ++i)
    {
        ReleaseStr(rgcpProduct[i].sczProductName);
        ReleaseStr(rgcpProduct[i].sczVersion);
        ReleaseStr(rgcpProduct[i].sczPublicKey);

        for (DWORD j = 0; j < rgcpProduct[i].cValues; ++j)
        {
            CfgReleaseEnumeration(rgcpProduct[i].rgcesValueEnumLocal[j]);
            CfgReleaseEnumeration(rgcpProduct[i].rgcesValueEnumRemote[j]);
        }

        ReleaseMem(rgcpProduct[i].rgcesValueEnumLocal);
        ReleaseMem(rgcpProduct[i].rgdwValueCountLocal);

        ReleaseMem(rgcpProduct[i].rgcesValueEnumRemote);
        ReleaseMem(rgcpProduct[i].rgdwValueCountRemote);

        ReleaseMem(rgcpProduct[i].rgrcValueChoices);
    }

    ReleaseMem(rgcpProduct);
}
    
extern "C" HRESULT CfgResolve(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_ecount(cProductCount) CONFLICT_PRODUCT *rgcpProduct,
    __in DWORD cProductCount
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LEGACY_SYNC_SESSION syncSession = { };
    DWORD dwProductIndex;
    DWORD dwValueIndex;
    BOOL fRevertProducts = FALSE;
    DWORD dwOriginalAppIDLocal = 0;
    DWORD dwOriginalAppIDRemote = 0;
    BOOL fLegacy = FALSE;

    // Check for invalid args
    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(pcdb->pcdbLocal, hr, E_INVALIDARG, "CfgResolve must be sent a remote database for its first parameter");
    ExitOnNull(rgcpProduct, hr, E_INVALIDARG, "CfgResolve cannot be sent NULL for its parameter");

    fRevertProducts = TRUE;
    dwOriginalAppIDLocal = pcdb->pcdbLocal->dwAppID;
    dwOriginalAppIDRemote = pcdb->dwAppID;

    hr = LegacySyncInitializeSession(TRUE, &syncSession);
    ExitOnFailure(hr, "Failed to initialize legacy sync session");

    for (dwProductIndex = 0; dwProductIndex < cProductCount; ++dwProductIndex)
    {
        // TODO: error out if the value changed since last sync
        hr = ProductSet(pcdb->pcdbLocal, rgcpProduct[dwProductIndex].sczProductName, rgcpProduct[dwProductIndex].sczVersion, rgcpProduct[dwProductIndex].sczPublicKey, TRUE, NULL);
        ExitOnFailure(hr, "Failed to set appropriate product in local DB");

        hr = ProductSet(pcdb, rgcpProduct[dwProductIndex].sczProductName, rgcpProduct[dwProductIndex].sczVersion, rgcpProduct[dwProductIndex].sczPublicKey, TRUE, &fLegacy);
        ExitOnFailure(hr, "Failed to set appropriate product in remote DB");

        if (fLegacy)
        {
            hr = LegacySyncSetProduct(pcdb->pcdbLocal, &syncSession, rgcpProduct[dwProductIndex].sczProductName);
            ExitOnFailure1(hr, "Failed to set legacy product: ", rgcpProduct[dwProductIndex].sczProductName);
        }

        for (dwValueIndex = 0; dwValueIndex < rgcpProduct[dwProductIndex].cValues; ++dwValueIndex)
        {
            hr = ConflictResolve(pcdb, &(rgcpProduct[dwProductIndex]), dwValueIndex);
            ExitOnFailure(hr, "Failed to resolve legacy value");
        }

        if (fLegacy)
        {
            hr = LegacySyncFinalizeProduct(pcdb->pcdbLocal, &syncSession);
            ExitOnFailure(hr, "Failed to finalize legacy product - this may have the effect that some resolved conflicts will appear again.");
        }
    }

LExit:
    LegacySyncUninitializeSession(pcdb->pcdbLocal, &syncSession);

    // Restore the previous AppIDs that were set before resolving began
    if (fRevertProducts)
    {
        if (NULL != pcdb)
        {
            pcdb->dwAppID = dwOriginalAppIDRemote;
            if (NULL != pcdb->pcdbLocal)
            {
                pcdb->pcdbLocal->dwAppID = dwOriginalAppIDLocal;
            }
        }
    }

    return hr;
}

extern "C" HRESULT CFGAPI CfgRegisterProduct(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzProductName,
    __in_z LPCWSTR wzVersion,
    __in_z LPCWSTR wzPublicKey
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    SCE_ROW_HANDLE sceRow = NULL;
    BOOL fInSceTransaction = FALSE;
    BOOL fRegistered = FALSE;
    LPWSTR sczLowPublicKey = NULL;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Must pass in database handle to CfgRegisterProduct()");
    ExitOnNull(wzProductName, hr, E_INVALIDARG, "Product Name must not be NULL");
    ExitOnNull(wzVersion, hr, E_INVALIDARG, "Version must not be NULL");
    ExitOnNull(wzPublicKey, hr, E_INVALIDARG, "Public Key must not be NULL");

    hr = StrAllocString(&sczLowPublicKey, wzPublicKey, 0);
    ExitOnFailure(hr, "Failed to allocate 2nd public key buffer");

    // Convert to lower case characters
    StrStringToLower(sczLowPublicKey);

    hr = ProductValidateName(wzProductName);
    ExitOnFailure1(hr, "Failed to validate ProductName: %ls", wzProductName);

    hr = ProductValidateVersion(wzVersion);
    ExitOnFailure1(hr, "Failed to validate Version: %ls", wzVersion);

    hr = ProductValidatePublicKey(sczLowPublicKey);
    ExitOnFailure1(hr, "Failed to validate Public Key: %ls", sczLowPublicKey);

    hr = ProductFindRow(pcdb, PRODUCT_INDEX_TABLE, wzProductName, wzVersion, sczLowPublicKey, &sceRow);
    if (E_NOTFOUND == hr)
    {
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, wzPublicKey, -1, wzLegacyPublicKey, -1))
        {
            hr = E_NOTIMPL;
            ExitOnFailure(hr, "Cannot register legacy product for which we have no legacy manifest!");
        }

        hr = SceBeginTransaction(pcdb->psceDb);
        ExitOnFailure(hr, "Failed to begin transaction");
        fInSceTransaction = TRUE;

        hr = ScePrepareInsert(pcdb->psceDb, PRODUCT_INDEX_TABLE, &sceRow);
        ExitOnFailure(hr, "Failed to prepare for insert");

        hr = SceSetColumnString(sceRow, PRODUCT_NAME, wzProductName);
        ExitOnFailure(hr, "Failed to set product name column");

        hr = SceSetColumnString(sceRow, PRODUCT_VERSION, wzVersion);
        ExitOnFailure(hr, "Failed to set version column");

        hr = SceSetColumnString(sceRow, PRODUCT_PUBLICKEY, sczLowPublicKey);
        ExitOnFailure(hr, "Failed to set publickey column");

        hr = SceSetColumnBool(sceRow, PRODUCT_REGISTERED, TRUE);
        ExitOnFailure(hr, "Failed to set registered column");

        hr = SceSetColumnBool(sceRow, PRODUCT_IS_LEGACY, FALSE);
        ExitOnFailure(hr, "Failed to set registered column");

        hr = SceFinishUpdate(sceRow);
        ExitOnFailure(hr, "Failed to finish update");

        hr = SceCommitTransaction(pcdb->psceDb);
        ExitOnFailure(hr, "Failed to commit transaction");
        fInSceTransaction = FALSE;
    }
    else
    {
        ExitOnFailure(hr, "Failed to query for product");

        hr = SceGetColumnBool(sceRow, PRODUCT_REGISTERED, &fRegistered);
        ExitOnFailure(hr, "Failed to check if product is already registered");

        if (!fRegistered)
        {
            hr = SceBeginTransaction(pcdb->psceDb);
            ExitOnFailure(hr, "Failed to begin transaction");
            fInSceTransaction = TRUE;

            hr = SceSetColumnBool(sceRow, PRODUCT_REGISTERED, TRUE);
            ExitOnFailure(hr, "Failed to set registered flag to true");

            hr = SceFinishUpdate(sceRow);
            ExitOnFailure(hr, "Failed to finish update into summary data table");

            hr = SceCommitTransaction(pcdb->psceDb);
            ExitOnFailure(hr, "Failed to commit transaction");
            fInSceTransaction = FALSE;
        }
    }

LExit:
    ReleaseSceRow(sceRow);
    if (fInSceTransaction)
    {
        SceRollbackTransaction(pcdb->psceDb);
    }
    ReleaseStr(sczLowPublicKey);

    return hr;
}

extern "C" HRESULT CFGAPI CfgUnregisterProduct(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzProductName,
    __in_z LPCWSTR wzVersion,
    __in_z LPCWSTR wzPublicKey
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    SCE_ROW_HANDLE sceRow = NULL;
    BOOL fInSceTransaction = FALSE;
    BOOL fRegistered = FALSE;
    LPWSTR sczLowPublicKey = NULL;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Must pass in database handle to CfgUnregisterProduct()");
    ExitOnNull(wzProductName, hr, E_INVALIDARG, "Product Name must not be NULL");
    ExitOnNull(wzVersion, hr, E_INVALIDARG, "Version must not be NULL");
    ExitOnNull(wzPublicKey, hr, E_INVALIDARG, "Public Key must not be NULL");

    hr = StrAllocString(&sczLowPublicKey, wzPublicKey, 0);
    ExitOnFailure(hr, "Failed to allocate 2nd public key buffer");

    // Convert to lower case characters
    StrStringToLower(sczLowPublicKey);

    hr = ProductValidateName(wzProductName);
    ExitOnFailure1(hr, "Failed to validate ProductName: %ls", wzProductName);

    hr = ProductValidateVersion(wzVersion);
    ExitOnFailure1(hr, "Failed to validate Version: %ls", wzVersion);

    hr = ProductValidatePublicKey(sczLowPublicKey);
    ExitOnFailure1(hr, "Failed to validate Public Key: %ls", sczLowPublicKey);

    hr = ProductFindRow(pcdb, PRODUCT_INDEX_TABLE, wzProductName, wzVersion, sczLowPublicKey, &sceRow);
    if (E_NOTFOUND == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    else
    {
        ExitOnFailure(hr, "Failed to query for product");

        hr = SceGetColumnBool(sceRow, PRODUCT_REGISTERED, &fRegistered);
        ExitOnFailure(hr, "Failed to check if product is already installed");

        if (fRegistered)
        {
            hr = SceBeginTransaction(pcdb->psceDb);
            ExitOnFailure(hr, "Failed to begin transaction");
            fInSceTransaction = TRUE;

            hr = SceSetColumnBool(sceRow, PRODUCT_REGISTERED, FALSE);
            ExitOnFailure(hr, "Failed to set installed flag to true");

            hr = SceFinishUpdate(sceRow);
            ExitOnFailure(hr, "Failed to finish update");

            hr = SceCommitTransaction(pcdb->psceDb);
            ExitOnFailure(hr, "Failed to commit transaction");
            fInSceTransaction = FALSE;
        }
    }

LExit:
    ReleaseSceRow(sceRow);
    if (fInSceTransaction)
    {
        SceRollbackTransaction(pcdb->psceDb);
    }
    ReleaseStr(sczLowPublicKey);

    return hr;
}

extern "C" HRESULT CFGAPI CfgIsProductRegistered(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzProductName,
    __in_z LPCWSTR wzVersion,
    __in_z LPCWSTR wzPublicKey,
    __out BOOL *pfRegistered
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    SCE_ROW_HANDLE sceRow = NULL;
    BOOL fInSceTransaction = FALSE;
    BOOL fRegistered = FALSE;
    LPWSTR sczLowPublicKey = NULL;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Must pass in database handle to CfgUnregisterProduct()");
    ExitOnNull(wzProductName, hr, E_INVALIDARG, "Product Name must not be NULL");
    ExitOnNull(wzVersion, hr, E_INVALIDARG, "Version must not be NULL");
    ExitOnNull(wzPublicKey, hr, E_INVALIDARG, "Public Key must not be NULL");

    hr = StrAllocString(&sczLowPublicKey, wzPublicKey, 0);
    ExitOnFailure(hr, "Failed to allocate 2nd public key buffer");

    // Convert to lower case characters
    StrStringToLower(sczLowPublicKey);

    hr = ProductValidateName(wzProductName);
    ExitOnFailure1(hr, "Failed to validate ProductName: %ls", wzProductName);

    hr = ProductValidateVersion(wzVersion);
    ExitOnFailure1(hr, "Failed to validate Version: %ls", wzVersion);

    hr = ProductValidatePublicKey(sczLowPublicKey);
    ExitOnFailure1(hr, "Failed to validate Public Key: %ls", sczLowPublicKey);

    hr = ProductFindRow(pcdb, PRODUCT_INDEX_TABLE, wzProductName, wzVersion, sczLowPublicKey, &sceRow);
    if (E_NOTFOUND == hr)
    {
        *pfRegistered = FALSE;
        ExitFunction1(hr = S_OK);
    }
    else
    {
        ExitOnFailure(hr, "Failed to query for product");

        hr = SceGetColumnBool(sceRow, PRODUCT_REGISTERED, &fRegistered);
        ExitOnFailure(hr, "Failed to check if product is already installed");

        *pfRegistered = fRegistered;
        ExitFunction1(hr = S_OK);
    }

LExit:
    ReleaseSceRow(sceRow);
    if (fInSceTransaction)
    {
        SceRollbackTransaction(pcdb->psceDb);
    }
    ReleaseStr(sczLowPublicKey);

    return hr;
}

HRESULT CFGAPI CfgForgetProduct(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzProductName,
    __in_z LPCWSTR wzVersion,
    __in_z LPCWSTR wzPublicKey
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Database handle must not be NULL");
    ExitOnNull(wzProductName, hr, E_INVALIDARG, "Product Name must not be NULL");
    ExitOnNull(wzVersion, hr, E_INVALIDARG, "Version must not be NULL");
    ExitOnNull(wzPublicKey, hr, E_INVALIDARG, "Public Key must not be NULL for non-legacy databases");

    // Immediately unset the previously set product in case we delete the currently set product
    pcdb->fProductSet = FALSE;

    hr = ProductForget(pcdb, wzProductName, wzVersion, wzPublicKey);
    ExitOnFailure(hr, "Failed to forget about product");

LExit:
    return hr;
}

static HRESULT InitializeImpersonationToken(
    __inout CFGDB_STRUCT *pcdb
    )
{
    HRESULT hr = S_OK;
    BOOL fImpersonating = FALSE;

    if (!ImpersonateSelf(SecurityImpersonation))
    {
        ExitWithLastError(hr, "Failed to impersonate self for access check");
    }
    fImpersonating = TRUE;

    if (!::OpenThreadToken(::GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &pcdb->hToken))
    {
        ExitOnLastError(hr, "Failed to open thread token.");
    }

LExit:
    if (fImpersonating)
    {
        RevertToSelf();
        fImpersonating = FALSE;
    }

    return hr;
}

