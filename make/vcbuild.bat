@ECHO OFF

REM Allow to explicitly specify the desired Visual Studio version
IF /I "%1" == "vc12" GOTO TRY_VS12
IF /I "%1" == "vc10" GOTO TRY_VS10
IF /I "%1" == "vc9" GOTO TRY_VS9

REM vs9 is VS 2008
:TRY_VS9
CALL "%VS90COMNTOOLS%\vsvars32.bat" 2>NUL
IF NOT ERRORLEVEL 1 GOTO BUILD

REM vs10 is VS 2010
:TRY_VS10
CALL "%VS100COMNTOOLS%\vsvars32.bat" 2>NUL
IF NOT ERRORLEVEL 1 GOTO BUILD

REM vs12 is VS 2012
:TRY_VS12
CALL "%VS110COMNTOOLS%\vsvars32.bat" 2>NUL
IF NOT ERRORLEVEL 1 GOTO BUILD

ECHO Visual Studio 2012, 2010, or 2008 doesn't seem to be installed
EXIT /B 1

:BUILD
nmake -f makefile.vc all

