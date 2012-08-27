//-------------------------------------------------------------------------------------------------
// <copyright file="detect.cpp" company="Outercurve Foundation">
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

// internal function definitions


// function definitions

extern "C" void DetectReset(
    __in BURN_PACKAGES* pPackages,
    __in BURN_UPDATE* /*pUpdate*/
    )
{
    for (DWORD iPackage = 0; iPackage < pPackages->cPackages; ++iPackage)
    {
        BURN_PACKAGE* pPackage = pPackages->rgPackages + iPackage;

        pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN;

        pPackage->cache = BURN_CACHE_STATE_NONE;
        for (DWORD iPayload = 0; iPayload < pPackage->cPayloads; ++iPayload)
        {
            BURN_PACKAGE_PAYLOAD* pPayload = pPackage->rgPayloads + iPayload;
            pPayload->fCached = FALSE;
        }

        if (BURN_PACKAGE_TYPE_MSI == pPackage->type)
        {
            for (DWORD iFeature = 0; iFeature < pPackage->Msi.cFeatures; ++iFeature)
            {
                BURN_MSIFEATURE* pFeature = pPackage->Msi.rgFeatures + iFeature;

                pFeature->currentState = BOOTSTRAPPER_FEATURE_STATE_UNKNOWN;
            }
        }
        else if (BURN_PACKAGE_TYPE_MSP == pPackage->type)
        {
            ReleaseNullMem(pPackage->Msp.rgTargetProducts);
            pPackage->Msp.cTargetProductCodes = 0;
        }
    }

    for (DWORD iPatchInfo = 0; iPatchInfo < pPackages->cPatchInfo; ++iPatchInfo)
    {
        MSIPATCHSEQUENCEINFOW* pPatchInfo = pPackages->rgPatchInfo + iPatchInfo;
        pPatchInfo->dwOrder = 0;
        pPatchInfo->uStatus = 0;
    }
}

// TODO: this function is an outline for what the future detection of updates by the
//       engine could look like.
extern "C" HRESULT DetectUpdate(
    __in_z LPCWSTR /*wzBundleId*/,
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_UPDATE* pUpdate
    )
{
    HRESULT hr = S_OK;
    int nResult = IDNOACTION;
    BOOL fBeginCalled = FALSE;
    LPWSTR sczUpdateId = NULL;

    // If no update source was specified, skip update detection.
    if (!pUpdate->sczUpdateSource || !*pUpdate->sczUpdateSource)
    {
        ExitFunction();
    }

    fBeginCalled = TRUE;

    nResult = pUX->pUserExperience->OnDetectUpdateBegin(pUpdate->sczUpdateSource, IDNOACTION);
    hr = UserExperienceInterpretResult(pUX, MB_OKCANCEL, nResult);
    ExitOnRootFailure(hr, "UX aborted detect update begin.");

    if (IDNOACTION == nResult)
    {
        //pUpdate->fUpdateAvailable = FALSE;
    }
    else if (IDOK == nResult)
    {
        ExitFunction1(hr = E_NOTIMPL);

        // TODO: actually check that a newer version is at the update source. For now we'll just
        //       pretend that if a source is provided that an update is available.
        //pUpdate->fUpdateAvailable = (pUpdate->sczUpdateSource && *pUpdate->sczUpdateSource);

        //hr = StrAllocFormatted(&sczUpdateId, L"%ls.update", wzBundleId);
        //ExitOnFailure(hr, "Failed to allocate update id.");

        //// Update bundle is always considered per-user since we do not have a secure way to inform the elevated engine
        //// about this detected bundle's data.
        //hr = PseudoBundleInitialize(FILEMAKEVERSION(rmj, rmm, rup, 0), &pUpdate->package, FALSE, sczUpdateId, BOOTSTRAPPER_RELATION_UPDATE, BOOTSTRAPPER_PACKAGE_STATE_ABSENT, L"update.exe", pUpdate->sczUpdateSource, pUpdate->qwSize, TRUE, L"-quiet", NULL, NULL, NULL, pUpdate->pbHash, pUpdate->cbHash);
        //ExitOnFailure(hr, "Failed to initialize update bundle.");
    }

LExit:
    if (fBeginCalled)
    {
        pUX->pUserExperience->OnDetectUpdateComplete(hr, /*pUpdate->fUpdateAvailable ? pUpdate->sczUpdateSource : */NULL);
    }

    ReleaseStr(sczUpdateId);
    return hr;
}
