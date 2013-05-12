//-------------------------------------------------------------------------------------------------
// <copyright file="handle.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Handle to a database
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const LPCWSTR wzCfgProductId = L"WiX_Cfg";
const LPCWSTR wzCfgVersion = L"1.0.0.0";
const LPCWSTR wzCfgPublicKey = L"B77A5C561934E089";

struct LEGACY_SYNC_SESSION;

struct CFGDB_STRUCT
{
    CRITICAL_SECTION cs;

    BOOL fRemote;
    LPWSTR sczDbPath; // The full path to the database file that was opened
    LPWSTR sczDbDir; // The directory this DB was opened or created in
    LPWSTR sczStreamsDir; // The directory where external streams for this database should be stored
    SCE_DATABASE *psceDb;

    BOOL fProductSet;
    BOOL fProductIsLegacy;
    DWORD dwAppID;
    LPWSTR sczProductName;
    
    // The AppID for WiX_Cfg settings
    DWORD dwCfgAppID;

    // The GUID for this endpoint
    LPWSTR sczGuid;

    // This defines our database schema (to instruct SceUtil how to create it)
    SCE_DATABASE_SCHEMA dsSceDb;

    // Local Db
    CFGDB_STRUCT *pcdbLocal;

    // Admin Db
    CFGDB_STRUCT *pcdbAdmin;

    // Impersonation token, used by some legacy database operations
    HANDLE hToken;
};

struct CFG_GLOBAL_STATE
{
    CRITICAL_SECTION cs;

    CFGDB_STRUCT **rgpcdbOpenDatabases;
    DWORD cOpenDatabases;
};

extern CFG_GLOBAL_STATE cfgState;

#ifdef __cplusplus
}
#endif
