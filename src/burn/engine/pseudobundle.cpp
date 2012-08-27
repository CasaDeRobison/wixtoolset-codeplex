//-------------------------------------------------------------------------------------------------
// <copyright file="pseudobundle.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


extern "C" HRESULT PseudoBundleInitialize(
    __in DWORD64 qwEngineVersion,
    __in BURN_PACKAGE* pPackage,
    __in BOOL fPerMachine,
    __in_z LPCWSTR wzId,
    __in BOOTSTRAPPER_RELATION_TYPE relationType,
    __in BOOTSTRAPPER_PACKAGE_STATE state,
    __in_z LPCWSTR wzFilePath,
    __in_z LPCWSTR wzLocalSource,
    __in_z_opt LPCWSTR wzDownloadSource,
    __in DWORD64 qwSize,
    __in BOOL fVital,
    __in_z LPCWSTR wzInstallArguments,
    __in_z_opt LPCWSTR wzRepairArguments,
    __in_z_opt LPCWSTR wzUninstallArguments,
    __in_opt BURN_DEPENDENCY_PROVIDER* pDependencyProvider,
    __in_opt BYTE* pbHash,
    __in DWORD cbHash
    )
{
    HRESULT hr = S_OK;
    LPCWSTR wzRelationTypeCommandLine = NULL;

    switch (relationType)
    {
    case BOOTSTRAPPER_RELATION_DETECT:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_DETECT;
        break;
    case BOOTSTRAPPER_RELATION_UPGRADE:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_UPGRADE;
        break;
    case BOOTSTRAPPER_RELATION_ADDON:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_ADDON;
        break;
    case BOOTSTRAPPER_RELATION_PATCH:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_PATCH;
        break;
    case BOOTSTRAPPER_RELATION_UPDATE:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_UPDATE;
        break;
    case BOOTSTRAPPER_RELATION_DEPENDENT:
        break;
    case BOOTSTRAPPER_RELATION_NONE: __fallthrough;
    default:
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        ExitOnFailure1(hr, "Internal error: bad relation type %d", relationType);
        break;
    }

    // Initialize the single payload, and fill out all the necessary fields
    pPackage->rgPayloads = (BURN_PACKAGE_PAYLOAD *)MemAlloc(sizeof(BURN_PACKAGE_PAYLOAD), TRUE); 
    ExitOnNull(pPackage->rgPayloads, hr, E_OUTOFMEMORY, "Failed to allocate space for burn package payload inside of related bundle struct");
    pPackage->cPayloads = 1;

    pPackage->rgPayloads->pPayload = (BURN_PAYLOAD *)MemAlloc(sizeof(BURN_PAYLOAD), TRUE); 
    ExitOnNull(pPackage->rgPayloads, hr, E_OUTOFMEMORY, "Failed to allocate space for burn payload inside of related bundle struct");
    pPackage->rgPayloads->pPayload->packaging = BURN_PAYLOAD_PACKAGING_EXTERNAL;
    pPackage->rgPayloads->pPayload->qwFileSize = qwSize;

    hr = StrAllocString(&pPackage->rgPayloads->pPayload->sczKey, wzId, 0);
    ExitOnFailure(hr, "Failed to copy key for pseudo bundle payload.");

    hr = StrAllocString(&pPackage->rgPayloads->pPayload->sczFilePath, wzFilePath, 0);
    ExitOnFailure(hr, "Failed to copy filename for pseudo bundle.");

    hr = StrAllocString(&pPackage->rgPayloads->pPayload->sczSourcePath, wzLocalSource, 0);
    ExitOnFailure(hr, "Failed to copy local source path for pseudo bundle.");

    if (wzDownloadSource && *wzDownloadSource)
    {
        hr = StrAllocString(&pPackage->rgPayloads->pPayload->downloadSource.sczUrl, wzDownloadSource, 0);
        ExitOnFailure(hr, "Failed to copy download source for pseudo bundle.");
    }

    if (pbHash)
    {
        pPackage->rgPayloads->pPayload->pbHash = static_cast<BYTE*>(MemAlloc(cbHash, FALSE));
        ExitOnNull(pPackage->rgPayloads->pPayload->pbHash, hr, E_OUTOFMEMORY, "Failed to allocate memory for pseudo bundle payload hash.");

        pPackage->rgPayloads->pPayload->cbHash = cbHash;
        memcpy_s(pPackage->rgPayloads->pPayload->pbHash, pPackage->rgPayloads->pPayload->cbHash, pbHash, cbHash);
    }

    pPackage->rgPayloads->fCached = (BOOTSTRAPPER_PACKAGE_STATE_PRESENT == state || BOOTSTRAPPER_PACKAGE_STATE_CACHED == state);

    pPackage->type = BURN_PACKAGE_TYPE_EXE;
    pPackage->fPerMachine = fPerMachine;
    pPackage->currentState = state;
    pPackage->cache = (BOOTSTRAPPER_PACKAGE_STATE_PRESENT == state || BOOTSTRAPPER_PACKAGE_STATE_CACHED == state) ? BURN_CACHE_STATE_COMPLETE : BURN_CACHE_STATE_NONE;
    pPackage->qwInstallSize = qwSize;
    pPackage->qwSize = qwSize;
    pPackage->fVital = fVital;

    hr = StrAllocString(&pPackage->sczId, wzId, 0);
    ExitOnFailure(hr, "Failed to copy key for pseudo bundle.");

    hr = StrAllocString(&pPackage->sczCacheId, wzId, 0);
    ExitOnFailure(hr, "Failed to copy cache id for pseudo bundle.");

    hr = StrAllocFormatted(&pPackage->Exe.sczInstallArguments, L"%ls -%ls", wzInstallArguments, wzRelationTypeCommandLine);
    ExitOnFailure(hr, "Failed to copy install arguments for related bundle package");

    if (wzRepairArguments)
    {
        hr = StrAllocFormatted(&pPackage->Exe.sczRepairArguments, L"%ls -%ls", wzRepairArguments, wzRelationTypeCommandLine);
        ExitOnFailure(hr, "Failed to copy repair arguments for related bundle package");

        pPackage->Exe.fRepairable = TRUE;
    }

    if (wzUninstallArguments)
    {
        hr = StrAllocFormatted(&pPackage->Exe.sczUninstallArguments, L"%ls -%ls", wzUninstallArguments, wzRelationTypeCommandLine);
        ExitOnFailure(hr, "Failed to copy uninstall arguments for related bundle package");

        pPackage->fUninstallable = TRUE;
    }

    // Only support progress from engines that are compatible (aka: version greater than or equal to last protocol breaking change *and* versions that are older or the same as this engine).
    pPackage->Exe.protocol = (FILEMAKEVERSION(3, 6, 2221, 0) <= qwEngineVersion && qwEngineVersion <= FILEMAKEVERSION(rmj, rmm, rup, 0)) ? BURN_EXE_PROTOCOL_TYPE_BURN : BURN_EXE_PROTOCOL_TYPE_NONE;

    if (pDependencyProvider)
    {
        pPackage->rgDependencyProviders = (BURN_DEPENDENCY_PROVIDER*)MemAlloc(sizeof(BURN_DEPENDENCY_PROVIDER), TRUE);
        ExitOnNull(pPackage->rgDependencyProviders, hr, E_OUTOFMEMORY, "Failed to allocate memory for dependency providers.");
        pPackage->cDependencyProviders = 1;

        pPackage->rgDependencyProviders[0].fImported = pDependencyProvider->fImported;

        hr = StrAllocString(&pPackage->rgDependencyProviders[0].sczKey, pDependencyProvider->sczKey, 0);
        ExitOnFailure(hr, "Failed to copy key for pseudo bundle.");

        hr = StrAllocString(&pPackage->rgDependencyProviders[0].sczVersion, pDependencyProvider->sczVersion, 0);
        ExitOnFailure(hr, "Failed to copy version for pseudo bundle.");

        hr = StrAllocString(&pPackage->rgDependencyProviders[0].sczDisplayName, pDependencyProvider->sczDisplayName, 0);
        ExitOnFailure(hr, "Failed to copy display name for pseudo bundle.");
    }

LExit:
    return hr;
}
