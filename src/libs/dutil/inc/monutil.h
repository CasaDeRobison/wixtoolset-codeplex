#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="monutil.h" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Header for utilities to help monitor changes under filesystem directories or registry keys
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#define ReleaseMon(mh) if (mh) { MonDestroy(mh); }
#define ReleaseNullMon(mh) if (mh) { MonDestroy(mh); mh = NULL; }

typedef void* MON_HANDLE;
typedef const void* C_MON_HANDLE;

// Defined in regutil.h
enum REG_KEY_BITNESS;

extern const int MON_HANDLE_BYTES;

// Note: callbacks must be implemented in a thread-safe manner. They will be called asynchronously by a MonUtil-spawned thread.
// They must also be written to return as soon as possible - they are called from the waiter thread
typedef void (*PFN_MONGENERAL)(
    __in HRESULT hr,
    __in_opt LPVOID pvContext
    );
typedef void (*PFN_MONDIRECTORY)(
    __in HRESULT hr,
    __in_z LPCWSTR wzPath,
    __in BOOL fRecursive,
    __in_opt LPVOID pvContext,
    __in_opt LPVOID pvDirectoryContext
    );
typedef void (*PFN_MONREGKEY)(
    __in HRESULT hr,
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in REG_KEY_BITNESS kbKeyBitness,
    __in BOOL fRecursive,
    __in_opt LPVOID pvContext,
    __in_opt LPVOID pvRegKeyContext
    );

// Silence period allows you to avoid lots of notifications when a lot of writes are going on in a directory
// MonUtil will wait until the directory has been "silent" for at least dwSilencePeriodInMs milliseconds
// The drawback to setting this to a value higher than zero is that even single write notifications
// are delayed by this amount
HRESULT DAPI MonCreate(
    __out_bcount(MON_HANDLE_BYTES) MON_HANDLE *pHandle,
    __in PFN_MONGENERAL vpfMonGeneral,
    __in_opt PFN_MONDIRECTORY vpfMonDirectory,
    __in_opt PFN_MONREGKEY vpfMonRegKey,
    __in_opt LPVOID pvContext
    );
// Don't add multiple identical waits! Not only is it wasteful and will cause multiple fires for the exact same change, it will also
// result in slightly odd behavior when you remove a duplicated wait (removing a wait may or may not remove multiple waits)
// This is due to the way coordinator thread and waiter threads handle removing, and while it is possible to solve, doing so would complicate the code.
// So instead, de-dupe your wait requests before sending them to monutil.
HRESULT DAPI MonAddDirectory(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in_z LPCWSTR wzPath,
    __in BOOL fRecursive,
    __in DWORD dwSilencePeriodInMs,
    __in_opt LPVOID pvDirectoryContext
    );
HRESULT DAPI MonAddRegKey(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in REG_KEY_BITNESS kbKeyBitness,
    __in BOOL fRecursive,
    __in DWORD dwSilencePeriodInMs,
    __in_opt LPVOID pvRegKeyContext
    );
HRESULT DAPI MonRemoveDirectory(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in_z LPCWSTR wzPath,
    __in BOOL fRecursive
    );
HRESULT DAPI MonRemoveRegKey(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle,
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in REG_KEY_BITNESS kbKeyBitness,
    __in BOOL fRecursive
    );
void DAPI MonDestroy(
    __in_bcount(MON_HANDLE_BYTES) MON_HANDLE handle
    );

#ifdef __cplusplus
}
#endif
