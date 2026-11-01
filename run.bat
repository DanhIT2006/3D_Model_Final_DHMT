@echo off
echo =========================================================================
echo   DO HOA MAY TINH (CG.3 va CG.4) - DAI HOC CNTT VA TT VIET - HAN (VKU)
echo   De tai: Mo phong 3D va So do dau noi 2D Xe Robot Tay Gap Arduino
echo   (11 Linh kien - 34 Day noi)
echo =========================================================================
echo Dang kiem tra va bien dich chinh.cpp voi MinGW g++ (-O2)...

g++ chinh.cpp -O2 -o mo_phong_xe_robot_3d.exe -lfreeglut -lopengl32 -lglu32
if %ERRORLEVEL% equ 0 (
    echo [THANH CONG] Bien dich hoan tat! Dang khoi chay chuong trinh...
    mo_phong_xe_robot_3d.exe
) else (
    echo [LOI] Bien dich that bai! Vui long kiem tra MinGW g++ va thu vien FreeGLUT.
    pause
)
