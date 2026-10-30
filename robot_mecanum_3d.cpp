#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =========================================================================
// 1. DINH NGHIA MA NHAN DIEN LINH KIEN CHO 3D COLOR PICKING
// =========================================================================
enum ComponentID {
    COMP_NONE        = 0,
    COMP_MEGA2560    = 1, // Bo mach Arduino Mega 2560
    COMP_RAMPS14     = 2, // Shield mo rong RAMPS 1.4
    COMP_L298N       = 3, // Module dieu khien dong co L298N
    COMP_BATTERY18650= 4, // Khay pin Li-ion 18650 (3S 11.1V)
    COMP_BUCK_LM2596 = 5, // Mach giam ap Buck DC-DC LM2596
    COMP_ROBOT_ARM   = 6, // Canh tay Robot 3-DOF & Ngam gap
    COMP_MECANUM     = 7  // He thong 4 banh xe da huong Mecanum
};

int selectedComponent = COMP_NONE;

// =========================================================================
// 2. BIEN TOAN CUC DIEU KHIEN DONG HOC, CAMERA & EXPLODED VIEW
// =========================================================================

// Camera Orbit 360 & Zoom (He toa do quan sat 3D - Slide 38, 49)
float camYaw   = -38.0f; // Goc xoay ngang (Theta)
float camPitch =  26.0f; // Goc nang/ha (Phi)
float camDist  =   8.2f; // Khoang cach R tu camera den xe
int mouseLastX = 0, mouseLastY = 0;
bool isOrbiting = false;

// Dong hoc Xe Mecanum (Tren mat phang XZ, truc Y huong thang dung len tren)
float carX = 0.0f, carZ = 0.0f;
float carYaw = 0.0f;
float wheelAngleFL = 0.0f, wheelAngleFR = 0.0f, wheelAngleRL = 0.0f, wheelAngleRR = 0.0f;

// Dong hoc Canh tay Robot 3-DOF & Ngam gap
float armBaseYaw      = 0.0f;   // Khop 1: Xoay de quanh truc Y
float armShoulderPitch= 25.0f;  // Khop 2: Gap vai
float armElbowPitch   = 55.0f;  // Khop 3: Gap khuyu
float gripperOpen     = 0.16f;  // Do mo ngam gap
bool isHoldingObject  = false;
float targetObjPos[3] = {2.35f, 0.22f, 0.0f}; // Vi tri khoi vat the can gap

// Che do Boc tach Khong gian (Exploded View theo truc Y)
bool isExploded = false;
float explodeFactor = 0.0f; // Chay muot tu 0.0 -> 1.0

// Hieu ung xung tin hieu chay tren duong day GL_LINES & Auto Demo
float signalPulse = 0.0f;
bool autoDemoMode = false;
float demoTimer   = 0.0f;

// =========================================================================
// 3. KHOI TAO TEXTURE MAPPING CHO BO MACH PCB (GL_TEXTURE_2D)
// =========================================================================
GLuint texMega2560, texRamps14, texL298N, texBuckLM2596;

void taoTextureMachInPCB() {
    const int W = 128, H = 128;
    unsigned char data[W][H][3];

    // --- 3.1. Texture Arduino Mega 2560 (Xanh duong Arduino + mach dong + chip ATmega2560) ---
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            // Nen xanh PCB Arduino Mega
            data[y][x][0] = 10; data[y][x][1] = 85; data[y][x][2] = 145;
            // Duong mach in (Traces)
            if ((x % 8 == 0 || y % 8 == 0) && (x > 10 && x < 118 && y > 10 && y < 118)) {
                data[y][x][0] = 20; data[y][x][1] = 115; data[y][x][2] = 175;
            }
            // Chip trung tam ATmega2560-16AU (Hinh vuong den o giua)
            if (x >= 44 && x <= 76 && y >= 44 && y <= 76) {
                data[y][x][0] = 25; data[y][x][1] = 25; data[y][x][2] = 28;
            } else if (x >= 40 && x <= 80 && y >= 40 && y <= 80 && (x % 2 == 0 || y % 2 == 0)) {
                // Chan vang IC QFP-100
                data[y][x][0] = 210; data[y][x][1] = 185; data[y][x][2] = 60;
            }
            // Pad dong ma vang hai ben vien bo mach
            if ((y < 8 || y > 120) && (x % 4 < 2)) {
                data[y][x][0] = 230; data[y][x][1] = 190; data[y][x][2] = 40;
            }
        }
    }
    glGenTextures(1, &texMega2560);
    glBindTexture(GL_TEXTURE_2D, texMega2560);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // --- 3.2. Texture RAMPS 1.4 Shield (Bo mach Do + 5 khe Stepper Driver Vang/Den) ---
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            data[y][x][0] = 165; data[y][x][1] = 22; data[y][x][2] = 25; // PCB Do
            if ((x + y) % 10 == 0) {
                data[y][x][0] = 195; data[y][x][1] = 40; data[y][x][2] = 35;
            }
            // 5 Khe cam module A4988 / DRV8825
            if (y > 25 && y < 95 && (x % 22 > 6 && x % 22 < 18)) {
                data[y][x][0] = 220; data[y][x][1] = 180; data[y][x][2] = 30;
            }
        }
    }
    glGenTextures(1, &texRamps14);
    glBindTexture(GL_TEXTURE_2D, texRamps14);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // --- 3.3. Texture Module L298N (PCB Do Sam + Duong mach cong suat) ---
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            data[y][x][0] = 140; data[y][x][1] = 18; data[y][x][2] = 20;
            if (x % 12 < 3) {
                data[y][x][0] = 200; data[y][x][1] = 150; data[y][x][2] = 50;
            }
        }
    }
    glGenTextures(1, &texL298N);
    glBindTexture(GL_TEXTURE_2D, texL298N);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // --- 3.4. Texture Mach Buck LM2596 (PCB Xanh + Cuon cam dong) ---
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            data[y][x][0] = 15; data[y][x][1] = 65; data[y][x][2] = 130;
            if (x % 10 == 0 || y % 10 == 0) {
                data[y][x][0] = 40; data[y][x][1] = 140; data[y][x][2] = 190;
            }
        }
    }
    glGenTextures(1, &texBuckLM2596);
    glBindTexture(GL_TEXTURE_2D, texBuckLM2596);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
}

// =========================================================================
// 4. THIET LAP ANH SANG (SPOTLIGHT) & VAT LIEU (LIGHTING & MATERIALS)
// =========================================================================

void thietLapAnhSangVaVatLieu() {
    glEnable(GL_DEPTH_TEST); // Khu mat khuat bang Z-Buffer
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);     // Nguon sang Spotlight chieu tu tren xuong
    glEnable(GL_NORMALIZE);  // Tu dong chuan hoa Normal Vectors khi Scale

    // Nguon sang 0: Anh sang moi truong & khuech tan tong the
    GLfloat ambient0[]  = {0.28f, 0.30f, 0.35f, 1.0f};
    GLfloat diffuse0[]  = {0.65f, 0.65f, 0.68f, 1.0f};
    GLfloat specular0[] = {0.50f, 0.50f, 0.50f, 1.0f};
    GLfloat pos0[]      = {5.0f, 10.0f, 6.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuse0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    // Nguon sang 1: Spotlight tap trung vao cum vi dieu khien xe Robot
    GLfloat diffuse1[]  = {0.95f, 0.92f, 0.85f, 1.0f};
    GLfloat specular1[] = {1.0f,  1.0f,  1.0f,  1.0f};
    GLfloat spotPos[]   = {0.0f,  7.5f,  0.0f,  1.0f};
    GLfloat spotDir[]   = {0.0f, -1.0f,  0.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  diffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);
    glLightfv(GL_LIGHT1, GL_POSITION, spotPos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
    glLightf(GL_LIGHT1,  GL_SPOT_CUTOFF, 45.0f);
    glLightf(GL_LIGHT1,  GL_SPOT_EXPONENT, 12.0f);
}

// Ham tuy chinh vat lieu: Kim loai bong (shininess cao) hoac Nhua nham (shininess thap)
void datVatLieu(float r, float g, float b, float shininess, float specularStrength = 0.6f, float alpha = 1.0f) {
    GLfloat amb[]  = {r * 0.35f, g * 0.35f, b * 0.35f, alpha};
    GLfloat diff[] = {r, g, b, alpha};
    GLfloat spec[] = {specularStrength, specularStrength, specularStrength, alpha};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  spec);
    glMaterialf(GL_FRONT_AND_BACK,  GL_SHININESS, shininess);
}

// =========================================================================
// 5. CAC HAM DUNG KHOI HINH HOC CO BAN CO VECTOR PHAP TUYEN & TEXTURE
// =========================================================================

// Ve khoi hop co Normal Vectors chuan va tuy chon phu Texture PCB o mat tren
void veKhoiHop(float dx, float dy, float dz, GLuint texID = 0) {
    float x = dx * 0.5f, y = dy * 0.5f, z = dz * 0.5f;

    if (texID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
        datVatLieu(1.0f, 1.0f, 1.0f, 35.0f, 0.4f);
    }

    glBegin(GL_QUADS);
    // Mat tren (Y+) - Phu Texture PCB day du
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-x,  y,  z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( x,  y,  z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y, -z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-x,  y, -z);

    // Mat duoi (Y-)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-x, -y, -z); glVertex3f( x, -y, -z);
    glVertex3f( x, -y,  z); glVertex3f(-x, -y,  z);

    // Mat truoc (Z+)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-x, -y,  z); glVertex3f( x, -y,  z);
    glVertex3f( x,  y,  z); glVertex3f(-x,  y,  z);

    // Mat sau (Z-)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f( x, -y, -z); glVertex3f(-x, -y, -z);
    glVertex3f(-x,  y, -z); glVertex3f( x,  y, -z);

    // Mat phai (X+)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f( x, -y,  z); glVertex3f( x, -y, -z);
    glVertex3f( x,  y, -z); glVertex3f( x,  y,  z);

    // Mat trai (X-)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-x, -y, -z); glVertex3f(-x, -y,  z);
    glVertex3f(-x,  y,  z); glVertex3f(-x,  y, -z);
    glEnd();

    if (texID != 0) {
        glDisable(GL_TEXTURE_2D);
    }
}

// Ve khoi tru 3D doc theo truc Y (Dung cho Tu dien, Pin 18650, Con lan Mecanum, Khop Servo)
void veKhoiTruY(float radius, float height, int slices = 16) {
    float halfH = height * 0.5f;
    // Than tru
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float ang = 2.0f * M_PI * i / slices;
        float nx = cos(ang), nz = sin(ang);
        glNormal3f(nx, 0.0f, nz);
        glVertex3f(radius * nx, -halfH, radius * nz);
        glVertex3f(radius * nx,  halfH, radius * nz);
    }
    glEnd();

    // Nap tren & duoi
    for (int side = -1; side <= 1; side += 2) {
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, (float)side, 0.0f);
        glVertex3f(0.0f, side * halfH, 0.0f);
        for (int i = 0; i <= slices; ++i) {
            float ang = (side > 0) ? (2.0f * M_PI * i / slices) : (-2.0f * M_PI * i / slices);
            glVertex3f(radius * cos(ang), side * halfH, radius * sin(ang));
        }
        glEnd();
    }
}

// =========================================================================
// 6. XAY DUNG CHI TIET TUNG CUM LINH KIEN PHAN CUNG TREN XE ROBOT
// =========================================================================

// 6.1. Banh xe Da huong Mecanum (Mam nhom + 8 Con lan cao su nghieng 45 do)
void veBanhXeMecanum(float wheelRotDeg, bool isLeftHanded) {
    glPushMatrix();
    // Quay banh xe quanh truc Z cuc bo (truc ngang cua xe)
    glRotatef(wheelRotDeg, 0.0f, 0.0f, 1.0f);

    // Mam banh xe bang kim loai nhom bong
    datVatLieu(0.75f, 0.78f, 0.82f, 90.0f, 0.9f);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    veKhoiTruY(0.32f, 0.26f, 16);
    glPopMatrix();

    // 8 Con lan cao su nham (Rollers) dat nghieng 45 do quanh chu vi banh
    datVatLieu(0.18f, 0.20f, 0.22f, 10.0f, 0.1f); // Cao su nham
    int numRollers = 8;
    float rollerTilt = isLeftHanded ? 45.0f : -45.0f;
    for (int i = 0; i < numRollers; ++i) {
        glPushMatrix();
        glRotatef(i * (360.0f / numRollers), 0.0f, 0.0f, 1.0f);
        glTranslatef(0.34f, 0.0f, 0.0f);
        glRotatef(rollerTilt, 1.0f, 0.0f, 0.0f); // Goc nghieng 45 do dac trung cua Mecanum
        veKhoiTruY(0.085f, 0.28f, 10);
        glPopMatrix();
    }
    glPopMatrix();
}

// 6.2. Bo mach Arduino Mega 2560
void veArduinoMega2560() {
    // Bo mach PCB phu Texture Mega 2560
    veKhoiHop(1.30f, 0.06f, 0.72f, texMega2560);

    // Cong nap USB Type-B bang kim loai sang bong
    datVatLieu(0.82f, 0.85f, 0.88f, 100.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.56f, 0.09f, 0.16f);
    veKhoiHop(0.22f, 0.14f, 0.16f);
    glPopMatrix();

    // Jack nguon DC den nham
    datVatLieu(0.15f, 0.15f, 0.18f, 15.0f, 0.2f);
    glPushMatrix();
    glTranslatef(-0.55f, 0.08f, -0.18f);
    veKhoiHop(0.24f, 0.13f, 0.14f);
    glPopMatrix();

    // Chip vi dieu khien ATmega2560-16AU o trung tam
    datVatLieu(0.12f, 0.12f, 0.14f, 20.0f, 0.3f);
    glPushMatrix();
    glTranslatef(-0.05f, 0.045f, 0.0f);
    veKhoiHop(0.26f, 0.04f, 0.26f);
    glPopMatrix();

    // Day chan cam Header Pin doc 2 ben va cụm Digital 22-53 o cuoi mach
    datVatLieu(0.18f, 0.18f, 0.20f, 10.0f, 0.1f);
    glPushMatrix(); glTranslatef(0.05f, 0.08f,  0.31f); veKhoiHop(0.85f, 0.10f, 0.05f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.05f, 0.08f, -0.31f); veKhoiHop(0.85f, 0.10f, 0.05f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.58f, 0.08f,  0.00f); veKhoiHop(0.08f, 0.10f, 0.62f); glPopMatrix();
}

// 6.3. Bo mach RAMPS 1.4 Shield (Gan chong len Arduino Mega 2560)
void veRamps14Shield() {
    // Bo mach PCB phu Texture RAMPS 1.4
    veKhoiHop(1.15f, 0.05f, 0.68f, texRamps14);

    // 5 Module Stepper Driver (A4988 / DRV8825) kem tan nhiet nhom tren tung driver
    for (int i = 0; i < 5; ++i) {
        float xPos = -0.32f + i * 0.18f;
        // Bo mach con cua Driver
        datVatLieu(0.65f, 0.15f, 0.18f, 25.0f, 0.3f);
        glPushMatrix();
        glTranslatef(xPos, 0.07f, 0.05f);
        veKhoiHop(0.13f, 0.06f, 0.22f);
        glPopMatrix();

        // Tan nhiet nhom mau bac tren IC Driver
        datVatLieu(0.80f, 0.82f, 0.86f, 95.0f, 0.9f);
        glPushMatrix();
        glTranslatef(xPos, 0.13f, 0.05f);
        veKhoiHop(0.11f, 0.07f, 0.14f);
        glPopMatrix();
    }

    // Domino nguon xanh la & 2 Cau chi PTC mau vang o dau mach RAMPS 1.4
    datVatLieu(0.15f, 0.68f, 0.28f, 30.0f, 0.3f);
    glPushMatrix();
    glTranslatef(-0.48f, 0.09f, -0.15f);
    veKhoiHop(0.16f, 0.13f, 0.32f);
    glPopMatrix();

    datVatLieu(0.92f, 0.72f, 0.15f, 40.0f, 0.4f);
    glPushMatrix();
    glTranslatef(-0.32f, 0.12f, -0.22f);
    veKhoiHop(0.04f, 0.16f, 0.14f);
    glPopMatrix();
}

// 6.4. Module Dieu khien Dong co L298N
void veModuleL298N() {
    veKhoiHop(0.62f, 0.05f, 0.62f, texL298N);

    // Khoi tan nhiet nhom den trung tam co khe ranh
    datVatLieu(0.16f, 0.18f, 0.20f, 80.0f, 0.8f);
    for (int i = -1; i <= 1; ++i) {
        glPushMatrix();
        glTranslatef(0.0f, 0.15f, i * 0.06f);
        veKhoiHop(0.24f, 0.22f, 0.03f);
        glPopMatrix();
    }

    // 2 Tu hoa loc nguon hinh tru
    datVatLieu(0.15f, 0.25f, 0.75f, 50.0f, 0.5f);
    glPushMatrix(); glTranslatef(-0.18f, 0.12f, -0.16f); veKhoiTruY(0.065f, 0.18f, 12); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.18f, 0.12f, -0.16f); veKhoiTruY(0.065f, 0.18f, 12); glPopMatrix();

    // 3 Domino dau day dong co (Xanh duong)
    datVatLieu(0.12f, 0.48f, 0.85f, 30.0f, 0.3f);
    glPushMatrix(); glTranslatef(-0.24f, 0.08f, 0.0f); veKhoiHop(0.10f, 0.11f, 0.24f); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.24f, 0.08f, 0.0f); veKhoiHop(0.10f, 0.11f, 0.24f); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.00f, 0.08f, 0.24f); veKhoiHop(0.24f, 0.11f, 0.10f); glPopMatrix();
}

// 6.5. Khay Pin 18650 Li-ion (3 cell 11.1V)
void veKhayPin18650() {
    // Khay nhua den nham
    datVatLieu(0.14f, 0.15f, 0.17f, 10.0f, 0.15f);
    veKhoiHop(0.82f, 0.08f, 0.72f);

    // 3 Cell pin 18650 hinh tru nam ngang
    datVatLieu(0.88f, 0.22f, 0.45f, 65.0f, 0.6f);
    for (int i = -1; i <= 1; ++i) {
        glPushMatrix();
        glTranslatef(0.0f, 0.11f, i * 0.21f);
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        veKhoiTruY(0.09f, 0.74f, 14);
        glPopMatrix();
    }
}

// 6.6. Mach Giam Ap Buck Converter LM2596
void veMachBuckLM2596() {
    veKhoiHop(0.52f, 0.04f, 0.32f, texBuckLM2596);

    // Cuon cam xuyến dong & 2 Tu loc nhom bac
    datVatLieu(0.78f, 0.52f, 0.20f, 85.0f, 0.8f);
    glPushMatrix(); glTranslatef(0.0f, 0.08f, 0.0f); veKhoiTruY(0.08f, 0.10f, 12); glPopMatrix();

    datVatLieu(0.82f, 0.85f, 0.88f, 95.0f, 0.9f);
    glPushMatrix(); glTranslatef(-0.16f, 0.09f, 0.0f); veKhoiTruY(0.055f, 0.13f, 10); glPopMatrix();
    glPushMatrix(); glTranslatef( 0.16f, 0.09f, 0.0f); veKhoiTruY(0.055f, 0.13f, 10); glPopMatrix();

    // Bien tro vi chinh (Trim Pot) mau xanh duong
    datVatLieu(0.15f, 0.45f, 0.85f, 20.0f, 0.2f);
    glPushMatrix(); glTranslatef(0.0f, 0.08f, -0.10f); veKhoiHop(0.10f, 0.09f, 0.06f); glPopMatrix();
}

// =========================================================================
// 7. MO HINH PHAN CAP TOAN XE & CANH TAY ROBOT (glPushMatrix / glPopMatrix)
// =========================================================================

// Ham ho tro dat mau cho Hitbox khi o che do 3D Color Picking
void chonMauPicking(int compID) {
    glColor3ub((GLubyte)(compID * 25), 100, 150);
}

void veToanBoXeRobot(bool isPickingPass) {
    float yExp = explodeFactor * 1.45f; // Do gian cach theo truc Y khi bat Exploded View

    glPushMatrix();
    // --- GOC PHAN CAP: Tịnh tiến và Xoay toàn bộ khung xe ---
    glTranslatef(carX, 0.36f, carZ);
    glRotatef(carYaw, 0.0f, 1.0f, 0.0f);

    if (!isPickingPass) {
        // 1. Khung gam nhom duoi (Bottom Chassis Plate)
        datVatLieu(0.28f, 0.31f, 0.36f, 75.0f, 0.7f);
        veKhoiHop(3.20f, 0.08f, 1.75f);

        // 4 Tru dong (Brass Standoffs) nang tam mica tang tren
        datVatLieu(0.85f, 0.68f, 0.22f, 90.0f, 0.85f);
        const float sx[2] = {-1.35f, 1.35f}, sz[2] = {-0.72f, 0.72f};
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j) {
                glPushMatrix();
                glTranslatef(sx[i], 0.38f, sz[j]);
                veKhoiTruY(0.04f, 0.76f, 8);
                glPopMatrix();
            }
    }

    // 2. HE THONG 4 BANH XE DA HUONG MECANUM (COMP_MECANUM)
    const float wx[2] = {-1.10f, 1.10f}; // Sau, Truoc
    const float wz[2] = {-1.02f, 1.02f}; // Phai, Trai
    for (int ix = 0; ix < 2; ++ix) {
        for (int iz = 0; iz < 2; ++iz) {
            glPushMatrix();
            glTranslatef(wx[ix], -0.04f, wz[iz]);
            if (isPickingPass) {
                chonMauPicking(COMP_MECANUM);
                veKhoiHop(0.72f, 0.72f, 0.35f); // Invisible Hitbox Banh Mecanum
            } else {
                float wAng = (ix == 1 && iz == 1) ? wheelAngleFL :
                             (ix == 1 && iz == 0) ? wheelAngleFR :
                             (ix == 0 && iz == 1) ? wheelAngleRL : wheelAngleRR;
                bool leftHand = ((ix + iz) % 2 == 0);
                veBanhXeMecanum(wAng, leftHand);
            }
            glPopMatrix();
        }
    }

    // 3. CUM VI DIEU KHIEN TRUNG TAM: ARDUINO MEGA 2560 + RAMPS 1.4 SHIELD
    // 3.1. Arduino Mega 2560 (Tach lop theo truc Y: yExp * 0.55)
    glPushMatrix();
    glTranslatef(0.15f, 0.08f + yExp * 0.55f, 0.0f);
    if (isPickingPass) {
        chonMauPicking(COMP_MEGA2560);
        veKhoiHop(1.35f, 0.22f, 0.78f); // Invisible Hitbox Mega 2560
    } else {
        veArduinoMega2560();
    }
    glPopMatrix();

    // 3.2. RAMPS 1.4 Shield (Gan chong len Mega 2560, tach cao hon theo truc Y: yExp * 1.25)
    glPushMatrix();
    glTranslatef(0.12f, 0.22f + yExp * 1.25f, 0.0f);
    if (isPickingPass) {
        chonMauPicking(COMP_RAMPS14);
        veKhoiHop(1.20f, 0.25f, 0.72f); // Invisible Hitbox RAMPS 1.4
    } else {
        veRamps14Shield();
    }
    glPopMatrix();

    // 4. CUM CHAP HANH & NGUON: L298N, KHAY PIN 18650, BUCK LM2596
    // 4.1. Module L298N (Dat phia truoc khoang mach)
    glPushMatrix();
    glTranslatef(1.12f, 0.08f + yExp * 0.80f, -0.32f);
    if (isPickingPass) {
        chonMauPicking(COMP_L298N);
        veKhoiHop(0.68f, 0.32f, 0.68f); // Hitbox L298N
    } else {
        veModuleL298N();
    }
    glPopMatrix();

    // 4.2. Mach giam ap Buck LM2596 (Dat canh L298N)
    glPushMatrix();
    glTranslatef(1.12f, 0.08f + yExp * 0.95f, 0.42f);
    if (isPickingPass) {
        chonMauPicking(COMP_BUCK_LM2596);
        veKhoiHop(0.58f, 0.25f, 0.38f); // Hitbox Buck LM2596
    } else {
        veMachBuckLM2596();
    }
    glPopMatrix();

    // 4.3. Khay Pin 18650 Li-ion (Dat phia sau khoang mach)
    glPushMatrix();
    glTranslatef(-1.05f, 0.08f + yExp * 0.45f, 0.0f);
    if (isPickingPass) {
        chonMauPicking(COMP_BATTERY18650);
        veKhoiHop(0.88f, 0.30f, 0.78f); // Hitbox Pin 18650
    } else {
        veKhayPin18650();
    }
    glPopMatrix();

    // 5. HIỆU ỨNG ĐƯỜNG TRUYỀN TÍN HIỆU (GL_LINES) PHÁT SÁNG GIỮA CÁC BO MẠCH
    if (!isPickingPass && (selectedComponent != COMP_NONE || isExploded)) {
        glDisable(GL_LIGHTING);
        glLineWidth(2.8f);
        float yMega  = 0.12f + yExp * 0.55f;
        float yRamps = 0.25f + yExp * 1.25f;
        float yL298  = 0.15f + yExp * 0.80f;
        float yBuck  = 0.15f + yExp * 0.95f;
        float yBat   = 0.15f + yExp * 0.45f;
        float yArm   = 0.82f + yExp * 1.75f;

        glBegin(GL_LINES);
        // Duong nguon 11.1V (Do cam): Pin 18650 -> Buck LM2596 & L298N
        glColor3f(1.0f, 0.35f, 0.15f);
        glVertex3f(-1.05f, yBat, 0.0f); glVertex3f(1.12f, yBuck, 0.42f);
        glVertex3f(-1.05f, yBat, 0.0f); glVertex3f(1.12f, yL298, -0.32f);

        // Duong nguon 5V (Vang sang): Buck LM2596 -> Arduino Mega 2560 -> RAMPS 1.4
        glColor3f(1.0f, 0.92f, 0.20f);
        glVertex3f(1.12f, yBuck, 0.42f); glVertex3f(0.15f, yMega, 0.0f);
        glVertex3f(0.15f, yMega, 0.0f);  glVertex3f(0.12f, yRamps, 0.0f);

        // Duong tin hieu PWM & Stepper/Servo (Xanh Cyan phat sang): RAMPS 1.4 -> L298N & Canh tay Robot
        glColor3f(0.15f, 0.98f, 0.95f);
        glVertex3f(0.12f, yRamps, 0.0f); glVertex3f(1.12f, yL298, -0.32f);
        glVertex3f(0.12f, yRamps, 0.0f); glVertex3f(0.65f, yArm, 0.0f);
        glEnd();

        // Hat xung tin hieu (Data Packets) chay doc tren duong day
        float t = fmod(signalPulse, 1.0f);
        glPointSize(7.0f);
        glBegin(GL_POINTS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex3f(-1.05f + t * (1.12f - (-1.05f)), yBat + t * (yBuck - yBat), t * 0.42f);
        glVertex3f(0.15f  + t * (0.12f - 0.15f),    yMega + t * (yRamps - yMega), 0.0f);
        glVertex3f(0.12f  + t * (1.12f - 0.12f),    yRamps + t * (yL298 - yRamps), -t * 0.32f);
        glVertex3f(0.12f  + t * (0.65f - 0.12f),    yRamps + t * (yArm - yRamps), 0.0f);
        glEnd();
        glEnable(GL_LIGHTING);
    }

    // 6. TAM NẮP MICA TRÊN & CÁNH TAY ROBOT 3 BẬC TỰ DO (HIERARCHICAL MODELING)
    glPushMatrix();
    glTranslatef(0.0f, 0.76f + yExp * 1.75f, 0.0f);

    if (!isPickingPass) {
        // Tam de lap Canh tay Robot tren noc xe
        datVatLieu(0.24f, 0.27f, 0.32f, 60.0f, 0.5f);
        veKhoiHop(3.00f, 0.06f, 1.65f);
    }

    // --- BẬC 1: ĐẾ XOAY SERVO (Base Yaw - Quay quanh trục Y) ---
    glTranslatef(0.65f, 0.05f, 0.0f);
    glRotatef(armBaseYaw, 0.0f, 1.0f, 0.0f);

    if (isPickingPass) {
        chonMauPicking(COMP_ROBOT_ARM);
        veKhoiHop(0.55f, 0.40f, 0.55f);
    } else {
        datVatLieu(0.20f, 0.22f, 0.25f, 40.0f, 0.4f);
        veKhoiTruY(0.32f, 0.14f, 16);
        datVatLieu(0.92f, 0.48f, 0.12f, 70.0f, 0.7f);
        glPushMatrix(); glTranslatef(0.0f, 0.16f, 0.0f); veKhoiHop(0.28f, 0.22f, 0.28f); glPopMatrix();
    }

    // --- BẬC 2: KHỚP VAI & CÁNH TAY DƯỚI (Shoulder Pitch - Quay quanh trục Z) ---
    glTranslatef(0.0f, 0.24f, 0.0f);
    glRotatef(-armShoulderPitch, 0.0f, 0.0f, 1.0f);

    if (isPickingPass) {
        chonMauPicking(COMP_ROBOT_ARM);
        glPushMatrix(); glTranslatef(0.0f, 0.45f, 0.0f); veKhoiHop(0.25f, 0.95f, 0.25f); glPopMatrix();
    } else {
        // Khop xoay Servo Vai
        datVatLieu(0.75f, 0.78f, 0.82f, 90.0f, 0.85f);
        glPushMatrix(); glRotatef(90.0f, 1.0f, 0.0f, 0.0f); veKhoiTruY(0.12f, 0.32f, 12); glPopMatrix();
        // Thanh canh tay duoi (L1 = 0.90)
        datVatLieu(0.92f, 0.52f, 0.15f, 65.0f, 0.6f);
        glPushMatrix(); glTranslatef(0.0f, 0.45f, 0.0f); veKhoiHop(0.14f, 0.90f, 0.18f); glPopMatrix();
    }

    // --- BẬC 3: KHỚP KHUỶU & CÁNH TAY TRÊN (Elbow Pitch - Quay quanh trục Z) ---
    glTranslatef(0.0f, 0.90f, 0.0f);
    glRotatef(-armElbowPitch, 0.0f, 0.0f, 1.0f);

    if (isPickingPass) {
        chonMauPicking(COMP_ROBOT_ARM);
        glPushMatrix(); glTranslatef(0.0f, 0.40f, 0.0f); veKhoiHop(0.25f, 0.85f, 0.35f); glPopMatrix();
    } else {
        // Khop xoay Servo Khuyu
        datVatLieu(0.75f, 0.78f, 0.82f, 90.0f, 0.85f);
        glPushMatrix(); glRotatef(90.0f, 1.0f, 0.0f, 0.0f); veKhoiTruY(0.10f, 0.28f, 12); glPopMatrix();
        // Thanh canh tay tren (L2 = 0.80)
        datVatLieu(0.18f, 0.62f, 0.86f, 70.0f, 0.65f);
        glPushMatrix(); glTranslatef(0.0f, 0.40f, 0.0f); veKhoiHop(0.12f, 0.80f, 0.15f); glPopMatrix();

        // --- CỤM NGÀM GẮP (GRIPPER) TỊNH TIẾN ĐÓNG/MỞ PHỤ THUỘC KHỚP KHUỶU ---
        glTranslatef(0.0f, 0.82f, 0.0f);
        datVatLieu(0.25f, 0.28f, 0.32f, 50.0f, 0.5f);
        veKhoiHop(0.16f, 0.08f, 0.42f);

        // 2 Ngon kep Trai & Phai
        datVatLieu(0.90f, 0.22f, 0.25f, 60.0f, 0.6f);
        glPushMatrix(); glTranslatef(0.0f, 0.16f, -gripperOpen); veKhoiHop(0.10f, 0.28f, 0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.0f, 0.16f,  gripperOpen); veKhoiHop(0.10f, 0.28f, 0.05f); glPopMatrix();

        // Neu dang kep vat the -> Vat the di chuyen theo dung he toa do phan cap cua ngam gap!
        if (isHoldingObject) {
            datVatLieu(0.20f, 0.92f, 0.45f, 80.0f, 0.8f);
            glPushMatrix();
            glTranslatef(0.0f, 0.20f, 0.0f);
            veKhoiHop(0.28f, 0.28f, 0.28f);
            glPopMatrix();
        }
    }

    glPopMatrix(); // Ket thuc nhanh Canh tay Robot
    glPopMatrix(); // Ket thuc Goc Khung xe

    // Ve Khoi Vat The tren mat san neu chua bi gap
    if (!isPickingPass && !isHoldingObject) {
        datVatLieu(0.20f, 0.92f, 0.45f, 80.0f, 0.8f);
        glPushMatrix();
        glTranslatef(targetObjPos[0], targetObjPos[1], targetObjPos[2]);
        veKhoiHop(0.32f, 0.32f, 0.32f);
        glPopMatrix();
    }
}

// =========================================================================
// 8. GIAO DIEN HUD 2D HIEN THI THONG SO KY THUAT LINH KIEN KHI CLICK CHUOT
// =========================================================================

void veText2D(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        ++text;
    }
}

void veBangThongSoLinhKienHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1080, 0, 720);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Thanh tieu de tren cung
    glColor4f(0.06f, 0.10f, 0.16f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(10, 645); glVertex2f(1070, 645);
    glVertex2f(1070, 712); glVertex2f(10, 712);
    glEnd();

    glColor3f(0.25f, 0.95f, 0.85f);
    veText2D(22, 692, "DIGITAL TWIN: MO PHONG & TUONG TAC LINH KIEN VI DIEU KHIEN TREN XE ROBOT MECANUM GAP DO VAT", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.88f, 0.92f, 0.98f);
    veText2D(22, 672, "[Click Chuot Trai vao Linh Kien]: Xem Thong So & Duong Tin Hieu (3D Picking)  |  [Keo Chuot Phai / Phim Mui Ten]: Xoay Camera 360  |  [Lan Chuot / +-]: Zoom");
    veText2D(22, 653, "[Phim E]: Boc Tach Khong Gian (Exploded View Truc Y)  |  [W/S/A/D/Q/Z]: Lai Xe Mecanum 8 Huong  |  [J/L/I/K/U/O]: Dieu Khien Tay Robot & Ngam Gap  |  [SPACE]: Auto Demo");

    // Bang Thong So Ky Thuat (Datasheet Inspector) o goc duoi ben phai khi Click chon linh kien
    glColor4f(0.05f, 0.12f, 0.20f, 0.90f);
    glBegin(GL_QUADS);
    glVertex2f(640, 15);  glVertex2f(1065, 15);
    glVertex2f(1065, 195); glVertex2f(640, 195);
    glEnd();

    glColor3f(0.20f, 0.85f, 1.0f);
    glLineWidth(1.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(640, 15);  glVertex2f(1065, 15);
    glVertex2f(1065, 195); glVertex2f(640, 195);
    glEnd();

    const char* title = "TRANG THAI: CHUA CHON LINH KIEN (CLICK CHUOT TRAI VAO BO MACH)";
    const char* line1 = "- Hay bam phím [E] de Boc tach cac lop linh kien theo truc Y.";
    const char* line2 = "- Click chuot trai truc tiep vao Arduino, RAMPS, L298N, Pin, Buck...";
    const char* line3 = "- He thong se hien thi Datasheet & kich hoat duong tin hieu GL_LINES.";
    const char* line4 = "- Che do Exploded View: Tat / Bat bang phim [E].";

    if (selectedComponent == COMP_MEGA2560) {
        title = "[1] BO MACH VI DIEU KHIEN TRUNG TAM: ARDUINO MEGA 2560";
        line1 = "- Vi dieu khien: ATmega2560-16AU (8-bit AVR, Xung nhip 16 MHz)";
        line2 = "- Bo nho: 256 KB Flash, 8 KB SRAM, 4 KB EEPROM";
        line3 = "- Giao tiep: 54 chan Digital I/O (15 PWM), 16 Analog Input, 4 UART";
        line4 = "- Vai tro: Xu ly dong hoc Mecanum & dieu khien 3 truc Servo tay gap.";
    } else if (selectedComponent == COMP_RAMPS14) {
        title = "[2] BO MACH MO RONG: RAMPS 1.4 SHIELD (STACKED ON MEGA)";
        line1 = "- Kien truc: Cam chong truc tiep len header cua Arduino Mega 2560";
        line2 = "- Khe cam: 5 khay Stepper Driver (A4988/DRV8825) + 3 MOSFET cong suat";
        line3 = "- Ngo ra Servo: Cung cap 4 cong PWM 5V dieu khien truc tiep Canh tay";
        line4 = "- Bao ve: Tich hop cau chi nhiet tu phuc hoi PTC 5A & 11A.";
    } else if (selectedComponent == COMP_L298N) {
        title = "[3] MODULE DIEU KHIEN DONG CO: DUAL H-BRIDGE L298N";
        line1 = "- IC cong suat: L298N kep (2 cau H doc lap kem tan nhiet nhom)";
        line2 = "- Dien ap hoat dong: 5V - 35V DC | Dong dinh muc: 2A / kenh (Max 3A)";
        line3 = "- Tin hieu dieu khien: Nhan xung PWM (ENA, ENB, IN1..IN4) tu Mega 2560";
        line4 = "- Vai tro: Dieu toc & dao chieu 4 dong co DC giam toc cua banh Mecanum.";
    } else if (selectedComponent == COMP_BATTERY18650) {
        title = "[4] NGUON CAP CHINH: KHAY PIN LI-ION 18650 (3S 11.1V)";
        line1 = "- Cau hinh: 3 cell 18650 (3.7V/cell) mac noi tiep -> Dien ap 11.1V - 12.6V";
        line2 = "- Dung luong: 2600 mAh | Dong xa lien tuc: 15A (High-Drain)";
        line3 = "- Phan phoi: Cap truc tiep 11.1V cho L298N va dau vao mach Buck LM2596";
        line4 = "- Tich hop mach bao ve BMS chong qua dong va qua ap.";
    } else if (selectedComponent == COMP_BUCK_LM2596) {
        title = "[5] MACH GIAM AP DC-DC: BUCK CONVERTER LM2596";
        line1 = "- IC chuyen doi: LM2596S Step-Down Switching Regulator (Tan so 150 kHz)";
        line2 = "- Dien ap Vao: 11.1V (Tu Pin 18650) -> Dien ap Ra: 5.0V DC on dinh";
        line3 = "- Dong tai toi da: 3.0A (Hieu suat chuyen doi dat ~92%)";
        line4 = "- Vai tro: Cap nguon 5V nuoi Arduino Mega 2560, RAMPS 1.4 va 4 Servo.";
    } else if (selectedComponent == COMP_ROBOT_ARM) {
        title = "[6] CO CAU CHAP HANH: CANH TAY ROBOT 3-DOF & NGAM GAP";
        line1 = "- Cau truc: 3 khop xoay (Base Yaw, Shoulder Pitch, Elbow Pitch) + Gripper";
        line2 = "- Dong co: 4x Servo MG996R banh rang kim loai (Momen xoan 11 kg.cm)";
        line3 = "- Thuat toan: Mo hinh phan cap (glPushMatrix / glPopMatrix)";
        line4 = "- Tam với: 185 mm | Kha nang tai: 450g.";
    } else if (selectedComponent == COMP_MECANUM) {
        title = "[7] HE THONG DI CHUYEN: 4 BANH XE DA HUONG MECANUM";
        line1 = "- Cau tao: Mam nhom ket hop 8 con lan cao su (Rollers) dat nghieng 45 do";
        line2 = "- Nguyen ly: Tong hop vector luc ma sat nghieng giup xe di chuyen 8 huong";
        line3 = "- Che do: Tien/Lui (W/S), Truot ngang (A/D), Quay tai cho (Q/Z)";
        line4 = "- Dong co: 4x Motor DC Giam toc JGB37-520 co Encoder.";
    }

    glColor3f(1.0f, 0.90f, 0.25f);
    veText2D(655, 170, title);
    glColor3f(0.90f, 0.95f, 1.0f);
    veText2D(655, 140, line1);
    veText2D(655, 112, line2);
    veText2D(655, 84,  line3);
    glColor3f(0.35f, 1.0f, 0.65f);
    veText2D(655, 52,  line4);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// =========================================================================
// 9. HAM RENDER CHINH & XU LY BAT DIEM 3D (COLOR PICKING)
// =========================================================================

void thietLapCamera() {
    glLoadIdentity();
    float radYaw   = camYaw * M_PI / 180.0f;
    float radPitch = camPitch * M_PI / 180.0f;
    float eyeX = carX + camDist * cos(radPitch) * sin(radYaw);
    float eyeY = 0.8f + camDist * sin(radPitch);
    float eyeZ = carZ + camDist * cos(radPitch) * cos(radYaw);
    gluLookAt(eyeX, eyeY, eyeZ, carX, 0.75f + explodeFactor * 0.6f, carZ, 0.0, 1.0, 0.0);
}

void veLuoiSanNha() {
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = -12; i <= 12; ++i) {
        glColor3f(0.18f, 0.22f, 0.28f);
        glVertex3f((float)i, 0.0f, -12.0f); glVertex3f((float)i, 0.0f, 12.0f);
        glVertex3f(-12.0f, 0.0f, (float)i); glVertex3f(12.0f, 0.0f, (float)i);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    thietLapCamera();
    thietLapAnhSangVaVatLieu();

    veLuoiSanNha();
    veToanBoXeRobot(false); // Render that voi Lighting & Texture
    veBangThongSoLinhKienHUD();

    glutSwapBuffers();
}

// Thuat toan 3D Color Picking su dung Invisible Hitboxes
void xuLyClickChonLinhKien3D(int mouseX, int mouseY) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    thietLapCamera();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // Ve cac khoi Hitbox tang hinh voi ma mau dinh danh duy nhat
    veToanBoXeRobot(true);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLubyte pixel[3] = {0, 0, 0};
    glReadPixels(mouseX, viewport[3] - mouseY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

    int idPicked = (int)round((float)pixel[0] / 25.0f);
    if (idPicked >= COMP_MEGA2560 && idPicked <= COMP_MECANUM && pixel[1] == 100) {
        selectedComponent = idPicked;
    } else {
        selectedComponent = COMP_NONE;
    }

    glEnable(GL_LIGHTING);
    glutPostRedisplay();
}

// =========================================================================
// 10. XU LY BAN PHIM, CHUOT & VONG LAP CHUYEN DONG (IDLE)
// =========================================================================

void idle() {
    bool needRedraw = false;
    signalPulse += 0.018f;
    if (selectedComponent != COMP_NONE || isExploded) needRedraw = true;

    // Chuyen dong muot cho Exploded View theo truc Y
    if (isExploded && explodeFactor < 1.0f) {
        explodeFactor = std::min(1.0f, explodeFactor + 0.035f);
        needRedraw = true;
    } else if (!isExploded && explodeFactor > 0.0f) {
        explodeFactor = std::max(0.0f, explodeFactor - 0.035f);
        needRedraw = true;
    }

    // Kich ban Auto Demo: Xe tien -> Gap vat -> Nang len -> Xoay -> Truot ngang Mecanum
    if (autoDemoMode) {
        demoTimer += 0.015f;
        if (demoTimer > 8.0f) demoTimer = 0.0f;

        if (demoTimer < 2.0f) {
            float t = demoTimer / 2.0f;
            carX = t * 0.65f;
            wheelAngleFL -= 5.0f; wheelAngleFR -= 5.0f; wheelAngleRL -= 5.0f; wheelAngleRR -= 5.0f;
            armShoulderPitch = 25.0f + t * 42.0f;
            armElbowPitch    = 55.0f + t * 22.0f;
            isHoldingObject  = false;
            gripperOpen      = 0.16f;
        } else if (demoTimer < 3.2f) {
            gripperOpen     = 0.09f;
            isHoldingObject = true;
        } else if (demoTimer < 5.5f) {
            float t = (demoTimer - 3.2f) / 2.3f;
            armShoulderPitch = 67.0f - t * 50.0f;
            armElbowPitch    = 77.0f - t * 35.0f;
            armBaseYaw       = t * 90.0f;
            // Truot ngang (Strafe) Mecanum theo truc Z
            carZ = t * 0.90f;
            wheelAngleFL += 5.0f; wheelAngleFR -= 5.0f; wheelAngleRL -= 5.0f; wheelAngleRR += 5.0f;
        } else {
            float t = (demoTimer - 5.5f) / 2.5f;
            carZ = 0.90f * (1.0f - t);
            carX = 0.65f * (1.0f - t);
            armBaseYaw = 90.0f * (1.0f - t);
        }
        needRedraw = true;
    }

    if (needRedraw) glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    float rad = carYaw * M_PI / 180.0f;
    float speed = 0.14f;
    switch (key) {
        // --- DIEU KHIEN DONG HOC XE MECANUM (TIEN, LUI, TRUOT NGANG, XOAY) ---
        case 'w': case 'W': // Tien
            carX += speed * cos(rad); carZ -= speed * sin(rad);
            wheelAngleFL -= 14; wheelAngleFR -= 14; wheelAngleRL -= 14; wheelAngleRR -= 14;
            break;
        case 's': case 'S': // Lui
            carX -= speed * cos(rad); carZ += speed * sin(rad);
            wheelAngleFL += 14; wheelAngleFR += 14; wheelAngleRL += 14; wheelAngleRR += 14;
            break;
        case 'a': case 'A': // Truot ngang sang Trai (Mecanum Strafe Left)
            carX -= speed * sin(rad); carZ -= speed * cos(rad);
            wheelAngleFL += 14; wheelAngleFR -= 14; wheelAngleRL += 14; wheelAngleRR += 14;
            break;
        case 'd': case 'D': // Truot ngang sang Phai (Mecanum Strafe Right)
            carX += speed * sin(rad); carZ += speed * cos(rad);
            wheelAngleFL -= 14; wheelAngleFR -= 14; wheelAngleRL -= 14; wheelAngleRR -= 14;
            break;
        case 'q': case 'Q': // Xoay xe sang Trai
            carYaw += 5.0f;
            wheelAngleFL += 12; wheelAngleFR -= 12; wheelAngleRL += 12; wheelAngleRR -= 12;
            break;
        case 'z': case 'Z': // Xoay xe sang Phai
            carYaw -= 5.0f;
            wheelAngleFL -= 12; wheelAngleFR += 12; wheelAngleRL += 12; wheelAngleRR += 12;
            break;

        // --- DIEU KHIEN CANH TAY ROBOT 3 BAC TU DO & NGAM GAP ---
        case 'j': case 'J': armBaseYaw += 6.0f; break;
        case 'l': case 'L': armBaseYaw -= 6.0f; break;
        case 'i': case 'I': armShoulderPitch = std::max(-35.0f, armShoulderPitch - 4.0f); break;
        case 'k': case 'K': armShoulderPitch = std::min( 75.0f, armShoulderPitch + 4.0f); break;
        case 'u': case 'U': armElbowPitch    = std::max(  0.0f, armElbowPitch - 4.0f);    break;
        case 'o': case 'O': armElbowPitch    = std::min(100.0f, armElbowPitch + 4.0f);    break;
        case 'g': case 'G':
            isHoldingObject = !isHoldingObject;
            gripperOpen = isHoldingObject ? 0.09f : 0.16f;
            break;

        // --- BOC TACH KHONG GIAN (EXPLODED VIEW TRUC Y) & AUTO DEMO ---
        case 'e': case 'E': isExploded = !isExploded; break;
        case ' ': autoDemoMode = !autoDemoMode; break;

        // Phim tat chon nhanh linh kien 1..7
        case '1': selectedComponent = COMP_MEGA2560;     break;
        case '2': selectedComponent = COMP_RAMPS14;      break;
        case '3': selectedComponent = COMP_L298N;        break;
        case '4': selectedComponent = COMP_BATTERY18650; break;
        case '5': selectedComponent = COMP_BUCK_LM2596;  break;
        case '6': selectedComponent = COMP_ROBOT_ARM;    break;
        case '7': selectedComponent = COMP_MECANUM;      break;

        case '+': case '=': camDist = std::max(3.0f, camDist - 0.4f); break;
        case '-': case '_': camDist = std::min(15.0f, camDist + 0.4f); break;
        case '0':
            carX = carZ = carYaw = 0.0f;
            armBaseYaw = 0.0f; armShoulderPitch = 25.0f; armElbowPitch = 55.0f;
            isExploded = false; autoDemoMode = false; isHoldingObject = false;
            selectedComponent = COMP_NONE;
            break;
        case 27: exit(0);
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:  camYaw   -= 5.0f; break;
        case GLUT_KEY_RIGHT: camYaw   += 5.0f; break;
        case GLUT_KEY_UP:    camPitch = std::min(82.0f, camPitch + 4.0f); break;
        case GLUT_KEY_DOWN:  camPitch = std::max( 5.0f, camPitch - 4.0f); break;
    }
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Click chuot trai -> Thuc hien 3D Color Picking chon linh kien!
        xuLyClickChonLinhKien3D(x, y);
    } else if (button == GLUT_RIGHT_BUTTON) {
        // Giữ chuột phải kéo để xoay Camera Orbit 360 độ
        if (state == GLUT_DOWN) {
            isOrbiting = true;
            mouseLastX = x; mouseLastY = y;
        } else {
            isOrbiting = false;
        }
    } else if (button == 3 && state == GLUT_DOWN) { // Scroll Up -> Zoom In
        camDist = std::max(3.0f, camDist - 0.4f);
        glutPostRedisplay();
    } else if (button == 4 && state == GLUT_DOWN) { // Scroll Down -> Zoom Out
        camDist = std::min(15.0f, camDist + 0.4f);
        glutPostRedisplay();
    }
}

void mouseMotion(int x, int y) {
    if (isOrbiting) {
        camYaw   += (x - mouseLastX) * 0.45f;
        camPitch += (y - mouseLastY) * 0.35f;
        if (camPitch > 82.0f) camPitch = 82.0f;
        if (camPitch <  5.0f) camPitch =  5.0f;
        mouseLastX = x; mouseLastY = y;
        glutPostRedisplay();
    }
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1080, 720);
    glutInitWindowPosition(80, 40);
    glutCreateWindow("DIGITAL TWIN: Mo Phong Linh Kien Vi Dieu Khien Tren Xe Robot Mecanum Gap Do Vat");

    glClearColor(0.08f, 0.10f, 0.14f, 1.0f);
    taoTextureMachInPCB();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}
