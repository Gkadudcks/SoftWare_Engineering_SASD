@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

gcc -std=c11 ..\main.c ..\rvc.c -o rvc_test.exe
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

set PASS=0
set FAIL=0
for /d %%D in (case*) do (
    pushd "%%D"
    ..\rvc_test.exe > output.txt
    fc output.txt expected.txt > nul
    if errorlevel 1 (
        echo [FAIL] %%D
        set /a FAIL+=1
    ) else (
        echo [PASS] %%D
        set /a PASS+=1
    )
    popd
)

echo.
echo PASS: !PASS!  FAIL: !FAIL!
