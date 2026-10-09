@echo off
setlocal
if "%~1"=="" (
  echo Usage: run_case.bat TC02_turn_left_5ticks
  exit /b 1
)
if not exist "%~dp0%~1\sensors.txt" (
  echo Unknown case: %~1
  exit /b 1
)
if not exist "%~dp0..\rvc.exe" (
  echo Build project first: gcc -std=c11 main.c rvc.c -o rvc.exe
  exit /b 1
)
pushd "%~dp0%~1"
"%~dp0..\rvc.exe"
set "EXITCODE=%ERRORLEVEL%"
popd
exit /b %EXITCODE%
