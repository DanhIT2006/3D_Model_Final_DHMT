@echo off
echo =========================================================================
echo   DIGITAL TWIN: MO PHONG LINH KIEN VI DIEU KHIEN
echo         TREN XE ROBOT MECANUM GAP DO VAT 3D
echo =========================================================================
echo Dang kiem tra va bien dich robot_mecanum_3d.cpp...
g++ robot_mecanum_3d.cpp -O2 -o robot_mecanum_3d.exe -lfreeglut -lopengl32 -lglu32
if %ERRORLEVEL% equ 0 (
    echo [THANH CONG] Bien dich hoan tat! Dang khoi chay chuong trinh...
    robot_mecanum_3d.exe
) else (
    echo [LOI] Bien dich that bai! Vui long kiem tra MinGW va FreeGLUT.
    pause
)
