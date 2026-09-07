@echo off
setlocal

set "ScriptDir=%~dp0"
set "TargetArg=%~1"

if "%TargetArg%"=="" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%ScriptDir%Generate-VcxprojFilters.ps1"
) else (
    if /I "%~x1"==".vcxproj" (
        powershell -NoProfile -ExecutionPolicy Bypass -File "%ScriptDir%Generate-VcxprojFilters.ps1" -VcxprojPath "%TargetArg%"
    ) else (
        powershell -NoProfile -ExecutionPolicy Bypass -File "%ScriptDir%Generate-VcxprojFilters.ps1" -Profile "%TargetArg%"
    )
)

exit /b %ERRORLEVEL%
