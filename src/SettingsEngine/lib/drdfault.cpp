//-------------------------------------------------------------------------------------------------
// <copyright file="drdfault.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
// Internal utility functions for Cfg Legacy API (for purposes of default directory handling)
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

HRESULT DirDefaultReadFile(
    __in CFGDB_STRUCT *pcdb,
    __in LEGACY_SYNC_PRODUCT_SESSION *pSyncProductSession,
    __in_z LPCWSTR wzName,
    __in_z LPCWSTR wzFilePath,
    __in_z LPCWSTR wzSubPath
    )
{
    HRESULT hr = S_OK;
    FILETIME ft;
    SYSTEMTIME st;
    LPWSTR sczValueName = NULL;
    SCE_ROW_HANDLE sceValueRow = NULL;
    BOOL fRet = FALSE;
    BOOL fIgnore = FALSE;
    BYTE *pbBuffer = NULL;
    DWORD cbBuffer = 0;
    CONFIG_VALUE cvExistingValue = { };
    CONFIG_VALUE cvNewValue = { };

    hr = MapFileToCfgName(wzName, wzSubPath, &sczValueName);
    ExitOnFailure2(hr, "Failed to get cfg name for file at name %ls, subpath %ls", wzName, wzSubPath);

    hr = FilterCheckValue(&pSyncProductSession->product, sczValueName, &fIgnore);
    ExitOnFailure1(hr, "Failed to check if cfg blob value should be ignored: %ls", sczValueName);

    if (fIgnore)
    {
        ExitFunction1(hr = S_OK);
    }

    hr = FileGetTime(wzFilePath, NULL, NULL, &ft);
    ExitOnFailure1(hr, "failed to get modified time of file : %ls", wzFilePath);

    fRet = FileTimeToSystemTime(&ft, &st);
    if (!fRet)
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Failed to convert file time to system time for file: %ls", wzFilePath);
    }

    // In the case of files susceptible to the "VirtualStore\" directory,
    // we check virtualstore first, then the regular directory, so skip the file
    // if we've already seen it on an earlier pass
    hr = DictKeyExists(pSyncProductSession->shDictValuesSeen, sczValueName);
    if (E_NOTFOUND == hr)
    {
        hr = S_OK;
    }
    else
    {
        ExitOnFailure(hr, "Failed to check if file was already seen before reading");

        ExitFunction1(hr = S_OK);
    }

    hr = DictAddKey(pSyncProductSession->shDictValuesSeen, sczValueName);
    ExitOnFailure1(hr, "Failed to add file to list of files seen: %ls", sczValueName);

    hr = ValueFindRow(pcdb, VALUE_INDEX_TABLE, pcdb->dwAppID, sczValueName, &sceValueRow);
    if (S_OK == hr)
    {
        hr = ValueRead(pcdb, sceValueRow, &cvExistingValue);
        ExitOnFailure(hr, "Failed to get existing file in database's timestamp when setting file");

        // If the timestamps are identical, don't bother reading the file
        if (0 == UtilCompareSystemTimes(&st, &cvExistingValue.stWhen))
        {
            ExitFunction();
        }
    }

    hr = FileRead(&pbBuffer, &cbBuffer, wzFilePath);
    ExitOnFailure1(hr, "Failed to read file into memory: %ls", wzFilePath);

    hr = ValueSetBlob(pbBuffer, cbBuffer, FALSE, &st, pcdb->sczGuid, &cvNewValue);
    ExitOnFailure1(hr, "Failed to set value in memory for file %ls", sczValueName);

    hr = ValueWrite(pcdb, pcdb->dwAppID, sczValueName, &cvNewValue);
    ExitOnFailure1(hr, "Failed to set blob in cfg database, blob is named: %ls", sczValueName);

LExit:
    ReleaseSceRow(sceValueRow);
    ReleaseStr(sczValueName);
    ReleaseMem(pbBuffer);
    ReleaseCfgValue(cvExistingValue);
    ReleaseCfgValue(cvNewValue);

    return hr;
}

HRESULT DirDefaultWriteFile(
    __in LEGACY_PRODUCT *pProduct,
    __in_z LPCWSTR wzName,
    __in const CONFIG_VALUE *pcvValue,
    __out BOOL *pfHandled
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDir = NULL;
    LPWSTR sczPath = NULL;
    LPWSTR sczVirtualStorePath = NULL;
    SYSTEMTIME stDisk;
    FILETIME ftDisk;
    FILETIME ftCfg;
    BYTE *pbBuffer = NULL;
    DWORD cbBuffer = 0;
    BOOL fRet = FALSE;

    *pfHandled = FALSE;

    hr = MapCfgNameToFile(pProduct, wzName, &sczPath);
    if (E_INVALIDARG == hr)
    {
        // Doesn't map to an actual file, so leave it alone
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure1(hr, "Failed to get path to write for legacy file: %ls", wzName);
    
    *pfHandled = TRUE;

    if (VALUE_BLOB != pcvValue->cvType)
    {
        hr = FileEnsureDelete(sczPath);
        if (E_ACCESSDENIED == hr)
        {
            hr = S_OK;
        }
        ExitOnFailure1(hr, "Failed to delete file: %ls", sczPath);

        // Delete the virtual store file as well
        hr = UtilConvertToVirtualStorePath(sczPath, &sczVirtualStorePath);
        ExitOnFailure1(hr, "Failed to convert to virtualstore path: %ls", sczPath);

        hr = FileEnsureDelete(sczVirtualStorePath);
        ExitOnFailure1(hr, "Falied to delete virtual store path: %ls", sczVirtualStorePath);
        
        ExitFunction1(hr = S_OK);
    }

    hr = FileGetTime(sczPath, NULL, NULL, &ftDisk);
    if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr)
    {
        hr = S_OK;
    }
    else
    {
        // If the file exists, check if it has the same timestamp - if it does, don't write it
        ExitOnFailure1(hr, "Failed to get time of file: %ls", sczPath);

        fRet = ::FileTimeToSystemTime(&ftDisk, &stDisk);
        if (!fRet)
        {
            ExitWithLastError(hr, "Failed to convert disk file time to system time");
        }

        if (0 == UtilCompareSystemTimes(&stDisk, &pcvValue->stWhen))
        {
            // Modified time on disk matches what's in the cfg database - so no need to write the file out to disk again
            ExitFunction1(hr = S_OK);
        }
    }

    if (CFG_BLOB_DB_STREAM != pcvValue->blob.cbType)
    {
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Unexpected blob value type while writing file: %d", pcvValue->blob.cbType);
    }

    hr = StreamRead(pcvValue->blob.dbstream.pcdb, pcvValue->blob.dbstream.dwContentID, NULL, &pbBuffer, &cbBuffer);
    ExitOnFailure1(hr, "Failed to get binary content of ID: %u", pcvValue->blob.dbstream.dwContentID);

    fRet = ::SystemTimeToFileTime(&pcvValue->stWhen, &ftCfg);
    if (!fRet)
    {
        ExitWithLastError(hr, "Failed to convert cfg system time to file time");
    }

    hr = PathGetDirectory(sczPath, &sczDir);
    ExitOnFailure1(hr, "Failed to get directory of file: %ls", sczPath);

    hr = DirEnsureExists(sczDir, NULL);
    if (E_ACCESSDENIED == hr)
    {
        hr = UtilConvertToVirtualStorePath(sczDir, &sczVirtualStorePath);
        ExitOnFailure1(hr, "Failed to convert to virtualstore path: %ls", sczDir);

        hr = DirEnsureExists(sczVirtualStorePath, NULL);
    }
    ExitOnFailure1(hr, "Failed to ensure directory exists: %ls", sczDir);

    hr = FileWrite(sczPath, 0, pbBuffer, cbBuffer, NULL);
    if (E_ACCESSDENIED == hr)
    {
        hr = UtilConvertToVirtualStorePath(sczPath, &sczVirtualStorePath);
        ExitOnFailure1(hr, "Failed to convert to virtualstore path: %ls", sczPath);
        
        hr = FileWrite(sczVirtualStorePath, 0, pbBuffer, cbBuffer, NULL);
        ExitOnFailure1(hr, "Failed to write virtualstore content to file: %ls", sczPath);

        hr = FileSetTime(sczVirtualStorePath, NULL, NULL, &ftCfg);
        ExitOnFailure1(hr, "Failed to set virtualstore timestamp on file: %ls", sczPath);
    }
    else
    {
        ExitOnFailure1(hr, "Failed to write content to file: %ls", sczPath);

        hr = FileSetTime(sczPath, NULL, NULL, &ftCfg);
        ExitOnFailure1(hr, "Failed to set timestamp on file: %ls", sczPath);
    }

LExit:
    ReleaseMem(pbBuffer);
    ReleaseStr(sczDir);
    ReleaseStr(sczPath);
    ReleaseStr(sczVirtualStorePath);

    return hr;
}
