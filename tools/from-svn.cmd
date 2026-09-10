@echo off
setlocal enabledelayedexpansion

if "%~1"=="" (
    echo Usage: %~nx0 ^<svn-url^> [folder-name]
    echo   folder-name - optional, if not specified uses last part of URL
    exit /b 1
)

set "url=%~1"

:: Если указан второй параметр, используем его как имя папки
if not "%~2"=="" (
    set "repo_name=%~2"
) else (
    :: Иначе извлекаем последний сегмент URL
    for %%a in ("%url:\=" "%") do set "repo_name=%%~a"
    set "repo_name=!repo_name:"=!"
)

if not exist libraries (
    mkdir libraries
    if errorlevel 1 (
        echo Error: Failed to create 'libraries' directory
        exit /b 1
    )
)

if exist "libraries\!repo_name!" (
    echo Note: Directory 'libraries\!repo_name!' already exists. Skipping checkout.
    exit /b 0
)

echo Checking out !repo_name!...
svn checkout !url! libraries\!repo_name!
if errorlevel 1 (
    echo Error: Failed to checkout !repo_name! repository
    exit /b 1
)

echo Successfully checked out !repo_name!
endlocal
