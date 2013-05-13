enum RESOLUTION_CHOICE;
typedef void * CFGDB_HANDLE;
typedef void * CFG_ENUMERATION_HANDLE;

namespace CfgTests
{
    public ref class CfgTest
    {
    public:
        // Init / uninit
        void TestInitialize();
        void TestUninitialize();
        void RedirectDatabases();

        // Non-Cfg Commands
        void SetARP(LPCWSTR wzKeyName, LPCWSTR wzDisplayName, LPCWSTR wzInstallLocation, LPCWSTR wzUninstallString);
        void SetApplication(LPCWSTR wzFileName, LPCWSTR wzFilePath);
        void ResetApplications();

        // Cfg Commands
        void SyncIgnoreResolve(CFGDB_HANDLE cdhDb);
        void SyncNoResolve(CFGDB_HANDLE cdhDb);
        void SyncResolveAll(CFGDB_HANDLE cdhDb, RESOLUTION_CHOICE rcChoice);
        void ReadLatestLegacy(CFGDB_HANDLE cdhDb);

        // Plain read Expectations
        void ExpectProductRegistered(CFGDB_HANDLE cdhAdminDb, LPCWSTR wzProductName, LPCWSTR wzVersion, LPCWSTR wzPublicKey);
        void ExpectProductUnregistered(CFGDB_HANDLE cdhAdminDb, LPCWSTR wzProductName, LPCWSTR wzVersion, LPCWSTR wzPublicKey);
        void ExpectAdminProductRegistered(CFGDB_HANDLE cdhAdminDb, LPCWSTR wzProductName, LPCWSTR wzVersion, LPCWSTR wzPublicKey);
        void ExpectAdminProductUnregistered(CFGDB_HANDLE cdhAdminDb, LPCWSTR wzProductName, LPCWSTR wzVersion, LPCWSTR wzPublicKey);
        void CheckCfgAndRegValueFlag(CFGDB_HANDLE cdhDb, HKEY hk, LPCWSTR wzCfgName, LPCWSTR wzName, BOOL fExpectedValue, DWORD dwOffset);
        void CheckCfgAndRegValueString(CFGDB_HANDLE cdhDb, HKEY hk, LPCWSTR wzCfgName, LPCWSTR wzName, LPCWSTR wzExpectedValue);
        void CheckCfgAndRegValueDword(CFGDB_HANDLE cdhDb, HKEY hk, LPCWSTR wzCfgName, LPCWSTR wzName, DWORD dwExpectedValue);
        void CheckCfgAndRegValueDwordAsBool(CFGDB_HANDLE cdhDb, HKEY hk, LPCWSTR wzCfgName, LPCWSTR wzName, BOOL fExpectedValue);
        void CheckCfgAndRegValueQword(CFGDB_HANDLE cdhDb, HKEY hk, LPCWSTR wzCfgName, LPCWSTR wzName, DWORD64 qwExpectedValue);
        void CheckCfgAndRegValueDeleted(CFGDB_HANDLE cdhDb, HKEY hk, LPCWSTR wzCfgName, LPCWSTR wzName);
        void CheckCfgAndFile(CFGDB_HANDLE cdhDb, LPCWSTR wzFileName, LPCWSTR wzFilePath, BYTE *pbBuffer, SIZE_T cbBuffer);
        void CheckCfgAndFileDeleted(CFGDB_HANDLE cdhDb, LPCWSTR wzFileName, LPCWSTR wzFilePath);
        void ExpectString(CFGDB_HANDLE cdhDb, LPCWSTR wzValueName, LPCWSTR wzExpectedValue);
        void ExpectDword(CFGDB_HANDLE cdhDb, LPCWSTR wzValueName, DWORD dwExpectedValue);
        void ExpectBool(CFGDB_HANDLE cdhDb, LPCWSTR wzValueName, BOOL fExpectedValue);
        void ExpectNoValue(CFGDB_HANDLE cdhDb, LPCWSTR wzValueName);
        void ExpectFile(CFGDB_HANDLE cdhDb, LPCWSTR wzFileName, BYTE *pbBuffer, SIZE_T cbBuffer);
        void ExpectNoFile(CFGDB_HANDLE cdhDb, LPCWSTR wzFileName);
        void ExpectNoKey(HKEY hk, LPCWSTR wzKeyName);
        void ExpectIniValue(INI_HANDLE iniHandle, LPCWSTR wzValueName, LPCWSTR wzExpectedValue);
        void ExpectNoIniValue(INI_HANDLE iniHandle, LPCWSTR wzValueName);

        // Expectations for Enumerations
        void ExpectDatabaseInEnum(CFG_ENUMERATION_HANDLE cehHandle, LPCWSTR wzExpectedFriendlyName, BOOL fExpectedSyncByDefault, LPCWSTR wzExpectedPath);
        void ExpectNoDatabaseInEnum(CFG_ENUMERATION_HANDLE cehHandle, LPCWSTR wzExpectedFriendlyName);
    };
}
