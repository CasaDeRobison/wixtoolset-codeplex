//-------------------------------------------------------------------------------------------------
// <copyright file="WixBootstrapperBAFuntion.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

#include <windows.h>

#include "IBootstrapperEngine.h"

interface IWixBootstrapperBAFuntion
{
public:
    virtual STDMETHODIMP OnDetectBAFuntion() { return S_OK; };
    virtual STDMETHODIMP OnDetectCompleteBAFuntion() { return S_OK; };
    virtual STDMETHODIMP OnPlanBAFuntion() { return S_OK; };
    virtual STDMETHODIMP OnPlanCompleteBAFuntion() { return S_OK; };
};

extern "C" typedef HRESULT (WINAPI *PFN_BOOTSTRAPPER_BA_FUNCTION_CREATE)(
    __in IBootstrapperEngine* pEngine,
    __in HMODULE hModule,
    __out IWixBootstrapperBAFuntion** ppBAFuntion
    );

