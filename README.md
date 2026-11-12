# Digital Twin 3D - Mô Phỏng Xe Robot Gắp Đồ Vật & 11 Linh Kiện Vi Điều Khiển

Dự án ứng dụng **C++ và OpenGL (FreeGLUT)** xây dựng **Bản sao số (Digital Twin 3D)** của xe robot gắp đồ vật với **11 linh kiện phần cứng chuẩn hóa**, ánh xạ trực tiếp và đầy đủ các thuật toán nền tảng trong giáo trình Đồ họa máy tính:
* `CG.3 - 2D Graphics`: Đoạn thẳng Bresenham/Midpoint, Đường tròn/Elip Midpoint, Tô màu dòng quét Scanline Fill, Tô loang Stack-based, Xén hình Cohen-Sutherland (mã vùng 4-bit) & Sutherland-Hodgman, Font chữ Bitmap/Vector.
* `CG.4 - 3D Graphics`: Hệ tọa độ thuần nhất $4 \times 4$, Biến đổi Affine 3D (Tịnh tiến, Tỉ lệ, Quay $X/Y/Z$, Quay quanh trục bất kỳ), Mô hình Khung dây (WireFrame) & Mặt đa giác (FaceModel), Đồ thị phân cảnh (Scene Graph) cha-con, Camera cầu quan sát $(\theta, \Phi, R, D)$, Chiếu phối cảnh & Chiếu trực giao kỹ thuật.

---

## 1. Kiến Trúc 4 Tầng Hệ Thống (4-Layer Architecture)

```text
+-------------------------------------------------------------------------------+
|                       TẦNG 4: RENDER, CAMERA & GIAO DIỆN HUD                  |
|  - Camera cầu (theta, Phi, R, D), 5 chế độ góc nhìn (Free 3D, Top, Front, Side)|
|  - HUD 2D: Radar siêu âm HC-SR04, Dao động ký xung PWM, Đồng hồ góc Servo     |
|  - Thuật toán CG.3: Bresenham, Midpoint Circle/Ellipse, Scanline, Cohen-Clip  |
+-------------------------------------------------------------------------------+
                                       ▲
+-------------------------------------------------------------------------------+
|                 TẦNG 3: THỰC THỂ LINH KIỆN & MẠCH ĐIỆN (CIRCUIT)              |
|  - Quản lý 11 linh kiện (BAT_SW, DOMINO, BUCK, ARD, L298N, MOT, SRV, HCSR04) |
|  - Mạng lưới chân cắm (Pins) & Dây nối 3D (Wires) phát sáng theo dòng điện    |
|  - Bộ giải mạch CircuitSimulator: lan truyền điện áp 7.4V/5V, tính công suất  |
+-------------------------------------------------------------------------------+
                                       ▲
+-------------------------------------------------------------------------------+
|                       TẦNG 2: ĐỒ THỊ PHÂN CẢNH (SCENE GRAPH)                  |
|  - Cây phân cấp cha-con: CarRoot -> Floor1/Floor2 -> SRV_BASE -> ELBOW -> GRIP|
|  - Tích lũy ma trận thế giới: M_world = M_parent * M_local                    |
|  - Động học xe vi sai (Differential Drive) & Cơ chế gắp/nhấc vật thể          |
+-------------------------------------------------------------------------------+
                                       ▲
+-------------------------------------------------------------------------------+
|                 TẦNG 1: TOÁN HỌC & HÌNH HỌC CƠ SỞ (MATH & GEOMETRY)           |
|  - Vec3, Mat4 thuần nhất 4x4 (Row/Column-major tương thích OpenGL)            |
|  - Biến đổi Affine: Translate, Scale, RotateX/Y/Z, RotateArbitraryAxis (CG.4) |
|  - Mesh3D: cấu trúc WireFrame (Edges) & FaceModel (Faces + Normals)           |
+-------------------------------------------------------------------------------+
```

---

## 2. Bảng 11 Linh Kiện Phần Cứng Chuẩn Hóa

| STT | Mã LK | Tên Linh Kiện | Node Cha | Tọa độ $(x, y, z)\text{ (cm)}$ | Kích thước CAD $(W \times H \times D)\text{ (cm)}$ | Chức Năng Chính |
| :-: | :--- | :--- | :--- | :---: | :---: | :--- |
| **01** | `BAT_SW` | Hộp 2 Pin 18650 & Công Tắc | `Floor2` | $(0.0, 1.10, -8.5)$ | $7.8 \times 2.2 \times 4.2$ | Nguồn động lực chính 7.4V, công tắc ngắt toàn xe |
| **02** | `DOMINO` | Trạm Cầu Đấu 3 Bus Nguồn | `Floor1` | $(0.0, 0.75, 0.0)$ | $6.0 \times 1.5 \times 2.5$ | Trạm chia 3 ray nguồn: Bus 7.4V, Bus 5.0V, Bus GND |
| **03** | `BUCK` | Mạch Hạ Áp LM2596 | `Floor1` | $(+3.0, 0.70, -7.0)$ | $4.3 \times 1.4 \times 2.1$ | Giảm áp 7.4V xuống 5.0V (chỉnh biến trở), nuôi 3 Servo |
| **04** | `ARD` | Arduino Uno R3 | `Floor2` | $(-3.5, 0.75, 0.0)$ | $6.8 \times 1.5 \times 5.3$ | Vi điều khiển trung tâm, đọc HC-SR04, xuất PWM |
| **05** | `L298N` | Module Cầu H Kép | `Floor1` | $(-2.5, 0.80, -7.0)$ | $4.3 \times 2.7 \times 4.3$ | Điều khiển đảo chiều và tốc độ 2 động cơ DC |
| **06** | `MOT_L` | Động Cơ & Bánh Trái | `Floor1` | $(-8.0, -1.00, -4.0)$ | $6.5 \times 4.0 \times 8.0$ | Động cơ giảm tốc TT + bánh cao su $R=3.3\text{ cm}$ |
| **07** | `MOT_R` | Động Cơ & Bánh Phải | `Floor1` | $(+8.0, -1.00, -4.0)$ | $6.5 \times 4.0 \times 8.0$ | Động cơ giảm tốc TT + bánh cao su $R=3.3\text{ cm}$ |
| **08** | `SRV_BASE`| Khung Trụ Ray Đứng AGV | `Floor1` | $(0.0, 0.00, +11.5)$ | $7.6 \times 16.0 \times 2.6$ | Khung tháp 2 thanh ray mạ crom + vít me T8 + động cơ kéo đỉnh, bắt vào sàn T1 & T2 |
| **09** | `SRV_ELBOW`| Bàn Trượt Nâng Hạ Thẳng Đứng| `SRV_BASE` | $(0.0, 1.50, 0.0)$ | $7.2 \times 2.6 \times 1.2$ | Bàn trượt hợp kim nhôm + 2 bạc đạn SC8UU tịnh tiến thẳng đứng theo phương $Y$ |
| **10** | `SRV_GRIP`| Cụm Ngàm Kẹp Đôi Song Song | `SRV_ELBOW`| $(0.0, 0.00, +1.4)$ | $7.2 \times 2.2 \times 4.0$ | Cụm ngàm kẹp đối xứng có 2 trục trượt ngang và đệm cao su bám ma sát |

---

## 3. Tài Liệu Báo Cáo Chuyên Sâu
Tài liệu thuyết minh chi tiết về kiến trúc 4 tầng, ánh xạ 7 thuật toán đồ họa máy tính (CG.3 & CG.4), mô hình hóa CAD và kịch bản thuyết trình bảo vệ đồ án:
👉 Xem tại: `docs/BaoCao_KienTruc_ThuatToan_MoPhong_Xe3D.md`

---

## 3. Hướng Dẫn Điều Khiển & Phím Tắt

### Điều Khiển Chuyển Động & Cơ Cấu Nâng Hạ Thẳng Đứng:
* `W` / `S`: Lái xe Chạy tiến / Chạy lùi
* `A` / `D`: Bẻ lái Rẽ trái / Rẽ phải (Quay 2 bánh xe vi sai)
* `Space`: Phanh dừng xe khẩn cấp
* `I` / `K`: Nâng lên / Hạ xuống theo đường thẳng đứng (`SRV_ELBOW / LINEAR_LIFT`)
* `O` / `P`: Mở rộng / Khép chặt ngàm gắp (`SRV_GRIP`)
* `J` / `L`: Căn chỉnh vị trí đế trụ nâng (`SRV_BASE`)

### Vận Hành Hệ Thống & Tự Hành:
* `T`: Bật / Tắt công tắc nguồn pin (`BAT_SW`). Khi tắt, toàn bộ xe ngắt điện, đèn LED tắt và hệ thống dừng hoạt động.
* `M`: Chuyển đổi giữa chế độ **Thủ công (Manual)** và **Tự hành gắp vật (Auto Grab)**:
  * Xe tự động di chuyển tiến về phía khối vật thể mẫu.
  * Cảm biến siêu âm `HCSR04` đo cự ly; khi đến gần $< 12\text{ cm}$, Arduino phanh dừng động cơ.
  * Bàn nâng hạ thẳng đứng xuống sàn sát vật thể $\rightarrow$ 2 càng kẹp khép lại ôm chặt vật thể $\rightarrow$ Bàn nâng kéo thẳng đứng lên cao ($10.5\text{ cm}$) mang theo vật thể!
  * Tự động hạ khuỷu tay, mở ngàm, kẹp chặt vật thể, nâng bổng lên và xoay bệ đế!
* `R`: Đặt lại toàn bộ mô phỏng về trạng thái xuất phát ban đầu (Reset).
* `+` / `-`: Tinh chỉnh điện áp ngõ ra của mạch BUCK LM2596 ($3.3\text{V} - 6.0\text{V}$).

### Quan Sát & Đồ Họa 3D:
* `1`: Góc nhìn phối cảnh 3D tự do (Perspective Orbit).
* `2`: Hình chiếu bằng kỹ thuật từ trên xuống (Top View - Ortho).
* `3`: Hình chiếu đứng kỹ thuật từ trước mũi xe (Front View - Ortho).
* `4`: Hình chiếu cạnh kỹ thuật từ bên sườn (Side View - Ortho).
* `5`: Góc nhìn bám theo sau xe (Car Follow / Chase Camera).
* `F`: Bật / Tắt chế độ xem khung dây (`WireFrame`) và khối đặc (`Solid Mesh`).
* `C`: Ẩn / Hiện mạng lưới đường dây điện 3D.
* `H`: Bật / Tắt bảng hướng dẫn phím tắt trực tiếp trên màn hình.
* `Chuột Trái`: Click trực tiếp vào linh kiện 3D để xem thông số kỹ thuật; giữ chuột kéo để xoay Camera.
* `Chuột Phải`: Trượt góc nhìn (Pan Camera).
* `Cuộn Chuột`: Phóng to / Thu nhỏ (Zoom Camera).

---

## 4. Hướng Dẫn Biên Dịch & Chạy

Yêu cầu môi trường: Windows với MinGW/MSYS2 (GCC hỗ trợ C++17) và thư viện FreeGLUT / OpenGL.

### Cách 1: Chạy trực tiếp file thực thi có sẵn
Chỉ cần nhấp đúp vào:
* `run.bat` (hoặc chạy trực tiếp `XeDieuKhien3D.exe`).

### Cách 2: Biên dịch tự động bằng file lệnh
Nhấp đúp chuột vào file:
* `build.bat`

### Cách 3: Biên dịch bằng CMake & Ninja
```bash
cmake -B build -G "Ninja"
cmake --build build
```
File thực thi `XeDieuKhien3D.exe` sẽ được tạo ra tại thư mục `build/` và thư mục gốc của dự án.
