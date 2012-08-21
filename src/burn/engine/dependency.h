//-------------------------------------------------------------------------------------------------
// <copyright file="dependency.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    Dependency functions for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

// constants

const LPCWSTR DEPENDENCY_IGNOREDEPENDENCIES = L"IGNOREDEPENDENCIES";


// function declarations

/********************************************************************
 DependencyUninitialize - Frees and zeros memory allocated in the
  dependency.

*********************************************************************/
void DependencyUninitialize(
    __in BURN_DEPENDENCY_PROVIDER* pProvider
    );

/********************************************************************
 DependencyParseProvidersFromXml - Parses dependency information
  from the manifest for the specified package.

*********************************************************************/
HRESULT DependencyParseProvidersFromXml(
    __in BURN_PACKAGE* pPackage,
    __in IXMLDOMNode* pixnPackage
    );

/********************************************************************
 DependencyPlanInitialize - Initializes the plan.

*********************************************************************/
HRESULT DependencyPlanInitialize(
    __in const BURN_ENGINE_STATE* pEngineState,
    __in BURN_PLAN* pPlan
    );

/********************************************************************
 DependencyAllocIgnoreDependencies - Allocates the dependencies to
  ignore as a semicolon-delimited string.

*********************************************************************/
HRESULT DependencyAllocIgnoreDependencies(
    __in const BURN_PLAN *pPlan,
    __out_z LPWSTR* psczIgnoreDependencies
    );

/********************************************************************
 DependencyPlanPackageBegin - Updates the dependency registration
  action depending on the calculated state for the package.

*********************************************************************/
HRESULT DependencyPlanPackageBegin(
    __in_opt DWORD *pdwInsertSequence,
    __in BOOL fPerMachine,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey
    );

/********************************************************************
 DependencyPlanPackageComplete - Updates the dependency registration
  action depending on the planned action for the package.

*********************************************************************/
HRESULT DependencyPlanPackageComplete(
    __in BOOL fPerMachine,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey
    );

/********************************************************************
 DependencyExecuteAction - Registers or unregisters dependency
  information for the package contained within the action.

*********************************************************************/
HRESULT DependencyExecuteAction(
    __in BOOL fPerMachine,
    __in const BURN_EXECUTE_ACTION* pAction
    );

/********************************************************************
 DependencyRegisterBundle - Registers the bundle dependency provider.

*********************************************************************/
HRESULT DependencyRegisterBundle(
    __in const BURN_REGISTRATION* pRegistration
    );

/********************************************************************
 DependencyRegisterPackage - Registers each dependency provider
  defined for the package (if not imported from the package itself).

 Note: Returns S_OK if the package is non-vital.
*********************************************************************/
HRESULT DependencyRegisterPackage(
    __in const BURN_PACKAGE* pPackage
    );

/********************************************************************
 DependencyUnregisterBundle - Removes the bundle dependency provider.

 Note: Does not check for existing dependents before removing the key.
*********************************************************************/
void DependencyUnregisterBundle(
    __in const BURN_REGISTRATION* pRegistration
    );

/********************************************************************
 DependencyUnregisterPackage - Removes each dependency provider
  for the package (if not imported from the package itself).

 Note: Does not check for existing dependents before removing the key.
*********************************************************************/
void DependencyUnregisterPackage(
    __in const BURN_PACKAGE* pPackage
    );

#if defined(__cplusplus)
}
#endif
