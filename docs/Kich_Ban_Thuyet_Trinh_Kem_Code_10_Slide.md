# KỊCH BẢN THUYẾT TRÌNH & BẢO VỆ ĐỒ ÁN XEN KẼ DEMO VÀ MÃ NGUỒN C++
## ĐỀ TÀI: ỨNG DỤNG C++/OPENGL XÂY DỰNG CHƯƠNG TRÌNH MÔ PHỎNG VÀ TƯƠNG TÁC LINH KIỆN VI ĐIỀU KHIỂN TRÊN MÔ HÌNH XE ROBOT GẮP ĐỒ VẬT (DIGITAL TWIN 3D)

* **Thư mục dự án:** `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`
* **File thực thi:** `XeDieuKhien3D.exe` (Tốc độ ổn định 60 - 62 FPS)
* **Slide trình chiếu đồng bộ:** `Slide_Trinh_Bay_Xen_Ke_Demo_XeDieuKhien3D.pptx` (10 Slides)
* **Cơ chế tìm code nhanh trong VS Code:** Bấm `Ctrl + F` gõ `[SLIDE 3]`, `[SLIDE 4]`, `[SLIDE 5]`, `[SLIDE 6]`, `[SLIDE 7]`, `[SLIDE 8]`, `[SLIDE 9]` để nhảy ngay đến khối code cần select.

---

## MẸO BỐ TRÍ MÀN HÌNH MÁY CHIẾU (TỶ LỆ 50% - 50%)

```
+------------------------------------------+------------------------------------------+
|          NỬA BÊN TRÁI (50%)              |           NỬA BÊN PHẢI (50%)             |
|   1. PowerPoint chế độ Reading View     |   1. Ứng dụng OpenGL XeDieuKhien3D.exe   |
|   2. VS Code (Font 16-18pt, mở sẵn 5 tab)|   2. Đang chạy mượt 60 FPS               |
|      chờ sẵn phím tắt tìm code           |   3. Thao tác Demo trực tiếp theo Slide  |
+------------------------------------------+------------------------------------------+
```

---

## SLIDE 1 & 2: MỞ ĐẦU — GIỚI THIỆU ĐỀ TÀI & BỐ CỤC 5 PHẦN XEN KẼ
* **Thời lượng:** 1 Phút
* **Slide đồng bộ:** Slide 1 (Bìa đề tài) & Slide 2 (Nội dung 5 phần)

### 1. Mục tiêu & Giảng viên muốn thấy gì?
* Thấy được phong thái tự tin, tác phong trình bày khoa học.
* Định vị rõ đề tài không phải mô hình đồ họa tĩnh (game 3D), mà là một **Bản sao số (Digital Twin)**: Có hình học CAD chuẩn, có mạch điện lan truyền tín hiệu thời gian thực, và có tương tác linh kiện 2 cấp độ.

### 2. Lời thoại trình bày của sinh viên:
> *"Em chào Thầy/Cô trong Hội đồng chấm đồ án. Hôm nay em xin phép bảo vệ đề tài: **Ứng dụng C++/OpenGL xây dựng chương trình mô phỏng và tương tác linh kiện vi điều khiển trên mô hình xe Robot gắp đồ vật (Digital Twin 3D)**.*
>
> *Để Thầy/Cô tiện đối chiếu giữa lý thuyết đồ họa máy tính với khả năng triển khai mã nguồn thực tế, em xin phép chia đôi màn hình:*
> * *Nửa bên trái: Slide trình bày công thức toán học và kiến trúc code tương ứng trong VS Code.*
> * *Nửa bên phải: Cửa sổ chương trình OpenGL `XeDieuKhien3D` đang chạy trực tiếp.*
>
> *Nội dung báo cáo gồm 5 phần chính tương ứng từ Slide 3 đến Slide 10: Kiến trúc OOP 4 tầng & mô hình WireFrame/FaceModel; Thuật toán Camera cầu 3D & phép chiếu; Hệ thống 11 linh kiện & chế độ cô lập giải phẫu bo mạch; Mô phỏng mạch điện 3 Bus; và Động học cánh tay gắp kết hợp giao diện đồ họa 2D HUD ạ."*

### 3. Thao tác Demo (Bên phải):
* Để xe ở trạng thái chờ xuất phát, toàn xe 2 tầng hiển thị đầy đủ, đồng hồ FPS báo ~60 FPS, điện áp Pin 7.4V, Buck 5.00V.

---

## SLIDE 3: KIẾN TRÚC OOP 4 TẦNG & MÔ HÌNH HÌNH HỌC WIREFRAME / FACEMODEL
* **Thời lượng:** 2 Phút
* **Lý thuyết bài giảng:** Giáo trình `CG.4-3D Graphics-fix.pdf`, trang 20 – 26 (`Mesh3D`)

### 1. Mục tiêu & Giảng viên muốn thấy gì?
* Chứng minh sinh viên tự quản lý đối tượng 3D bằng cấu trúc mảng đỉnh, cạnh, mặt chứ không gọi thư viện dựng sẵn như `glutSolidCube`.
* Hiểu rõ sự khác biệt giữa mô hình khung dây (`WireFrame`) và mô hình mặt đa giác (`FaceModel`).

### 2. Lời thoại trình bày của sinh viên:
> *"Thưa Thầy/Cô, như trên Slide 3, toàn bộ dự án được tổ chức theo kiến trúc OOP 4 tầng: Toán học (`math/`), Đồ họa 2D (`graphics2d/`), Phân cảnh 3D (`scene/`) và Hệ thống nhúng/Mạch điện (`components/`, `core/`).*
>
> *Tại tầng hình học, lớp `Mesh3D` hiện thực hóa đúng cấu trúc dữ liệu trong giáo trình CG.4 trang 20–26:*
> * *Một mảng đỉnh duy nhất `vertices`.*
> * *Khi em bấm phím **F** bên cửa sổ Demo, chương trình chuyển sang chế độ **Khung dây (WireFrame)**, duyệt mảng `edges` nối cặp chỉ số đỉnh `beginP` và `endP`.*
> * *Khi ở chế độ **Khối đặc (FaceModel)**, chương trình duyệt mảng `faces` lấy vector pháp tuyến $\vec{N}$ để tính toán ánh sáng.*
> * *Đặc biệt, để khắc phục triệt để hiện tượng sọc rác Z-fighting khi vẽ viền cạnh đè lên mặt phẳng, em áp dụng `glPolygonOffset(1.0f, 1.0f)` như trong file `HardwareComponent.cpp` ạ."*

### 3. Thao tác Demo (Bên phải):
1. Bấm phím **F**: Toàn bộ xe và 11 linh kiện chuyển sang dạng khung dây neon xanh ngọc.
2. Dùng chuột xoay nhẹ để thấy rõ cấu trúc các cạnh bên trong bo mạch.
3. Bấm phím **F** lần nữa để trở về khối đặc hoàn chỉnh.

### 4. Vị trí Code cần Select trong VS Code (Bên trái):
* **File 1:** [Mesh3D.h](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/include/scene/Mesh3D.h#L6-L26) (Dòng 6 – 26)
* **Từ khóa tìm nhanh:** `[SLIDE 3]`

```cpp
// [include/scene/Mesh3D.h]: Cấu trúc đỉnh, cạnh (WireFrame) và mặt (FaceModel)
struct EdgeType {
    int beginP; // Chỉ số đỉnh đầu của cạnh
    int endP;   // Chỉ số đỉnh cuối của cạnh
};

struct FaceType {
    std::vector<int> indexFace; // Danh sách chỉ số đỉnh ngược chiều kim đồng hồ
    Vec3 normal;                // Vector pháp tuyến mặt dùng cho ánh sáng
    Vec3 faceColor;             // Màu mặt đa giác (RGB)
};

class Mesh3D {
public:
    std::vector<Vec3> vertices;   // 1. Danh sách đỉnh (Vertices)
    std::vector<EdgeType> edges;  // 2. Danh sách cạnh (Edges cho WireFrame mode)
    std::vector<FaceType> faces;  // 3. Danh sách mặt (Faces cho Solid mode)
```

* **File 2:** [Mesh3D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/scene/Mesh3D.cpp#L197-L251) (Dòng 197 – 251)
* **Từ khóa tìm nhanh:** `drawWireframe`

```cpp
// [src/scene/Mesh3D.cpp]: Duyệt mảng edges với GL_LINES theo CG.4 tr.20-26
void Mesh3D::drawWireframe(Vec3 lineColor, float lineWidth) const {
    glDisable(GL_LIGHTING);
    glLineWidth(lineWidth);
    glColor3f(lineColor.x, lineColor.y, lineColor.z);

    glBegin(GL_LINES);
    for (const auto& edge : edges) {
        if (edge.beginP >= 0 && edge.beginP < (int)vertices.size() &&
            edge.endP >= 0 && edge.endP < (int)vertices.size()) {
            const Vec3& v0 = vertices[edge.beginP];
            const Vec3& v1 = vertices[edge.endP];
            glVertex3f(v0.x, v0.y, v0.z);
            glVertex3f(v1.x, v1.y, v1.z);
        }
    }
    glEnd();
}
```

---

## SLIDE 4: THUẬT TOÁN CAMERA CẦU 3D, PHÉP CHIẾU & CAMERA CỔ TAY PiP
* **Thời lượng:** 2.5 Phút
* **Lý thuyết bài giảng:** Giáo trình `CG.4-3D Graphics-fix.pdf`, trang 28 – 49 (`Camera3D`)

### 1. Mục tiêu & Giảng viên muốn thấy gì?
* Hiểu và viết được ma trận chuyển hệ tọa độ thế giới sang hệ quan sát $T = A \cdot B \cdot C \cdot D$ với 4 tham số cầu $(\theta, \Phi, R, D)$.
* Nắm vững sự khác nhau giữa phép chiếu phối cảnh (`gluPerspective` - chia độ sâu $z$) và 3 hình chiếu trực giao song song (`glOrtho` - Top/Front/Side View).

### 2. Lời thoại trình bày của sinh viên:
> *"Thưa Thầy/Cô, nửa phải của Slide 4 là công thức ma trận biến đổi hệ quan sát $T = A \cdot B \cdot C \cdot D$ (trang 45 giáo trình CG.4) được em cài đặt trong lớp `Camera3D` với 4 tham số cầu $(\theta, \Phi, R, D)$:*
> * *Tọa độ điểm trong hệ quan sát được tính toán theo:*
>   $$x_0 = -x\sin\theta + y\cos\theta$$
>   $$y_0 = -x\cos\theta\sin\Phi - y\sin\theta\sin\Phi + z\cos\Phi$$
>   $$z_0 = -x\cos\theta\cos\Phi - y\sin\theta\cos\Phi - z\sin\Phi + R$$
> * *Khi ở chế độ **Phối cảnh (Phím 1)**: Tọa độ được chia cho độ sâu $z_0$ theo tỉ lệ $x' = x_0 \cdot D / z_0$, tạo cảm giác vật ở gần lớn hơn ở xa.*
> * *Khi em bấm phím **2, 3, 4** sang **Hình chiếu trực giao (Top, Front, Side View)**: Hàm `glOrtho` triệt tiêu thành phần chiều sâu, giữ nguyên tỉ lệ kích thước CAD của xe.*
> * *Đặc biệt, khung hình nhỏ ở góc trên phải **[C]** là một Viewport OpenGL thứ hai mô phỏng Camera gắn tại cổ tay ngàm gắp (`PiP Claw Cam`), hỗ trợ quan sát cự ly khi gắp khối hàng ạ."*

### 3. Thao tác Demo (Bên phải):
1. Kéo chuột trái xoay ngang quanh xe ($\theta$), nâng hạ góc nhìn ($\Phi$), cuộn bánh xe chuột thu phóng bán kính ($R$).
2. Bấm phím **2** (Top View): Kiểm tra cách bố trí linh kiện trên Sàn Tầng 2 từ trên cao.
3. Bấm phím **3** (Front View): Nhìn chính diện cản trước xe và cảm biến HC-SR04.
4. Bấm phím **4** (Side View): Xem khoảng cách 2 tầng sàn và 4 cột trụ đồng.
5. Bấm phím **1** (Perspective View): Trở về góc nhìn phối cảnh 3D tự do.
6. Bấm phím **C**: Bật/tắt camera phụ PiP ở góc trên phải màn hình.

### 4. Vị trí Code cần Select trong VS Code (Bên trái):
* **File 1:** [Camera3D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/math/Camera3D.cpp#L112-L160) (Dòng 112 – 160)
* **Từ khóa tìm nhanh:** `[SLIDE 4]`

```cpp
// [src/math/Camera3D.cpp]: Ma trận hệ trục quan sát T = A.B.C.D & Phép chiếu (CG.4 tr.45)
void Camera3D::computeCameraCoords(const Vec3& pWorld, float& x0, float& y0, float& z0, float& xProj, float& yProj) const {
    float x = pWorld.x - target.x;
    float y = pWorld.y - target.y;
    float z = pWorld.z - target.z;

    float sinT = std::sin(theta); float cosT = std::cos(theta);
    float sinP = std::sin(phi);   float cosP = std::cos(phi);

    // 1. Hệ trục quan sát T = A.B.C.D
    x0 = -x * sinT + y * cosT;
    y0 = -x * cosT * sinP - y * sinT * sinP + z * cosP;
    z0 = -x * cosT * cosP - y * sinT * cosP - z * sinP + radius;

    // 2. Phép chiếu phối cảnh vs trực giao
    if (mode == ViewMode::PERSPECTIVE_ORBIT || mode == ViewMode::CAR_FOLLOW) {
        float zDepth = (z0 > 0.1f) ? z0 : 0.1f;
        xProj = x0 * focalD / zDepth; // Chia cho độ sâu z (Phối cảnh)
        yProj = y0 * focalD / zDepth;
    } else {
        xProj = x0; // Giữ nguyên tọa độ phẳng (Trực giao)
        yProj = y0;
    }
}
```

* **File 2:** [Camera3D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/math/Camera3D.cpp#L141-L157) (`applyProjection` với `glOrtho` và `gluPerspective`)

---

## SLIDE 5 & 6: HỆ THỐNG 11 LINH KIỆN & CÔ LẬP GIẢI PHẪU BO MẠCH (WORKBENCH MODE)
* **Thời lượng:** 3.5 Phút
* **Kiến trúc cốt lõi:** Lớp `HardwareComponent` và mảng phần tử con `SubComponent`

### 1. Mục tiêu & Giảng viên muốn thấy gì?
* Thấy được 11 linh kiện được dựng theo đúng kích thước CAD thực tế (cm), không phải vẽ đại khái.
* Chế độ **Cô lập Bo mạch (Workbench Mode)** là điểm sáng công nghệ: Khi bấm `I`, camera thu hẹp tiêu cự, ẩn khung xe, cho phép click vào từng tụ điện, chip IC, diode, nút nhấn reset, thạch anh và jumper.

### 2. Lời thoại trình bày của sinh viên:
> *"Thưa Thầy/Cô, để thể hiện đúng tinh thần **Tương tác linh kiện vi điều khiển** trong tên đề tài, toàn bộ 11 linh kiện trên xe được em mô hình hóa chính xác theo kích thước CAD (cm) như bảng thống kê ở Slide 5.*
>
> *Đặc biệt, em thiết kế cơ chế **Giải phẫu bo mạch 2 cấp độ (2-Tier Component Inspector)**:*
> * *Ở cấp độ toàn xe (`CAR_OVERVIEW`): Click chuột vào bất kỳ linh kiện nào, bảng Inspector hiển thị kích thước $W \times H \times D$, nhóm chức năng và số chân Pin.*
> * *Khi bấm phím **I** (hoặc nút **QUAN SAT CHI TIET**): Chương trình lập tức chuyển sang chế độ **Bàn làm việc cô lập (Workbench Mode)**. Khung xe được ẩn đi, linh kiện được đưa lên bàn xoay CAD độc lập.*
> * *Trên **Arduino Uno R3**: Em mô hình hóa 8 khối cốt lõi: Chip ATmega328P DIP-28, chip nạp ATmega16U2 & cổng USB, thạch anh 16.000 MHz (chu kỳ 62.5 ns), cặp tụ lọc 47 µF và nút Reset vật lý bấm được.*
> * *Trên **Cầu H L298N**: Em dựng tản nhiệt nhôm 7 lá xẻ rãnh, 2 tụ hóa 220 µF/35V tách rời chống Z-fighting, 8 Diode bảo vệ dòng ngược Back-EMF và chốt Jumper 5V Enable có thể click rút ra trực tiếp.*
> * *Đường chỉ dẫn Callout nối từ linh kiện ra bảng mô tả sử dụng thuật toán đoạn thẳng Bresenham và đường tròn Midpoint ạ."*

### 3. Thao tác Demo (Bên phải):
1. **Tại Slide 5:** Click chuột trái vào `L298N` (Tầng 1), `BUCK LM2596` (Tầng 1), và `ARD` (Tầng 2): Bảng Inspector bên trái đổi thông số tương ứng.
2. **Tại Slide 6 (Arduino):** Click chọn `ARD` $\rightarrow$ bấm phím **I**: Xe biến mất, bo mạch Arduino Uno hiện to rõ trên bàn xoay.
   * Click chọn chip **ATmega328P-PU** $\rightarrow$ đọc mô tả CPU 16MHz.
   * Click chọn **Thạch anh 16.000 MHz** $\rightarrow$ đọc chu kỳ lệnh 62.5 ns.
   * Click chọn **Nút Reset màu đỏ** $\rightarrow$ bấm nút "NHAN RESET ARDUINO" trên màn hình.
3. Bấm **ESC** quay lại toàn xe.
4. **Tại Slide 6 (L298N):** Click chọn `L298N` $\rightarrow$ bấm phím **I**:
   * Click vào **Cặp tụ hóa 220 µF / 35V**.
   * Click vào cụm **8 Diode D1 - D8**.
   * Click vào **Chốt Jumper 5V Enable** $\rightarrow$ bấm nút "RUT/CAM JUMPER 5V" để thấy trạng thái chuyển đổi.
5. Bấm **ESC** để trở về toàn xe.

### 4. Vị trí Code cần Select trong VS Code (Bên trái):
* **File 1:** [ControlComponents.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/components/ControlComponents.cpp#L41-L85) (Dòng 41 – 85)
* **Từ khóa tìm nhanh:** `[SLIDE 6]`

```cpp
// [src/components/ControlComponents.cpp]: 8 khối phần tử con của Arduino Uno R3
void ArduinoUnoComp::initSubComponents() {
    subComponents.clear();

    // 1. Vi điều khiển chính ATmega328P-PU (DIP-28)
    SubComponent atmega;
    atmega.id = "ARD_MCU_328P";
    atmega.name = "Vi Dieu Khien Chinh ATmega328P-PU (DIP-28)";
    atmega.rating = "CPU 8-bit AVR RISC 16MHz / 32KB Flash / 2KB SRAM";
    atmega.liveStatusText = "Tan so Clock: 16MHz | Chu ky lenh: 62.5ns";
    atmega.mesh = Mesh3D::createBox(3.50f, 0.42f, 0.75f, atmega.color);
    addSubComponent(atmega);

    // 2. Nút nhấn Reset phần cứng có thể click tương tác
    SubComponent btnRst;
    btnRst.id = "ARD_RESET_BTN";
    btnRst.isInteractive = true;
    btnRst.actionName = "NHAN RESET ARDUINO";
    addSubComponent(btnRst);
    // ... Thạch anh 16MHz, Cặp tụ 47uF, Cổng USB, LDO 5V ...
}
```

* **File 2:** [ActuatorComponents.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/components/ActuatorComponents.cpp#L255-L295) (Dòng 255 – 295: `L298NDriverComp::initSubComponents` với Tản nhiệt nhôm, 2 tụ 220uF, 8 diode, jumper 5V).

---

## SLIDE 7: MÔ PHỎNG MẠCH NGUỒN 3 BUS & TÁC ĐỘNG ĐIỆN ÁP THỜI GIAN THỰC
* **Thời lượng:** 2 Phút
* **Kiến thức cốt lõi:** Lớp `CircuitSimulator.cpp` và hàm `onElectricalUpdate()`

### 1. Mục tiêu & Giảng viên muốn thấy gì?
* Chứng minh hệ thống không chỉ vẽ hoạt họa mà có **logic mạch điện thật**: Pack pin 7.4V $\rightarrow$ Domino 3 Bus $\rightarrow$ Buck LM2596 (5.0V/3A) nuôi 3 Servo MG996R (dòng đỉnh 2.5A).
* Khi ngắt nguồn hoặc giảm áp dưới $4.8\text{V}$, động cơ/servo bị khóa cứng.

### 2. Lời thoại trình bày của sinh viên:
> *"Thưa Thầy/Cô, trong `CircuitSimulator.cpp`, hệ thống mô phỏng đúng tuyến điện áp công nghiệp thực tế:*
> * *Nguồn cấp: Hộp 2 pin 18650 ($7.4\text{V}, 2600\text{mAh}$) qua công tắc `BAT_SW` dẫn vào trạm cầu đấu `DOMINO` 3 Bus.*
> * *Bus 1 ($7.4\text{V}$): Cấp nguồn công suất trực tiếp cho mạch cầu H `L298N` để lái 4 động cơ DC và cấp vào chân `VIN` của Arduino.*
> * *Bus 2 (Mạch hạ áp `BUCK LM2596`): Xuất $5.00\text{V} / 3\text{A}$ để nuôi riêng 3 động cơ Servo `MG996R` có dòng khởi động lên tới $2.5\text{A}$. Việc tách nguồn Buck riêng là nguyên lý sống còn trong phần cứng để tránh sụt áp gây sập nguồn vi điều khiển.*
> * *Trong hàm `ServoMG996RComp::onElectricalUpdate()`, em đặt điều kiện kiểm tra điện áp chân `VCC >= 4.8V`. Do đó khi em giảm áp Buck hoặc tắt công tắc nguồn bên Demo, cánh tay robot lập tức bị ngắt điện khóa cứng ạ."*

### 3. Thao tác Demo (Bên phải):
1. **Thử nghiệm tắt nguồn:** Bấm phím **T**: Công tắc `BAT_SW` chuyển sang OFF, dây dẫn tắt phát sáng ($0\text{V}$), 4 bánh xe và cánh tay ngừng hoạt động, dòng tổng tụt về $0\text{mA}$. Bấm **T** bật lại.
2. **Thử nghiệm giảm áp Buck:** Click chọn mạch `BUCK` $\rightarrow$ bấm nút **GIAM AP [-]** (hoặc phím `-`): Điện áp Buck giảm từ $5.0\text{V} \rightarrow 4.6\text{V}$.
3. Thử bấm phím **I / K** điều khiển nâng/hạ tay gắp: Cánh tay đứng im không nhúc nhích vì $V_{CC} < 4.8\text{V}$.
4. Bấm nút **TANG AP [+]** (hoặc phím `+`): Đưa áp trở lại $5.0\text{V} \rightarrow$ cánh tay hoạt động bình thường!

### 4. Vị trí Code cần Select trong VS Code (Bên trái):
* **File 1:** [CircuitSimulator.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/core/CircuitSimulator.cpp#L90-L135) (Dòng 90 – 135)
* **Từ khóa tìm nhanh:** `[SLIDE 7]`

```cpp
// [src/core/CircuitSimulator.cpp]: Lan truyền điện áp thời gian thực 3 Bus
void CircuitSimulator::stepCircuit(float dt) {
    if (bat) bat->onElectricalUpdate();

    // 1. Lan truyền BAT_SW (7.4V) -> DOMINO Bus 1
    if (bat && domino) {
        Pin* swOut = bat->getPin("SW_OUT");
        Pin* domIn7 = domino->getPin("IN_7V4");
        if (swOut && domIn7) domIn7->voltage = swOut->voltage;
        domino->onElectricalUpdate();
    }

    // 2. Lan truyền DOMINO -> BUCK hạ áp 5.0V nuôi riêng 3 Servo
    if (domino && buck) {
        Pin* domBuck = domino->getPin("OUT_BUCK");
        Pin* buckIn  = buck->getPin("IN+");
        if (domBuck && buckIn) buckIn->voltage = domBuck->voltage;
        buck->onElectricalUpdate();

        Pin* buckOut = buck->getPin("OUT+");
        Pin* domIn5  = domino->getPin("IN_BUCK_5V");
        if (buckOut && domIn5) domIn5->voltage = buckOut->voltage;
        domino->onElectricalUpdate();
    }
```

* **File 2:** [ActuatorComponents.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/components/ActuatorComponents.cpp#L1040-L1055) (Dòng 1040 – 1055: `isPowered = (vcc && vcc->voltage >= 4.8f);`)

---

## SLIDE 8: ĐỘNG HỌC CÁNH TAY 2 KHÂU (3 DOF) & GẮP KHỐI HÀNG
* **Thời lượng:** 3 Phút
* **Lý thuyết bài giảng:** Giáo trình `CG.4-3D Graphics-fix.pdf`, trang 5 – 16 (`Mat4` & Kinematics)

### 1. Mục tiêu & Giảng viên muốn thấy gì?
* Thấy được giải pháp hình học xuất sắc: Cánh tay vừa không cọ xuyên sàn Tầng 2, vừa vươn qua đầu cảm biến HC-SR04 để gắp vật ở cự ly 5 cm.
* Hiểu cách nhân dồn chuỗi ma trận thuần nhất $M_{world} = M_{parent\_world} \cdot M_{local}$ và cơ chế bắt dính vật thể (`Soft Magnetic Grasp`).

### 2. Lời thoại trình bày của sinh viên:
> *"Thưa Thầy/Cô, bài toán khó nhất của cánh tay robot trên xe là: Làm sao để cánh tay hạ thấp gắp được khối hàng dưới đất mà **không đâm xuyên sàn Tầng 2** và **không va đụng vào 2 mắt cảm biến HC-SR04** ở mũi xe.*
>
> *Em đã giải quyết trọn vẹn bằng mô hình động học 2 khâu (3 DOF) ở Slide 8:*
> * *Đế xoay `SRV_BASE` đặt tịnh tiến ra mũi xe tại $z = +6.20\text{ cm}$.*
> * *Khâu vai $L_1 = 8.20\text{ cm}$ luôn quay góc dương hướng lên trời ($+14^\circ$ đến $+62^\circ$), đẩy khớp khuỷu vươn vượt qua mép sàn Tầng 2.*
> * *Khâu khuỷu $L_2 = 8.60\text{ cm}$ từ trên cao chúc xuống mặt đất một góc $+68^\circ$, đưa tâm ngàm kẹp tới đúng cự ly $z = +18.2\text{ cm}$ và cao độ $y = +1.65\text{ cm}$ (ngay chính giữa khối hàng).*
> * *Toàn bộ chuỗi chuyển động được tính bằng hàm `computeGripTipWorldPos` đồng bộ 100% với ma trận OpenGL.*
> * *Khi khép ngàm kẹp trong vùng hợp lệ ($\Delta XZ \le 3.8\text{cm}, \Delta Y \le 3.2\text{cm}$), khối hàng tự động khóa tâm vào 2 miếng đệm cao su đỏ và nâng lên theo cánh tay ạ."*

### 3. Thao tác Demo (Bên phải):
1. Dùng phím **W / S / A / D** lái xe tiến lại gần khối hàng màu cam cho đến khi cảm biến HC-SR04 báo cự ly ~5 cm.
2. Bấm phím **K** hạ cánh tay xuống mức thấp nhất (`liftDeg = 0`): Vòng tròn tâm ngắm chiếu dưới sàn chuyển từ màu Xanh dương sang **Xanh lá cây sáng** (`RGB: 0.15, 1.0, 0.35`).
3. Bấm phím **P** (hoặc `G`): Ngàm kẹp đóng chặt ôm lấy khối hàng.
4. Bấm phím **I**: Nâng cánh tay lên cao $\rightarrow$ khối hàng được nhấc bổng lên không trung!
5. Bấm phím **J / L**: Xoay đế `SRV_BASE` sang trái / phải $\rightarrow$ khối hàng xoay chuyển mượt mà theo cánh tay!
6. (Tùy chọn): Bấm **R** reset, sau đó bấm phím **M** để trình diễn xe **tự hành dò đường & gắp vật tự động (`AUTO_GRAB`)**.

### 4. Vị trí Code cần Select trong VS Code (Bên trái):
* **File 1:** [RoboticArmRenderer.h](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/include/scene/RoboticArmRenderer.h#L170-L215) (Dòng 170 – 215)
* **Từ khóa tìm nhanh:** `[SLIDE 8]`

```cpp
// [include/scene/RoboticArmRenderer.h]: Hàm tính tọa độ 3D tâm điểm kẹp (Grip Tip Center)
static Vec3 computeGripTipWorldPos(
    const Vec3& carWorldPos, float carYawRad,
    float floor2Height, float baseYawDeg, float liftDeg)
{
    float t = std::max(0.0f, std::min(1.0f, liftDeg / 90.0f));
    float shoulderUpDeg  = 14.0f + t * 48.0f; // 14° (gắp) -> 62° (nâng cao)
    float forearmBendDeg = 68.0f - t * 36.0f; // 68° (chúc xuống) -> 32°

    const float baseLocalZ = 6.20f; // Đế tiến ra mũi sàn Tầng 2
    const float L1         = 8.20f; // Khâu vai 1
    const float L2         = 8.60f; // Khâu cẳng tay 2
    const float clawReach  = 2.65f; // Tới giữa 2 đệm cao su đỏ

    float sRad = shoulderUpDeg * 3.14159265f / 180.0f;
    float pitchDownDeg = forearmBendDeg - shoulderUpDeg;
    float fRad = pitchDownDeg * 3.14159265f / 180.0f;
    float cRad = 25.0f * 3.14159265f / 180.0f; // Bù góc cổ tay nghiêng 25°

    float forwardFromBase = L1 * std::cos(sRad) + L2 * std::cos(fRad) + clawReach * std::cos(cRad);
    float localY = floor2Height + 3.00f + L1 * std::sin(sRad) - L2 * std::sin(fRad) - clawReach * std::sin(cRad);
    // ... Chuyển sang World Space theo góc carYawRad ...
}
```

* **File 2:** [SceneNode.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/scene/SceneNode.cpp#L42-L52) (Dòng 42 – 52: `M_world = M_parent * M_local`)

---

## SLIDE 9 & 10: ĐỒ HỌA 2D TRÊN HUD & KẾT THÚC CẢM ƠN
* **Thời lượng:** 2 Phút
* **Lý thuyết bài giảng:** Giáo trình `CG.3-2D Graphics.pdf`, trang 12 – 88 (`Raster2D`, `Clipper2D`)

### 1. Mục tiêu & Giảng viên muốn thấy gì?
* Chứng minh sinh viên vận dụng đầy đủ các thuật toán 2D của học phần vào giao diện HUD: Bresenham, Midpoint Circle, Cohen-Sutherland 4-bit, Scanline Fill và BoundaryFill khử đệ quy bằng Stack.

### 2. Lời thoại trình bày của sinh viên:
> *"Thưa Thầy/Cô, phần cuối cùng là giao diện điều khiển 2D (HUD Dashboard) bên Demo, nơi em áp dụng trực tiếp toàn bộ học phần **2D Graphics (CG.3)**:*
> 1. *Tia quét Radar và đường chỉ dẫn Callout được vẽ bằng **Thuật toán đoạn thẳng Bresenham nguyên hóa** (`drawLineBresenham`, tr.12–22), hoàn toàn dùng số nguyên với biến quyết định `err = dx - dy`.*
> 2. *Các vòng chia cự ly trên Radar và vòng tròn tâm ngắm dưới sàn được vẽ bằng **Thuật toán đường tròn Midpoint** (`drawCircleMidpoint`, tr.29–37) tận dụng tính chất đối xứng 8 phần `put8pixel`.*
> 3. *Để tia quét và các đường vẽ không bị tràn ra ngoài khung hiển thị, em áp dụng **Thuật toán xén hình Cohen-Sutherland 4-bit** (`clipLineCohenSutherland`, tr.55–60). Nhờ phép kiểm tra bit `(code0 & code1) != 0`, những đoạn thẳng nằm ngoài được loại bỏ ngay lập tức mà không tốn phép chia số thực.*
> 4. *Các thanh trạng thái và nút bấm được tô màu bằng **Scanline Fill** và hàm **BoundaryFill_Stack** khử đệ quy bằng `std::stack` (tr.84–88) nhằm ngăn ngừa tuyệt đối lỗi tràn bộ nhớ ngăn xếp.*
>
> *Em xin kết thúc bài thuyết trình ở Slide 10 và rất mong nhận được những câu hỏi nhận xét, phản biện từ Thầy/Cô ạ!"*

### 3. Thao tác Demo (Bên phải):
1. Chỉ chuột vào **Radar cự ly 40cm** ở góc trên trái: Thấy tia quét quay tròn liên tục $360^\circ$ và điểm Blip đỏ phản hồi cự ly vật thể.
2. Chỉ vào **Khung viền PiP Claw Cam** góc trên phải và các đường viền HUD sắc nét, không bị lem mép.
3. Chuyển sang Slide 10 và giữ nguyên cửa sổ chương trình OpenGL cùng VS Code để sẵn sàng trả lời phản biện.

### 4. Vị trí Code cần Select trong VS Code (Bên trái):
* **File 1:** [Raster2D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/graphics2d/Raster2D.cpp#L14-L40) (Dòng 14 – 40: `drawLineBresenham`) & (Dòng 50 – 85: `drawCircleMidpoint` + `put8Pixel`)
* **Từ khóa tìm nhanh:** `[SLIDE 9]`
* **File 2:** [Clipper2D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/graphics2d/Clipper2D.cpp#L3-L28) (Dòng 3 – 28: `computeOutCode` & `clipLineCohenSutherland` với `code0 & code1`)
* **File 3:** [Raster2D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/graphics2d/Raster2D.cpp#L190-L230) (`boundaryFillStack` dùng `std::stack<Point2D>`)

---

## BỘ CÂU HỎI VẤN ĐÁP PHẢN BIỆN "ĂN ĐIỂM TUYỆT ĐỐI" (TỔNG HỢP 6 CÂU HỎI HỘI ĐỒNG HAY HỎI NHẤT)

### Câu 1: "Tại sao em phải tự viết lớp `Mat4` mà không dùng luôn hàm có sẵn của OpenGL như `glTranslatef`, `glRotatef`?"
* **Ý đồ kiểm tra:** Xem sinh viên hiểu bản chất ma trận thuần nhất $4 \times 4$ hay chỉ gọi thư viện.
* **Cách trả lời chuẩn:**
  > *"Dạ thưa Thầy/Cô, nếu chỉ gọi hàm OpenGL dựng sẵn thì các ma trận bị giấu kín bên trong OpenGL State Machine. Em sẽ **không thể lấy được tọa độ thế giới thực tế của ngàm kẹp ($z = 18.2\text{ cm}$)** để tính va chạm gắp vật thể, và cũng không thể thực hiện thuật toán bắn tia chuột Raycasting từ màn hình 2D vào 3D. Việc tự viết lớp `Mat4` giúp em làm chủ hoàn toàn cây phân cảnh Scene Graph và độc lập tính toán động học ạ."*

### Câu 2: "Làm sao em xử lý để cánh tay robot không đâm xuyên sàn Tầng 2 và không đụng cảm biến HC-SR04?"
* **Ý đồ kiểm tra:** Quy tắc dấu góc quay 3D và tư duy động học thuận.
* **Cách trả lời chuẩn:**
  > *"Dạ trong hệ trục OpenGL (+Y hướng lên, +Z hướng ra mũi xe), phép quay góc dương quanh trục +X sẽ làm cánh tay chúi xuống. Em đã đảo dấu góc khâu vai thành `-shoulderUpDeg` (với góc luôn dương từ $+14^\circ$ đến $+62^\circ$) để thanh dầm $L_1 = 8.2\text{ cm}$ luôn vươn vòng lên trên sàn Tầng 2. Sau đó, khâu khuỷu $L_2 = 8.6\text{ cm}$ mới quay chúc xuống $+68^\circ$ vươn qua đầu HC-SR04 xuống sát sàn đất ạ."*

### Câu 3: "Trong chế độ cô lập bo mạch, em đã khắc phục hiện tượng sọc rác (Z-fighting) giữa các lớp PCB và linh kiện bằng cách nào?"
* **Ý đồ kiểm tra:** Kiến thức thực chiến về Depth Buffer (Z-Buffer).
* **Cách trả lời chuẩn:**
  > *"Dạ em kết hợp 2 giải pháp:*
  > *1. Trong `HardwareComponent.cpp`, em kích hoạt `glEnable(GL_POLYGON_OFFSET_FILL)` và gọi `glPolygonOffset(1.0f, 1.0f)` để đẩy độ sâu đa giác nền ra xa một đơn vị nhỏ.*
  > *2. Trong mô hình 3D, đáy của mọi linh kiện con đều được nâng cao hơn mặt PCB một khoảng $\epsilon = 0.015\text{ cm}$, và tụ hóa 220 µF được dựng thành 2 đoạn nối tiếp (thân cách điện dưới + nắp nhôm trên) thay vì 2 hình trụ lồng nhau ạ."*

### Câu 4: "Trong thuật toán Cohen-Sutherland, làm sao biết ngay một đoạn thẳng nằm hoàn toàn bên ngoài cửa sổ mà không cần tính giao điểm?"
* **Ý đồ kiểm tra:** Mã vùng 4-bit và phép toán bit AND (CG.3, trang 57).
* **Cách trả lời chuẩn:**
  > *"Dạ em kiểm tra điều kiện `(code0 & code1) != 0` trong file `Clipper2D.cpp`. Nếu kết quả khác `0000`, nghĩa là cả hai đầu mút của đoạn thẳng cùng chia sẻ ít nhất một bit 1 (cùng nằm hẳn về phía Trái, Phải, Trên hoặc Dưới cửa sổ), hàm lập tức `return false` loại bỏ đoạn thẳng mà không tốn bất kỳ phép chia số thực nào ạ."*

### Câu 5: "Tại sao không dùng hàm BoundaryFill đệ quy 4 dòng cho ngắn gọn mà phải dùng Stack?"
* **Ý đồ kiểm tra:** Cảnh báo trang 84–88 giáo trình CG.3 về Stack Overflow.
* **Cách trả lời chuẩn:**
  > *"Dạ phương pháp đệ quy gọi hàm liên tục sẽ đẩy con trỏ và biến cục bộ vào Call Stack của hệ điều hành. Khi vùng tô màu có diện tích lớn hoặc độ phân giải cao, Call Stack sẽ bị tràn (Stack Overflow) làm chương trình bị crash đột ngột. Hàm `boundaryFillStack` trong `Raster2D.cpp` chuyển sang dùng `std::stack` trên bộ nhớ Heap, vừa chạy an toàn tuyệt đối vừa giữ vững tốc độ 60+ FPS ạ."*

### Câu 6: "Tại sao trên bo mạch Arduino đã có chân cấp nguồn 5V rồi mà trên xe vẫn phải gắn thêm mạch hạ áp BUCK LM2596?"
* **Ý đồ kiểm tra:** Kiến thức thực tế về hệ thống nhúng và phần cứng.
* **Cách trả lời chuẩn:**
  > *"Dạ chip ổn áp tuyến tính LDO trên Arduino Uno R3 chỉ chịu được dòng tối đa khoảng $500\text{mA} - 800\text{mA}$. Trong khi đó, 3 động cơ Servo MG996R khi tải nặng có dòng khởi động lên tới $2.5\text{A}$. Nếu cắm servo trực tiếp vào chân 5V của Arduino sẽ gây sụt áp làm sập nguồn vi điều khiển (Brownout Reset). Vì vậy, xe bắt buộc phải có mạch BUCK LM2596 (5.0V / 3A) cấp nguồn riêng cho servo và chỉ nối chung mass GND về Arduino ạ."*
