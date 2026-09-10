@echo off
setlocal

call tools\from-git.cmd "https://github.com/agusevtec/eva-core-sk.git"
if errorlevel 1 exit /b %errorlevel%

call tools\from-git.cmd "https://github.com/bxparks/EpoxyDuino.git"
if errorlevel 1 exit /b %errorlevel%

call tools\from-git.cmd "https://github.com/bxparks/AUnit.git"
if errorlevel 1 exit /b %errorlevel%

echo All repositories cloned successfully!
endlocal