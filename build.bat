@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo =======================================================================
echo   BIEN DICH DU AN DIGITAL TWIN 3D - XE ROBOT (C++/OPENGL)
echo =======================================================================

where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [1/2] Dang cau hinh bang CMake va Ninja...
    cmake -B build -G "Ninja"
    echo [2/2] Dang bien dich chuong trinh...
    cmake --build build
    copy /y "build\XeDieuKhien3D.exe" "XeDieuKhien3D.exe" >nul
    echo.
    echo ==^> Bien dich THANH CONG file XeDieuKhien3D.exe!
) else (
    echo [CMake khong co san, dang bien dich bang g++ truc tiep...]
    g++ -O2 -Iinclude src/math/*.cpp src/graphics2d/*.cpp src/scene/*.cpp src/components/*.cpp src/core/*.cpp main.cpp -o XeDieuKhien3D.exe -lfreeglut -lopengl32 -lglu32
    echo ==^> Da bien dich xong XeDieuKhien3D.exe bang g++!
)

echo.
pause
