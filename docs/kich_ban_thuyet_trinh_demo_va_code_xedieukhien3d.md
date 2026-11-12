# KỊCH BẢN THUYẾT TRÌNH & BẢO VỆ ĐỒ ÁN (DEMO + LIVE CODE WALKTHROUGH)
**Đề tài:** Ứng dụng C++/OpenGL xây dựng chương trình mô phỏng và tương tác linh kiện vi điều khiển trên mô hình xe Robot gắp đồ vật (Digital Twin 3D)  
**Thư mục dự án:** `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`  
**Nhánh GitHub:** `Nguyen` (`https://github.com/DanhIT2006/3D_Model_Final_DHMT/tree/Nguyen`)

---

## PHẦN 1: GÓC NHÌN CỦA GIẢNG VIÊN (HỘI ĐỒNG CHẤM ĐỒ ÁN MUỐN THẤY GÌ?)

Khi chấm một đồ án kết hợp giữa **Đồ họa máy tính (2D/3D)** và **Lập trình hệ thống C++/OpenGL**, giảng viên không chấm điểm dựa trên việc mô hình đẹp như game thương mại, mà tìm kiếm **5 bằng chứng cốt lõi** sau đây để đánh giá điểm Giỏi/Xuất sắc (8.5 – 10):

### 1.1. Sự ăn khớp thật sự giữa lý thuyết trên lớp (`CG.3` & `CG.4`) và code chạy thực tế
* **Điều giảng viên muốn thấy:** Sinh viên không chỉ gọi các hàm dựng sẵn của thư viện một cách máy móc, mà **hiểu và tự cài đặt được bản chất toán học** trong slide bài giảng:
  * Biểu diễn vật thể bằng **Danh sách đỉnh – Danh sách cạnh (`WireFrame`)** và **Danh sách đỉnh – Danh sách mặt (`FaceModel`)** *(CG.4, tr.20–26)* trong lớp `Mesh3D`.
  * Tự xây dựng ma trận thuần nhất $4 \times 4$ trong lớp `Mat4` và tính được **phép biến đổi kết hợp** $M = M_1 \cdot M_2 \cdot M_3$ cho cây phân cấp chuyển động *(CG.4, tr.5–16)*.
  * Hiểu rõ **4 tham số quan sát 3D ($\theta, \Phi, R, D$)** trong lớp `Camera3D` và sự khác biệt giữa phép chiếu phối cảnh (`gluPerspective`) với 3 hình chiếu song song trực giao kỹ thuật Top/Front/Side View (`glOrtho`) *(CG.4, tr.28–49)*.
  * Tự cài đặt chay các thuật toán 2D: **Bresenham** vẽ đoạn thẳng, **Midpoint** vẽ đường tròn (`put8pixel`), **Cohen-Sutherland mã 4-bit** xén hình, **Scanline Fill** tô màu vào bảng giao diện HUD/Radar 2D trên màn hình *(CG.3, tr.12–102)*.

### 1.2. Tư duy thiết kế hệ thống (OOP & Scene Graph) rõ ràng, dễ bảo trì
* **Điều giảng viên muốn thấy:** Tại sao khi xe chạy thì toàn bộ linh kiện trên 2 tầng sàn trôi theo? Tại sao khi bàn trượt `SRV_ELBOW` nâng lên thì ngàm gắp `SRV_GRIP` và hộp quà tự động nhấc lên theo mà không cần tính tọa độ thủ công? Câu trả lời thuyết phục nhất là **cây phân cấp `SceneNode` (Scene Graph)** và lớp cơ sở trừu tượng `HardwareComponent`.

### 1.3. Chiều sâu của chữ "TƯƠNG TÁC LINH KIỆN VI ĐIỀU KHIỂN" trong tên đề tài
* **Điều giảng viên muốn thấy:** Đề tài không phải là một mô hình 3D tĩnh mà là một **Bản sao số (Digital Twin)**:
  * Có **luồng điện áp & tín hiệu thời gian thực**: Bật công tắc `BAT_SW` ($7.4\text{V}$) $\rightarrow$ qua trạm `DOMINO` $\rightarrow$ qua `BUCK LM2596` hạ áp xuống $5.0\text{V}$ nuôi 3 Servo; nếu ngắt `BUCK` hoặc tắt `BAT_SW` trên giao diện, người dùng bấm phím điều khiển cơ cấu nâng gắp sẽ **không nhúc nhích** vì mất nguồn.
  * Có **thuật toán Raycasting từ 2D vào 3D**: Click chuột lên màn hình phẳng sẽ bắn tia tìm giao điểm với hộp bao AABB/OBB để chọn linh kiện và hiển thị thông số điện áp/dòng điện trên bảng Inspector.

### 1.4. Khả năng làm chủ mã nguồn ("Chỉ trên màn hình Demo – Bật đúng dòng Code")
* **Điều giảng viên muốn thấy:** Khi giảng viên hỏi bất kỳ chi tiết nào (vị trí lắp đặt, công thức động học vi sai, cách xén hình Cohen-Sutherland), sinh viên mở ngay đúng file trong `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`, chỉ đúng hàm, giải thích đúng biến trong 10 giây.

---

## PHẦN 2: KỊCH BẢN THUYẾT TRÌNH ĐÓNG VAI SINH VIÊN (VỪA DEMO VỪA GIẢI THÍCH CODE)

> **Mẹo bố trí màn hình khi lên bảng bảo vệ:**
> Chia màn hình máy chiếu theo tỉ lệ **50% bên trái – 50% bên phải**:
> * **Nửa bên trái:** Cửa sổ chương trình OpenGL đang chạy trực tiếp (`XeDieuKhien3D.exe`).
> * **Nửa bên phải:** VS Code đang mở sẵn thư mục `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`, mở sẵn 5 tab code quan trọng nhất tương ứng với 5 chặng demo dưới đây.

---

### MỞ ĐẦU (1 Phút): Giới Thiệu Bài Toán & Cấu Trúc Dự Án
**Lời dẫn của bạn với Giảng viên:**
> *"Em chào Thầy/Cô. Hôm nay em xin trình bày đề tài: **Ứng dụng C++/OpenGL xây dựng chương trình mô phỏng và tương tác linh kiện vi điều khiển trên mô hình xe Robot gắp đồ vật (Digital Twin 3D)**. Thay vì chỉ vẽ một mô hình 3D tĩnh, mục tiêu của em là xây dựng một bản sao số có tính toán vật lý, mạch điện và chuyển động cơ khí thực tế, áp dụng trực tiếp các thuật toán trong hai học phần Đồ họa máy tính 2D (CG.3) và 3D (CG.4).*
>
> *Mã nguồn dự án được tổ chức tại thư mục `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D` chia làm 4 tầng độc lập:
> 1. `include/math/`: Hệ toán học vector và ma trận $4 \times 4$.
> 2. `include/graphics2d/`: Thuật toán vẽ 2D cơ sở Bresenham, Midpoint, Cohen-Sutherland.
> 3. `include/scene/`: Cây phân cảnh Scene Graph và mô hình Mesh 3D.
> 4. `include/components/` & `core/`: 10 linh kiện phần cứng chuẩn CAD và bộ giải mạch điện thời gian thực.
>
> *Sau đây em xin vừa thao tác trực tiếp trên bản Demo bên trái, vừa đối chiếu sang cách triển khai trong Code bên phải ạ."*

---

### CHẶNG 1 (3 Phút): Demo Quan Sát 3D & Phép Chiếu $\leftrightarrow$ Giải Thích `Camera3D.h`
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 28 – 49)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * Giữ chuột trái xoay vòng quanh xe (thay đổi góc $\theta$), nâng hạ góc nhìn từ trên xuống/dưới lên (thay đổi góc $\Phi$), lăn chuột phóng to/thu nhỏ khoảng cách quan sát ($R$).
   * Bấm lần lượt các phím số `1`, `2`, `3`, `4`, `5` để chuyển đổi:
     * Phím `1`: Góc nhìn phối cảnh tự do (Perspective Orbit).
     * Phím `2`: Chiếu bằng kỹ thuật (Top View – nhìn vuông góc từ trên xuống).
     * Phím `3`: Chiếu đứng kỹ thuật (Front View – nhìn thẳng vào đầu xe).
     * Phím `4`: Chiếu cạnh kỹ thuật (Side View – nhìn ngang sườn xe).
     * Phím `5`: Bám sau xe (Chase Camera).

2. **Chỉ vào Code (Bên phải – mở file `include/math/Camera3D.h` và `src/math/Camera3D.cpp`):**
   * **Vị trí code:**
     * `include/math/Camera3D.h` (dòng 22–25): khai báo 4 tham số cầu `theta`, `phi`, `radius` ($R$), `focalD` ($D$).
     * `src/math/Camera3D.cpp` (dòng 58–69 trong hàm `Camera3D::update`): công thức tính tọa độ mắt camera từ toạ độ cầu sang không gian Descart:
       $$x_{eye} = x_{target} + R \cdot \cos\Phi \cdot \sin\theta$$
       $$y_{eye} = y_{target} + R \cdot \sin\Phi$$
       $$z_{eye} = z_{target} + R \cdot \cos\Phi \cdot \cos\theta$$
     * `src/math/Camera3D.cpp` (dòng 71–85 trong hàm `Camera3D::applyProjection`):
       * Khi `mode == PERSPECTIVE_ORBIT || CAR_FOLLOW`: gọi `gluPerspective(fovY, aspect, zNear, zFar)`.
       * Khi ở chế độ hình chiếu kỹ thuật (Top/Front/Side): gọi `glOrtho(-orthoW, orthoW, -orthoH, orthoH, zNear, zFar)` triệt tiêu hoàn toàn độ biến dạng phối cảnh.
   * **Lời giải thích ghi điểm:**
     > *"Thưa Thầy/Cô, khi em kéo chuột xoay camera trên demo, hàm `Camera3D::update` áp dụng đúng tọa độ cầu $(\theta, \Phi, R, D)$ ở **CG.4 trang 28-35**. Khi em bấm phím 2/3/4 để xem hình chiếu kỹ thuật Top/Front/Side View, code chuyển từ ma trận phối cảnh sang **ma trận chiếu trực giao song song `glOrtho`** (CG.4 trang 31 & 48), giúp quan sát chính xác tỉ lệ kích thước CAD của 2 tầng sàn và các linh kiện mà không bị biến dạng xa gần ạ."*

---

### CHẶNG 2 (3 Phút): Demo Khung Dây Wireframe & Mặt Đa Giác Mesh3D $\leftrightarrow$ Giải Thích `Mesh3D.h`
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 20 – 26)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * Bấm phím `F` trên bàn phím: toàn bộ chiếc xe chuyển từ chế độ **Khối đặc đổ bóng (Polygon Mesh)** sang chế độ **Khung dây kết nối (WireFrame)** để lộ rõ từng cạnh và đỉnh. Bấm `F` lần nữa để trở lại bình thường.

2. **Chỉ vào Code (Bên phải – mở file `include/scene/Mesh3D.h` & `src/scene/Mesh3D.cpp`):**
   * **Vị trí code:**
     * `include/scene/Mesh3D.h` (dòng 9–25): định nghĩa `struct EdgeType { int beginP, endP; }` và `struct FaceType { vector<int> indexFace; Vec3 normal; Vec3 faceColor; }`.
     * `src/scene/Mesh3D.cpp` (dòng 15–55): hàm `Mesh3D::createBox(W, H, D, color)` tự động sinh 8 đỉnh, 12 cạnh wireframe và 6 mặt tứ giác kèm vector pháp tuyến $\vec{N}$ cho từng mặt.
     * `src/scene/Mesh3D.cpp` (dòng 197–233): hàm `Mesh3D::drawSolid()` duyệt danh sách `faces`, truyền `glNormal3f` phục vụ chiếu sáng Shading.
     * `src/scene/Mesh3D.cpp` (dòng 235–250): hàm `Mesh3D::drawWireframe()` duyệt danh sách `edges`, vẽ bằng `GL_LINES`.
   * **Lời giải thích ghi điểm:**
     > *"Thưa Thầy/Cô, em kết hợp trọn vẹn cả 2 mô hình biểu diễn vật thể trong bài giảng **CG.4 trang 20-26**: Lớp `Mesh3D` lưu một danh sách đỉnh `vertices` duy nhất. Khi bấm phím `F` sang chế độ WireFrame, hàm `drawWireframe()` nối các cặp đỉnh theo danh sách cạnh `edges`. Còn ở chế độ khối đặc, hàm `drawSolid()` duyệt danh sách mặt đa giác `faces` theo thứ tự đỉnh ngược chiều kim đồng hồ kèm vector pháp tuyến chuẩn hóa $\vec{N}$ để OpenGL tính toán ánh sáng bề mặt ạ."*

---

### CHẶNG 3 (4 Phút): Demo Động Học Xe Vi Sai & Trụ Nâng AGV $\leftrightarrow$ Giải Thích `Mat4.h`, `SceneNode.h` & `RobotCarScene.cpp`
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 5 – 16: Hệ tọa độ thuần nhất 4x4, Phép quay trục bất kỳ & Cây phân cảnh)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * Dùng phím `W / A / S / D` lái xe chạy: 2 bánh xe cao su quay tròn quanh trục ngang của nó, thân xe quay góc Yaw quanh trục đứng.
   * Lái xe tiến sát khối vật thể màu cam ở phía trước:
     * Bấm `I / K` để **nâng / hạ bàn trượt thẳng đứng** dọc theo 2 thanh ray mạ crom.
     * Bấm `O / P` để **mở rộng / kẹp chặt ngàm gắp đôi**.
     * Khi ngàm kẹp chặt ($< 25^\circ$) ở khoảng cách gần ($< 4.2\text{ cm}$), bấm `I` nhấc bổng khối hộp màu cam lên độ cao $10\text{ cm}$!
     * Hoặc bấm phím `M`: biểu diễn xe **tự hành gắp vật thông minh (Auto Grab chuỗi 5 bước)** từ tiếp cận $\rightarrow$ dừng xe $\rightarrow$ hạ bàn nâng $\rightarrow$ kẹp ngàm $\rightarrow$ nâng hàng lên cao!

2. **Chỉ vào Code (Bên phải – mở file `src/math/Mat4.cpp`, `src/scene/SceneNode.cpp` và `src/scene/RobotCarScene.cpp`):**
   * **Vị trí code:**
     * `src/math/Mat4.cpp`:
       * Dòng 18–48: `Mat4::translate`, `rotateX`, `rotateY`, `rotateZ`.
       * Dòng 68–95: `Mat4::rotateArbitraryAxis(Vec3 p0, Vec3 p1, float angleRad)` cài đặt đúng công thức đưa trục về gốc tọa độ và quay quanh 3 trục chính theo **CG.4 trang 13–14**:
         $$M = Tr(-P_0) \cdot R_x(\phi) \cdot R_y(-\theta) \cdot R_z(\alpha) \cdot R_y(\theta) \cdot R_x(-\phi) \cdot Tr(P_0)$$
     * `src/scene/SceneNode.cpp` (dòng 24–40): hàm `getWorldMatrix()` tính toán ma trận thế giới lũy tích theo quan hệ cha – con:
       $$M_{world} = M_{parent\_world} \cdot M_{local}$$
     * `src/scene/RobotCarScene.cpp` (dòng 122–144): hàm `updateDifferentialDrive(dt)` cài đặt công thức động học vi sai 2 bánh:
       $$v = \frac{v_R + v_L}{2}, \quad \omega = \frac{v_R - v_L}{W}$$
       $$\theta_{car} = \theta_{car} + \omega \Delta t, \quad X = X + v\sin(\theta)\Delta t, \quad Z = Z + v\cos(\theta)\Delta t$$
     * `src/scene/RobotCarScene.cpp` (dòng 144–185): hàm `updateArmAndGrasping(dt)` tính tọa độ thế giới của tâm điểm ngàm gắp:
       $$\vec{P}_{grip} = M_{grip\_world} \cdot \begin{bmatrix} 0 & 0 & 3.4 & 1 \end{bmatrix}^T$$
       Khi kẹp dính (`isObjectGrasped = true`), vật thể được gán tọa độ bám chặt theo tâm ngàm: `targetObject->localPos = gripWorldPos;` nên khi bàn trượt tịnh tiến theo phương thẳng đứng $Y$, vật thể tự động được nâng lên cao.
   * **Lời giải thích ghi điểm:**
     > *"Thưa Thầy/Cô, cây phân cảnh Scene Graph là chìa khóa giúp cơ cấu hoạt động mượt mà: `CarRoot` $\rightarrow$ `Floor1` $\rightarrow$ `SRV_BASE` (Trụ) $\rightarrow$ `SRV_ELBOW` (Bàn trượt) $\rightarrow$ `SRV_GRIP` (Ngàm kẹp) $\rightarrow$ `TargetObject`. Mỗi khớp chỉ quản lý biến đổi cục bộ $M_{local}$. Hàm `getWorldMatrix()` nhân dồn từ nút cha xuống nút con. Do đó khi xe quay hay chạy, toàn bộ trụ nâng, bàn trượt, ngàm kẹp và khối vật thể đang kẹp đều tự động biến đổi chính xác mà không cần tính toán thủ công từng tọa độ ạ."*

---

### CHẶNG 4 (3 Phút): Demo Tương Tác Mạch Điện & Raycasting Chọn Linh Kiện $\leftrightarrow$ Giải Thích `CircuitSimulator.cpp`
*(Điểm nhấn chứng minh đúng tính chất "Tương tác linh kiện vi điều khiển" của đề tài)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * **Thử nghiệm 1 (Bắn tia Raycasting chọn linh kiện 3D):** Dùng chuột click vào bo mạch Arduino, cầu H L298N, mạch Buck, hay Hộp pin $\rightarrow$ Linh kiện được click lập tức sáng viền màu vàng; bảng **INSPECTOR** góc phải màn hình cập nhật ngay tên, kích thước CAD chuẩn, điện áp vào và dòng tiêu thụ thực tế.
   * **Thử nghiệm 2 (Ngắt nguồn tổng bằng phím `T`):** Bấm phím `T` để gạt tắt công tắc pin `BAT_SW` $\rightarrow$ Toàn bộ dây dẫn 3D tắt phát sáng ($0\text{V}$), động cơ báo `NGAT NGUON (OFF)`, dòng tiêu thụ tổng $I_{total}$ trên đồng hồ HUD tụt về $0\text{mA}$, bấm lái xe hay bấm `I/K/O/P` ngàm gắp đều đứng im. Bấm `T` bật lại $\rightarrow$ hệ thống hồi sinh, dây dẫn sáng viền neon theo màu chuẩn công nghiệp (Đỏ 7.4V, Cam 5V, Vàng PWM).
   * **Thử nghiệm 3 (Điều chỉnh chiết áp Buck LM2596 bằng phím `+` / `-`):** Bấm phím `+` hoặc `-` để tăng giảm điện áp ra của mạch Buck từ 3.0V đến 6.0V.

2. **Chỉ vào Code (Bên phải – mở file `src/core/CircuitSimulator.cpp` và `src/scene/SceneNode.cpp`):**
   * **Vị trí code:**
     * `src/scene/SceneNode.cpp` (dòng 50–75 trong hàm `SceneNode::raycast`): biến đổi tia Ray từ Camera vào hệ tọa độ cục bộ của từng linh kiện qua ma trận nghịch đảo $M_{world}^{-1}$, kiểm tra giao cắt với hình hộp bao AABB kích thước `cadSize * 0.5f`.
     * `src/core/CircuitSimulator.cpp` (dòng 90–247 trong hàm `stepCircuit`):
       * Kiểm tra đóng ngắt nguồn từ `BAT_SW`.
       * Lan truyền $7.4\text{V}$ vào trạm `DOMINO`, phân phối sang chân `VIN` của `ARD`, cọc `12V` của `L298N` và đầu vào của `BUCK LM2596`.
       * Mạch `BUCK` hạ áp xuất $5.0\text{V}$ ổn định cấp riêng cho 3 servo (`SRV_BASE`, `SRV_ELBOW`, `SRV_GRIP`) để tránh sụt áp hệ thống.
       * Dòng điện tổng $I_{total}$ tính toán liên tục: $I_{total} = I_{Arduino} + I_{Buck} + I_{Motors}(\text{RPM}) + I_{Servos}$.
   * **Lời giải thích ghi điểm:**
     > *"Mô hình của em là một Digital Twin hoạt động thật chứ không phải ảnh diễn họa. Mỗi linh kiện kế thừa từ `HardwareComponent` có hệ thống chân cắm `pins` thực tế. Trong hàm `stepCircuit`, điện áp được giải tuần tự theo sơ đồ mạch: pin $7.4\text{V} \rightarrow$ Domino $\rightarrow$ Buck hạ áp $5\text{V} \rightarrow$ Servo. Nếu tắt nguồn hoặc sụt áp dưới $4.5\text{V}$, hàm `updateMotion` của Servo sẽ `return;` lập tức, ngắt toàn bộ chuyển động cơ khí đúng như mạch thật ngoài đời ạ."*

---

### CHẶNG 5 (3 Phút): Demo Thuật Toán Đồ Họa 2D Trên Dashboard HUD $\leftrightarrow$ Giải Thích `Raster2D.cpp` & `Clipper2D.cpp`
*(Kiến thức cốt lõi: Slide `CG.3-2D Graphics.pdf`, trang 12 – 102)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * Chỉ chuột vào bảng HUD 2D trên màn hình:
     * **Widget Radar cự ly (góc dưới trái):** Các vòng tròn đồng tâm khoảng cách $10\text{cm}, 20\text{cm}, 30\text{cm}, 40\text{cm}$; tia quét radar quay liên tục $360^\circ$; khi xe lại gần hộp mẫu màu cam, một điểm Blip đỏ nhấp nháy xuất hiện đúng vị trí cự ly thực.
     * **Máy hiện sóng Oscilloscope (góc trên trái):** Hiển thị dạng sóng vuông xung PWM điều khiển tốc độ động cơ theo tần số thời gian thực.
     * **Tính năng xén hình:** Tia quét Radar và đường sóng Oscilloscope luôn nằm gọn gàng bên trong khung cửa sổ, không bao giờ bị lem nét vẽ ra ngoài.

2. **Chỉ vào Code (Bên phải – mở file `src/graphics2d/Raster2D.cpp` & `src/graphics2d/Clipper2D.cpp`):**
   * **Vị trí code:**
     * `src/graphics2d/Raster2D.cpp`:
       * Dòng 12–50: hàm `Raster2D::drawLineBresenham` cài đặt **thuật toán đoạn thẳng Bresenham nguyên hóa** *(CG.3 trang 12–25)* với tham số quyết định $p_0 = 2\Delta y - \Delta x$, chỉ dùng phép cộng và dịch bit số nguyên để vẽ tia quét và lưới tọa độ.
       * Dòng 55–95: hàm `Raster2D::drawCircleMidpoint` cài đặt **thuật toán đường tròn Midpoint** *(CG.3 trang 28–36)* với tham số $p_0 = 1 - R$, tận dụng tính đối xứng 8 phần của hàm `put8pixel` để vẽ các vòng tròn cự ly radar.
       * Dòng 100–145: hàm `Raster2D::drawEllipseMidpoint` với tham số khởi tạo 2 vùng $p_{10}$ và $p_{20}$ kèm hàm `put4pixel` *(CG.3 trang 39–44)*.
       * Dòng 150–170: hàm `Raster2D::fillRect` tô màu khối theo thuật toán quét dòng **Scanline Fill** *(CG.3 trang 78–82)*.
     * `src/graphics2d/Clipper2D.cpp`:
       * Dòng 10–25: hàm `computeOutCode(x, y, win)` gán mã nhị phân 4-bit `[TOP, BOTTOM, RIGHT, LEFT]` *(CG.3 trang 54–61)*.
       * Dòng 30–75: hàm `clipLineCohenSutherland` thực hiện phép toán bit `Code1 & Code2 != 0` để loại bỏ nhanh và tìm giao điểm chính xác tại 4 cạnh cửa sổ xén.
   * **Lời giải thích ghi điểm:**
     > *"Toàn bộ giao diện 2D được em tự viết bằng các thuật toán cơ sở trong **CG.3**: Vòng tròn radar dùng thuật toán Midpoint `put8pixel` để tránh dùng hàm lượng giác số thực; tia quét radar vẽ bằng Bresenham số nguyên; và đặc biệt trước khi vẽ bất kỳ đoạn thẳng nào lên HUD, code đều cho đi qua hàm `clipLineCohenSutherland` để xén hình bằng mã vùng 4-bit theo đúng giáo trình trang 54-68, đảm bảo đồ thị và tia quét không bao giờ bị tràn ra ngoài màn hình ạ."*

---

## PHẦN 3: BỘ CÂU HỎI PHẢN BIỆN GIẢNG VIÊN HAY HỎI NHẤT & CÁCH TRẢ LỜI "ĂN ĐIỂM TUYỆT ĐỐI"

| Câu hỏi của Giảng viên | Ý đồ kiểm tra của Giảng viên | Cách bạn trả lời + Thao tác chỉ vào Code |
| :--- | :--- | :--- |
| **Câu 1:** *"Tại sao em phải tự viết lớp `Mat4` mà không dùng luôn ma trận sẵn có của OpenGL như `glTranslatef`, `glRotatef`?"* | Kiểm tra xem sinh viên có hiểu bản chất toán học hay chỉ lạm dụng OpenGL. | *"Thưa Thầy/Cô, nếu dùng hàm OpenGL dựng sẵn thì ma trận bị giấu trong OpenGL State Machine, em sẽ không thể lấy được toạ độ thế giới của ngàm kẹp để tính va chạm gắp vật thể, và cũng không thể tính toán Raycasting từ chuột 2D vào 3D. Việc tự xây dựng lớp `Mat4` với hệ toạ độ thuần nhất $4 \times 4$ giúp em hoàn toàn làm chủ thuật toán, kiểm soát cây phân cảnh Scene Graph và tính toán động học vi sai một cách độc lập ạ."* |
| **Câu 2:** *"Trong thuật toán Cohen-Sutherland, làm sao chương trình biết ngay một đoạn thẳng nằm hoàn toàn bên ngoài cửa sổ mà không cần tính giao điểm?"* | Kiểm tra hiểu biết về mã vùng 4-bit và phép toán `AND` bit *(CG.3 tr.57)*. | *"Dạ em dùng phép toán bit `(code0 & code1) != 0` trong file `src/graphics2d/Clipper2D.cpp`. Nếu kết quả khác `0000`, nghĩa là cả hai đầu mút cùng chia sẻ ít nhất một bit `1` (cùng nằm hẳn về phía Trái, Phải, Trên hoặc Dưới cửa sổ), nên đoạn thẳng chắc chắn nằm ngoài và hàm `return false` loại bỏ ngay lập tức mà không tốn phép chia số thực nào ạ."* |
| **Câu 3:** *"Tại sao thuật toán Bresenham lại tối ưu hơn thuật toán DDA khi vẽ đoạn thẳng?"* | Kiểm tra kiến thức trang 12–25 của slide `CG.3-2D Graphics.pdf`. | *"Dạ thuật toán DDA phải tính toán trên số thực (cộng dồn hệ số góc $m = \Delta y / \Delta x$ rồi làm tròn `round`), gây sai số tích lũy và tốc độ chậm. Thuật toán Bresenham mà em cài đặt trong `Raster2D.cpp` đã nguyên hóa hoàn toàn bài toán: dùng biến quyết định $p_k = 2\Delta y - \Delta x$, chỉ sử dụng phép cộng và phép nhân 2 (dịch bit) trên số nguyên, do đó tối ưu tuyệt đối cho phần cứng đồ họa ạ."* |
| **Câu 4:** *"Tại sao trên xe có pin $7.4\text{V}$ cấp vào chân `VIN` của Arduino (Arduino cũng có chân ra $5\text{V}$), mà em lại phải thiết kế thêm mạch hạ áp `BUCK LM2596`?"* | Kiểm tra độ hiểu thực tế về thiết kế hệ thống nhúng của sinh viên. | *"Dạ bộ ổn áp LDO $5\text{V}$ tích hợp trên mạch `Arduino Uno R3` chỉ chịu được dòng tối đa khoảng $500\text{mA} - 800\text{mA}$. Trong khi đó mỗi động cơ Servo `MG996R` khi nâng tải có dòng khởi động lên tới $1.5\text{A} - 2.5\text{A}$. Nếu cắm 3 Servo vào chân $5\text{V}$ của Arduino sẽ gây sụt áp làm vi điều khiển bị sập nguồn (Brownout Reset). Do đó mô hình Digital Twin của em dùng mạch `BUCK LM2596` ($5\text{V}/3\text{A}$) cấp nguồn riêng cho 3 Servo, chỉ nối chung dây `GND` và dây tín hiệu `SIG` về Arduino ạ."* |
| **Câu 5:** *"Cơ chế phát hiện và kẹp vật thể được xử lý trong code như thế nào?"* | Kiểm tra sự liên kết giữa Scene Graph và logic vật lý. | *"Dạ trong hàm `updateArmAndGrasping` ([src/scene/RobotCarScene.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/scene/RobotCarScene.cpp)), em lấy ma trận thế giới của ngàm kẹp `compSrvGrip->getWorldMatrix()`, nhân với vector tâm ngàm $[0, 0, 3.4, 1]^T$ để ra tọa độ thực trong thế giới. Khi khoảng cách tới vật thể $< 4.2\text{ cm}$ và góc ngàm khép lại $< 25^\circ$, cờ `isObjectGrasped` kích hoạt, tọa độ vật thể sẽ khóa cứng theo tâm ngàm và nâng lên theo bàn trượt ạ."* |

---

## PHẦN 4: DANH SÁCH KIỂM TRA (CHECKLIST) 15 PHÚT TRƯỚC KHI LÊN BẢNG

- [ ] **1. Chuẩn bị sẵn 2 cửa sổ trên màn hình (Tỉ lệ 50% - 50%):**
  - Nửa bên trái: Chạy sẵn file `XeDieuKhien3D.exe`.
  - Nửa bên phải: Mở sẵn VS Code thư mục `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D` và ghim (Pin) 5 tab code chính:
    1. `include/math/Camera3D.h` & `src/math/Camera3D.cpp` *(Chặng 1: Camera cầu $\theta, \Phi, R, D$ & Phép chiếu)*
    2. `include/scene/Mesh3D.h` & `src/scene/Mesh3D.cpp` *(Chặng 2: Khung dây `WireFrame` & Mặt `FaceModel`)*
    3. `src/math/Mat4.cpp` & `src/scene/SceneNode.cpp` *(Chặng 3: Ma trận Affine $4 \times 4$ & Cây Scene Graph)*
    4. `src/core/CircuitSimulator.cpp` *(Chặng 4: Lan truyền điện áp & Ngắt nguồn `BAT_SW`)*
    5. `src/graphics2d/Raster2D.cpp` & `src/graphics2d/Clipper2D.cpp` *(Chặng 5: Bresenham, Midpoint, Cohen-Sutherland)*
- [ ] **2. Phóng to chữ trong VS Code:** Bấm `Ctrl + +` tăng cỡ chữ lên **16pt – 18pt** để Thầy/Cô ngồi xa cũng nhìn rõ từng dòng công thức ma trận.
- [ ] **3. Bảng tra cứu phím tắt điều khiển nhanh (Cheat Sheet):**
  - `W / A / S / D`: Lái xe tiến / trái / lùi / phải.
  - `Space`: Phanh dừng xe khẩn cấp.
  - `I / K`: Nâng lên / Hạ xuống bàn trượt thẳng đứng (Linear Lift).
  - `O / P`: Mở rộng / Kẹp chặt ngàm gắp (Gripper).
  - `M`: Chuyển đổi Thủ công $\leftrightarrow$ Tự hành gắp vật thông minh (Auto Grab 5 bước).
  - `F`: Bật/Tắt khung dây Wireframe (CG.4).
  - `1, 2, 3, 4, 5`: Đổi góc nhìn Camera (3D tự do, Chiếu bằng, Chiếu đứng, Chiếu cạnh, Bám sau xe).
  - `T`: Bật/Tắt công tắc nguồn pin `BAT_SW` (7.4V).
  - `C`: Ẩn/Hiện đường dây điện 3D.
  - `+ / -`: Tăng/Giảm điện áp mạch Buck LM2596.
  - `R`: Reset toàn bộ xe, cơ cấu nâng và vật thể về vị trí xuất phát.
  - `Click chuột trái`: Raycast chọn linh kiện 3D xem thông số trên bảng Inspector.
