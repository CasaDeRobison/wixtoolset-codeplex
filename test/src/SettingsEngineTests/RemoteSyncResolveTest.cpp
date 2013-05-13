#include "precomp.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Xunit;

namespace CfgTests
{
    public ref class RemoteSyncResolve : public CfgTest
    {
    public: 
        HRESULT GetRemotePaths(
            __out_z LPWSTR *psczRemote1,
            __out_z LPWSTR *psczRemote2
            )
        {
            HRESULT hr = S_OK;
            LPWSTR sczDirRemote1 = NULL;
            LPWSTR sczDirRemote2 = NULL;

            hr = PathExpand(&sczDirRemote1, L"%TEMP%\\TestRemote1\\", PATH_EXPAND_ENVIRONMENT);
            ExitOnFailure(hr, "Failed to expand path to remote1 database");

            hr = PathExpand(&sczDirRemote2, L"%TEMP%\\TestRemote2\\", PATH_EXPAND_ENVIRONMENT);
            ExitOnFailure(hr, "Failed to expand path to remote2 database");

            hr = PathConcat(sczDirRemote1, L"Remote1.sdf", psczRemote1);
            ExitOnFailure(hr, "Failed to concat path to remote1 database");

            hr = PathConcat(sczDirRemote2, L"Remote2.sdf", psczRemote2);
            ExitOnFailure(hr, "Failed to concat path to remote2 database");

        LExit:
            ReleaseStr(sczDirRemote1);
            ReleaseStr(sczDirRemote2);

            return hr;
        }

        HRESULT DeleteRemoteDatabases()
        {
            HRESULT hr = S_OK;
            LPWSTR sczDirRemote1 = NULL;
            LPWSTR sczDirRemote2 = NULL;

            hr = PathExpand(&sczDirRemote1, L"%TEMP%\\TestRemote1\\", PATH_EXPAND_ENVIRONMENT);
            ExitOnFailure(hr, "Failed to expand path to remote1 database");

            hr = PathExpand(&sczDirRemote2, L"%TEMP%\\TestRemote2\\", PATH_EXPAND_ENVIRONMENT);
            ExitOnFailure(hr, "Failed to expand path to remote2 database");

            hr = DirEnsureDelete(sczDirRemote1, TRUE, TRUE);
            if (E_PATHNOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to delete remote1 database directory");
            }
            hr = S_OK;

            hr = DirEnsureDelete(sczDirRemote2, TRUE, TRUE);
            if (E_PATHNOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to delete remote2 database directory");
            }
            hr = S_OK;

        LExit:
            ReleaseStr(sczDirRemote1);
            ReleaseStr(sczDirRemote2);

            return hr;
        }

        void TestSyncFilesWithoutResolve(
            CFGDB_HANDLE cdhLocal,
            CFGDB_HANDLE cdhRemote1,
            CFGDB_HANDLE cdhRemote2
            )
        {
            HRESULT hr = S_OK;
            CONFLICT_PRODUCT *pcplProductList = NULL;
            DWORD dwConflictProductCount = 0;
            BYTE rgbFile1[150] = { };
            rgbFile1[0] = 0x89;
            rgbFile1[10] = 0x98;
            BYTE rgbFile2[250] = { };
            rgbFile2[0] = 0x44;
            rgbFile2[10] = 0x55;
            BYTE rgbFile3[100] = { };
            rgbFile3[0] = 0x11;
            rgbFile3[10] = 0x22;

            hr = CfgSetProduct(cdhLocal, L"TestRemoteSyncNoResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in local db");

            // Make sure remote2 gets different AppID's than local
            hr = CfgSetProduct(cdhRemote2, L"DummyProductWithNoValues", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 1");

            hr = CfgSetBlob(cdhLocal, L"File1", rgbFile1, sizeof(rgbFile1));
            ExitOnFailure(hr, "Failed to set file File1");

            hr = CfgSetProduct(cdhRemote2, L"TestRemoteSyncNoResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set remote2's product");

            hr = CfgSync(cdhRemote1, &pcplProductList, &dwConflictProductCount);
            ExitOnFailure(hr, "Failed to sync with remote1");

            if (0 != dwConflictProductCount || NULL != pcplProductList)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "There shouldn't be any conflicts here!")
            }

            hr = CfgSync(cdhRemote2, &pcplProductList, &dwConflictProductCount);
            ExitOnFailure(hr, "Failed to sync with remote2");

            if (0 != dwConflictProductCount || NULL != pcplProductList)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "There shouldn't be any conflicts here!")
            }

            hr = CfgSetProduct(cdhRemote1, L"TestRemoteSyncNoResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 1");

            ExpectFile(cdhRemote2, L"File1", rgbFile1, sizeof(rgbFile1));

            hr = CfgDeleteValue(cdhRemote2, L"File1");
            ExitOnFailure(hr, "Failed to delete File1 from remote2");

            hr = CfgSync(cdhRemote2, &pcplProductList, &dwConflictProductCount);
            ExitOnFailure(hr, "Failed to sync with remote2");

            ExpectNoFile(cdhLocal, L"File1");
            ExpectNoFile(cdhRemote2, L"File1");

            hr = CfgSetBlob(cdhLocal, L"File1", rgbFile3, sizeof(rgbFile3));
            ExitOnFailure(hr, "Failed to set file1 in local DB");

            hr = CfgSetBlob(cdhRemote2, L"File1", rgbFile2, sizeof(rgbFile2));
            ExitOnFailure(hr, "Failed to set file1 in remote DB 2");

            // Test that if there is a conflict we don't explicitly resolve, the file doesn't sync over.
            SyncIgnoreResolve(cdhRemote2);
            ExitOnFailure(hr, "Failed to sync (ignoring conflicts) with remote2");

            ExpectFile(cdhLocal, L"File1", rgbFile3, sizeof(rgbFile3));
            ExpectFile(cdhRemote2, L"File1", rgbFile2, sizeof(rgbFile2));

        LExit:
            return;
        }

        void TestSyncValuesWithoutResolve(
            CFGDB_HANDLE cdhLocal,
            CFGDB_HANDLE cdhRemote1,
            CFGDB_HANDLE cdhRemote2
            )
        {
            HRESULT hr = S_OK;
            LPWSTR sczValue = NULL;
            CONFLICT_PRODUCT *pcplProductList = NULL;
            DWORD dwConflictProductCount = 0;
            DWORD dwValue = 0;
            BOOL fValue = FALSE;

            hr = CfgSetProduct(cdhLocal, L"TestRemoteSyncNoResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in local db");

            // Make sure remote2 gets different AppID's than local
            hr = CfgSetProduct(cdhRemote2, L"DummyProductWithNoValues", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 1");

            hr = CfgSetString(cdhLocal, L"String1", L"StringValue1");
            ExitOnFailure(hr, "Failed to set string1 in local DB");

            hr = CfgSetString(cdhLocal, L"String2", L"StringValue2");
            ExitOnFailure(hr, "Failed to set string2 in local DB");

            hr = CfgSetDword(cdhLocal, L"Dword1", 100);
            ExitOnFailure(hr, "Failed to set dword in local DB");

            hr = CfgSetBool(cdhLocal, L"Bool1", TRUE);
            ExitOnFailure(hr, "Failed to set bool in local DB");

            hr = CfgSetProduct(cdhRemote2, L"TestRemoteSyncNoResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set remote2's product");

            hr = CfgSync(cdhRemote1, &pcplProductList, &dwConflictProductCount);
            ExitOnFailure(hr, "Failed to sync with remote1");

            if (0 != dwConflictProductCount || NULL != pcplProductList)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "There shouldn't be any conflicts here!")
            }

            hr = CfgSync(cdhRemote2, &pcplProductList, &dwConflictProductCount);
            ExitOnFailure(hr, "Failed to sync with remote2");

            if (0 != dwConflictProductCount || NULL != pcplProductList)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "There shouldn't be any conflicts here!")
            }

            hr = CfgSetProduct(cdhRemote1, L"TestRemoteSyncNoResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 1");

            hr = CfgGetString(cdhRemote1, L"String1", &sczValue);
            ExitOnFailure(hr, "Failed to get string1 from remote1");

            if (0 != lstrcmpW(sczValue, L"StringValue1"))
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "String1's value in Remote1 is incorrect - should be: StringValue1, instead is: %ls", sczValue);
            }

            hr = CfgSetProduct(cdhRemote1, L"TestRemoteSyncNoResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 2");

            hr = CfgGetString(cdhRemote2, L"String2", &sczValue);
            ExitOnFailure(hr, "Failed to get string2 from remote2");

            if (0 != lstrcmpW(sczValue, L"StringValue2"))
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "String2's value in Remote2 is incorrect - should be: StringValue2, instead is: %ls", sczValue);
            }

            hr = CfgGetDword(cdhRemote1, L"Dword1", &dwValue);
            ExitOnFailure(hr, "Failed to get dword1 from remote1");

            if (100 != dwValue)
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "Dword1's value in Remote1 is incorrect - should be: 100, instead is: %u", dwValue);
            }

            hr = CfgGetBool(cdhRemote2, L"Bool1", &fValue);
            ExitOnFailure(hr, "Failed to get bool1 from remote2");

            if (TRUE != fValue)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Bool1's value should have been TRUE!");
            }

            hr = CfgDeleteValue(cdhRemote1, L"String2");
            ExitOnFailure(hr, "Failed to delete value string2 from remote 1");

            hr = CfgGetString(cdhLocal, L"String2", &sczValue);
            ExitOnFailure(hr, "Failed to get string2 from local database");

            hr = CfgSync(cdhRemote1, &pcplProductList, &dwConflictProductCount);
            ExitOnFailure(hr, "Failed to sync with remote1");

            if (0 != dwConflictProductCount || NULL != pcplProductList)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "There shouldn't be any conflicts here!")
            }

            hr = CfgGetString(cdhLocal, L"String2", &sczValue);
            if (E_NOTFOUND != hr)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Shouldn't have found String2 in local DB after sync!");
            }

            hr = CfgGetString(cdhRemote2, L"String2", &sczValue);
            ExitOnFailure(hr, "Failed to get string2 from remote 2");
            
            hr = CfgSync(cdhRemote2, &pcplProductList, &dwConflictProductCount);
            ExitOnFailure(hr, "Failed to sync with remote1");

            if (0 != dwConflictProductCount || NULL != pcplProductList)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "There shouldn't be any conflicts here!")
            }

            hr = CfgGetString(cdhRemote2, L"String2", &sczValue);
            if (E_NOTFOUND != hr)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Shouldn't have found String2 in remote2 DB after sync!");
            }

        LExit:
            ReleaseStr(sczValue);
        }

        void TestSyncValuesWithResolve(
            CFGDB_HANDLE cdhLocal,
            CFGDB_HANDLE cdhRemote1,
            CFGDB_HANDLE cdhRemote2
            )
        {
            HRESULT hr = S_OK;
            LPWSTR sczValue = NULL;

            hr = CfgSetProduct(cdhLocal, L"TestRemoteSyncWithResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in local db");

            hr = CfgSetProduct(cdhRemote1, L"TestRemoteSyncWithResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 1");

            hr = CfgSetProduct(cdhRemote2, L"TestRemoteSyncWithResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 2");

            hr = CfgSetString(cdhLocal, L"String1", L"StringValueLocal");
            ExitOnFailure(hr, "Failed to set string1 in local db");

            hr = CfgSetString(cdhRemote2, L"String1", L"StringValueRemote2");
            ExitOnFailure(hr, "Failed to set string1 in remote 2 db");

            SyncResolveAll(cdhRemote2, RESOLUTION_LOCAL);

            hr = CfgGetString(cdhLocal, L"String1", &sczValue);
            ExitOnFailure(hr, "Failed to get string1 from local db");

            if (0 != lstrcmpW(sczValue, L"StringValueLocal"))
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "Local db's string1 value should be StringValueLocal, was instead: %ls", sczValue);
            }

            hr = CfgGetString(cdhRemote2, L"String1", &sczValue);
            ExitOnFailure(hr, "Failed to get string1 from remote db 2");

            if (0 != lstrcmpW(sczValue, L"StringValueLocal"))
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "Remote db 2's string1 value should be StringValueLocal, was instead: %ls", sczValue);
            }

            hr = CfgSetString(cdhLocal, L"String1", L"StringValueLocalNew");
            ExitOnFailure(hr, "Failed to set string1 in local db");

            hr = CfgSetString(cdhRemote2, L"String1", L"StringValueRemote3");
            ExitOnFailure(hr, "Failed to set string1 in remote 2 db");

            SyncResolveAll(cdhRemote2, RESOLUTION_REMOTE);

            hr = CfgGetString(cdhRemote2, L"String1", &sczValue);
            ExitOnFailure(hr, "Failed to get string1 from remote db 2");

            if (0 != lstrcmpW(sczValue, L"StringValueRemote3"))
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "Remote db 2's string1 value should be StringValueRemote3, was instead: %ls", sczValue);
            }

            hr = CfgGetString(cdhLocal, L"String1", &sczValue);
            ExitOnFailure(hr, "Failed to get string1 from local db");

            if (0 != lstrcmpW(sczValue, L"StringValueRemote3"))
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "Local db's string1 value should be StringValueRemote3, was instead: %ls", sczValue);
            }

            hr = CfgDeleteValue(cdhLocal, L"String1");
            ExitOnFailure(hr, "Failed to delete string1 from local db");

            hr = CfgSetString(cdhRemote2, L"String1", L"Resurrected");
            ExitOnFailure(hr, "Failed to set string1 in remote 2 db");

            SyncResolveAll(cdhRemote2, RESOLUTION_REMOTE);

            hr = CfgGetString(cdhRemote2, L"String1", &sczValue);
            ExitOnFailure(hr, "Failed to get string1 from remote db 2");

            if (0 != lstrcmpW(sczValue, L"Resurrected"))
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "Remote db 2's string1 value should be Resurrected, was instead: %ls", sczValue);
            }

            hr = CfgGetString(cdhLocal, L"String1", &sczValue);
            ExitOnFailure(hr, "Failed to get string1 from local db");

            if (0 != lstrcmpW(sczValue, L"Resurrected"))
            {
                hr = E_FAIL;
                ExitOnFailure1(hr, "Local db's string1 value should be Resurrected, was instead: %ls", sczValue);
            }

            hr = CfgDeleteValue(cdhLocal, L"String1");
            ExitOnFailure(hr, "Failed to delete string1 from local db");

            hr = CfgSetString(cdhRemote2, L"String1", L"Resurrected2");
            ExitOnFailure(hr, "Failed to set string1 in remote 2 db");

            SyncResolveAll(cdhRemote2, RESOLUTION_LOCAL);

            hr = CfgGetString(cdhLocal, L"String1", &sczValue);
            if (E_NOTFOUND != hr)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Shouldn't have found String1 in local DB after sync / resolve!");
            }

            hr = CfgGetString(cdhRemote2, L"String1", &sczValue);
            if (E_NOTFOUND != hr)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Shouldn't have found String1 in remote 2 DB after sync / resolve!");
            }

        LExit:
            ReleaseStr(sczValue);
        }

        void TestSyncFilesWithResolve(
            CFGDB_HANDLE cdhLocal,
            CFGDB_HANDLE cdhRemote1,
            CFGDB_HANDLE cdhRemote2
            )
        {
            HRESULT hr = S_OK;
            BYTE rgbFile1[110] = { };
            rgbFile1[0] = 0x89;
            rgbFile1[10] = 0x98;
            BYTE rgbFile2[120] = { };
            rgbFile2[0] = 0x44;
            rgbFile2[10] = 0x55;
            BYTE rgbFile3[130] = { };
            rgbFile3[0] = 0x11;
            rgbFile3[10] = 0x22;
            BYTE rgbFile4[140] = { };
            rgbFile4[0] = 0x33;
            rgbFile4[10] = 0x44;

            hr = CfgSetProduct(cdhLocal, L"TestRemoteSyncWithResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in local db");

            hr = CfgSetProduct(cdhRemote1, L"TestRemoteSyncWithResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 1");

            hr = CfgSetProduct(cdhRemote2, L"TestRemoteSyncWithResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set remote2's product");

            hr = CfgSetBlob(cdhLocal, L"File1", rgbFile1, sizeof(rgbFile1));
            ExitOnFailure(hr, "Failed to set file File1");

            hr = CfgSetBlob(cdhRemote2, L"File1", rgbFile2, sizeof(rgbFile2));
            ExitOnFailure(hr, "Failed to set file File1");

            SyncResolveAll(cdhRemote2, RESOLUTION_REMOTE);

            ExpectFile(cdhLocal, L"File1", rgbFile2, sizeof(rgbFile2));
            ExpectFile(cdhRemote2, L"File1", rgbFile2, sizeof(rgbFile2));

            hr = CfgSetBlob(cdhLocal, L"File1", rgbFile1, sizeof(rgbFile1));
            ExitOnFailure(hr, "Failed to set file File1");

            hr = CfgSetBlob(cdhRemote2, L"File1", rgbFile3, sizeof(rgbFile3));
            ExitOnFailure(hr, "Failed to set file File1");

            SyncResolveAll(cdhRemote2, RESOLUTION_LOCAL);
            ExitOnFailure(hr, "Failed to sync with remote2");

            ExpectFile(cdhLocal, L"File1", rgbFile1, sizeof(rgbFile1));
            ExpectFile(cdhRemote2, L"File1", rgbFile1, sizeof(rgbFile1));

            hr = CfgSetBlob(cdhLocal, L"File1", rgbFile3, sizeof(rgbFile3));
            ExitOnFailure(hr, "Failed to set file File1");

            hr = CfgSetBlob(cdhRemote2, L"File1", rgbFile3, sizeof(rgbFile3));
            ExitOnFailure(hr, "Failed to set file File1");

            hr = CfgDeleteValue(cdhRemote2, L"File1");
            ExitOnFailure(hr, "Failed to set file File1");

            SyncResolveAll(cdhRemote2, RESOLUTION_REMOTE);
            ExitOnFailure(hr, "Failed to sync with remote2");

            ExpectNoFile(cdhLocal, L"File1");
            ExpectNoFile(cdhRemote2, L"File1");

        LExit:
            return;
        }

        void TestSyncMissingProducts(
            CFGDB_HANDLE cdhLocal,
            CFGDB_HANDLE cdhRemote1
            )
        {
            HRESULT hr = S_OK;
            LPWSTR sczValue = NULL;
            BYTE rgbFile1[450] = { };
            rgbFile1[0] = 0x45;
            rgbFile1[10] = 0x56;
            BYTE rgbFile2[460] = { };
            rgbFile2[0] = 0x46;
            rgbFile2[10] = 0x57;

            hr = CfgSetProduct(cdhLocal, L"TestRemoteSyncProductNotRegistered", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in local db");

            hr = CfgSetProduct(cdhRemote1, L"TestRemoteSyncProductNotRegistered", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to set product in remote db 1");

            hr = CfgSetString(cdhRemote1, L"String1", L"StringValueRemote1");
            ExitOnFailure(hr, "Failed to set string1 in remote 2 db");

            hr = CfgSetBlob(cdhRemote1, L"File1", rgbFile1, sizeof(rgbFile1));
            ExitOnFailure(hr, "Failed to set file File1");

            SyncNoResolve(cdhRemote1);
            ExpectNoValue(cdhLocal, L"String1");
            ExpectNoFile(cdhLocal, L"File1");
            ExpectString(cdhRemote1, L"String1", L"StringValueRemote1");
            ExpectFile(cdhRemote1, L"File1", rgbFile1, sizeof(rgbFile1));

            hr = CfgGetString(cdhRemote1, L"String1", &sczValue);
            ExitOnFailure(hr, "Failed to get string from remote DB");

            hr = CfgSetString(cdhLocal, L"String2", L"StringFromLocal");
            ExitOnFailure(hr, "Failed to set string2");

            hr = CfgSetBlob(cdhLocal, L"File2", rgbFile2, sizeof(rgbFile2));
            ExitOnFailure(hr, "Failed to set file2");

            SyncNoResolve(cdhRemote1);
            ExpectNoValue(cdhLocal, L"String1");
            ExpectNoFile(cdhLocal, L"File1");
            ExpectString(cdhRemote1, L"String1", L"StringValueRemote1");
            ExpectFile(cdhRemote1, L"File1", rgbFile1, sizeof(rgbFile1));
            ExpectString(cdhLocal, L"String2", L"StringFromLocal");
            ExpectString(cdhRemote1, L"String2", L"StringFromLocal");
            ExpectFile(cdhLocal, L"File2", rgbFile2, sizeof(rgbFile2));
            ExpectFile(cdhRemote1, L"File2", rgbFile2, sizeof(rgbFile2));

        LExit:
            ReleaseStr(sczValue);
            return;
        }

        [Fact]
        void RemoteSyncResolveTest()
        {
            HRESULT hr = S_OK;
            LPWSTR sczPathRemote1 = NULL;
            LPWSTR sczPathRemote2 = NULL;
            CFGDB_HANDLE cdhAdmin = NULL;
            CFGDB_HANDLE cdhLocal = NULL;
            CFGDB_HANDLE cdhRemote1 = NULL;
            CFGDB_HANDLE cdhRemote2 = NULL;
            CFG_ENUMERATION_HANDLE cehDatabaseList = NULL;

            hr = GetRemotePaths(&sczPathRemote1, &sczPathRemote2);
            ExitOnFailure(hr, "Failed to get remote paths");
                
            hr = DeleteRemoteDatabases();
            ExitOnFailure(hr, "Failed to delete remote databases");

            TestInitialize();

            hr = CfgAdminInitialize(&cdhAdmin, TRUE);
            ExitOnFailure(hr, "Failed to initialize admin settings engine");

            hr = CfgInitialize(&cdhLocal);
            ExitOnFailure(hr, "Failed to initialize user settings engine");

            hr = CfgEnumDatabaseList(cdhLocal, &cehDatabaseList, NULL);
            ExitOnFailure(hr, "Failed to enumerate list of databases");
            if (NULL != cehDatabaseList)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Database list should have been empty!");
            }

            hr = CfgCreateRemoteDatabase(sczPathRemote1, &cdhRemote1);
            ExitOnFailure(hr, "Failed to create remote1 database");
            
            hr = CfgRememberDatabase(cdhLocal, cdhRemote1, L"Remote1", TRUE);
            ExitOnFailure(hr, "Failed to record remote database 1 in database list");

            hr = CfgEnumDatabaseList(cdhLocal, &cehDatabaseList, NULL);
            ExitOnFailure(hr, "Failed to enumerate list of databases");
            ExpectDatabaseInEnum(cehDatabaseList, L"Remote1", TRUE, sczPathRemote1);
            ExpectNoDatabaseInEnum(cehDatabaseList, L"Remote2");
            CfgReleaseEnumeration(cehDatabaseList);
            cehDatabaseList = NULL;

            hr = CfgRememberDatabase(cdhLocal, cdhRemote1, L"Remote1", FALSE);
            ExitOnFailure(hr, "Failed to record remote database 1 in database list");

            hr = CfgEnumDatabaseList(cdhLocal, &cehDatabaseList, NULL);
            ExitOnFailure(hr, "Failed to enumerate list of databases");
            ExpectDatabaseInEnum(cehDatabaseList, L"Remote1", FALSE, sczPathRemote1);
            ExpectNoDatabaseInEnum(cehDatabaseList, L"Remote2");
            CfgReleaseEnumeration(cehDatabaseList);
            cehDatabaseList = NULL;

            hr = CfgRememberDatabase(cdhLocal, cdhRemote1, L"Remote1", TRUE);
            ExitOnFailure(hr, "Failed to record remote database 1 in database list");

            hr = CfgCreateRemoteDatabase(sczPathRemote2, &cdhRemote2);
            ExitOnFailure(hr, "Failed to create remote2 database (in 'forget' mode)");

            hr = CfgEnumDatabaseList(cdhLocal, &cehDatabaseList, NULL);
            ExitOnFailure(hr, "Failed to enumerate list of databases");
            ExpectDatabaseInEnum(cehDatabaseList, L"Remote1", TRUE, sczPathRemote1);
            ExpectNoDatabaseInEnum(cehDatabaseList, L"Remote2");
            CfgReleaseEnumeration(cehDatabaseList);
            cehDatabaseList = NULL;

            hr = CfgRememberDatabase(cdhLocal, cdhRemote2, L"Remote2", FALSE);
            ExitOnFailure(hr, "Failed to record remote database 2 in database list");

            hr = CfgEnumDatabaseList(cdhLocal, &cehDatabaseList, NULL);
            ExitOnFailure(hr, "Failed to enumerate list of databases");
            ExpectDatabaseInEnum(cehDatabaseList, L"Remote1", TRUE, sczPathRemote1);
            ExpectDatabaseInEnum(cehDatabaseList, L"Remote2", FALSE, sczPathRemote2);
            CfgReleaseEnumeration(cehDatabaseList);
            cehDatabaseList = NULL;

            hr = CfgForgetDatabase(cdhLocal, L"Remote2");
            ExitOnFailure(hr, "Failed to forget remote database 2 from database list");

            hr = CfgEnumDatabaseList(cdhLocal, &cehDatabaseList, NULL);
            ExitOnFailure(hr, "Failed to enumerate list of databases");
            ExpectDatabaseInEnum(cehDatabaseList, L"Remote1", TRUE, sczPathRemote1);
            ExpectNoDatabaseInEnum(cehDatabaseList, L"Remote2");
            CfgReleaseEnumeration(cehDatabaseList);
            cehDatabaseList = NULL;

            hr = CfgAdminRegisterProduct(cdhAdmin, L"TestRemoteSyncWithResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to register withresolve product");

            hr = CfgAdminRegisterProduct(cdhAdmin, L"TestRemoteSyncNoResolve", L"1.0.0.0", L"abcdabcdabcdabcd");
            ExitOnFailure(hr, "Failed to register noresolve product");

            hr = CfgRemoteDisconnect(cdhRemote1);
            ExitOnFailure(hr, "Failed to disconnect remote database 1");

            hr = CfgOpenKnownRemoteDatabase(cdhLocal, L"Remote1", &cdhRemote1);
            ExitOnFailure(hr, "Failed to re-connect to remote database 1 from database list");

            hr = CfgEnumDatabaseList(cdhLocal, &cehDatabaseList, NULL);
            ExitOnFailure(hr, "Failed to enumerate list of databases");
            ExpectDatabaseInEnum(cehDatabaseList, L"Remote1", TRUE, sczPathRemote1);
            ExpectNoDatabaseInEnum(cehDatabaseList, L"Remote2");
            CfgReleaseEnumeration(cehDatabaseList);
            cehDatabaseList = NULL;

            TestSyncValuesWithoutResolve(cdhLocal, cdhRemote1, cdhRemote2);
            TestSyncFilesWithoutResolve(cdhLocal, cdhRemote1, cdhRemote2);
            TestSyncValuesWithResolve(cdhLocal, cdhRemote1, cdhRemote2);
            TestSyncFilesWithResolve(cdhLocal, cdhRemote1, cdhRemote2);

            TestSyncMissingProducts(cdhLocal, cdhRemote1);

            hr = CfgRemoteDisconnect(cdhRemote1);
            ExitOnFailure(hr, "Failed to disconnect remote database 1");

            hr = CfgRemoteDisconnect(cdhRemote2);
            ExitOnFailure(hr, "Failed to disconnect remote database 2");

            hr = CfgOpenKnownRemoteDatabase(cdhLocal, L"Remote2", &cdhRemote2);
            if (E_NOTFOUND != hr)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Should not be able to open Remote2 from known remote database list!");
            }

            hr = CfgUninitialize(cdhLocal);
            ExitOnFailure(hr, "Failed to shutdown user settings engine");

            hr = CfgAdminUninitialize(cdhAdmin);
            ExitOnFailure(hr, "Failed to shutdown admin settings engine");

        LExit:
            ReleaseStr(sczPathRemote1);
            ReleaseStr(sczPathRemote2);
            TestUninitialize();
        }
    };
}
