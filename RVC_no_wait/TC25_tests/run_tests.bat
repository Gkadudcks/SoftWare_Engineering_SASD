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
set LOG=%~dp0all_outputs.txt
if exist "%LOG%" del "%LOG%"

for /d %%D in (TC*) do (
    pushd "%%D"
    ..\rvc_test.exe %%D > output.txt
    powershell -NoProfile -Command "(Get-Content -LiteralPath 'output.txt') -replace 'ASYNC STOP @ \d+ms','ASYNC STOP @ Xms' | Set-Content -LiteralPath 'output_norm.txt' -Encoding ascii"
    fc output_norm.txt expected.txt > nul
    if errorlevel 1 (
        set RESULT=FAIL
        set /a FAIL+=1
    ) else (
        set RESULT=PASS
        set /a PASS+=1
    )
    echo.
    echo ############################################################
    echo # %%D  -  !RESULT!
    echo ############################################################
    type output.txt
    >>"%LOG%" echo ############################################################
    >>"%LOG%" echo # %%D  -  !RESULT!
    >>"%LOG%" echo ############################################################
    >>"%LOG%" type output.txt
    >>"%LOG%" echo.
    popd
)

echo.
echo ============================================================
echo PASS: !PASS!  FAIL: !FAIL!
echo All outputs saved to all_outputs.txt
>>"%LOG%" echo PASS: !PASS!  FAIL: !FAIL!
