@echo off
cd /d "%~dp0"

echo =======================================================================
echo   KHOI CHAY DIGITAL TWIN 3D - XE ROBOT GAP DO VAT (OPENGL/C++)
echo =======================================================================

if not exist "%~dp0XeDieuKhien3D.exe" (
    echo [Canh bao] Chua tim thay XeDieuKhien3D.exe. Dang tien hanh bien dich...
    call "%~dp0build.bat"
)

if exist "%~dp0XeDieuKhien3D.exe" (
    echo [Thanh cong] Dang mo cua so mo phong 3D...
    start "" /D "%~dp0" "%~dp0XeDieuKhien3D.exe"
) else (
    echo [Loi] Khong the khoi chay do chua bien dich thanh cong!
    pause
)
