//-------------------------------------------------------------------------------------------------
// <copyright file="detect.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants


// structs


// functions

void DetectReset(
    __in BURN_PACKAGES* pPackages,
    __in BURN_UPDATE* pUpdate
    );

HRESULT DetectUpdate(
    __in_z LPCWSTR wzBundleId,
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_UPDATE* pUpdate
    );

#if defined(__cplusplus)
}
#endif
