//-------------------------------------------------------------------------------------------------
// <copyright file="manifest.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
// Internal utility functions / structures related to legacy manifests
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

static HRESULT AddLegacyRegistrySpecialCfgValuesToDict(
    __in const LEGACY_REGISTRY_KEY *pRegKey,
    __in const LEGACY_REGISTRY_SPECIAL *pRegKeySpecial,
    __inout STRINGDICT_HANDLE *pshDictCfgValuesSeen
    );
static void ManifestFreeFileIniInfo(
    LEGACY_FILE_INI_INFO *pIniInfo
    );
static void ManifestFreeFileSpecial(
    LEGACY_FILE_SPECIAL *pFileSpecial
    );
static void ManifestFreeFile(
    LEGACY_FILE *pFile
    );
static void ManifestFreeRegistrySpecial(
    LEGACY_REGISTRY_SPECIAL *pRegKeySpecial
    );
static void ManifestFreeRegistryKey(
    LEGACY_REGISTRY_KEY *pRegKey
    );
static void ManifestFreeDetect(
    LEGACY_DETECT *pDetect
    );
static void ManifestFreeFilter(
    LEGACY_VALUE_FILTER *pFilter
    );
static void ManifestFreeAutoSyncProcess(
    LEGACY_AUTOSYNC_PROCESS * pProcess
    );

FILE_ENCODING IniFileEncodingToFileEncoding(
    __in PERSISTED_FILE_ENCODING_TYPE fetEncoding
    )
{
    switch (fetEncoding)
    {
    case PERSISTED_FILE_ENCODING_UNSPECIFIED:
        return FILE_ENCODING_UNSPECIFIED;
        break;
    case PERSISTED_FILE_ENCODING_UTF8:
        return FILE_ENCODING_UTF8;
        break;
    case PERSISTED_FILE_ENCODING_UTF8_WITH_BOM:
        return FILE_ENCODING_UTF8_WITH_BOM;
        break;
    case PERSISTED_FILE_ENCODING_UTF16:
        return FILE_ENCODING_UTF16;
        break;
    case PERSISTED_FILE_ENCODING_UTF16_WITH_BOM:
        return FILE_ENCODING_UTF16_WITH_BOM;
        break;
    default:
        return FILE_ENCODING_UNSPECIFIED;
        break;
    }
}

PERSISTED_FILE_ENCODING_TYPE FileEncodingToIniFileEncoding(
    __in FILE_ENCODING feEncoding
    )
{
    switch (feEncoding)
    {
    case FILE_ENCODING_UNSPECIFIED:
        return PERSISTED_FILE_ENCODING_UNSPECIFIED;
        break;
    case FILE_ENCODING_UTF8:
        return PERSISTED_FILE_ENCODING_UTF8;
        break;
    case FILE_ENCODING_UTF8_WITH_BOM:
        return PERSISTED_FILE_ENCODING_UTF8_WITH_BOM;
        break;
    case FILE_ENCODING_UTF16:
        return PERSISTED_FILE_ENCODING_UTF16;
        break;
    case FILE_ENCODING_UTF16_WITH_BOM:
        return PERSISTED_FILE_ENCODING_UTF16_WITH_BOM;
        break;
    default:
        return PERSISTED_FILE_ENCODING_UNSPECIFIED;
        break;
    }
}

HKEY ManifestConvertToRootKey(
    __in DWORD dwRootEnum
    )
{
    switch (dwRootEnum)
    {
    case CfgLegacyDbRegistryRootCurrentUser:
        return HKEY_CURRENT_USER;

    case CfgLegacyDbRegistryRootLocalMachine:
        return HKEY_LOCAL_MACHINE;

    default:
        return NULL;
    }
}

void ManifestFreeProductStruct(
    __inout LEGACY_PRODUCT *pProduct
    )
{
    ReleaseStr(pProduct->sczProductId);
    ReleaseDict(pProduct->shRegistrySpeciallyHandled);

    for (DWORD i = 0; i < pProduct->detect.cDetects; ++i)
    {
        ManifestFreeDetect(pProduct->detect.rgDetects + i);
    }
    ReleaseMem(pProduct->detect.rgDetects);

    for (DWORD i = 0; i < pProduct->cRegKeys; ++i)
    {
        ManifestFreeRegistryKey(pProduct->rgRegKeys + i);
    }
    ReleaseMem(pProduct->rgRegKeys);

    for (DWORD i = 0; i < pProduct->cFiles; ++i)
    {
        ManifestFreeFile(pProduct->rgFiles + i);
    }
    ReleaseMem(pProduct->rgFiles);

    for (DWORD i = 0; i < pProduct->cDisplayNames; ++i)
    {
        ReleaseStr(pProduct->rgDisplayNames[i].sczName);
    }
    ReleaseMem(pProduct->rgDisplayNames);

    for (DWORD i = 0; i < pProduct->cFilters; ++i)
    {
        ManifestFreeFilter(pProduct->rgFilters + i);
    }
    ReleaseMem(pProduct->rgFilters);

    for (DWORD i = 0; i < pProduct->autoSync.cProcesses; ++i)
    {
        ManifestFreeAutoSyncProcess(pProduct->autoSync.rgProcesses + i);
    }
    ReleaseMem(pProduct->autoSync.rgProcesses);
}

void ManifestFreeFileIniInfo(
    LEGACY_FILE_INI_INFO *pIniInfo
    )
{
    ReleaseStr(pIniInfo->sczNamespace);

    ReleaseStr(pIniInfo->sczSectionPrefix);
    ReleaseStr(pIniInfo->sczSectionPostfix);
    ReleaseStr(pIniInfo->sczValuePrefix);
    ReleaseStr(pIniInfo->sczValueSeparator);

    for (DWORD i = 0; i < pIniInfo->cValueSeparatorException; ++i)
    {
        ReleaseStr(pIniInfo->rgsczValueSeparatorException[i]);
    }
    ReleaseMem(pIniInfo->rgsczValueSeparatorException);

    ReleaseStr(pIniInfo->sczCommentPrefix);
}


void ManifestFreeFileSpecial(
    LEGACY_FILE_SPECIAL *pFileSpecial
    )
{
    ReleaseStr(pFileSpecial->sczLocation);

    for (DWORD i = 0; i < pFileSpecial->cIniInfo; ++i)
    {
        ManifestFreeFileIniInfo(pFileSpecial->rgIniInfo + i);
    }
    ReleaseMem(pFileSpecial->rgIniInfo);
}

void ManifestFreeFile(
    LEGACY_FILE *pFile
    )
{
    ReleaseStr(pFile->sczName);
    ReleaseStr(pFile->sczLocation);
    ReleaseStr(pFile->sczExpandedPath);

    for (DWORD i = 0; i < pFile->cFileSpecials; ++i)
    {
        ManifestFreeFileSpecial(pFile->rgFileSpecials + i);
    }
    ReleaseMem(pFile->rgFileSpecials);
}

void ManifestFreeRegistrySpecial(
    LEGACY_REGISTRY_SPECIAL *pRegKeySpecial
    )
{
    ReleaseStr(pRegKeySpecial->sczRegValueName);

    for (DWORD i = 0; i < pRegKeySpecial->cFlagsInfo; ++i)
    {
        ReleaseStr(pRegKeySpecial->rgFlagsInfo[i].sczCfgValueName);
    }

    ReleaseMem(pRegKeySpecial->rgFlagsInfo);
}

void ManifestFreeRegistryKey(
    LEGACY_REGISTRY_KEY *pRegKey
    )
{
    ReleaseStr(pRegKey->sczKey);
    ReleaseStr(pRegKey->sczNamespace);

    for (DWORD i = 0; i < pRegKey->cRegKeySpecials; ++i)
    {
        ManifestFreeRegistrySpecial(pRegKey->rgRegKeySpecials + i);
    }

    ReleaseMem(pRegKey->rgRegKeySpecials);
}

void ManifestFreeDetect(
    LEGACY_DETECT *pDetect
    )
{
    switch (pDetect->ldtType)
    {
    case LEGACY_DETECT_TYPE_ARP:
        ReleaseStr(pDetect->arp.sczInstallLocationProperty);
        ReleaseStr(pDetect->arp.sczUninstallStringDirProperty);
        ReleaseStr(pDetect->arp.sczDisplayIconDirProperty);
        break;

    case LEGACY_DETECT_TYPE_EXE:
        ReleaseStr(pDetect->exe.sczFileName);
        ReleaseStr(pDetect->exe.sczDetectedFileDir);
        ReleaseStr(pDetect->exe.sczFileDirProperty);
        break;

    case LEGACY_DETECT_TYPE_INVALID:
        // Nothing to free, but don't error, because this one likely was just never initialized
        break;

    default:
        AssertSz(FALSE, "Unexpected legacy detect type found while freeing detect list");
        break;
    }
}

void ManifestFreeFilter(
    LEGACY_VALUE_FILTER *pFilter
    )
{
    ReleaseStr(pFilter->sczExactName);
    ReleaseStr(pFilter->sczPrefix);
    ReleaseStr(pFilter->sczPostfix);
}

static void ManifestFreeAutoSyncProcess(
    LEGACY_AUTOSYNC_PROCESS * pProcess
    )
{
    ReleaseStr(pProcess->sczProcessName);
}
