//-------------------------------------------------------------------------------------------------
// <copyright file="variant.h" company="Outercurve Foundation">
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

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#define ReleaseCfgVariant(pv) VariantFree(pv);
#define ReleaseNullCfgVariant(pv) VariantFree(pv); ZeroMemory(pv, sizeof(CFG_VARIANT));

enum CFG_VARIANT_TYPE
{
    CFG_VARIANT_INVALID = 0,
    CFG_VARIANT_NOTHING, // Represents a value that doesn't yet exist or a value being deleted
    CFG_VARIANT_BLOB,
    CFG_VARIANT_STRING,
    CFG_VARIANT_DWORD,
    CFG_VARIANT_QWORD,
    CFG_VARIANT_BOOL
};

enum CFG_BLOB_TYPE
{
    CFG_BLOB_INVALID = 0,
    CFG_BLOB_POINTER,
    CFG_BLOB_DB_STREAM
};

struct CFG_VARIANT
{
    CFG_VARIANT_TYPE cvtType;

    union
    {
        struct
        {
            CFG_BLOB_TYPE cbType;

            union
            {
                struct
                {
                    BYTE* pbValue;
                    SIZE_T cbValue;

                    // Whether to release the memory when the variant is freed
                    BOOL fRelease;
                } pointer;
                struct
                {
                    DWORD dwContentID;
                    CFGDB_STRUCT *pcdb;
                } dbstream;
            };
        } blob;
        struct
        {
            LPWSTR sczValue;

            // Whether to release the memory when the variant is freed
            BOOL fRelease;
        } string;
        struct
        {
            DWORD dwValue;
        } dword;
        struct
        {
            DWORD64 qwValue;
        } qword;
        struct
        {
            BOOL fValue;
        } boolean;
    };
};

BOOL VariantEqualityComparison(
    __in CFG_VARIANT * pcvVariant1,
    __in CFG_VARIANT * pcvVariant2
    );
HRESULT VariantSetFromValueRow(
    __in SCE_ROW_HANDLE sceRow,
    __deref_out CFG_VARIANT *pcvVariant
    );
HRESULT VariantSetFromValueHistoryRow(
    __in SCE_ROW_HANDLE sceRow,
    __deref_out CFG_VARIANT *pcvVariant
    );
HRESULT VariantSetDelete(
    __deref_out CFG_VARIANT *pcvVariant
    );
HRESULT VariantSetBlob(
    __in BYTE* pbValue,
    __in SIZE_T cbValue,
    __in BOOL fCopy,
    __deref_out CFG_VARIANT *pcvVariant
    );
HRESULT VariantSetBlobDbStream(
    __in CFGDB_STRUCT *pcdb,
    __in DWORD dwContentID,
    __deref_out CFG_VARIANT *pcvVariant
    )
HRESULT VariantSetString(
    __in_z LPCWSTR wzValue,
    __in BOOL fCopy,
    __deref_out CFG_VARIANT *pcvVariant
    );
HRESULT VariantSetDword(
    __in DWORD dwValue,
    __deref_out CFG_VARIANT *pcvVariant
    );
HRESULT VariantSetQword(
    __in DWORD64 qwValue,
    __deref_out CFG_VARIANT *pcvVariant
    );
HRESULT VariantSetBool(
    __in BOOL fValue,
    __deref_out CFG_VARIANT *pcvVariant
    );
HRESULT VariantFree(
    __inout CFG_VARIANT *pcvVariant
    );

#ifdef __cplusplus
}
#endif
