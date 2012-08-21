
@echo off
setlocal

set _THIS=%~dp0
pushd %_THIS%

if ""=="%WIX_BUILDROOT%" set WIX_BUILDROOT=%WIX_ROOT%\build\
set _B=..\..\build\
set _BP=%_B%obj\packages\
set _T=%WIX_BUILDROOT%debug\x86\

%_T%candle.exe Dv1-Patch.wxs -o %_BP%\
%_T%light.exe %_BP%\Dv1-Patch.wixobj -out %_BP%\Dv1-Patch.wixmsp
%_T%torch.exe -p -xi %_BP%\Dv1.wixpdb %_BP%\Dv1.0.1-PerMachine\Dv1.wixpdb -out %_BP%\Dv1.0.1.wixmst
%_T%pyro.exe %_BP%\Dv1-Patch.wixmsp -t RTM %_BP%\Dv1.0.1.wixmst -out %_BP%\Dv1-Patch.msp
