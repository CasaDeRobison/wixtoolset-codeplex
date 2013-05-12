//-------------------------------------------------------------------------------------------------
// <copyright file="util.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Internal utility functions for Cfg API
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

struct LEGACY_DIRECTORY_MAP
{
    int nFolder; // CSIDL, i.e. CSIDL_MYDOCUMENTS
    LPCWSTR wzInput; // what appears in the XML, i.e. "MyDocumentsFolder:\"
    LPCWSTR wzAppend;
};

HRESULT UtilSyncAllProducts(
    __in CFGDB_STRUCT *pcdb,
    __out CONFLICT_PRODUCT **prgConflictProducts,
    __out DWORD *pcConflictProducts
    );
HRESULT UtilExpandLegacyPath(
    __in LPCWSTR wzInput,
    __in LEGACY_DETECTION *pDetect,
    __deref_out LPWSTR *psczOutput
    );
int UtilCompareSystemTimes(
    __in const SYSTEMTIME *pst1,
    __in const SYSTEMTIME *pst2
    );
HRESULT UtilTestWriteAccess(
    __in HANDLE hToken,
    __in_z LPCWSTR wzPath
    );
HRESULT UtilConvertToVirtualStorePath(
    __in_z LPCWSTR wzOriginalPath,
    __out LPWSTR *psczOutput
    );

#ifdef __cplusplus
}
#endif
