# HỆ THỐNG MÔ PHỎNG 3D & SƠ ĐỒ ĐẤU NỐI 2D XE ROBOT TAY GẮP ARDUINO

> **Học phần:** Đồ Họa Máy Tính (CG.3 & CG.4)  
> **Đơn vị:** Trường Đại học Công nghệ Thông tin và Truyền thông Việt - Hàn (VKU) - Đại học Đà Nẵng  
> **Ngôn ngữ & Thư viện:** C++11 / OpenGL / FreeGLUT  
> **Chủ đề:** Mô phỏng 3D tương tác kiến trúc phần cứng vi điều khiển, động học cánh tay 3-DOF, sơ đồ mạch nguyên lý 2D và đấu nối 34 đường dây dẫn trực quan.

---

## 1. TỔNG QUAN HỆ THỐNG & ĐẶC ĐIỂM NỔI BẬT

Ứng dụng là một hệ sinh thái **Digital Twin** hoàn chỉnh kết hợp giữa không gian 3D tương tác thời gian thực và môi trường mô phỏng sơ đồ mạch nguyên lý 2D. Dự án không chỉ tái hiện chuẩn xác 11 khối linh kiện phần cứng và 34 đường dây đấu nối vật lý, mà còn là nền tảng trình diễn trực quan **toàn bộ các thuật toán cốt lõi** trong giáo trình Đồ họa Máy tính:

1. **Kiến trúc phần mềm phân tầng, rõ ràng, dễ bảo trì**:
   - Toàn bộ tên file và hàm được đặt bằng **tiếng Việt có quy chuẩn**, có chú thích chi tiết giải thuật toán học.
   - Độc lập giữa tầng Toán học - Raster hóa - Mô hình dữ liệu - Giao diện UI - Vòng lặp điều khiển chính.
2. **Triển khai thuật toán từ gốc (From Scratch)**:
   - Tự xây dựng Ma trận 4x4, 3x3 theo vector hàng $Q = P \cdot M$.
   - Bộ đệm khung hình phần mềm (Software FrameBuffer $1280 \times 800$) kết hợp `glDrawPixels`.
   - Bresenham vẽ đoạn thẳng (đầy đủ 8 bát phân) và vẽ hình tròn.
   - Thuật toán xén tỉa Cohen-Sutherland (4-bit outcode) và Sutherland-Hodgman (xén đa giác lồi).
   - Tô màu đa giác Scanline và Tô màu loang Boundary Fill (sử dụng ngăn xếp Stack chống tràn bộ nhớ).
   - Khử mặt khuất bằng Back-Face Culling $(\vec{N} \cdot \vec{V})$ kết hợp Thuật toán Thợ sơn (Painter's Algorithm) và Z-Buffer phần cứng.
3. **Mô phỏng Động học & Tương tác thực tế**:
   - Cánh tay robot 3 bậc tự do (3-DOF) với chuyển động phân cấp Forward Kinematics.
   - Tính toán độ rộng xung PWM Servo chính xác ($0^\circ \to 544\mu s, 180^\circ \to 2400\mu s$).
   - Cơ chế gắp/thả vật thể tương tác: Khi kẹp trúng, vật thể được gắn kết chặt chẽ vào hệ tọa độ ngàm gắp thông qua ma trận nghịch đảo vật rắn (Rigid Body Inverse Matrix).
   - Cơ chế Bóc tách không gian (Exploded View) theo trục $Y$ mượt mà.
   - Nhận diện linh kiện 3D bằng kỹ thuật Color Picking trên bộ đệm ẩn (Off-screen Back Buffer).
   - Hệ thống kiểm tra toàn vẹn 34 đường dây dẫn (Wire Integrity Validator), ngăn chặn hiện tượng trùng chân hay thiếu tín hiệu.

---

## 2. KIẾN TRÚC MÃ NGUỒN (MODULAR ARCHITECTURE)

Hệ thống được tổ chức thành các module chuyên biệt:

```
CongCuDoHoa3D/
│
├── cau_hinh.h              # Hằng số giao diện, bảng màu Cyberpunk CAD, mã định danh 11 linh kiện & chế độ
├── toan_hoc_do_hoa.h       # Vector hàng Q = P * M, Ma trận 4x4, Ma trận 3x3, Pipeline tọa độ quan sát & chiếu
├── raster_co_so.h          # FrameBuffer RAM, Bresenham đoạn thẳng (8 octants), Bresenham đường tròn, Font chữ
├── xen_va_to_mau.h         # Cohen-Sutherland, Sutherland-Hodgman, Scanline Fill (ET/AET), Stack Boundary Fill
├── linh_kien_va_mesh.h     # Mesh 3D procedural (Hộp, Trụ, Chân cắm Pin) cho 11 linh kiện chuẩn cơ khí
├── bang_dau_day.h          # Danh mục 34 dây nối (5 nhóm), Định tuyến dây 3D trực giao, Bộ kiểm tra toàn vẹn
├── dong_hoc_xe_tay_gap.h   # Động học vi sai xe, Động học cánh tay 3-DOF, PWM Servo, Gắp thả vật thể bằng ma trận nghịch đảo
├── giao_dien_ui.h          # 5 Phân vùng UI (TopBar, LeftPanel, CenterViewport, RightPanel, BottomBar), Camera Fly-to
├── chinh.cpp               # Master file tích hợp vòng lặp GLUT, bộ đệm phần mềm glDrawPixels, 3D & 2D rendering
│
├── kiem_thu_he_thong.cpp   # Bộ kiểm thử tự động (Unit Test) cho PWM, Cohen-Sutherland, Sutherland-Hodgman, 34 dây nối
├── run.bat                 # Script tự động biên dịch MinGW g++ (-O2) và khởi chạy chương trình
└── README.md               # Tài liệu thuyết minh kỹ thuật và kịch bản demo bảo vệ
```

---

## 3. DANH SÁCH 11 LINH KIỆN & THỐNG KÊ 34 ĐƯỜNG DÂY NỐI

### 3.1. Danh mục 11 Linh kiện Phần cứng
| STT | Mã LK | Tên Linh Kiện | Vị trí lắp ráp vật lý | Chức năng trong hệ thống |
|:---:|:---|:---|:---|:---|
| 1 | `ARD` | Arduino Uno R3 | Khung xe tầng giữa | Vi điều khiển trung tâm xử lý dữ liệu và điều khiển PWM |
| 2 | `L298N` | Module Cầu H L298N | Sàn xe phía sau | Điều khiển công suất động cơ DC bánh xe |
| 3 | `MOT_L` | Động cơ giảm tốc Trái | Trục bánh xe trái | Dẫn động bánh xe di chuyển bên trái |
| 4 | `MOT_R` | Động cơ giảm tốc Phải | Trục bánh xe phải | Dẫn động bánh xe di chuyển bên phải |
| 5 | `SRV_BASE` | Servo Đế Xoay (MG996R) | Gốc gắn cánh tay | Xoay toàn bộ cánh tay theo trục Yaw ($0^\circ - 180^\circ$) |
| 6 | `SRV_ELBOW`| Servo Khớp Nâng (MG996R)| Khớp vai/khuỷu tay | Nâng/hạ cánh tay theo trục Pitch ($0^\circ - 135^\circ$) |
| 7 | `SRV_GRIP` | Micro Servo Ngàm (SG90) | Cổ tay robot | Đóng/mở 2 ngàm gắp cơ khí kẹp vật thể |
| 8 | `HCSR04` | Cảm biến siêu âm | Đầu trước xe | Đo khoảng cách vật cản phát hiện hàng hóa |
| 9 | `BUCK_5V` | Mạch giảm áp LM2596 | Khoang nguồn trung gian | Hạ áp từ 11.1V/7.4V xuống ổn áp 5V nuôi Servo |
| 10 | `PIN_18650`| Khay Pin 2S/3S Li-ion | Đáy sàn xe | Cung cấp nguồn tổng 7.4V - 11.1V cho hệ thống |
| 11 | `DOMINO` | Trạm cầu đấu Domino | Cạnh mạch giảm áp | Phân phối chung đường nguồn âm (GND) và nguồn dương |

### 3.2. Bảng phân nhóm 34 Đường Dây Nối Vật Lý
- **Nhóm 1: Nguồn động lực & Hạ áp (11 dây):**
  - Pin $\to$ Cầu đấu Domino, Domino $\to$ L298N (VCC, GND), Domino $\to$ Buck LM2596 (IN+, IN-), Buck $\to$ Arduino (VIN, GND), Buck $\to$ Cụm Servo (VCC_5V, GND_SERVO), và Arduino 5V/GND.
- **Nhóm 2: Tín hiệu Cầu H L298N $\leftrightarrow$ Arduino (6 dây):**
  - `ENA` (D5 - PWM), `IN1` (D4), `IN2` (D2), `IN3` (D7), `IN4` (D8), `ENB` (D6 - PWM).
- **Nhóm 3: Công suất Động cơ Trái/Phải (4 dây):**
  - `L298N_OUT1` & `OUT2` $\to$ `MOT_L`, `L298N_OUT3` & `OUT4` $\to$ `MOT_R`.
- **Nhóm 4: Điều khiển Servo Cánh tay Robot (9 dây - 3 dây mỗi Servo):**
  - Servo Đế: 5V, GND, Tín hiệu PWM pin 9.
  - Servo Khớp: 5V, GND, Tín hiệu PWM pin 10.
  - Servo Ngàm: 5V, GND, Tín hiệu PWM pin 11.
- **Nhóm 5: Tín hiệu Cảm biến siêu âm HC-SR04 (4 dây):**
  - VCC (Arduino 5V), GND (Arduino GND), TRIG (Arduino pin 12), ECHO (Arduino pin 13).

---

## 4. BẢNG ĐỐI CHIẾU THUẬT TOÁN ĐỒ HỌA MÁY TÍNH (CG.3 & CG.4)

| STT | Tên Thuật toán | Vị trí cài đặt trong Code | Ứng dụng cụ thể trong Phần mềm |
|:---:|:---|:---|:---|
| **1** | **Toán tử Vector hàng & Ma trận 4x4 / 3x3** | `toan_hoc_do_hoa.h` | Tự cài đặt biến đổi tịnh tiến, tỉ lệ, quay quanh trục tọa độ và trục bất kỳ theo chuẩn $Q = P \cdot M$; tính toán ma trận nghịch đảo vật rắn để kẹp đồ vật. |
| **2** | **Bresenham vẽ đoạn thẳng (8 Bát phân)** | `raster_co_so.h` (`veDoanThangBresenham`) | Vẽ 34 đường dây nối ở chế độ Sơ đồ 2D và vẽ đường biên cho các bảng mạch, thanh trượt UI. |
| **3** | **Bresenham vẽ đường tròn & đĩa đặc** | `raster_co_so.h` (`veDuongTronBresenham`, `veHinhTronDacBresenham`) | Vẽ các nút chân cắm (Pin), đinh ốc vít lắp ráp bo mạch, và các khớp servo. |
| **4** | **Cohen-Sutherland Line Clipping (Mã 4-bit)** | `xen_va_to_mau.h` (`xenDoanThangCohenSutherland`) | Xén tỉa các đường dây nối và lưới tọa độ khi bị kéo tràn ra ngoài vùng Center Viewport $([240, 1040] \times [40, 740])$. |
| **5** | **Sutherland-Hodgman Polygon Clipping** | `xen_va_to_mau.h` (`xenDaGiacSutherlandHodgman`) | Cắt tỉa các đa giác 2D (khung bo mạch, tấm mica) theo 4 cạnh của cửa sổ hiển thị. |
| **6** | **Tô màu đa giác Scanline (ET / AET)** | `xen_va_to_mau.h` (`toMauScanline`) | Tô màu các bề mặt bo mạch PCB (xanh lục Arduino, đỏ L298N, xanh dương Buck) với bảng cạnh tích cực và danh sách giao điểm đã sắp xếp. |
| **7** | **Tô loang Boundary Fill (Ngăn xếp Stack)** | `xen_va_to_mau.h` (`toLoangBoundaryFillStack`) | Tô loang vùng khép kín khi click chuột vào sơ đồ bo mạch 2D, giải quyết triệt để lỗi tràn stack (Stack Overflow) của đệ quy thông thường. |
| **8** | **Khử mặt khuất Back-Face Culling** | `chinh.cpp` (`tinhTichVoHuong(phapTuyen, huongNhin) <= 0`) | Loại bỏ các mặt đáy/mặt sau của các khối hộp 3D trước khi chiếu lên màn hình, tăng tốc độ dựng hình. |
| **9** | **Thuật toán Thợ sơn (Painter's Algorithm)** | `chinh.cpp` (`std::sort` theo độ sâu $Z_{cam}$) | Sắp xếp các chi tiết từ xa đến gần để vẽ tuần tự lên bộ đệm khung hình phần mềm. |
| **10** | **Bắt điểm 3D (3D Color Picking)** | `chinh.cpp` (`glReadPixels` trên Back Buffer) | Nhận diện click chuột chính xác vào từng linh kiện 3D nhờ gán mã màu RGB độc nhất cho hitbox tàng hình. |

---

## 5. HỆ THỐNG GIAO DIỆN & PHÍM TẮT ĐIỀU KHIỂN

### 5.1. Bố cục 5 Phân vùng Màn hình ($1280 \times 800$)
1. **TopBar** ($y \in [0, 40]$): Chứa 4 Tab chuyển đổi chế độ (`F1`: 3D Toàn cảnh, `F2`: Sơ đồ 2D, `F3`: Đấu nối dây, `F4`: Bóc tách không gian).
2. **LeftPanel** ($x \in [0, 240]$): Danh sách 11 linh kiện dạng cây phân cấp, hỗ trợ click chọn để xem chi tiết.
3. **CenterViewport** ($x \in [240, 1040], y \in [40, 740]$): Khung nhìn chính hiển thị không gian 3D xe robot hoặc sơ đồ mạch 2D.
4. **RightPanel** ($x \in [1040, 1280]$): Bảng Datasheet kỹ thuật (điện áp, dòng tải, số chân) và đồng hồ Telemetry động (tọa độ xe, góc xoay các khớp, trạng thái gắp).
5. **BottomBar** ($y \in [740, 800]$): Thanh trượt điều khiển góc xoay Servo, nút bấm Reset, nút chạy Kịch bản Demo tự động.

### 5.2. Bảng Phím tắt Thao tác
- **Chuyển chế độ hiển thị:**
  - `F1`: Chế độ 3D Xe Robot (Mặc định).
  - `F2`: Chế độ Sơ đồ Mạch Nguyên Lý 2D.
  - `F3`: Chế độ Đấu Nối & Đường Đi Dây Dẫn.
  - `F4` hoặc `E`: Chế độ Bóc Tách Linh Kiện (Exploded View).
- **Lái xe robot:**
  - `W` / `S`: Tiến / Lùi xe.
  - `A` / `D`: Quay thân xe sang Trái / Phải.
- **Điều khiển Cánh tay 3-DOF & Ngàm kẹp:**
  - `J` / `L`: Xoay Đế Servo MG996R ($0^\circ - 180^\circ$).
  - `I` / `K`: Nâng / Hạ Khớp Vai MG996R ($0^\circ - 135^\circ$).
  - `G`: Đóng / Mở Ngàm Gắp (Kẹp / Thả khối hộp hàng hóa).
- **Tương tác chuột:**
  - **Click chuột trái (Left Click)**: Chọn linh kiện trên cây thư mục, click trực tiếp vào khối 3D (Color Picking), hoặc tương tác với thanh trượt/nút bấm UI.
  - **Giữ chuột phải & kéo (Right Drag)**: Xoay góc nhìn Camera Orbit $360^\circ$.
  - **Lăn chuột (Scroll Wheel)** hoặc `+` / `-`: Phóng to / Thu nhỏ (Zoom In / Out).
- **Phím chức năng:**
  - `SPACE`: Bật / Tắt kịch bản Demo tự động (Auto Demo).
  - `R` hoặc `0`: Reset toàn bộ xe, khớp xoay và camera về trạng thái gốc.
  - `ESC`: Thoát ứng dụng.

---

## 6. HƯỚNG DẪN BIÊN DỊCH & CHẠY ỨNG DỤNG

### Yêu cầu môi trường:
- Hệ điều hành: Windows 10 / 11.
- Trình biên dịch: MinGW-w64 (`g++` hỗ trợ C++11 trở lên).
- Thư viện: FreeGLUT, OpenGL (`-lfreeglut -lopengl32 -lglu32`).

### Cách 1: Khởi chạy nhanh bằng File Batch (Khuyên dùng)
Chỉ cần nhấp đúp vào file `run.bat` hoặc chạy trong PowerShell / Command Prompt:
```powershell
.\run.bat
```

### Cách 2: Biên dịch thủ công từ dòng lệnh
```powershell
# Biên dịch chương trình chính
g++ chinh.cpp -O2 -o mo_phong_xe_robot_3d.exe -lfreeglut -lopengl32 -lglu32

# Chạy ứng dụng
.\mo_phong_xe_robot_3d.exe
```

### Cách 3: Chạy bộ kiểm thử tự động (Unit Test Suite)
```powershell
# Biên dịch và chạy bộ test thuật toán
g++ kiem_thu_he_thong.cpp -o kiem_thu_he_thong.exe
.\kiem_thu_he_thong.exe
```
*Kết quả kiểm thử tự động xác nhận 100% test cases cho PWM Mapping, Cohen-Sutherland, Sutherland-Hodgman và tính toàn vẹn của 34 đường dây nối đều đạt chuẩn.*

---

## 7. KỊCH BẢN 5 BƯỚC DEMO BẢO VỆ ĐỒ ÁN ĐẠT ĐIỂM TỐI ĐA

Khi báo cáo trước Hội đồng / Giảng viên, sinh viên thực hiện lần lượt 5 bước:

1. **Bước 1 (Trình diễn Thuật toán Mô hình phân cấp & Động học):**
   - Bấm `W`/`S`/`A`/`D` để lái xe di chuyển trên lưới sàn.
   - Bấm `J`/`L` xoay đế, `I`/`K` hạ vai gập xuống sát khối hộp màu cam, sau đó bấm `G` để kẹp hộp.
   - Lái xe di chuyển và xoay cánh tay; giải thích cách khối hộp tự động di chuyển theo ngàm gắp nhờ **Ma trận nghịch đảo vật rắn (Rigid Body Inverse Matrix)**.
2. **Bước 2 (Trình diễn Bóc tách không gian Exploded View):**
   - Bấm phím **`E`** (hoặc click Tab `F4: Bóc Tách`).
   - Quan sát các tầng linh kiện (Arduino, L298N, Pin, Cánh tay) tịnh tiến tách rời lơ lửng dọc trục $Y$.
   - Giữ chuột phải xoay góc nhìn $360^\circ$ để chỉ rõ kết cấu lắp ráp bên trong từng module.
3. **Bước 3 (Trình diễn 3D Color Picking & Datasheet Inspector):**
   - Click chuột trái trực tiếp vào Bo mạch Arduino hoặc Module Cầu H L298N trên màn hình 3D.
   - Quan sát khung viền vàng phát sáng bao quanh linh kiện được chọn và bảng thông số kỹ thuật (Datasheet) hiển thị tức thì ở Right Panel kèm các tia tín hiệu phát sáng.
4. **Bước 4 (Trình diễn Sơ đồ Mạch Nguyên Lý 2D & Thuật toán Tô màu / Xén hình):**
   - Bấm phím **`F2`** để chuyển sang chế độ Sơ đồ 2D.
   - Thuyết minh việc ứng dụng **Bresenham Line** vẽ dây dẫn, **Bresenham Circle** vẽ chân cắm (Pin), **Cohen-Sutherland** xén dây ra khỏi khung hình, và thuật toán **Scanline / Stack Boundary Fill** tô màu các khối IC mạch.
5. **Bước 5 (Trình diễn Chế độ Đấu nối 34 Đường Dây Nối):**
   - Bấm phím **`F3`** để mở chế độ Đấu Dây.
   - Giải thích thuật toán định tuyến dây trực giao 3D (3D Orthogonal Waypoint Routing), phân tích sự an toàn và toàn vẹn của 34 đường dây nguồn/tín hiệu, đảm bảo không có bất kỳ xung đột chân cắm nào.
