//-------------------------------------------------------------------------------------------------
// <copyright file="WixBootstrapperBAFunction.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

#include <windows.h>

#include "IBootstrapperEngine.h"

interface IWixBootstrapperBAFunction
{
    virtual STDMETHODIMP OnDetectBAFunction() { return S_OK; };
    virtual STDMETHODIMP OnDetectCompleteBAFunction() { return S_OK; };
    virtual STDMETHODIMP OnPlanBAFunction() { return S_OK; };
    virtual STDMETHODIMP OnPlanCompleteBAFunction() { return S_OK; };
};

extern "C" typedef HRESULT (WINAPI *PFN_BOOTSTRAPPER_BA_FUNCTION_CREATE)(
    __in IBootstrapperEngine* pEngine,
    __in HMODULE hModule,
    __out IWixBootstrapperBAFunction** ppBAFunction
    );

