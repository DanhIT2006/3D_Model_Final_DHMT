# TÀI LIỆU HƯỚNG DẪN BẢO VỆ ĐỒ ÁN MÔN HỌC
# KỊCH BẢN THUYẾT TRÌNH & BẢO VỆ ĐỒ ÁN (TRÌNH CHIẾU SLIDE SONG SONG + DEMO TRỰC TIẾP & GIẢI THÍCH MÃ NGUỒN)

* **Đề tài:** Ứng dụng C++/OpenGL xây dựng chương trình mô phỏng và tương tác linh kiện vi điều khiển trên mô hình xe Robot gắp đồ vật (Digital Twin 3D)
* **Thư mục dự án:** `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`
* **Slide đồng bộ:** `Slide_Trinh_Bay_Xen_Ke_Demo_XeDieuKhien3D.pptx` (10 Slides)

---

## PHẦN 1: GÓC NHÌN CỦA GIẢNG VIÊN (HỘI ĐỒNG CHẤM ĐỒ ÁN MUỐN THẤY GÌ?)

Khi chấm một đồ án kết hợp giữa **Đồ họa máy tính (2D/3D)** và **Lập trình hệ thống C++/OpenGL**, giảng viên không đánh giá dựa trên việc mô hình đẹp như game thương mại, mà tìm kiếm **4 bằng chứng cốt lõi** sau để chấm điểm Giỏi / Xuất sắc (8.5 – 10):

### 1.1. Sự ăn khớp thật sự giữa lý thuyết trên lớp và code chạy thực tế
* **Mô hình 3D:** Biểu diễn vật thể 3D bằng Danh sách đỉnh – Danh sách cạnh (`WireFrame`) và Danh sách đỉnh – Danh sách mặt đa giác (`FaceModel`) *(CG.4, tr. 20–26)*.
* **Biến đổi 3D:** Tự xây dựng ma trận thuần nhất $4 \times 4$ và phép nhân kết hợp $M = M_1 \cdot M_2 \cdot M_3$ cho chuỗi chuyển động của khung xe và cánh tay 2 khâu 3 bậc tự do *(CG.4, tr. 5–16)*.
* **Quan sát & Phép chiếu:** Hiểu rõ 4 tham số quan sát cầu $(\theta, \Phi, R, D)$ trong ma trận $T = A \cdot B \cdot C \cdot D$, sự khác biệt giữa phép chiếu phối cảnh ($x' = x_0 \cdot D / z_0$) với 3 hình chiếu song song trực giao (Top / Front / Side) và cửa sổ Camera cổ tay PiP *(CG.4, tr. 28–49)*.
* **Đồ họa 2D trên HUD:** Vận dụng các thuật toán 2D (Bresenham / Midpoint vẽ Radar 40 cm & đường chỉ dẫn Callout, Cohen-Sutherland 4-bit xén hình, Scanline & BoundaryFill_Stack tô màu) vào giao diện HUD *(CG.3, tr. 12–102)*.

### 1.2. Tư duy thiết kế hệ thống (OOP, Scene Graph & 2-Tier Component Inspector)
* **Cây phân cấp Scene Graph:** Toàn bộ 11 linh kiện kế thừa từ lớp cơ sở `HardwareComponent` (dẫn xuất từ `SceneNode`), giúp khi xe di chuyển hoặc khi đế xoay `SRV_BASE` / khuỷu `SRV_ELBOW` chuyển động, mọi nút con (gồm cả khối hàng đang gắp) tự động thừa hưởng ma trận thế giới.
* **Giải phẫu bo mạch 2 cấp:** Kiến trúc quan sát 2 cấp độ (`CAR_OVERVIEW` $\leftrightarrow$ `COMPONENT_INSPECT`) cho phép cô lập từng bo mạch lên bàn xoay và duyệt mảng `SubComponent` (tụ hóa, IC, diode, thạch anh, jumper) sạch sẽ, dễ mở rộng.

### 1.3. Chiều sâu của chữ “TƯƠNG TÁC LINH KIỆN VI ĐIỀU KHIỂN” trong tên đề tài
* **Luồng điện áp thời gian thực:** Bật/tắt công tắc nguồn `BAT_SW` ($7.4\text{V}$) qua trạm `DOMINO` 3 Bus và mạch hạ áp `BUCK LM2596` ($5.0\text{V} / 3\text{A}$) nuôi riêng 3 Servo `MG996R` (dòng đỉnh $2.5\text{A}$); khi giảm áp Buck dưới $4.8\text{V}$ hoặc ngắt nguồn, cánh tay lập tức khóa lại.
* **Tương tác phần tử con:** Cô lập riêng Arduino Uno R3 hoặc Module L298N để click tra cứu tụ $220\mu\text{F} / 47\mu\text{F}$, 8 Diode chống dòng ngược, thạch anh 16 MHz và tương tác vật lý trực tiếp (nhấn Reset, rút/cắm 5V Enable Jumper).
* **Phản hồi cảm biến & Gắp vật:** Cảm biến HC-SR04 đo khoảng cách 3D thực tế (cự ly dừng 5 cm) báo về Arduino để dừng xe, kết hợp vòng tròn tâm ngắm chiếu sàn và Camera PiP giúp ngàm gắp ôm chuẩn khối hàng.

### 1.4. Khả năng làm chủ mã nguồn (“Slide có Code – Demo trực tiếp bên cạnh”)
* **Phản xạ đối chiếu trực tiếp:** Trình chiếu song song: khi Slide hiện công thức `Camera3D` hay chuỗi ma trận cánh tay, nửa màn hình bên cạnh lập tức thao tác chứng minh; khi giảng viên yêu cầu kiểm tra file trong `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D`, mở đúng hàm và giải thích ngay lập tức.

---

## PHẦN 2: KỊCH BẢN THUYẾT TRÌNH ĐÓNG VAI SINH VIÊN (SLIDE SONG SONG + DEMO TRỰC TIẾP)

> **Mẹo bố trí màn hình khi lên bảng bảo vệ (Chia đôi 50% – 50%):**
> * **Nửa bên trái (50%):** Trình chiếu PowerPoint `Slide_Trinh_Bay_Xen_Ke_Demo_XeDieuKhien3D.pptx` ở chế độ cửa sổ (Reading View) để hiện sơ đồ, công thức và đoạn code C++ cốt lõi (kèm VS Code thu gọn phía sau sẵn sàng bật khi giảng viên hỏi sâu).
> * **Nửa bên phải (50%):** Cửa sổ chương trình OpenGL đang chạy trực tiếp (`XeDieuKhien3D.exe` ở tốc độ 61 FPS) để thao tác minh họa ngay tại mỗi slide tương ứng.

---

### MỞ ĐẦU (1 Phút) — Slide 1 & Slide 2: Giới Thiệu Đề Tài & Bố Cục 5 Phần Xen Kẽ
**Lời dẫn của bạn với Giảng viên:**
> *"Em chào Thầy/Cô. Hôm nay em xin trình bày đề tài: **Ứng dụng C++/OpenGL xây dựng chương trình mô phỏng và tương tác linh kiện vi điều khiển trên mô hình xe Robot gắp đồ vật (Digital Twin 3D)**. Thay vì thuyết trình hết slide rồi mới chạy demo, em xin phép chia đôi màn hình: nửa bên trái là Slide trình bày thuật toán và đoạn code cốt lõi của từng phần, nửa bên phải em sẽ thao tác Demo trực tiếp trên chương trình `XeDieuKhien3D` để Thầy/Cô đối chiếu ngay lập tức. Bài trình bày gồm 5 phần như trên Slide 2 ạ."*

---

### CHẶNG 1 (2 Phút) — Slide 3: Kiến Trúc OOP 4 Tầng & Mô Hình WireFrame / FaceModel
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 20 – 26 - `Mesh3D`)*

1. **Thao tác trên cửa sổ Demo (Bên phải):**
   * Chỉ vào mô hình toàn xe 2 tầng đang chạy mượt ở tốc độ 61 FPS.
   * Bấm phím **F** để chuyển toàn bộ 11 linh kiện từ Khối đặc (`Polygon Mesh`) sang Khung dây (`WireFrame`) rồi bật lại.

2. **Trình bày trên Slide 3 & Giải thích Code (`Mesh3D.h` & `Mesh3D.cpp`):**
   * **Từ khóa tìm nhanh trong VS Code:** `[SLIDE 3]`
   * **Vị trí code:** [Mesh3D.h](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/include/scene/Mesh3D.h#L6-L26) & [Mesh3D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/scene/Mesh3D.cpp#L197-L251)
   > *"Thưa Thầy/Cô, như trên Slide 3, toàn bộ dự án tại `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D` được em tổ chức theo kiến trúc OOP 4 tầng độc lập để dễ đọc và bảo trì: `Mesh3D`, `Mat4`/`SceneNode`, `Hardware`/`CircuitSimulator` và `Camera3D`/`HUD 2D`. Khi em bấm phím **F** bật chế độ khung dây bên phải, lớp `Mesh3D` sẽ chuyển từ việc duyệt Danh sách mặt đa giác (`FaceModel` với `indexFace`) sang duyệt Danh sách cạnh (`WireFrame` với cặp chỉ số đỉnh `beginP` – `endP`) đúng cấu trúc bài giảng CG.4 trang 20–26. Đồng thời em bật `glPolygonOffset(1.0f, 1.0f)` kết hợp viền cạnh mảnh để khử triệt để hiện tượng sọc rác Z-fighting ạ."*

---

### CHẶNG 2 (2.5 Phút) — Slide 4: Thuật Toán Camera Cầu 3D, Phép Chiếu & Camera PiP
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 28 – 49 - `Camera3D.cpp`)*

1. **Thao tác trên cửa sổ Demo (Bên phải):**
   * Kéo chuột trái xoay ngang quanh xe (thay đổi góc $\theta$), nâng hạ góc nhìn (thay đổi góc $\Phi$), cuộn chuột phóng to / thu nhỏ bán kính quan sát ($R$).
   * Bấm các phím `1`, `2`, `3`, `4` chuyển đổi giữa Phối cảnh 3D (`Perspective Orbit`) và 3 Hình chiếu trực giao kỹ thuật (`Top / Front / Side View`); bấm phím **C** bật cửa sổ Camera cổ tay PiP ở góc trên phải.

2. **Trình bày trên Slide 4 & Chỉ vào Code `Camera3D.cpp` ngay trên Slide:**
   * **Từ khóa tìm nhanh trong VS Code:** `[SLIDE 4]`
   * **Vị trí code:** [Camera3D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/math/Camera3D.cpp#L112-L160)
   > *"Thưa Thầy/Cô, nửa phải của Slide 4 là đoạn code cốt lõi trong `Camera3D.cpp` cài đặt ma trận biến đổi hệ trục quan sát $T = A \cdot B \cdot C \cdot D$ (trang 45 giáo trình CG.4) với 4 tham số cầu $(\theta, \Phi, R, D)$:*
   > * $x_0 = -x\sin(\theta) + y\cos(\theta)$
   > * $y_0 = -x\cos(\theta)\sin(\Phi) - y\sin(\theta)\sin(\Phi) + z\cos(\Phi)$
   > * $z_0 = -x\cos(\theta)\cos(\Phi) - y\sin(\theta)\cos(\Phi) - z\sin(\Phi) + R$
   > * *Khi ở chế độ Phối cảnh, tọa độ được chia cho độ sâu $z_0$ theo tỉ lệ $x' = x_0 \cdot D / z_0$ nên vật ở gần lớn hơn ở xa. Khi em chuyển sang hình chiếu trực giao bên demo, code giữ nguyên $x' = x_0, y' = y_0$ và bỏ thành phần sâu giúp kiểm tra chính xác kích thước CAD của 2 tầng xe. Riêng khung góc trên phải **[C]** là một Viewport OpenGL thứ hai đặt mắt nhìn ngay tại cổ tay ngàm gắp ạ."*

---

### CHẶNG 3 (3.5 Phút) — Slide 5 & Slide 6: Hệ Thống 11 Linh Kiện & Cô Lập Giải Phẫu Bo Mạch
*(Điểm nhấn nổi bật: Chế độ Cô lập Workbench Mode & Tương tác Phần tử con `SubComponent`)*

1. **Thao tác trên cửa sổ Demo (Bên phải):**
   * **Tại Slide 5:** Click chuột vào vài linh kiện ở Tầng 1 (`L298N`, `BUCK`, `DOMINO`, `HCSR04`) và Tầng 2 (`ARD`, `BAT_SW`) để bảng Inspector hiển thị đúng kích thước CAD ($W \times H \times D\text{ cm}$) và số chân Pin.
   * **Tại Slide 6:** Click chọn Arduino Uno R3 $\rightarrow$ bấm phím **I** (hoặc nút **QUAN SAT CHI TIET**) để vào chế độ Cô lập. Click chọn Thạch anh 16 MHz, Cặp tụ lọc 47 µF và bấm Nút Reset. Sau đó bấm **ESC** ra ngoài, chọn L298N $\rightarrow$ bấm **I** $\rightarrow$ click vào Cặp tụ 220 µF/35 V, Cụm 8 Diode xung D1–D8, và bấm rút/cắm Jumper 5V Enable.

2. **Trình bày trên Slide 5–6 & Giải thích Code (`SubComponent.h`, `ControlComponents.cpp`, `ActuatorComponents.cpp`):**
   * **Từ khóa tìm nhanh trong VS Code:** `[SLIDE 6]`
   * **Vị trí code:** [ControlComponents.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/components/ControlComponents.cpp#L41-L85) & [ActuatorComponents.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/components/ActuatorComponents.cpp#L255-L295)
   > *"Thưa Thầy/Cô, để thể hiện rõ phần tương tác linh kiện vi điều khiển, ngoài việc dựng đúng 11 linh kiện theo kích thước CAD như bảng ở Slide 5, em thiết kế thêm cơ chế Cô lập Bo mạch (Workbench Mode). Khi bấm phím **I**, chương trình ẩn khung xe, thu bán kính camera về 6–25 cm và duyệt mảng `SubComponent` của đúng bo mạch đó:*
   > * *Trên **Arduino Uno R3**, em mô hình hóa 8 khối cốt lõi theo sơ đồ thực tế (chip ATmega328P-PU DIP-28, cổng USB & chip ATmega16U2, thạch anh 16.000 MHz chu kỳ 62.5 ns, tụ 47 µF và nút Reset bấm được).*
   > * *Trên **L298N**, em dựng tản nhiệt nhôm 7 lá, 2 tụ 220 µF/35 V tách rời nhau không dính Z-fighting, 2 hàng 8 Diode bảo vệ dòng ngược Back-EMF và chốt Jumper 5V có thể click rút ra trực tiếp. Đường chỉ dẫn Callout nối từ linh kiện ra bảng mô tả sử dụng thuật toán Bresenham và Midpoint Circle ạ."*

---

### CHẶNG 4 (2 Phút) — Slide 7: Mô Phỏng Mạch Nguồn 3 Bus & Tác Động Điện Áp
*(Kiến thức cốt lõi: Tương tác điện tử thời gian thực - `CircuitSimulator.cpp`)*

1. **Thao tác trên cửa sổ Demo (Bên phải):**
   * Bấm **ESC** quay lại toàn xe, click chọn mạch hạ áp `BUCK LM2596` $\rightarrow$ bấm nút **GIAM AP [-]** trên bảng Inspector (hoặc tắt công tắc nguồn `BAT_SW` bằng phím **T**).
   * Thử bấm điều khiển cánh tay Servo (phím **I / K**) để chứng minh cánh tay bị khóa khi mất nguồn $5\text{V}$ hoặc áp dưới $4.8\text{V}$; sau đó bấm tăng áp về $5.00\text{V}$ để khôi phục.

2. **Trình bày trên Slide 7 & Giải thích Code (`CircuitSimulator.cpp`):**
   * **Từ khóa tìm nhanh trong VS Code:** `[SLIDE 7]`
   * **Vị trí code:** [CircuitSimulator.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/core/CircuitSimulator.cpp#L90-L135) & [ActuatorComponents.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/components/ActuatorComponents.cpp#L1040-L1055)
   > *"Trong `CircuitSimulator.cpp`, hệ thống lan truyền điện áp thực tế từ hộp 2 pin 18650 ($7.4\text{V}, 2600\text{mAh}$) vào trạm `DOMINO` 3 Bus: Bus $7.4\text{V}$ nuôi trực tiếp mạch cầu H `L298N` và chân `VIN` Arduino; còn nhánh qua mạch hạ áp `BUCK LM2596` xuất $5.00\text{V} / 3\text{A}$ để nuôi riêng 3 Servo `MG996R` có dòng đỉnh $2.5\text{A}$ (tránh sụt áp làm treo Arduino). Trong hàm cập nhật Servo có điều kiện ràng buộc kiểm tra điện áp chân `VCC >= 4.8V`, nên khi em giảm áp Buck bên Demo, cánh tay lập tức ngừng hoạt động đúng nguyên lý phần cứng ạ."*

---

### CHẶNG 5 (3 Phút) — Slide 8: Động Học Cánh Tay 2 Khâu (3 DOF) & Gắp Khối Hàng
*(Kiến thức cốt lõi: Slide `CG.4-3D Graphics-fix.pdf`, trang 5 – 16 - `Mat4` & Kinematics)*

1. **Thao tác trên cửa sổ Demo (Bên phải):**
   * Lái xe tiến tới trước khối hàng màu cam cho đến khi cảm biến HC-SR04 báo cự ly 5 cm trên Radar và Camera PiP.
   * Hạ khớp nâng LIFT về $0^\circ$ (phím **K**) để vòng tròn tâm ngắm màu xanh lá cây sáng dưới sàn trùng khít khối hàng $\rightarrow$ đóng ngàm GRIP (phím **P**) ôm chặt khối hàng $\rightarrow$ nâng LIFT lên cao (phím **I**) và xoay đế MAST sang ngang (phím **J / L**)!

2. **Trình bày trên Slide 8 & Chỉ vào Code `computeGripTipWorldPos` trên Slide:**
   * **Từ khóa tìm nhanh trong VS Code:** `[SLIDE 8]`
   * **Vị trí code:** [RoboticArmRenderer.h](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/include/scene/RoboticArmRenderer.h#L170-L215) & [SceneNode.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/scene/SceneNode.cpp#L42-L52)
   > *"Để giải quyết bài toán cánh tay vừa không đâm xuyên sàn Tầng 2, vừa vươn qua đầu cảm biến HC-SR04 để ôm trúng khối hàng ở cự ly 5 cm, em thiết kế cánh tay 2 khâu gấp khúc: đế xoay đặt tại $z = 6.2\text{ cm}$, khâu vai $L_1 = 8.2\text{ cm}$ luôn nâng góc dương ($+14^\circ$ đến $+62^\circ$) và khâu khuỷu $L_2 = 8.6\text{ cm}$ vươn tới tâm kẹp $z = 18.2\text{ cm}$, độ cao $y = 1.65\text{ cm}$. Mọi chuyển động nối tiếp được tính bằng phép nhân chuỗi ma trận thuần nhất $4 \times 4$ ($M_{grip} = M_{car} \cdot M_{base} \cdot M_{arm} \cdot M_{claw}$). Khi khép ngàm tại vùng hợp lệ, khối hàng được gắn làm nút con của `SRV_GRIP` nên tự động nâng lên và xoay theo cánh tay ạ."*

---

### CHẶNG 6 (2 Phút) — Slide 9 & Slide 10: Đồ Họa 2D Trên HUD & Kết Thúc Cảm Ơn
*(Kiến thức cốt lõi: Slide `CG.3-2D Graphics.pdf`, trang 12 – 88 - `Raster2D.cpp` & `Clipper2D.cpp`)*

1. **Thao tác trên cửa sổ Demo (Bên phải):**
   * Chỉ chuột lần lượt vào các widget HUD 2D đang hoạt động: Radar cự ly 40 cm (góc trên trái), Khung viền công nghệ cao Camera cổ tay PiP (góc trên phải), và Bảng Inspector linh kiện (góc dưới trái).
   * Chuyển sang Slide 10 (Cảm Ơn Thầy/Cô) và giữ nguyên cửa sổ Demo + VS Code để sẵn sàng trả lời vấn đáp.

2. **Trình bày trên Slide 9–10:**
   * **Từ khóa tìm nhanh trong VS Code:** `[SLIDE 9]`
   * **Vị trí code:** [Raster2D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/graphics2d/Raster2D.cpp#L14-L40), [Clipper2D.cpp](file:///c:/XAMPP/htdocs/BaiTap/XeDieuKhien3D/src/graphics2d/Clipper2D.cpp#L3-L28)
   > *"Cuối cùng, toàn bộ giao diện HUD 2D bên Demo là nơi em áp dụng trực tiếp chương 2D Graphics (CG.3): tia quét Radar và đường Callout được vẽ bằng thuật toán đoạn thẳng Bresenham số nguyên; các vòng cự ly và tâm ngắm sàn vẽ bằng Midpoint Circle (đối xứng 8 điểm `put8pixel`); toàn bộ tia sóng đều đi qua hàm xén hình Cohen-Sutherland dùng mã vùng 4-bit để cắt gọn không tràn mép khung; các thanh trạng thái tô màu bằng Scanline và BoundaryFill khử đệ quy bằng Stack. Em xin kết thúc phần trình bày tại đây và rất mong nhận được các câu hỏi phản biện từ Thầy/Cô ạ!"*

---

## PHẦN 3: BỘ CÂU HỎI PHẢN BIỆN GIẢNG VIÊN HAY HỎI NHẤT & CÁCH TRẢ LỜI

| Câu hỏi của Giảng viên | Ý đồ kiểm tra | Cách trả lời & Thao tác chỉ vào Code |
| :--- | :--- | :--- |
| **Câu 1:** *"Tại sao em phải tự viết lớp `Mat4` mà không dùng luôn `glTranslatef`, `glRotatef` của OpenGL?"* | Kiểm tra xem sinh viên hiểu bản chất ma trận $4 \times 4$ hay chỉ gọi hàm thư viện. | *"Dạ nếu chỉ dùng hàm OpenGL dựng sẵn thì ma trận bị ẩn trong State Machine, em không lấy được tọa độ thế giới của tâm ngàm kẹp ($z = 18.2\text{ cm}$) để tính va chạm gắp vật và không làm được Raycasting từ chuột 2D vào 3D. Tự viết lớp `Mat4` giúp em làm chủ hoàn toàn cây Scene Graph ạ."* |
| **Câu 2:** *"Làm sao em xử lý để cánh tay robot không đâm xuyên sàn Tầng 2 và không đụng cảm biến HC-SR04?"* | Kiểm tra tư duy động học thuận và quy tắc dấu góc quay 3D. | *"Dạ trong OpenGL (trục +Y lên, +Z ra mũi xe), quay góc dương quanh +X làm tay chúc xuống. Em đảo dấu góc khâu vai (`-shoulderUpDeg` với góc luôn dương $14^\circ$–$62^\circ$) để tay đòn $L_1 = 8.2\text{ cm}$ luôn đi vòng lên trên sàn Tầng 2, rồi khâu khuỷu $L_2 = 8.6\text{ cm}$ mới vươn qua đầu HC-SR04 xuống tâm khối hàng ạ."* |
| **Câu 3:** *"Trong chế độ cô lập L298N, em đã khắc phục lỗi sọc đen trắng (Z-fighting) trên tụ hóa và PCB bằng cách nào?"* | Kiểm tra kinh nghiệm xử lý đồ họa thực tế (Depth Buffer). | *"Dạ em dùng `glEnable(GL_POLYGON_OFFSET_FILL)` với `glPolygonOffset(1.0f, 1.0f)`, nâng đáy mọi phần tử con cao hơn mặt PCB một khoảng $\epsilon = 0.015\text{ cm}$, và vẽ tụ hóa 220 µF thành 2 đoạn nối tiếp (88% vỏ cách điện dưới + 12% nắp nhôm trên) thay vì 2 hình trụ lồng nhau ạ."* |
| **Câu 4:** *"Trong thuật toán Cohen-Sutherland, làm sao biết ngay đoạn thẳng nằm ngoài cửa sổ mà không cần tính giao điểm?"* | Kiểm tra mã vùng 4-bit và phép toán bit AND *(CG.3, tr. 57)*. | *"Dạ em kiểm tra `(code0 & code1) != 0` trong `Clipper2D.cpp`. Nếu khác `0000` thì hai đầu mút cùng nằm về một phía (Trái/Phải/Trên/Dưới) ngoài khung HUD, nên hàm return false loại bỏ ngay mà không tốn phép chia số thực nào ạ."* |
| **Câu 5:** *"Tại sao không dùng BoundaryFill đệ quy 4 dòng cho ngắn mà phải dùng Stack?"* | Kiểm tra lưu ý trang 84–88 giáo trình CG.3-2D Graphics. | *"Dạ phương pháp đệ quy dễ gây tràn bộ nhớ ngăn xếp (Stack Overflow) khi vùng tô lớn. Cài đặt `BoundaryFill_Stack` bằng danh sách liên kết push/pop giúp chương trình chạy ổn định ở 61 FPS với mọi kích thước vùng tô ạ."* |
| **Câu 6:** *"Tại sao Arduino có chân ra 5V rồi mà trên xe vẫn phải gắn thêm mạch hạ áp BUCK LM2596?"* | Kiểm tra độ am hiểu phần cứng hệ thống nhúng thực tế. | *"Dạ mạch ổn áp LDO trên Arduino Uno chỉ chịu dòng khoảng 500–800 mA, trong khi 3 Servo MG996R có dòng đỉnh tới 2.5 A. Cắm trực tiếp vào Arduino sẽ gây sụt áp treo vi điều khiển, nên em tách qua mạch BUCK LM2596 (5V / 3A) cấp nguồn riêng cho cụm cánh tay ạ."* |

---

## PHẦN 4: DANH SÁCH KIỂM TRA (CHECKLIST) 15 PHÚT TRƯỚC KHI LÊN BẢNG

* [ ] **1. Chuẩn bị bố cục màn hình 50% – 50%:**
  * Nửa bên trái: Mở sẵn `Slide_Trinh_Bay_Xen_Ke_Demo_XeDieuKhien3D.pptx` (chế độ Reading View) và VS Code mở sẵn thư mục `C:\XAMPP\htdocs\BaiTap\XeDieuKhien3D` (ghim sẵn 5 tab: `Mesh3D.h`, `Camera3D.cpp`, `ControlComponents.cpp`, `CircuitSimulator.cpp`, `RoboticArmRenderer.h`, `Raster2D.cpp`).
* [ ] **2. Kiểm tra trạng thái chương trình Demo:**
  * Nửa bên phải: Chạy sẵn file `XeDieuKhien3D.exe`, kiểm tra FPS đạt ~60 FPS, pin 7.4V, Buck 5.00V, khối hàng đặt sẵn trước mũi xe.
* [ ] **3. Cỡ chữ trình chiếu mã nguồn:**
  * Bấm `Ctrl + +` trong VS Code phóng to font chữ lên **16 pt – 18 pt** để giảng viên ngồi dưới nhìn rõ từng dòng công thức.
* [ ] **4. Bảng tra cứu phím tắt điều khiển nhanh (Cheat Sheet):**
  * `Chuột trái / phải / cuộn`: Xoay góc nhìn cầu $(\theta, \Phi)$ / Di chuyển Pan / Zoom bán kính $R$.
  * `1, 2, 3, 4, 5`: Chuyển góc nhìn Camera (Phối cảnh 3D, Top, Front, Side, Bám sau xe).
  * `C`: Bật / Tắt cửa sổ Camera cổ tay ngàm gắp (`PiP Claw Cam`) góc trên phải.
  * `W / A / S / D & Space`: Lái xe tiến / trái / lùi / phải theo động học vi sai & Phanh dừng.
  * `Click chuột trái vào LK`: Raycast chọn 1 trong 11 linh kiện 3D hiển thị thông số trên bảng Inspector.
  * `I / Enter & ESC`: Vào chế độ Cô lập bo mạch (`Workbench Mode`) & Quay lại toàn xe (`ESC`).
  * `Tab / [ / ]`: Duyệt nhanh qua 8 phần tử con bên trong bo mạch đang cô lập.
  * `T & + / -`: Bật/Tắt công tắc pin `BAT_SW` ($7.4\text{V}$) & Tăng/Giảm điện áp `BUCK LM2596`.
  * `I / K & J / L & O / P`: Nâng/hạ cánh tay 2 khâu, xoay đế mâm, đóng/mở ngàm kẹp gắp khối hàng hóa.
  * `F & M & R & H`: Bật/tắt WireFrame (`F`), Đổi chế độ Tự hành (`M`), Reset xe (`R`), Xem phím tắt (`H`).
