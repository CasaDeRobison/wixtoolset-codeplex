#include "precomp.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Xunit;

namespace CfgTests
{
    public ref class LegacyDetectExe : public CfgTest
    {
    public:
        [Fact]
        void LegacyDetectExeTest()
        {
            HRESULT hr = S_OK;
            LPWSTR sczLegacySpecialsPath = NULL;
            LPWSTR sczDummyExePath = NULL;
            LPWSTR sczDummyIniPath = NULL;
            CFGDB_HANDLE cdhLocal = NULL;
            BYTE rgbFileA[1010] = { };
            rgbFileA[0] = 0xAA;
            rgbFileA[50] = 0xBB;

            TestInitialize();
            
            hr = CfgInitialize(&cdhLocal);
            ExitOnFailure(hr, "Failed to initialize user settings engine");
            
            hr = PathExpand(&sczLegacySpecialsPath, L"DetectExe.udm", PATH_EXPAND_FULLPATH);
            ExitOnFailure(hr, "Failed to get full path to detect exe legacy XML file");

            hr = PathExpand(&sczDummyExePath, L"%TEMP%\\Test.exe", PATH_EXPAND_ENVIRONMENT);
            ExitOnFailure(hr, "Failed to get full path to dummy exe");

            hr = PathExpand(&sczDummyIniPath, L"%TEMP%\\Test.ini", PATH_EXPAND_ENVIRONMENT);
            ExitOnFailure(hr, "Failed to get full path to dummy ini");

            hr = FileWrite(sczDummyExePath, 0, NULL, 0, NULL);
            ExitOnFailure(hr, "Failed to write blank dummy EXE to build output directory");

            hr = FileWrite(sczDummyIniPath, 0, rgbFileA, sizeof(rgbFileA), NULL);
            ExitOnFailure(hr, "Failed to write blank dummy EXE to build output directory");

            hr = CfgLegacyImportProductFromXMLFile(cdhLocal, sczLegacySpecialsPath);
            ExitOnFailure(hr, "Failed to load legacy product data from XML File");

            hr = CfgSetProduct(cdhLocal, L"CfgTestDetectExe", L"1.0.0.0", L"0000000000000000");
            ExitOnFailure(hr, "Failed to set product");

            ExpectProductUnregistered(cdhLocal, L"CfgTestDetectExe", L"1.0.0.0", L"0000000000000000");
            ExpectNoFile(cdhLocal, L"IniFile");

            ReadLatestLegacy(cdhLocal);
            ExpectProductUnregistered(cdhLocal, L"CfgTestDetectExe", L"1.0.0.0", L"0000000000000000");
            ExpectNoFile(cdhLocal, L"IniFile");

            SetApplication(L"Test.exe", sczDummyExePath);

            ReadLatestLegacy(cdhLocal);
            ExpectProductRegistered(cdhLocal, L"CfgTestDetectExe", L"1.0.0.0", L"0000000000000000");
            ExpectFile(cdhLocal, L"IniFile", rgbFileA, sizeof(rgbFileA));

            hr = CfgUninitialize(cdhLocal);
            ExitOnFailure(hr, "Failed to shutdown user settings engine");

        LExit:
            ReleaseStr(sczLegacySpecialsPath);
            ReleaseStr(sczDummyExePath);
            ReleaseStr(sczDummyIniPath);
            TestUninitialize();
        }
    };
}
