# KỊCH BẢN THUYẾT TRÌNH & BẢO VỆ ĐỒ ÁN (DEMO + LIVE CODE WALKTHROUGH)
**Đề tài:** Ứng dụng C++/OpenGL xây dựng chương trình mô phỏng và tương tác linh kiện vi điều khiển trên mô hình xe Robot gắp đồ vật (Digital Twin 3D)
**Thư mục dự án:** `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`

---

## PHẦN 1: GÓC NHÌN CỦA GIẢNG VIÊN (HỘI ĐỒNG CHẤM ĐỒ ÁN MUỐN THẤY GÌ?)

Khi chấm một đồ án kết hợp giữa **Đồ họa máy tính (2D/3D)** và **Lập trình hệ thống C++/OpenGL**, giảng viên không chấm điểm dựa trên việc mô hình đẹp như game thương mại, mà tìm kiếm **5 bằng chứng cốt lõi** sau đây để đánh giá điểm Giỏi/Xuất sắc (8.5 – 10):

### 1.1. Sự ăn khớp thật sự giữa lý thuyết trên lớp (`CG.3` & `CG.4`) và code chạy thực tế
* **Điều giảng viên muốn thấy:** Sinh viên không chỉ gọi các hàm dựng sẵn của thư viện (`glutSolidCube`, `gluLookAt`) một cách máy móc, mà **hiểu và tự cài đặt được bản chất toán học** trong slide bài giảng:
  * Biểu diễn vật thể bằng **Danh sách đỉnh – Danh sách cạnh (`WireFrame`)** và **Danh sách đỉnh – Danh sách mặt (`FaceModel`)** *(CG.4, tr.20–26)*.
  * Tự xây dựng ma trận thuần nhất $4 \times 4$ và tính được **phép biến đổi kết hợp** $M = M_1 \cdot M_2 \cdot M_3$ cho chuỗi chuyển động của xe và cơ cấu nâng gắp *(CG.4, tr.5–16)*.
  * Hiểu rõ **4 tham số quan sát 3D ($\theta, \Phi, R, D$)** trong lớp `Camera3D` và sự khác biệt giữa phép chiếu phối cảnh (`gluPerspective`) với 3 hình chiếu song song trực giao kỹ thuật Top/Front/Side View (`glOrtho`) *(CG.4, tr.28–49)*.
  * Vận dụng khéo léo các thuật toán 2D (**Bresenham/Midpoint** vẽ đoạn thẳng – đường tròn – elip, **Cohen-Sutherland** xén hình, **Scanline Fill** tô màu, **Font Bitmap**) vào bảng giao diện HUD/Radar 2D trên màn hình *(CG.3, tr.12–102)*.

### 1.2. Tư duy thiết kế hệ thống (OOP & Scene Graph) rõ ràng, dễ bảo trì
* **Điều giảng viên muốn thấy:** Tại sao khi xe chạy thì toàn bộ linh kiện trên 2 tầng sàn trôi theo? Tại sao khi bàn trượt `SRV_ELBOW` nâng lên thì ngàm gắp `SRV_GRIP` và hộp quà tự động nhấc lên theo mà không cần cộng tọa độ thủ công? Câu trả lời thuyết phục nhất là **cây phân cấp `SceneNode` (Scene Graph)** và lớp cơ sở trừu tượng `HardwareComponent`.

### 1.3. Chiều sâu của chữ "TƯƠNG TÁC LINH KIỆN VI ĐIỀU KHIỂN" trong tên đề tài
* **Điều giảng viên muốn thấy:** Đề tài không phải là một bức tượng 3D chỉ biết xoay qua xoay lại. Nó phải là một **Bản sao số (Digital Twin)**:
  * Có **luồng điện áp & tín hiệu thời gian thực**: Bật công tắc `BAT_SW` ($7.4\text{V}$) $\rightarrow$ qua `DOMINO` $\rightarrow$ qua `BUCK LM2596` hạ áp xuống $5.0\text{V}$ nuôi 3 Servo; nếu ngắt `BUCK` hoặc tắt `BAT_SW` trên giao diện, người dùng bấm phím điều khiển cơ cấu nâng gắp sẽ **không nhúc nhích** vì mất nguồn.
  * Có **phản hồi cự ly & tự hành gắp vật**: Khi xe tiến lại gần vật thể, radar cự ly tính khoảng cách 3D thực tế $\rightarrow$ gửi tín hiệu về `ARD` $\rightarrow$ tự động phanh động cơ `L298N` và kích hoạt chu trình 5 bước tự động gắp vật (Auto Grab).
  * Có **tương tác Raycasting từ 2D vào 3D**: Click chuột lên màn hình phẳng sẽ bắn tia tìm giao điểm với hộp bao AABB/OBB để chọn linh kiện và hiển thị thông số điện áp/dòng điện trên bảng Inspector.

### 1.4. Khả năng làm chủ mã nguồn ("Chỉ trên màn hình Demo – Bật đúng dòng Code")
* **Điều giảng viên muốn thấy:** Khi giảng viên hỏi *"Em thử đổi kích thước của mạch Arduino Uno, đổi công thức vi sai hay chỉ thuật toán xén hình ở đâu?"*, sinh viên mở ngay đúng file trong `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`, chỉ đúng hàm, giải thích đúng biến và sửa trong 10 giây.

---

## PHẦN 2: KỊCH BẢN THUYẾT TRÌNH ĐÓNG VAI SINH VIÊN (VỪA DEMO VỪA GIẢI THÍCH CODE)

> **Mẹo bố trí màn hình khi lên bảng bảo vệ:**
> Chia màn hình máy chiếu theo tỉ lệ **50% bên trái – 50% bên phải**:
> * **Nửa bên trái:** Cửa sổ chương trình OpenGL đang chạy trực tiếp (`XeDieuKhien3D.exe`).
> * **Nửa bên phải:** VS Code đang mở sẵn thư mục `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`, mở sẵn 5 tab code quan trọng nhất tương ứng với 5 bước demo dưới đây.

---

### MỞ ĐẦU (1 Phút): Giới Thiệu Bài Toán & Cấu Trúc Dự Án
**Lời dẫn của bạn với Giảng viên:**
> *"Em chào Thầy/Cô. Hôm nay em xin trình bày đề tài: **Ứng dụng C++/OpenGL xây dựng chương trình mô phỏng và tương tác linh kiện vi điều khiển trên mô hình xe Robot gắp đồ vật (Digital Twin 3D)**. Thay vì chỉ vẽ một mô hình 3D tĩnh, mục tiêu của em là xây dựng một **Bản sao số** gồm đúng **10 linh kiện phần cứng thực tế** theo kích thước CAD chuẩn, kết hợp đầy đủ các thuật toán Đồ họa 2D và 3D đã học.*
>
> *Toàn bộ mã nguồn dự án được em tổ chức tại thư mục `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D` chia làm 4 tầng độc lập: Toán học 3D (`math/`), Đồ họa cơ sở 2D (`graphics2d/`), Cây phân cảnh (`scene/`) và Logic mạch điện các linh kiện (`components/` & `core/`). Sau đây em xin vừa thao tác trực tiếp trên bản Demo bên trái, vừa đối chiếu sang cách triển khai trong Code bên phải ạ."*

---

### CHẶNG 1 (3 Phút): Demo Quan Sát 3D & Phép Chiếu $\leftrightarrow$ Giải Thích `Camera3D.h`
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 28 – 49)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * Dùng chuột kéo xoay vòng quanh xe theo chiều ngang (thay đổi góc $\theta$), nâng hạ góc nhìn từ trên xuống/dưới lên (thay đổi góc $\Phi$), lăn chuột phóng to/thu nhỏ khoảng cách quan sát ($R$).
   * Bấm lần lượt các phím số `1`, `2`, `3`, `4`, `5` để chuyển đổi giữa **Góc nhìn Phối cảnh 3D (Phím 1)** và **3 Hình chiếu Trực giao kỹ thuật (Top View phím 2 – nhìn từ trên, Front View phím 3 – nhìn từ trước, Side View phím 4 – nhìn từ bên, Chase Cam phím 5 – bám sau xe)** để thầy/cô thấy rõ cách bố trí Sàn Tầng 1 và Sàn Tầng 2.

2. **Chỉ vào Code (Bên phải – mở file `Camera3D.h` & `Camera3D.cpp`):**
   * Chỉ vào 4 biến thành viên `theta`, `phi`, `radius` ($R$), `focalD` ($D$) và hàm cập nhật tọa độ quan sát `Camera3D::update()`, hàm ma trận chiếu `Camera3D::applyProjection()`.
   * **Lời giải thích ghi điểm với Giảng viên:**
     > *"Thưa Thầy/Cô, khi em kéo chuột xoay camera trên màn hình demo, trong code tại lớp `Camera3D`, em áp dụng đúng hệ tọa độ cầu 4 tham số $(\theta, \Phi, R, D)$ ở trang 28–35 bài giảng 3D Graphics:*
     > * *Trong hàm `Camera3D::update`, tọa độ mắt được tính từ tọa độ cầu sang Descart:*
     >   $$x_{eye} = x_{target} + R \cdot \cos\Phi \cdot \sin\theta$$
     >   $$y_{eye} = y_{target} + R \cdot \sin\Phi$$
     >   $$z_{eye} = z_{target} + R \cdot \cos\Phi \cdot \cos\theta$$
     > * *Khi em bật chế độ **Phối cảnh (Phím 1)**, hàm `applyProjection()` gọi `gluPerspective` chia cho độ sâu $z$ theo tỉ lệ đồng dạng tam giác (vật ở gần mắt thì lớn, ở xa thì nhỏ). Còn khi em bấm sang chế độ **Bản vẽ kỹ thuật Top/Front/Side View (Phím 2, 3, 4)**, code chuyển sang **Phép chiếu song song trực giao** bằng hàm `glOrtho` (trang 31 & 48) triệt tiêu thành phần độ sâu, giúp kiểm tra chính xác kích thước CAD của các tầng sàn và linh kiện mà không bị biến dạng phối cảnh ạ."*

---

### CHẶNG 2 (3 Phút): Demo Mô Hình Khung Dây / Mặt Đa Giác & Linh Kiện $\leftrightarrow$ Giải Thích `Mesh3D.h`
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 20 – 26)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * Bấm phím `F` để chuyển toàn bộ chiếc xe từ chế độ **Khối đặc (Polygon Mesh)** sang chế độ **Khung dây kết nối (WireFrame)** rồi bật lại.
   * Click chuột vào một vài linh kiện tiêu biểu ở Tầng 1 (`DOMINO`, `L298N`, `BUCK`) và Tầng 2 (`ARD`, `BAT_SW`, `SRV_BASE`) để làm sáng viền vàng (highlight) linh kiện và hiển thị kích thước $(W \times H \times D)$ trên bảng Inspector.

2. **Chỉ vào Code (Bên phải – mở file `Mesh3D.h` & `Mesh3D.cpp`):**
   * Chỉ vào cấu trúc `EdgeType { int beginP, endP; }`, `FaceType { vector<int> indexFace; Vec3 normal; Vec3 faceColor; }` và hàm `Mesh3D::createBox(W, H, D, color)`.
   * **Lời giải thích ghi điểm với Giảng viên:**
     > *"Thưa Thầy/Cô, để dựng hình các linh kiện theo đúng bảng thông số kỹ thuật (ví dụ mạch `ARD` kích thước $6.8 \times 1.5 \times 5.3\text{ cm}$ ở Sàn Tầng 2 hay `L298N` kích thước $4.3 \times 2.7 \times 4.3\text{ cm}$ ở Sàn Tầng 1), em kết hợp cả 2 phương pháp biểu diễn đối tượng 3D trong bài giảng trang 20–26:*
     > * *Trong lớp `Mesh3D`, em lưu một **Danh sách đỉnh (`vertices`)** duy nhất.*
     > * *Khi em bấm phím `F` sang chế độ **WireFrame** trên demo, hàm `drawWireframe()` sẽ duyệt qua **Danh sách cạnh (`edges`)**, nối cặp chỉ số đỉnh đầu `beginP` và đỉnh cuối `endP`.*
     > * *Khi ở chế độ **Khối đặc**, hàm `drawSolid()` duyệt qua **Danh sách các mặt đa giác (`faces`)** theo thứ tự đỉnh `indexFace` ngược chiều kim đồng hồ để tính vector pháp tuyến $\vec{N}$ phục vụ tô bóng chiếu sáng.*
     > * *Nhờ hàm `Mesh3D::createBox(W, H, D)`, toàn bộ linh kiện được sinh tự động từ kích thước thực tế trong bảng thông số mà không phải gõ tay tọa độ từng đỉnh lẻ tẻ ạ."*

---

### CHẶNG 3 (4 Phút): Demo Chuyển Động Xe & Trụ Nâng AGV Gắp Vật $\leftrightarrow$ Giải Thích `Mat4.h` & `SceneNode.h`
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 5 – 16: Hệ tọa độ thuần nhất & Kết hợp các phép biến đổi)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * Dùng phím `W / A / S / D` điều khiển xe tiến/lùi/rẽ trái/rẽ phải: chỉ cho giảng viên thấy 2 bánh xe `MOT_L`, `MOT_R` quay tròn quanh trục của nó, thân xe chuyển động theo động học vi sai.
   * Điều khiển cơ cấu trụ nâng thẳng đứng AGV và ngàm kẹp gắp vật:
     * Bấm `I / K`: Nâng / hạ bàn trượt `SRV_ELBOW` tịnh tiến thẳng đứng dọc theo 2 thanh ray mạ crom.
     * Bấm `O / P`: Mở rộng / khép chặt cụm ngàm kẹp song song `SRV_GRIP`.
     * Khi ngàm kẹp chặt khối vật thể màu cam ở cự ly gần, bấm `I` nhấc bổng khối vật thể lên độ cao $10\text{ cm}$!
     * Hoặc bấm phím `M`: biểu diễn xe **tự hành gắp vật thông minh (Auto Grab 5 bước)** từ tiếp cận $\rightarrow$ dừng xe $\rightarrow$ hạ bàn nâng $\rightarrow$ kẹp ngàm $\rightarrow$ nâng hàng lên cao!

2. **Chỉ vào Code (Bên phải – mở file `Mat4.h`, `Mat4.cpp`, `SceneNode.h` & `RobotCarScene.cpp`):**
   * Chỉ vào hàm `SceneNode::getWorldMatrix()`, hàm `Mat4::rotateArbitraryAxis()`, và hàm `RobotCarScene::updateDifferentialDrive()`.
   * **Lời giải thích ghi điểm với Giảng viên:**
     > *"Đây là phần em tâm đắc nhất khi áp dụng bài học **Hệ tọa độ thuần nhất $4 \times 4$ và Kết hợp các phép biến đổi 3D** (trang 5–16):*
     > * *Trong lớp `Mat4`, em cài đặt đầy đủ các ma trận cơ sở `translate`, `rotateX/Y/Z` và hàm `rotateArbitraryAxis` theo đúng công thức đưa trục về gốc tọa độ và quay quanh 3 trục chính (trang 13–14):*
     >   $$M = Tr(-P_0) \cdot R_x(\phi) \cdot R_y(-\theta) \cdot R_z(\alpha) \cdot R_y(\theta) \cdot R_x(-\phi) \cdot Tr(P_0)$$
     > * *Để các bộ phận chuyển động nhịp nhàng, em thiết kế lớp `SceneNode` theo cấu trúc cây phân cấp Cha – Con:*
     >   $$\text{CarRoot} \rightarrow \text{Floor1} \rightarrow \text{SRV\_BASE (Trụ)} \rightarrow \text{SRV\_ELBOW (Bàn trượt)} \rightarrow \text{SRV\_GRIP (Ngàm)} \rightarrow \text{TargetObject}$$
     > * *Mỗi bộ phận chỉ cần lưu biến đổi cục bộ $M_{local}$. Hàm `getWorldMatrix()` tự động nhân dồn ma trận từ nút con lên nút gốc: $M_{world} = M_{parent\_world} \cdot M_{local}$.*
     > * *Khi bàn trượt `SRV_ELBOW` tịnh tiến thẳng đứng dọc trục $Y$, ngàm kẹp và khối vật thể thừa hưởng ma trận thế giới và đi theo đầu ngàm gắp trong mọi chuyển động một cách hoàn toàn tự động ạ."*

---

### CHẶNG 4 (3 Phút): Demo Tương Tác Mạch Điện Vi Điều Khiển $\leftrightarrow$ Giải Thích `HardwareComponent.h` & `CircuitSimulator.cpp`
*(Điểm nhấn chứng minh đúng tính chất "Tương tác linh kiện vi điều khiển" của đề tài)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * **Thử nghiệm 1 (Ngắt nguồn tổng bằng phím `T`):** Bấm phím `T` để gạt tắt công tắc pin `BAT_SW` ở đuôi xe Tầng 2 $\rightarrow$ dây dẫn tắt phát sáng ($0\text{V}$), động cơ báo `NGAT NGUON (OFF)`, dòng tiêu thụ tổng $I_{total}$ trên đồng hồ HUD tụt về $0\text{mA}$, bấm lái xe hay bấm `I/K/O/P` ngàm gắp đều đứng im. Bấm `T` bật lại $\rightarrow$ hệ thống hồi sinh, dây dẫn sáng viền neon theo màu chuẩn công nghiệp (Đỏ 7.4V, Cam 5V, Vàng PWM).
   * **Thử nghiệm 2 (Tương tác Raycasting chọn linh kiện 3D):** Dùng chuột click vào bo mạch Arduino, cầu H L298N, mạch Buck, hay Hộp pin $\rightarrow$ linh kiện được click lập tức sáng viền màu vàng; bảng **INSPECTOR** góc phải màn hình cập nhật ngay tên, kích thước CAD chuẩn, điện áp vào và dòng tiêu thụ thực tế.
   * **Thử nghiệm 3 (Điều chỉnh điện áp mạch Buck):** Bấm phím `+` hoặc `-` để tăng giảm điện áp mạch Buck LM2596 từ 3.0V đến 6.0V.

2. **Chỉ vào Code (Bên phải – mở file `HardwareComponent.h`, `CircuitSimulator.cpp` & `SceneNode.cpp`):**
   * Chỉ vào cấu trúc `Pin`, hàm `CircuitSimulator::stepCircuit(dt)`, và hàm `SceneNode::raycast()`.
   * **Lời giải thích ghi điểm với Giảng viên:**
     > *"Thưa Thầy/Cô, để mô phỏng đúng bản chất kỹ thuật của các linh kiện chứ không chỉ là hoạt cảnh dựng sẵn, em xây dựng lớp cơ sở `HardwareComponent` kế thừa từ `SceneNode`. Mỗi linh kiện sở hữu mảng `pins` đúng với số chân thực tế.*
     > *Trong hàm `stepCircuit` tại `CircuitSimulator.cpp`, hệ thống kiểm tra mạch điện thời gian thực:*
     > * *`BAT_SW` cấp $7.4\text{V}$ vào `DOMINO` Bus 1.*
     > * *`BUCK LM2596` nhận $7.4\text{V}$ từ `DOMINO`, hạ áp xuất $5.0\text{V}$ nuôi riêng 3 động cơ Servo dòng lớn (đúng nguyên lý thiết kế mạch thực tế để tránh sụt áp gây treo Arduino).*
     > * *Trong hàm `ServoMG996RComp::updateMotion(dt)`, nếu điện áp cấp dưới $4.5\text{V}$ thì cơ cấu lập tức ngừng hoạt động.*
     > * *Khi click chuột lên màn hình, hàm `Camera3D::screenToRay` và `SceneNode::raycast` biến đổi tia Ray vào không gian cục bộ qua ma trận nghịch đảo $M_{world}^{-1}$ để kiểm tra va chạm với hộp bao OBB/AABB của linh kiện ạ."*

---

### CHẶNG 5 (3 Phút): Demo Giao Diện HUD 2D, Radar Cự Ly & Xén Hình $\leftrightarrow$ Giải Thích `Raster2D.cpp` & `Clipper2D.cpp`
*(Kiến thức cốt lõi: Slide `CG.3-2D Graphics.pdf`, trang 12 – 102)*

1. **Thao tác trên cửa sổ Demo (Bên trái):**
   * Chỉ vào góc màn hình hiển thị **Bảng điều khiển 2D (HUD Dashboard)**:
     * **Widget Radar cự ly 40cm (góc dưới trái):** Các vòng tròn đồng tâm khoảng cách $10\text{cm}, 20\text{cm}, 30\text{cm}, 40\text{cm}$; tia quét radar quay liên tục $360^\circ$; khi xe lại gần hộp mẫu màu cam, một điểm Blip đỏ nhấp nháy xuất hiện đúng vị trí cự ly thực.
     * **Máy hiện sóng Oscilloscope (góc trên trái):** Hiển thị dạng sóng vuông xung PWM điều khiển tốc độ động cơ theo tần số thời gian thực.
     * **Tính năng xén hình:** Tia quét Radar và đường sóng Oscilloscope luôn nằm gọn gàng bên trong khung cửa sổ, không bao giờ bị lem nét vẽ ra ngoài.

2. **Chỉ vào Code (Bên phải – mở file `Raster2D.cpp` & `Clipper2D.cpp`):**
   * Chỉ vào các hàm `drawLineBresenham()`, `drawCircleMidpoint()`, `drawEllipseMidpoint()`, `fillRect()`, và `clipLineCohenSutherland()`.
   * **Lời giải thích ghi điểm với Giảng viên:**
     > *"Thưa Thầy/Cô, toàn bộ phần giao diện kỹ thuật 2D ở góc màn hình là nơi em áp dụng trực tiếp các thuật toán trong chương **2D Graphics (CG.3)**:*
     > 1. *Tia quét radar và trục chữ thập được vẽ bằng **Thuật toán đoạn thẳng Bresenham nguyên hóa** (`drawLineBresenham`, trang 12–25), hoàn toàn dùng số nguyên với biến quyết định $p_0 = 2\Delta y - \Delta x$ để tối ưu tốc độ.*
     > 2. *Các vòng chia khoảng cách trên Radar cự ly và đồng hồ góc Servo được vẽ bằng **Thuật toán đường tròn Midpoint** (`drawCircleMidpoint` với hàm đối xứng 8 phần `put8pixel`, trang 28–36) và **Midpoint vẽ Ellipse** (`drawEllipseMidpoint` với `put4pixel`, trang 39–44).*
     > 3. *Để tia quét và sóng xung PWM không bị vẽ tràn ra ngoài khung cửa sổ 2D, trước khi vẽ em truyền đoạn thẳng qua hàm `clipLineCohenSutherland()` cài đặt **Thuật toán xén hình Cohen-Sutherland** (trang 54–68): gán mã nhị phân 4-bit (`computeOutCode`), kiểm tra phép AND bit để loại bỏ nhanh hoặc tìm giao điểm cắt gọn tại biên cửa sổ.*
     > 4. *Các nút bấm và thanh trạng thái được tô đặc bằng thuật toán quét dòng **Scanline Fill** (`fillRect`, trang 78–82) kết hợp xuất nhãn thông số bằng **Font Bitmap** ạ."*

---

## PHẦN 3: BỘ CÂU HỎI PHẢN BIỆN GIẢNG VIÊN HAY HỎI NHẤT & CÁCH TRẢ LỜI "ĂN ĐIỂM TUYỆT ĐỐI"

| Câu hỏi của Giảng viên | Ý đồ kiểm tra của Giảng viên | Cách bạn trả lời + Thao tác chỉ vào Code |
| :--- | :--- | :--- |
| **Câu 1:** *"Tại sao em phải tự viết lớp `Mat4` mà không dùng luôn ma trận sẵn có của OpenGL như `glTranslatef`, `glRotatef`?"* | Kiểm tra xem sinh viên có hiểu bản chất toán học hay chỉ lạm dụng OpenGL. | *"Thưa Thầy/Cô, nếu dùng hàm OpenGL dựng sẵn thì ma trận bị giấu trong OpenGL State Machine, em sẽ không thể lấy được toạ độ thế giới của ngàm kẹp để tính va chạm gắp vật thể, và cũng không thể tính toán Raycasting từ chuột 2D vào 3D. Việc tự xây dựng lớp `Mat4` với hệ toạ độ thuần nhất $4 \times 4$ giúp em hoàn toàn làm chủ thuật toán, kiểm soát cây phân cảnh Scene Graph và tính toán động học vi sai một cách độc lập ạ."* |
| **Câu 2:** *"Trong thuật toán Cohen-Sutherland, làm sao chương trình biết ngay một đoạn thẳng nằm hoàn toàn bên ngoài cửa sổ mà không cần tính giao điểm?"* | Kiểm tra hiểu biết về mã vùng 4-bit và phép toán `AND` bit *(CG.3 tr.57)*. | *"Dạ em dùng phép toán bit `(code0 & code1) != 0` trong hàm `clipLineCohenSutherland` tại file `Clipper2D.cpp`. Nếu kết quả khác `0000`, nghĩa là cả hai đầu mút cùng chia sẻ ít nhất một bit `1` (cùng nằm hẳn về phía Trái, Phải, Trên hoặc Dưới cửa sổ), nên đoạn thẳng chắc chắn nằm ngoài và hàm `return false` loại bỏ ngay lập tức mà không tốn phép chia số thực nào ạ."* |
| **Câu 3:** *"Tại sao thuật toán Bresenham lại tối ưu hơn thuật toán DDA khi vẽ đoạn thẳng?"* | Kiểm tra kiến thức trang 12–25 của slide `CG.3-2D Graphics.pdf`. | *"Dạ thuật toán DDA phải tính toán trên số thực (cộng dồn hệ số góc $m = \Delta y / \Delta x$ rồi làm tròn `round`), gây sai số tích lũy và tốc độ chậm. Thuật toán Bresenham mà em cài đặt trong `Raster2D.cpp` đã nguyên hóa hoàn toàn bài toán: dùng biến quyết định $p_k = 2\Delta y - \Delta x$, chỉ sử dụng phép cộng và phép nhân 2 (dịch bit) trên số nguyên, do đó tối ưu tuyệt đối cho phần cứng đồ họa ạ."* |
| **Câu 4:** *"Tại sao trên xe có pin $7.4\text{V}$ cấp vào chân `VIN` của Arduino (Arduino cũng có chân ra $5\text{V}$), mà em lại phải thiết kế thêm mạch hạ áp `BUCK LM2596`?"* | Kiểm tra độ hiểu thực tế về thiết kế hệ thống nhúng của sinh viên. | *"Dạ bộ ổn áp LDO $5\text{V}$ tích hợp trên mạch `Arduino Uno R3` chỉ chịu được dòng tối đa khoảng $500\text{mA} - 800\text{mA}$. Trong khi đó mỗi động cơ Servo `MG996R` khi nâng tải có dòng khởi động lên tới $1.5\text{A} - 2.5\text{A}$. Nếu cắm 3 Servo vào chân $5\text{V}$ của Arduino sẽ gây sụt áp làm vi điều khiển bị sập nguồn (Brownout Reset). Do đó mô hình Digital Twin của em dùng mạch `BUCK LM2596` ($5\text{V}/3\text{A}$) cấp nguồn riêng cho 3 Servo, chỉ nối chung dây `GND` và dây tín hiệu `SIG` về Arduino ạ."* |
| **Câu 5:** *"Cơ chế phát hiện và kẹp vật thể được xử lý trong code như thế nào?"* | Kiểm tra sự liên kết giữa Scene Graph và logic vật lý. | *"Dạ trong hàm `updateArmAndGrasping` tại file `RobotCarScene.cpp`, em lấy ma trận thế giới của ngàm kẹp `compSrvGrip->getWorldMatrix()`, nhân với vector tâm ngàm $[0, 0, 3.4, 1]^T$ để ra tọa độ thực trong thế giới. Khi khoảng cách tới vật thể $< 4.2\text{ cm}$ và góc ngàm khép lại $< 25^\circ$, cờ `isObjectGrasped` kích hoạt, tọa độ vật thể sẽ khóa cứng theo tâm ngàm và nâng lên theo bàn trượt ạ."* |

---

## PHẦN 4: DANH SÁCH KIỂM TRA (CHECKLIST) 15 PHÚT TRƯỚC KHI LÊN BẢNG

- [ ] **1. Chuẩn bị sẵn 2 cửa sổ trên màn hình (Tỉ lệ 50% - 50%):**
  - Nửa bên trái: Chạy sẵn file `XeDieuKhien3D.exe`.
  - Nửa bên phải: Mở sẵn VS Code thư mục `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D` và ghim (Pin) 5 tab code chính:
    1. `Camera3D.h` & `Camera3D.cpp` *(Chặng 1: Camera cầu $\theta, \Phi, R, D$ & Phép chiếu)*
    2. `Mesh3D.h` & `Mesh3D.cpp` *(Chặng 2: Khung dây `WireFrame` & Mặt `FaceModel`)*
    3. `Mat4.cpp` & `SceneNode.cpp` *(Chặng 3: Ma trận Affine $4 \times 4$ & Cây Scene Graph)*
    4. `CircuitSimulator.cpp` *(Chặng 4: Lan truyền điện áp & Ngắt nguồn `BAT_SW`)*
    5. `Raster2D.cpp` & `Clipper2D.cpp` *(Chặng 5: Bresenham, Midpoint, Cohen-Sutherland)*
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
