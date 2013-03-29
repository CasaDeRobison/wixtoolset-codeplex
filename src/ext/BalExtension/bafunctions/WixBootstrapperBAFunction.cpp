//-------------------------------------------------------------------------------------------------
// <copyright file="WixBootstrapperBAFuntion.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------


#include "precomp.h"

class CWixBootstrapperBAFuntion : IWixBootstrapperBAFuntion
{
public:
    STDMETHODIMP OnDetectBAFuntion()
    {
        HRESULT hr = S_OK;

        BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "Running detect BA function");

        //-------------------------------------------------------------------------------------------------
        // YOUR CODE GOES HERE
        BalExitOnFailure(hr, "Dummy.");
        //-------------------------------------------------------------------------------------------------

    LExit:
        return hr;
    }


/*
    STDMETHODIMP OnDetectCompleteBAFuntion()
    {
        HRESULT hr = S_OK;

        BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "Running detect complete BA function");

        //-------------------------------------------------------------------------------------------------
        // YOUR CODE GOES HERE
        BalExitOnFailure(hr, "Dummy.");
        //-------------------------------------------------------------------------------------------------

    LExit:
        return hr;
    }

    
        STDMETHODIMP OnPlanBAFuntion()
    {
        HRESULT hr = S_OK;

        BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "Running plan BA function");

        //-------------------------------------------------------------------------------------------------
        // YOUR CODE GOES HERE
        BalExitOnFailure(hr, "Dummy.");
        //-------------------------------------------------------------------------------------------------

    LExit:
        return hr;
    }

    
    STDMETHODIMP OnPlanCompleteBAFuntion()
    {
        HRESULT hr = S_OK;

        BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "Running plan complete BA function");

        //-------------------------------------------------------------------------------------------------
        // YOUR CODE GOES HERE
        BalExitOnFailure(hr, "Dummy.");
        //-------------------------------------------------------------------------------------------------

    LExit:
        return hr;
    }
*/


private:
    HMODULE m_hModule;
    IBootstrapperEngine* m_pEngine;


public:
    //
    // Constructor - initialize member variables.
    //
    CWixBootstrapperBAFuntion(
        __in IBootstrapperEngine* pEngine,
        __in HMODULE hModule
        )
    {
        m_hModule = hModule;
        m_pEngine = pEngine;
    }

    //
    // Destructor - release member variables.
    //
    ~CWixBootstrapperBAFuntion()
    {
    }
};


extern "C" HRESULT WINAPI CreateBootstrapperBAFuntion(
    __in IBootstrapperEngine* pEngine,
    __in HMODULE hModule,
    __out CWixBootstrapperBAFuntion** ppBAFuntion
    )
{
    HRESULT hr = S_OK;

    // This is required to enable logging functions
    BalInitialize(pEngine);

    CWixBootstrapperBAFuntion* pBAFuntion = NULL;

    pBAFuntion = new CWixBootstrapperBAFuntion(pEngine, hModule);
    ExitOnNull(pBAFuntion, hr, E_OUTOFMEMORY, "Failed to create new bootstrapper BA function object.");

    *ppBAFuntion = pBAFuntion;
    pBAFuntion = NULL;

LExit:
    return hr;
}
