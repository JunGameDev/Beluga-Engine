CALL "$(VCINSTALLDIR)Auxiliary\Build\vcvars32.bat"

msbuild "Beluga.sln" /p:Platform="x86" /p:Configuration="Release" /m
if %ERRORLEVEL% NEQ 0 (
    ECHO Error building game!
    EXIT /B 1
)

vstest.console Beluga\Libs\x86_Release\BelugaTest.dll
IF %ERRORLEVEL% NEQ 0 (
    ECHO Error testing game!
    EXIT /B 1
)

rmdir /S /Q "Build"
mkdir "Build"

xcopy Project_Mario\Game\* Build\ /y /i /d /s /exclude:exclude.txt
xcopy Project_Mario\Game\x86_Release Build\ /y /i /d

del Build\BelugaTest.dll
del Build\log.txt
del Build\*pdb
del Build\*.ipdb
del Build\*.iobj
