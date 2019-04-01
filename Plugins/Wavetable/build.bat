@echo off

REM Remove previous build
pushd "%WWISEROOT%\SDK\x64_vc140\Release\bin"
del /F /Q Wavetable.*
popd

REM Build
python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" premake Windows_vc140
python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" premake Authoring
REM python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" build Windows_vc140 -c Debug
python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" build Windows_vc140 -c Release
REM python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" build Authoring -c Debug
python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" build Authoring -c Release
REM python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" build Documentation

REM Copy to Wwise path
pushd "%WWISEROOT%\Authoring\x64\Release\bin"
del /F /Q Wavetable.*
popd

pushd "%WWISEROOT%\SDK\x64_vc140\Release\bin"
copy /Y Wavetable.* "%WWISEROOT%\Authoring\x64\Release\bin"
popd

REM Package
REM python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" package Common --version=2019.0.0.1
REM python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" package Documentation --version=2019.0.0.1
REM python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" package Windows_vc140 --version=2019.0.0.1
REM python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" package Authoring --version=2019.0.0.1
REM python "%WWISEROOT%/Scripts/Build/Plugins/wp.py" generate-bundle --version=2019.0.0.1