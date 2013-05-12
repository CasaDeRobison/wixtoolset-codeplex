//-------------------------------------------------------------------------------------------------
// <copyright file="cfgadmin.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
// Core settings engine API (functions for apps related to global state)
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

volatile static DWORD s_dwRefCount = 0;
static CFGDB_STRUCT s_cdb = { };
static BOOL s_fAdminAccess = FALSE;

// If the database doesn't exist and we don't have permission to create it, instead of failing - this flag will be set to true
// This allows anything that just queries the admin database to succeed
// (the result will properly be that nothing is found, because nothing exists in an empty database)
static BOOL s_fDatabaseMissing = FALSE;

extern "C" HRESULT CfgAdminInitialize(
    __out CFGDB_HANDLE *pcdHandle,
    __in BOOL fDemandAdmin
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDbDir = NULL;
    LPWSTR sczDbFilePath = NULL;
    CFGDB_STRUCT *pcdb;

    ExitOnNull(pcdHandle, hr, E_INVALIDARG, "Database handle output pointer must not be NULL");

    *pcdHandle = &s_cdb;

    ::InterlockedIncrement(&s_dwRefCount);

    // TODO: the "fDemandAdmin" setting from the first call is dominant, but what if
    // the first consumers requests false, and the second requests true? We could support that, if there's a need

    if (1 == s_dwRefCount)
    {
        s_fAdminAccess = fDemandAdmin;

        ZeroMemory(&s_cdb, sizeof(s_cdb));
        pcdb = &s_cdb;
        pcdb->dwAppID = DWORD_MAX;

        hr = DatabaseGetAdminDir(&pcdb->sczDbDir);
        ExitOnFailure(hr, "Failed to get admin database directory");

        // Setup expected schema in memory
        hr = DatabaseSetupAdminSchema(&pcdb->dsSceDb);
        ExitOnFailure(hr, "Failed to setup admin database schema structure in memory");

        // Get the path to the exact file
        hr = DatabaseGetAdminPath(&sczDbFilePath);
        ExitOnFailure(hr, "Failed to get admin database path");

        // Create or open the database
        if (fDemandAdmin)
        {
            // If we were told to create it, make sure it's there
            hr = SceEnsureDatabase(sczDbFilePath, wzSqlCeDllPath, L"CfgAdmin", 1, &pcdb->dsSceDb, &pcdb->psceDb);
            ExitOnFailure(hr, "Failed to create SQL CE database");
        }
        else
        {
            if (FileExistsEx(sczDbFilePath, NULL))
            {
                hr = SceOpenDatabase(sczDbFilePath, wzSqlCeDllPath, L"CfgAdmin", 1, &pcdb->psceDb, TRUE);
                ExitOnFailure(hr, "Failed to open database");

                // Associate the schema with the database
                pcdb->psceDb->pdsSchema = &pcdb->dsSceDb;
            }
            else
            {
                s_fDatabaseMissing = TRUE;
            }
        }
    }

LExit:
    ReleaseStr(sczDbDir);
    ReleaseStr(sczDbFilePath);

    return hr;
}

extern "C" HRESULT CfgAdminUninitialize(
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
        ReleaseStr(pcdb->sczGuid);

        DatabaseReleaseSceSchema(&pcdb->dsSceDb);

        if (!s_fDatabaseMissing)
        {
            hr = SceCloseDatabase(pcdb->psceDb);
            ExitOnFailure(hr, "Failed to close admin database");
        }
        else
        {
            s_fDatabaseMissing = FALSE;
        }
    }

LExit:
    return hr;
}

extern "C" HRESULT CfgAdminRegisterProduct(
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
    LPWSTR sczLowPublicKey = NULL;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Must pass in database handle to CfgAdminRegisterProduct()");
    ExitOnNull(wzProductName, hr, E_INVALIDARG, "Product Name must not be NULL");
    ExitOnNull(wzVersion, hr, E_INVALIDARG, "Version must not be NULL");
    ExitOnNull(wzPublicKey, hr, E_INVALIDARG, "Public Key must not be NULL");

    if (!s_fAdminAccess)
    {
        ExitFunction1(hr = E_ACCESSDENIED);
    }

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

    // First query for an existing value - if it exists, we'll update it in place
    hr = ProductFindRow(pcdb, ADMIN_PRODUCT_INDEX_TABLE, wzProductName, wzVersion, sczLowPublicKey, &sceRow);
    if (E_NOTFOUND == hr)
    {
        hr = SceBeginTransaction(pcdb->psceDb);
        ExitOnFailure(hr, "Failed to begin transaction");
        fInSceTransaction = TRUE;

        hr = ScePrepareInsert(pcdb->psceDb, ADMIN_PRODUCT_INDEX_TABLE, &sceRow);
        ExitOnFailure(hr, "Failed to prepare for insert");

        hr = SceSetColumnString(sceRow, PRODUCT_NAME, wzProductName);
        ExitOnFailure(hr, "Failed to set product name column");

        hr = SceSetColumnString(sceRow, PRODUCT_VERSION, wzVersion);
        ExitOnFailure(hr, "Failed to set version column");

        hr = SceSetColumnString(sceRow, PRODUCT_PUBLICKEY, sczLowPublicKey);
        ExitOnFailure(hr, "Failed to set publickey column");

        hr = SceFinishUpdate(sceRow);
        ExitOnFailure(hr, "Failed to finish update");

        hr = SceCommitTransaction(pcdb->psceDb);
        ExitOnFailure(hr, "Failed to commit transaction");
        fInSceTransaction = FALSE;
    }
    ExitOnFailure(hr, "Failed to query for product");

LExit:
    ReleaseSceRow(sceRow);
    if (fInSceTransaction)
    {
        SceRollbackTransaction(pcdb->psceDb);
    }
    ReleaseStr(sczLowPublicKey);

    return hr;
}

extern "C" HRESULT CfgAdminUnregisterProduct(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzProductName,
    __in_z LPCWSTR wzVersion,
    __in_z LPCWSTR wzPublicKey
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LPWSTR sczLowPublicKey = NULL;
    SCE_ROW_HANDLE sceRow = NULL;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Must pass in database handle to CfgAdminUnregisterProduct()");
    ExitOnNull(wzProductName, hr, E_INVALIDARG, "Product Name must not be NULL");
    ExitOnNull(wzVersion, hr, E_INVALIDARG, "Version must not be NULL");
    ExitOnNull(wzPublicKey, hr, E_INVALIDARG, "Public Key must not be NULL");

    if (!s_fAdminAccess)
    {
        ExitFunction1(hr = E_ACCESSDENIED);
    }

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

    // First query for an existing value - if it exists, we'll update it in place
    hr = ProductFindRow(pcdb, ADMIN_PRODUCT_INDEX_TABLE, wzProductName, wzVersion, sczLowPublicKey, &sceRow);
    if (E_NOTFOUND != hr) // If we actually found it, delete it
    {
        hr = SceDeleteRow(&sceRow);
        ExitOnFailure(hr, "Failed to delete row");
    }
    else
    {
        hr = S_OK; // if it wasn't found, we just have no work to do. Return success.
    }

LExit:
    ReleaseSceRow(sceRow);
    ReleaseStr(sczLowPublicKey);

    return hr;
}

extern "C" HRESULT CfgAdminIsProductRegistered(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z LPCWSTR wzProductName,
    __in_z LPCWSTR wzVersion,
    __in_z LPCWSTR wzPublicKey,
    __out BOOL *pfRegistered
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    LPWSTR sczLowPublicKey = NULL;
    SCE_ROW_HANDLE sceRow = NULL;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Must pass in database handle to CfgAdminIsProductRegistered()");
    ExitOnNull(wzProductName, hr, E_INVALIDARG, "Must pass in productname to CfgAdminIsProductRegistered()");
    ExitOnNull(wzVersion, hr, E_INVALIDARG, "Must pass in productversion to CfgAdminIsProductRegistered()");
    ExitOnNull(pfRegistered, hr, E_INVALIDARG, "Must pass in output BOOL * to CfgAdminIsProductRegistered()");

    if (0 == s_dwRefCount)
    {
        ExitFunction1(hr = E_INVALIDARG);
    }

    if (s_fDatabaseMissing)
    {
        // TODO: What if the admin database is created after the call to CfgAdminInitialize()? Shouldn't we check for its creation again?
        // If the admin database doesn't exist at all, then the product is obviously not registered. Exit now.
        *pfRegistered = FALSE;
        ExitFunction1(hr = S_OK);
    }

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

    hr = ProductFindRow(pcdb, ADMIN_PRODUCT_INDEX_TABLE, wzProductName, wzVersion, sczLowPublicKey, &sceRow);
    if (E_NOTFOUND == hr)
    {
        *pfRegistered = FALSE;
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure3(hr, "Failed to search for product in admin database with name: '%ls' version '%ls' public key '%ls'", wzProductName, wzVersion, sczLowPublicKey);

    *pfRegistered = TRUE;

LExit:
    ReleaseSceRow(sceRow);
    ReleaseStr(sczLowPublicKey);

    return hr;
}

extern "C" HRESULT CfgAdminEnumerateProducts(
    __in_bcount(CFGDB_HANDLE_BYTES) CFGDB_HANDLE cdHandle,
    __in_z_opt LPCWSTR wzPublicKey,
    __out CFG_ENUMERATION_HANDLE *ppvHandle,
    __out DWORD *rgdwCount
    )
{
    HRESULT hr = S_OK;
    CFGDB_STRUCT *pcdb = static_cast<CFGDB_STRUCT *>(cdHandle);
    BOOL fSkip = FALSE;
    LPWSTR sczCurrentRowPublicKey = NULL;
    LPWSTR sczLowPublicKey = NULL;
    SCE_ROW_HANDLE sceRow = NULL;

    ExitOnNull(pcdb, hr, E_INVALIDARG, "Must pass in database handle to CfgAdminEnumerateProducts()");
    ExitOnNull(ppvHandle, hr, E_INVALIDARG, "Must pass in pointer to output handle to CfgAdminEnumerateProducts()");
    ExitOnNull(rgdwCount, hr, E_INVALIDARG, "Must pass in pointer to count to CfgAdminEnumerateProducts()");

    if (0 == s_dwRefCount)
    {
        ExitFunction1(hr = E_INVALIDARG);
    }

    if (s_fDatabaseMissing)
    {
        // TODO: What if the admin database is created after the call to CfgAdminInitialize()? Shouldn't we check for its creation again?
        // If the admin database doesn't exist at all, then the product is obviously not registered. Exit now.
        *ppvHandle = NULL;
        *rgdwCount = 0;
        ExitFunction1(hr = S_OK);
    }

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

    hr = SceGetFirstRow(pcdb->psceDb, ADMIN_PRODUCT_INDEX_TABLE, &sceRow);
    if (E_NOTFOUND == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to get first row in PRODUCT_INDEX_ADMIN table");

    while (E_NOTFOUND != hr)
    {
        hr = SceGetColumnString(sceRow, PRODUCT_PUBLICKEY, &sczCurrentRowPublicKey);
        ExitOnFailure(hr, "Failed to get current row's public key");

        // If they specified a public key and it doesn't equal the one we're looking at, skip it
        if (NULL != sczLowPublicKey && 0 != lstrcmpW(sczCurrentRowPublicKey, sczLowPublicKey))
        {
            fSkip = TRUE;
        }

        if (!fSkip)
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

            pcesEnum->products.rgsczPublicKey[pcesEnum->dwNumValues] = sczCurrentRowPublicKey;
            sczCurrentRowPublicKey = NULL;

            pcesEnum->products.rgfRegistered[pcesEnum->dwNumValues] = TRUE;

            ++pcesEnum->dwNumValues;
        }

        ReleaseNullSceRow(sceRow);
        hr = SceGetNextRow(pcdb->psceDb, ADMIN_PRODUCT_INDEX_TABLE, &sceRow);
        // If we get an E_NOTFOUND, it isn't an error, just break out of the loop
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
            break;
        }
        ExitOnFailure(hr, "Failed to move to next record");

        fSkip = FALSE;
    }

    if (E_NOTFOUND == hr)
    {
        hr = S_OK;
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
    if (NULL == pcesEnum)
    {
        *rgdwCount = 0;
    }
    else
    {
        *rgdwCount = pcesEnum->dwNumValues;
    }

LExit:
    ReleaseSceRow(sceRow);
    ReleaseStr(sczLowPublicKey);
    ReleaseStr(sczCurrentRowPublicKey);

    return hr;
}
