@echo off
chcp 65001 >nul

if "%1"=="" (
    echo Error: No replacement parameter specified
    echo Usage: %~nx0 "new_path_string"
    exit /b 1
)

set "config_file=%USERPROFILE%\.arduinoIDE\arduino-cli.yaml"

if not exist "%USERPROFILE%\.arduinoIDE\" (
    mkdir "%USERPROFILE%\.arduinoIDE\"
)

(
    echo board_manager:
    echo   additional_urls: []
    echo build_cache:
    echo   compilations_before_purge: 10
    echo   ttl: 720h0m0s
    echo daemon:
    echo   port: "50051"
    echo directories:
    echo   data: %LOCALAPPDATA%\Arduino15
    echo   downloads: %LOCALAPPDATA%\Arduino15\staging
    echo   user: %1
    echo library:
    echo   enable_unsafe_install: false
    echo logging:
    echo   file: ""
    echo   format: text
    echo   level: info
    echo metrics:
    echo   addr: :9090
    echo   enabled: true
    echo output:
    echo   no_color: false
    echo sketch:
    echo   always_export_binaries: false
    echo updater:
    echo   enable_notification: true
) > "%config_file%"

if %errorlevel% equ 0 (
    echo User value set to: %1
    echo File saved: %config_file%
) else (
    echo Error creating file
    exit /b 1
)