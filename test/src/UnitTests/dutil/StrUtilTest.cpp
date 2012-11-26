//-------------------------------------------------------------------------------------------------
// <copyright file="StrUtilTest.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

using namespace System;
using namespace System::Text;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;

namespace CfgTests
{
    [TestClass]
    public ref class StrUtil
    {
    public:
        [TestMethod]
        void StrUtilFormattedTest()
        {
            HRESULT hr = S_OK;
            LPWSTR sczText = NULL;

            hr = StrAllocFormatted(&sczText, L"%hs - %ls - %u", "ansi string", L"unicode string", 1234);
            ExitOnFailure(hr, "Failed to format string.");
            Assert::AreEqual(gcnew String(L"ansi string - unicode string - 1234"), gcnew String(sczText));

            ReleaseNullStr(sczText);

            hr = StrAllocString(&sczText, L"repeat", 0);
            ExitOnFailure(hr, "Failed to allocate string.");

            hr = StrAllocFormatted(&sczText, L"%ls and %ls", sczText, sczText);
            ExitOnFailure(hr, "Failed to format string unto itself.");
            Assert::AreEqual(gcnew String(L"repeat and repeat"), gcnew String(sczText));

        LExit:
            ReleaseStr(sczText);
        }

        [TestMethod]
        void StrUtilTrimTest()
        {
            TestTrim(L"", L"");
            TestTrim(L"Blah", L"Blah");
            TestTrim(L"\t\t\tBlah", L"Blah");
            TestTrim(L"\t    Blah     ", L"Blah");
            TestTrim(L"Blah     ", L"Blah");
            TestTrim(L"\t  Spaces  \t   Between   \t", L"Spaces  \t   Between");
            TestTrim(L"    \t\t\t    ", L"");

            TestTrimAnsi("", "");
            TestTrimAnsi("Blah", "Blah");
            TestTrimAnsi("\t\t\tBlah", "Blah");
            TestTrimAnsi("    Blah     ", "Blah");
            TestTrimAnsi("Blah     ", "Blah");
            TestTrimAnsi("\t  Spaces  \t   Between   \t", "Spaces  \t   Between");
            TestTrimAnsi("    \t\t\t    ", "");
        }

    private:
        void TestTrim(LPCWSTR wzInput, LPCWSTR wzExpectedResult)
        {
            HRESULT hr = S_OK;
            LPWSTR sczOutput = NULL;

            hr = StrTrimWhitespace(&sczOutput, wzInput);
            ExitOnFailure1(hr, "Failed to trim whitespace from string: %ls", wzInput);

            if (0 != wcscmp(wzExpectedResult, sczOutput))
            {
                hr = E_FAIL;
                ExitOnFailure3(hr, "Trimmed string \"%ls\", expected result \"%ls\", actual result \"%ls\"", wzInput, wzExpectedResult, sczOutput);
            }

        LExit:
            ReleaseStr(sczOutput);
        }

        void TestTrimAnsi(LPCSTR szInput, LPCSTR szExpectedResult)
        {
            HRESULT hr = S_OK;
            LPSTR sczOutput = NULL;

            hr = StrAnsiTrimWhitespace(&sczOutput, szInput);
            ExitOnFailure1(hr, "Failed to trim whitespace from string: \"%hs\"", szInput);

            if (0 != strcmp(szExpectedResult, sczOutput))
            {
                hr = E_FAIL;
                ExitOnFailure3(hr, "Trimmed string \"%hs\", expected result \"%hs\", actual result \"%ls\"", szInput, szExpectedResult, sczOutput);
            }

        LExit:
            ReleaseStr(sczOutput);
        }
    };
}
