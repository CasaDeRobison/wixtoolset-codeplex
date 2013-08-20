//-------------------------------------------------------------------------------------------------
// <copyright file="ui.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    UI-related helper functions
// </summary>
//-------------------------------------------------------------------------------------------------


#include "precomp.h"

static const LPCWSTR RESOLVE_MIXED_TEXT = L"(Mixed)";
static const LPCWSTR RESOLVE_MINE_TEXT = L"Mine";
static const LPCWSTR RESOLVE_NONE_TEXT = L"(Unresolved)";


LPCWSTR UIGetResolutionText(
    __in RESOLUTION_CHOICE rcChoice,
    __in_z LPCWSTR wzRemoteDatabaseName
    )
{
    switch (rcChoice)
    {
    case RESOLUTION_LOCAL:
        return RESOLVE_MINE_TEXT;

    case RESOLUTION_REMOTE:
        return wzRemoteDatabaseName;

    case RESOLUTION_UNRESOLVED: __fallthrough;
    default:
        return RESOLVE_NONE_TEXT;
    }
}


LPWSTR UIGetTypeDisplayName(
    __in CONFIG_VALUETYPE cvType
    )
{
    switch (cvType)
    {
    case VALUE_DELETED:
        return L"Deleted";
    case VALUE_BLOB:
        return L"Blob";
    case VALUE_BOOL:
        return L"Boolean";
    case VALUE_STRING:
        return L"String";
    case VALUE_DWORD:
        return L"Dword";
    case VALUE_QWORD:
        return L"Qword";
    case VALUE_INVALID:
        return L"Invalid";
    default:
        return L"Unknown";
    }
}

HRESULT UIGetSingleSelectedItemFromListView(
    __in HWND hwnd,
    __out_opt DWORD *pdwIndex, 
    __out_opt DWORD *pdwParam
    )
{
    HRESULT hr = S_OK;
    DWORD dwValueCount;
    LVITEM lvItem = { };
    DWORD dwFoundIndex = DWORD_MAX;
    DWORD dwFoundParam = DWORD_MAX;

    lvItem.mask = LVIF_PARAM;

    // Docs don't indicate any way for it to return failure
    dwValueCount = ::SendMessageW(hwnd, LVM_GETITEMCOUNT, 0, 0);

    for (DWORD i = 0; i < dwValueCount; ++i)
    {
        // If it's selected
        if (::SendMessageW(hwnd, LVM_GETITEMSTATE, i, LVIS_SELECTED))
        {
            if (DWORD_MAX != dwFoundIndex)
            {
                if (NULL != pdwIndex)
                {
                    *pdwIndex = DWORD_MAX;
                }
                if (NULL != pdwParam)
                {
                    *pdwParam = DWORD_MAX;
                }

                // More than one item found, so report the issue
                ExitFunction1(hr = E_NOTFOUND);
            }

            lvItem.iItem = i;

            if (!::SendMessageW(hwnd, LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&lvItem)))
            {
                ExitWithLastError(hr, "Failed to get item lparam from list view to delete values");
            }

            dwFoundIndex = i;
            dwFoundParam = lvItem.lParam;
        }
    }

    if (NULL != pdwIndex)
    {
        *pdwIndex = dwFoundIndex;
    }
    if (NULL != pdwParam)
    {
        *pdwParam = dwFoundParam;
    }

LExit:
    return hr;
}

HRESULT UISetListViewText(
    __in HWND hwnd,
    __in_z LPCWSTR sczText
    )
{
    HRESULT hr = S_OK;
    DWORD dwInsertIndex = 0;

    if (!::SendMessageW(hwnd, LVM_DELETEALLITEMS, 0, 0))
    {
        ExitWithLastError(hr, "Failed to delete all items from list view");
    }

    hr = UIListViewInsertItem(hwnd, &dwInsertIndex, sczText, 0, DWORD_MAX);
    ExitOnFailure(hr, "Failed to insert text into listview");

LExit:
    return hr;
}

HRESULT UISetListViewToProductEnum(
    __in HWND hwnd,
    __in C_CFG_ENUMERATION_HANDLE cehProducts,
    __in_opt const BOOL *rgfInstalled
    )
{
    HRESULT hr = S_OK;
    DWORD dwInsertIndex = 0;
    DWORD dwCount = 0;
    DWORD dwInsertImage = 0;
    LPCWSTR wzText = NULL;

    if (NULL != cehProducts)
    {
        hr = CfgEnumReadDword(cehProducts, 0, ENUM_DATA_COUNT, &dwCount);
        ExitOnFailure(hr, "Failed to read count of product enumeration");
    }

    if (0 == dwCount)
    {
        EnableWindow(hwnd, FALSE);

        hr = UISetListViewText(hwnd, L"No products to display.");
        ExitOnFailure(hr, "Failed to set 'no products to display' string in product list view");

        ExitFunction1(hr = S_OK);
    }

    if (!::SendMessageW(hwnd, LVM_DELETEALLITEMS, 0, 0))
    {
        ExitWithLastError(hr, "Failed to delete all items from list view");
    }
    EnableWindow(hwnd, TRUE);

    for (DWORD i = 0; i < dwCount; ++i)
    {
        dwInsertIndex = i;

        hr = CfgEnumReadString(cehProducts, i, ENUM_DATA_PRODUCTNAME, &wzText);
        ExitOnFailure(hr, "Failed to read product name from enum");

#pragma prefast(push)
#pragma prefast(disable:26007)
        if (NULL != rgfInstalled && rgfInstalled[i])
#pragma prefast(pop)
        {
            dwInsertImage = 1;
        }
        else
        {
            dwInsertImage = 0;
        }
        hr = UIListViewInsertItem(hwnd, &dwInsertIndex, wzText, i, dwInsertImage);
        ExitOnFailure(hr, "Failed to insert product into listview control");

        hr = CfgEnumReadString(cehProducts, i, ENUM_DATA_VERSION, &wzText);
        ExitOnFailure(hr, "Failed to read product version from enum");

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 1, wzText);
        ExitOnFailure(hr, "Failed to set version as listview subitem");

        hr = CfgEnumReadString(cehProducts, i, ENUM_DATA_PUBLICKEY, &wzText);
        ExitOnFailure(hr, "Failed to read product public key from enum");

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 2, wzText);
        ExitOnFailure(hr, "Failed to set public key as listview subitem");
    }

LExit:
    return hr;
}

HRESULT UIForgetProductsFromListView(
    __in HWND hwnd,
    __in CFGDB_HANDLE cdhHandle,
    __in C_CFG_ENUMERATION_HANDLE cehProducts
    )
{
    HRESULT hr = S_OK;
    DWORD dwValueCount;
    LVITEM lvItem = { };
    LPCWSTR wzProductName = NULL;
    LPCWSTR wzProductVersion = NULL;
    LPCWSTR wzProductPublicKey = NULL;

    lvItem.mask = LVIF_PARAM;

    // Docs don't indicate any way for it to return failure
    dwValueCount = ::SendMessageW(hwnd, LVM_GETITEMCOUNT, 0, 0);

    for (DWORD i = 0; i < dwValueCount; ++i)
    {
        // If it's selected
        if (::SendMessageW(hwnd, LVM_GETITEMSTATE, i, LVIS_SELECTED))
        {
            lvItem.iItem = i;

            if (!::SendMessageW(hwnd, LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&lvItem)))
            {
                ExitWithLastError(hr, "Failed to get item lparam from list view to delete values");
            }

            hr = CfgEnumReadString(cehProducts, lvItem.lParam, ENUM_DATA_PRODUCTNAME, &wzProductName);
            ExitOnFailure(hr, "Failed to read product name");

            hr = CfgEnumReadString(cehProducts, lvItem.lParam, ENUM_DATA_VERSION, &wzProductVersion);
            ExitOnFailure(hr, "Failed to read product version");

            hr = CfgEnumReadString(cehProducts, lvItem.lParam, ENUM_DATA_PUBLICKEY, &wzProductPublicKey);
            ExitOnFailure(hr, "Failed to read product publickey");

            // Ignore failures - what can we do? User will notice in UI that it's still there
            // TODO: kill this database interaction in UI thread, it's no good
            CfgForgetProduct(cdhHandle, wzProductName, wzProductVersion, wzProductPublicKey);
        }
    }

LExit:
    return hr;
}

HRESULT UISetListViewToValueEnum(
    __in HWND hwnd,
    __in_opt C_CFG_ENUMERATION_HANDLE cehValues
    )
{
    HRESULT hr = S_OK;

    DWORD i;
    SYSTEMTIME st = { };
    DWORD dwCount = 0;
    DWORD dwValue = 0;
    DWORD64 qwValue = 0;
    BOOL fValue = FALSE;
    DWORD dwInsertIndex;
    LPCWSTR wzText = NULL;
    LPWSTR sczText = NULL;
    CONFIG_VALUETYPE cvType = VALUE_INVALID;

    if (NULL != cehValues)
    {
        hr = CfgEnumReadDword(cehValues, 0, ENUM_DATA_COUNT, &dwCount);
        ExitOnFailure(hr, "Failed to read count of value enumeration");
    }

    if (0 == dwCount)
    {
        EnableWindow(hwnd, FALSE);

        hr = UISetListViewText(hwnd, L"No values to display");
        ExitOnFailure(hr, "Failed to set 'no values' text in value listview");

        ExitFunction1(hr = S_OK);
    }

    if (!::SendMessageW(hwnd, LVM_DELETEALLITEMS, 0, 0))
    {
        ExitWithLastError(hr, "Failed to delete all items from list view");
    }
    EnableWindow(hwnd, TRUE);

    for (i = 0; i < dwCount; ++i)
    {
        dwInsertIndex = i;

        hr = CfgEnumReadString(cehValues, i, ENUM_DATA_VALUENAME, &wzText);
        ExitOnFailure(hr, "Failed to read value enumeration");

        hr = UIListViewInsertItem(hwnd, &dwInsertIndex, wzText, i, 0);
        ExitOnFailure(hr, "Failed to insert value name into listview control");

        hr = CfgEnumReadDataType(cehValues, i, ENUM_DATA_VALUETYPE, &cvType);
        ExitOnFailure(hr, "Failed to read type of value from value enumeration");

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 1, UIGetTypeDisplayName(cvType));
        ExitOnFailure(hr, "Failed to set value as listview subitem");

        switch (cvType)
        {
        case VALUE_BLOB:
            hr = CfgEnumReadDword(cehValues, i, ENUM_DATA_BLOBSIZE, &dwValue);
            ExitOnFailure(hr, "Failed to read blob size from enumeration");

            hr = StrAllocFormatted(&sczText, L"(Size %u)", dwValue);
            ExitOnFailure(hr, "Failed to format DWORD value into string");

            wzText = sczText;
            break;
        case VALUE_STRING:
            hr = CfgEnumReadString(cehValues, i, ENUM_DATA_VALUESTRING, &wzText);
            ExitOnFailure(hr, "Failed to read string value from enumeration");
            break;
        case VALUE_DWORD:
            hr = CfgEnumReadDword(cehValues, i, ENUM_DATA_VALUEDWORD, &dwValue);
            ExitOnFailure(hr, "Failed to read dword value from enumeration");

            hr = StrAllocFormatted(&sczText, L"%u", dwValue);
            ExitOnFailure(hr, "Failed to format DWORD value into string");

            wzText = sczText;
            break;
        case VALUE_QWORD:
            hr = CfgEnumReadQword(cehValues, i, ENUM_DATA_VALUEQWORD, &qwValue);
            ExitOnFailure(hr, "Failed to read qword value from enumeration");

            hr = StrAllocFormatted(&sczText, L"%I64u", qwValue);
            ExitOnFailure(hr, "Failed to format QWORD value into string");

            wzText = sczText;
            break;
        case VALUE_BOOL:
            hr = CfgEnumReadBool(cehValues, i, ENUM_DATA_VALUEBOOL, &fValue);
            ExitOnFailure(hr, "Failed to read bool value from enumeration");

            wzText = fValue ? L"True" : L"False";
            break;
        }

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 2, wzText);
        ExitOnFailure(hr, "Failed to set value as listview subitem");

        hr = CfgEnumReadSystemTime(cehValues, i, ENUM_DATA_WHEN, &st);
        ExitOnFailure(hr, "Failed to read when string from value history enumeration");

        hr = TimeSystemDateTime(&sczText, &st, TRUE);
        ExitOnFailure(hr, "Failed to convert value 'when' time to text");

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 3, sczText);
        ExitOnFailure(hr, "Failed to set when string as listview subitem");
    }

LExit:
    ReleaseStr(sczText);

    return hr;
}

HRESULT UISetListViewToValueHistoryEnum(
    __in HWND hwnd,
    __in_opt C_CFG_ENUMERATION_HANDLE cehValueHistory
    )
{
    HRESULT hr = S_OK;

    DWORD dwCount = 0;
    DWORD dwValue = 0;
    DWORD64 qwValue = 0;
    BOOL fValue = FALSE;
    DWORD dwInsertIndex;
    LPCWSTR wzText = NULL;
    LPWSTR sczText = NULL;
    SYSTEMTIME st = { };
    CONFIG_VALUETYPE cvType = VALUE_INVALID;

    if (NULL != cehValueHistory)
    {
        hr = CfgEnumReadDword(cehValueHistory, 0, ENUM_DATA_COUNT, &dwCount);
        ExitOnFailure(hr, "Failed to read count of value history enumeration");
    }

    if (0 == dwCount)
    {
        EnableWindow(hwnd, FALSE);

        hr = UISetListViewText(hwnd, L"No value history to display");
        ExitOnFailure(hr, "Failed to set 'no value history' text in value history listview");

        ExitFunction1(hr = S_OK);
    }

    if (!::SendMessageW(hwnd, LVM_DELETEALLITEMS, 0, 0))
    {
        ExitWithLastError(hr, "Failed to delete all items from list view");
    }
    EnableWindow(hwnd, TRUE);

    for (DWORD i = 0; i < dwCount; ++i)
    {
        dwInsertIndex = i;

        hr = CfgEnumReadDataType(cehValueHistory, i, ENUM_DATA_VALUETYPE, &cvType);
        ExitOnFailure(hr, "Failed to read type of value from value history enumeration");

        switch (cvType)
        {
        case VALUE_DELETED:
            wzText = L"";
            break;
        case VALUE_BLOB:
            hr = CfgEnumReadDword(cehValueHistory, i, ENUM_DATA_BLOBSIZE, &dwValue);
            ExitOnFailure(hr, "Failed to read blob size from enumeration");

            hr = StrAllocFormatted(&sczText, L"(Size %u)", dwValue);
            ExitOnFailure(hr, "Failed to format DWORD value into string");

            wzText = sczText;
            break;
        case VALUE_STRING:
            hr = CfgEnumReadString(cehValueHistory, i, ENUM_DATA_VALUESTRING, &wzText);
            ExitOnFailure(hr, "Failed to read string value from value history enumeration");
            break;
        case VALUE_DWORD:
            hr = CfgEnumReadDword(cehValueHistory, i, ENUM_DATA_VALUEDWORD, &dwValue);
            ExitOnFailure(hr, "Failed to read dword value from value history enumeration");

            hr = StrAllocFormatted(&sczText, L"%u", dwValue);
            ExitOnFailure(hr, "Failed to format DWORD value into string");

            wzText = sczText;
            break;
        case VALUE_QWORD:
            hr = CfgEnumReadQword(cehValueHistory, i, ENUM_DATA_VALUEQWORD, &qwValue);
            ExitOnFailure(hr, "Failed to read qword value from value history enumeration");

            hr = StrAllocFormatted(&sczText, L"%I64u", qwValue);
            ExitOnFailure(hr, "Failed to format QWORD value into string");

            wzText = sczText;
            break;
        case VALUE_BOOL:
            hr = CfgEnumReadBool(cehValueHistory, i, ENUM_DATA_VALUEBOOL, &fValue);
            ExitOnFailure(hr, "Failed to read bool value from value history enumeration");

            wzText = fValue ? L"True" : L"False";
            break;
        }

        hr = UIListViewInsertItem(hwnd, &dwInsertIndex, UIGetTypeDisplayName(cvType), i, 0);
        ExitOnFailure(hr, "Failed to set value as listview subitem");

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 1, wzText);
        ExitOnFailure(hr, "Failed to insert value name into listview control");

        hr = CfgEnumReadString(cehValueHistory, i, ENUM_DATA_BY, &wzText);
        ExitOnFailure(hr, "Failed to read by string from value history enumeration");

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 2, wzText);
        ExitOnFailure(hr, "Failed to set value as listview subitem");

        hr = CfgEnumReadSystemTime(cehValueHistory, i, ENUM_DATA_WHEN, &st);
        ExitOnFailure(hr, "Failed to read when string from value history enumeration");

        hr = TimeSystemDateTime(&sczText, &st, TRUE);
        ExitOnFailure(hr, "Failed to convert value history time to text");

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 3, sczText);
        ExitOnFailure(hr, "Failed to set value as listview subitem");
    }

LExit:
    ReleaseStr(sczText);

    return hr;
}

HRESULT UIDeleteValuesFromListView(
    __in HWND hwnd,
    __in CFGDB_HANDLE cdhHandle,
    __in C_CFG_ENUMERATION_HANDLE cehValues
    )
{
    HRESULT hr = S_OK;
    DWORD dwValueCount;
    LVITEM lvItem = { };
    LPCWSTR wzValueName = NULL;

    lvItem.mask = LVIF_PARAM;

    // Docs don't indicate any way for it to return failure
    dwValueCount = ::SendMessageW(hwnd, LVM_GETITEMCOUNT, 0, 0);

    for (DWORD i = 0; i < dwValueCount; ++i)
    {
        // If it's selected
        if (::SendMessageW(hwnd, LVM_GETITEMSTATE, i, LVIS_SELECTED))
        {
            lvItem.iItem = i;

            if (!::SendMessageW(hwnd, LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&lvItem)))
            {
                ExitWithLastError(hr, "Failed to get item lparam from list view to delete values");
            }

            hr = CfgEnumReadString(cehValues, lvItem.lParam, ENUM_DATA_VALUENAME, &wzValueName);
            ExitOnFailure(hr, "Failed to read valuename from enumeration while deleting values");

            // Ignore failures - what can we do? User will notice in UI that it's still there
            // TODO: kill this database interaction in UI thread, it's no good
            CfgDeleteValue(cdhHandle, wzValueName);
        }
    }

LExit:
    return hr;
}

HRESULT UISetValueConflictsFromListView(
    __in HWND hwnd,
    __in_z LPCWSTR wzDatabaseName,
    __in CONFLICT_PRODUCT *pcpProductConflict,
    __in RESOLUTION_CHOICE rcChoice
    )
{
    HRESULT hr = S_OK;
    DWORD cValueCount;
    LVITEM lvItem = { };
    LPCWSTR wzText;

    wzText = UIGetResolutionText(rcChoice, wzDatabaseName);
    lvItem.mask = LVIF_PARAM;

    // Docs don't indicate any way for it to return failure
    cValueCount = ::SendMessageW(hwnd, LVM_GETITEMCOUNT, 0, 0);

    for (DWORD i = 0; i < cValueCount; ++i)
    {
        // If it's selected
        if (::SendMessageW(hwnd, LVM_GETITEMSTATE, i, LVIS_SELECTED))
        {
            lvItem.iItem = i;

            if (!::SendMessageW(hwnd, LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&lvItem)))
            {
                ExitWithLastError(hr, "Failed to get item lparam from list view to set product conflicts");
            }

            if (0 > lvItem.lParam || pcpProductConflict->cValues <= static_cast<DWORD>(lvItem.lParam))
            {
                hr = E_UNEXPECTED;
                ExitOnFailure1(hr, "Invalid param %d stored in listview!", lvItem.lParam);
            }

            pcpProductConflict->rgrcValueChoices[lvItem.lParam] = rcChoice;

            hr = UIListViewSetItemText(hwnd, i, 1, wzText);
            ExitOnFailure(hr, "Failed to set text in column 1 of listview control");
        }
    }

LExit:
    return hr;
}

HRESULT UISetProductConflictsFromListView(
    __in HWND hwnd,
    __in_z LPCWSTR wzDatabaseName,
    __in_ecount(cProductCount) CONFLICT_PRODUCT *pcplProductConflictList,
    __in DWORD cProductCount,
    __in RESOLUTION_CHOICE rcChoice
    )
{
    HRESULT hr = S_OK;
    DWORD cListViewItemCount = 0;
    LVITEM lvItem = { };
    LPCWSTR wzText;

    wzText = UIGetResolutionText(rcChoice, wzDatabaseName);
    lvItem.mask = LVIF_PARAM;

    // Docs don't indicate any way for it to return failure
    cListViewItemCount = ::SendMessageW(hwnd, LVM_GETITEMCOUNT, 0, 0);

    for (DWORD i = 0; i < cListViewItemCount; ++i)
    {
        // If it's selected
        if (::SendMessageW(hwnd, LVM_GETITEMSTATE, i, LVIS_SELECTED))
        {
            lvItem.iItem = i;

            if (!::SendMessageW(hwnd, LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&lvItem)))
            {
                ExitWithLastError(hr, "Failed to get item lparam from list view to set product conflicts");
            }
            
            if (0 > lvItem.lParam || cProductCount <= static_cast<DWORD>(lvItem.lParam))
            {
                hr = E_UNEXPECTED;
                ExitOnFailure1(hr, "Invalid param %d stored in listview!", lvItem.lParam);
            }

            for (DWORD j = 0; j < pcplProductConflictList[lvItem.lParam].cValues; ++j)
            {
                pcplProductConflictList[lvItem.lParam].rgrcValueChoices[j] = rcChoice;
            }

            hr = UIListViewSetItemText(hwnd, i, 1, wzText);
            ExitOnFailure(hr, "Failed to set text in column 1 of listview control");
        }
    }

LExit:
    return hr;
}

HRESULT UISetListViewToValueConflictArray(
    __in HWND hwnd,
    __in LPCWSTR wzDatabaseName,
    __in_ecount(cConflictValueCount) C_CFG_ENUMERATION_HANDLE *pcehHandle,
    __in_ecount(cConflictValueCount) const RESOLUTION_CHOICE *rgrcValueChoices,
    __in DWORD cConflictValueCount
    )
{
    HRESULT hr = S_OK;
    LPCWSTR wzValueName = NULL;
    DWORD i;
    DWORD dwInsertIndex;

    if (!::SendMessageW(hwnd, LVM_DELETEALLITEMS, 0, 0))
    {
        ExitWithLastError(hr, "Failed to delete all items from list view");
    }
    EnableWindow(hwnd, TRUE);

    for (i = 0; i < cConflictValueCount; ++i)
    {
        dwInsertIndex = i;

        hr = CfgEnumReadString(pcehHandle[i], 0, ENUM_DATA_VALUENAME, &wzValueName);
        ExitOnFailure(hr, "Failed to read value name from value conflict array");

        hr = UIListViewInsertItem(hwnd, &dwInsertIndex, wzValueName, i, 0);
        ExitOnFailure(hr, "Failed to insert value name into value conflict listview control");

        switch (rgrcValueChoices[i])
        {
        case RESOLUTION_UNRESOLVED:
            hr = UIListViewSetItemText(hwnd, dwInsertIndex, 1, L"(Unresolved)");
            ExitOnFailure(hr, "Failed to set text in column 1 of listview control");
            break;
        case RESOLUTION_LOCAL:
            hr = UIListViewSetItemText(hwnd, dwInsertIndex, 1, L"Mine");
            ExitOnFailure(hr, "Failed to set text in column 1 of listview control");
            break;
        case RESOLUTION_REMOTE:
            hr = UIListViewSetItemText(hwnd, dwInsertIndex, 1, wzDatabaseName);
            ExitOnFailure(hr, "Failed to set text in column 1 of listview control");
            break;
        default:
            hr = E_UNEXPECTED;
            ExitOnFailure(hr, "Unexpected resolution type encountered!");
            break;
        }
    }

LExit:
    return hr;
}

HRESULT UISetListViewToProductConflictArray(
    __in HWND hwnd,
    __in LPCWSTR wzDatabaseName,
    __in HRESULT hrSyncResult,
    __in_ecount(dwConflictProductCount) const CONFLICT_PRODUCT *pcpProductConflict,
    __in DWORD dwConflictProductCount
    )
{
    HRESULT hr = S_OK;
    DWORD dwInsertIndex;
    RESOLUTION_CHOICE rcChoice;
    BOOL fConsistentChoice;

    if (FAILED(hrSyncResult))
    {
        EnableWindow(hwnd, FALSE);
        hr = UISetListViewText(hwnd, L"Error synchronizing!");
        ExitOnFailure(hr, "Failed to set 'error synchronizing' text in product conflict listview");

        ExitFunction1(hr = S_OK);
    }
    if (0 == dwConflictProductCount)
    {
        EnableWindow(hwnd, FALSE);
        hr = UISetListViewText(hwnd, L"No conflicts to display");
        ExitOnFailure(hr, "Failed to set 'no conflicts' text in product conflict listview");

        ExitFunction1(hr = S_OK);
    }

    // Clear out and enable the listview
    if (!::SendMessageW(hwnd, LVM_DELETEALLITEMS, 0, 0))
    {
        ExitWithLastError(hr, "Failed to delete all items from list view");
    }
    EnableWindow(hwnd, TRUE);

    for (DWORD i = 0; i < dwConflictProductCount; ++i)
    {
        dwInsertIndex = i;

        fConsistentChoice = TRUE;

        // Take the first value's resolution, first
        if (1 <= pcpProductConflict[i].cValues)
        {
            rcChoice = pcpProductConflict[i].rgrcValueChoices[0];
        }
        else
        {
            rcChoice = RESOLUTION_UNRESOLVED;
        }

        // Then iterate to see if any other ones are different
        for (DWORD j = 1; j < pcpProductConflict[i].cValues; ++j)
        {
            if (rcChoice != pcpProductConflict[i].rgrcValueChoices[j])
            {
                fConsistentChoice = FALSE;
                break;
            }
        }

        hr = UIListViewInsertItem(hwnd, &dwInsertIndex, pcpProductConflict[i].sczProductName, i, 0);
        ExitOnFailure(hr, "Failed to insert value name into listview control");

        if (!fConsistentChoice)
        {
            hr = UIListViewSetItemText(hwnd, dwInsertIndex, 1, RESOLVE_MIXED_TEXT);
            ExitOnFailure(hr, "Failed to set text in column 1 of listview control");
        }
        else
        {
            hr = UIListViewSetItemText(hwnd, dwInsertIndex, 1, UIGetResolutionText(rcChoice, wzDatabaseName));
            ExitOnFailure(hr, "Failed to set text in column 1 of listview control");
        }

        hr = UIListViewSetItemText(hwnd, dwInsertIndex, 2, pcpProductConflict[i].sczVersion);
        ExitOnFailure(hr, "Failed to insert value into listview control");
    }

LExit:
    return hr;
}

HRESULT UIListViewInsertItem(
    __in const HWND hwnd,
    __inout DWORD *pdwIndex,
    __in LPCWSTR sczText,
    __in DWORD dwParam,
    __in DWORD dwImage
    )
{
    HRESULT hr = S_OK;
    LONG lRetVal = 0;
    LV_ITEMW lvi = { };

    lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
    lvi.iItem = *pdwIndex;
    lvi.iSubItem = 0;
    lvi.iImage = dwImage;
    lvi.lParam = dwParam;
    lvi.pszText = const_cast<LPWSTR>(sczText);

    lRetVal = ::SendMessageW(hwnd, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&lvi));
    if (-1 == lRetVal)
    {
        ExitWithLastError(hr, "Failed to insert row into listview");
    }
    else
    {
        *pdwIndex = lRetVal;
    }

LExit:
    return hr;
}


HRESULT UIListViewSetItemText(
    __in const HWND hwnd,
    __in DWORD dwIndex,
    __in DWORD dwColumnIndex,
    __in LPCWSTR sczText
    )
{
    HRESULT hr = S_OK;
    LV_ITEMW lvi = { };

    lvi.iSubItem = dwColumnIndex;
    lvi.pszText = const_cast<LPWSTR>(sczText);

    if (-1 == ::SendMessageW(hwnd, LVM_SETITEMTEXTW, static_cast<LPARAM>(dwIndex), reinterpret_cast<LPARAM>(&lvi)))
    {
        ExitWithLastError2(hr, "Failed to set field in listview - row %u, column %u", dwIndex, dwColumnIndex);
    }

LExit:
    return hr;
}

HRESULT UIExportFile(
    __in const HWND hwnd,
    __out_z LPWSTR *psczPath
    )
{
    HRESULT hr = S_OK;
    OPENFILENAMEW ofn = { };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    hr = StrAlloc(&ofn.lpstrFile, NUM_FILE_SELECTION_DIALOG_CHARACTERS);
    ExitOnFailure(hr, "Failed to allocate space for file selection dialog while exporting file");
    ofn.nMaxFile = NUM_FILE_SELECTION_DIALOG_CHARACTERS;
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrTitle = L"Export File";

    if (::GetOpenFileNameW(&ofn))
    {
        *psczPath = ofn.lpstrFile;
        ofn.lpstrFile = NULL;
    }

LExit:
    ReleaseStr(ofn.lpstrFile);

    return hr;
}

HRESULT UIMessageBoxDisplayError(
    __in HWND hwnd,
    __in LPCWSTR wzErrorMessage,
    __in HRESULT hrDisplay
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDisplayMessage = NULL;
    
    hr = StrAllocFormatted(&sczDisplayMessage, L"HRESULT 0x%x - %ls",hrDisplay, wzErrorMessage);
    if (SUCCEEDED(hr))
    {
        ::MessageBoxW(hwnd, sczDisplayMessage, L"Error", MB_OK | MB_ICONERROR);
    }
    else
    {
        ::MessageBoxW(hwnd, L"Error encountered, and due to extreme conditions (such as lack of memory) the error code could not be displayed.", L"Error", MB_OK | MB_ICONERROR);
    }

    ReleaseStr(sczDisplayMessage);

    return hr;
}
