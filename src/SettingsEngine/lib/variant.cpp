//-------------------------------------------------------------------------------------------------
// <copyright file="variant.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Internal utility functions for dealing with cfg variants
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

HRESULT VariantEqualityComparison(
    __in CFG_VARIANT * pcvVariant1,
    __in CFG_VARIANT * pcvVariant2,
    __out BOOL * pfResult
    )
{
    HRESULT hr = S_OK;
    BOOL fReleaseBlob1 = FALSE;
    BOOL fReleaseBlob2 = FALSE;
    BYTE *pbBlob1 = NULL;
    BYTE *pbBlob2 = NULL;
    SIZE_T cbBlob1 = 0;
    SIZE_T cbBlob2 = 0;

    if (pcvVariant1->cvtType != pcvVariant2->cvtType)
    {
        *pfResult = FALSE;
        ExitFunction1(hr = S_OK);
    }

    switch (pcvVariant1->cvtType)
    {
    case CFG_VARIANT_NOTHING:
        *pfResult = TRUE;
        break;
    case CFG_VARIANT_BLOB:
        // TODO: this function always pulls the entire buffer into memory
        // could be optimized by just comparing hashes when available or, if both variants
        // are db streams in the same database, just comparing their content IDs
        if (pcvVariant1->blob.cbType == CFG_BLOB_POINTER)
        {
            pbBlob1 = pcvVariant1->blob.pointer.pbValue;
            cbBlob1 = pcvVariant1->blob.pointer.cbValue;
        }
        else if (pcvVariant1->blob.cbType == CFG_BLOB_DB_STREAM)
        {
            fReleaseBlob1 = TRUE;
            hr = StreamGet(pcvVariant1->blob.dbstream.pcdb, pcvVariant1->blob.dbstream.dwContentID, NULL, &pbBlob1, &cbBlob1);
            ExitOnFailure1(hr, "Failed to get stream from database 1 with content ID: %u", pcvVariant1->blob.dbstream.dwContentID);
        }
        else
        {
            hr = E_INVALIDARG;
            ExitOnFailure1(hr, "Invalid blob type encountered %d", pcvVariant1->blob.cbType);
        }

        if (pcvVariant2->blob.cbType == CFG_BLOB_POINTER)
        {
            pbBlob2 = pcvVariant2->blob.pointer.pbValue;
            cbBlob2 = pcvVariant2->blob.pointer.cbValue;
        }
        else if (pcvVariant2->blob.cbType == CFG_BLOB_DB_STREAM)
        {
            fReleaseBlob2 = TRUE;
            hr = StreamGet(pcvVariant2->blob.dbstream.pcdb, pcvVariant2->blob.dbstream.dwContentID, NULL, &pbBlob2, &cbBlob2);
            ExitOnFailure1(hr, "Failed to get stream from database 2 with content ID: %u", pcvVariant2->blob.dbstream.dwContentID);
        }
        else
        {
            hr = E_INVALIDARG;
            ExitOnFailure1(hr, "Invalid blob type encountered %d", pcvVariant1->blob.cbType);
        }

        if (cbBlob1 != cbBlob2)
        {
            *pfResult = FALSE;
        }
        else if (0 != memcmp(pbBlob1, pbBlob2, cbBlob1))
        {
            *pfResult = FALSE;
        }
        else
        {
            *pfResult = TRUE;
        }
        break;
    case CFG_VARIANT_STRING:
        if (0 != lstrcmpW(pcvVariant1->string.sczValue, pcvVariant2->string.sczValue))
        {
            *pfResult = FALSE;
        }
        else
        {
            *pfResult = TRUE;
        }
        break;
    case CFG_VARIANT_DWORD:
        if (pcvVariant1->dword.dwValue != pcvVariant2->dword.dwValue)
        {
            *pfResult = FALSE;
        }
        else
        {
            *pfResult = TRUE;
        }
        break;
    case CFG_VARIANT_QWORD:
        if (pcvVariant1->qword.qwValue != pcvVariant2->qword.qwValue)
        {
            *pfResult = FALSE;
        }
        else
        {
            *pfResult = TRUE;
        }
        break;
    case CFG_VARIANT_BOOL:
        if (pcvVariant1->boolean.fValue != pcvVariant2->boolean.fValue)
        {
            *pfResult = FALSE;
        }
        else
        {
            *pfResult = TRUE;
        }
        break;
    default:
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Invalid variant type encountered %d", pcvVariant1->cvtType);
        break;
    }

LExit:
    if (fReleaseBlob1)
    {
        ReleaseMem(pbBlob1);
    }
    if (fReleaseBlob2)
    {
        ReleaseMem(pbBlob2);
    }

    return hr;
}

HRESULT VariantSetFromValueRow(
    __in SCE_ROW_HANDLE sceRow,
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    HRESULT hr = S_OK;

LExit:
    return hr;
}

HRESULT VariantSetFromValueHistoryRow(
    __in SCE_ROW_HANDLE sceRow,
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    HRESULT hr = S_OK;

LExit:
    return hr;
}

HRESULT VariantSetDelete(
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    pcvVariant->cvtType = CFG_VARIANT_NOTHING;

    return S_OK;
}

HRESULT VariantSetBlob(
    __in BYTE* pbValue,
    __in SIZE_T cbValue,
    __in BOOL fCopy,
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    HRESULT hr = S_OK;

    if (fCopy)
    {
        pcvVariant->blob.pointer.pbValue = static_cast<BYTE *>(MemAlloc(cbValue, FALSE));
        ExitOnNull1(pcvVariant->blob.pointer.pbValue, hr, E_OUTOFMEMORY, "Failed to allocate space for blob of size %u", cbValue);
        memcpy(pcvVariant->blob.pointer.pbValue, pbValue, cbValue);
        pcvVariant->blob.pointer.cbValue = cbValue;
        pcvVariant->cvtType = CFG_VARIANT_BLOB;
        pcvVariant->blob.cbType = CFG_BLOB_POINTER;
        pcvVariant->blob.pointer.fRelease = true;
    }
    else
    {
        pcvVariant->blob.pointer.pbValue = pbValue;
        pcvVariant->blob.pointer.cbValue = cbValue;
        pcvVariant->cvtType = CFG_VARIANT_BLOB;
        pcvVariant->blob.cbType = CFG_BLOB_POINTER;
        pcvVariant->blob.pointer.fRelease = false;
    }

LExit:
    return hr;
}

HRESULT VariantSetBlobDbStream(
    __in CFGDB_STRUCT *pcdb,
    __in DWORD dwContentID,
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    HRESULT hr = S_OK;

    pcvVariant->blob.dbstream.dwContentID = dwContentID;
    pcvVariant->blob.dbstream.pcdb = pcdb;
    pcvVariant->cvtType = CFG_VARIANT_BLOB;
    pcvVariant->blob.cbType = CFG_BLOB_DB_STREAM;

LExit:
    return hr;
}

HRESULT VariantSetString(
    __in_z LPCWSTR wzValue,
    __in BOOL fCopy,
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    HRESULT hr = S_OK;

    if (fCopy)
    {
        hr = StrAllocString(&pcvVariant->string.sczValue, wzValue, 0);
        ExitOnFailure(hr, "Failed to copy string into variant");

        pcvVariant->cvtType = CFG_VARIANT_STRING;
        pcvVariant->string.fRelease = true;
    }
    else
    {
        pcvVariant->string.sczValue = const_cast<LPWSTR>(wzValue);
        pcvVariant->cvtType = CFG_VARIANT_STRING;
        pcvVariant->string.fRelease = false;
    }

LExit:
    return hr;
}

HRESULT VariantSetDword(
    __in DWORD dwValue,
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    pcvVariant->cvtType = CFG_VARIANT_DWORD;
    pcvVariant->dword.dwValue = dwValue;

    return S_OK;
}

HRESULT VariantSetQword(
    __in DWORD64 qwValue,
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    pcvVariant->cvtType = CFG_VARIANT_QWORD;
    pcvVariant->qword.qwValue = qwValue;

    return S_OK;
}

HRESULT VariantSetBool(
    __in BOOL fValue,
    __deref_out CFG_VARIANT *pcvVariant
    )
{
    pcvVariant->cvtType = CFG_VARIANT_BOOL;
    pcvVariant->boolean.fValue = fValue;

    return S_OK;
}

HRESULT VariantFree(
    __in CFG_VARIANT *pcvVariant
    )
{
    switch (pcvVariant->cvtType)
    {
    case CFG_VARIANT_BLOB:
        if (pcvVariant->blob.cbType == CFG_BLOB_POINTER && pcvVariant->blob.pointer.fRelease)
        {
            ReleaseMem(pcvVariant->blob.pointer.pbValue);
        }
        break;
    case CFG_VARIANT_STRING:
        if (pcvVariant->string.fRelease)
        {
            ReleaseMem(pcvVariant->string.sczValue);
        }
        break;
    default:
        // Nothing to do
        break;
    }

    return S_OK;
}
