# 🌿 Carbon Guardian Project

<p align="center">
  Hệ thống giám sát chất lượng không khí dùng ESP32, MQ-7, MQ-2 và OLED SSD1306
</p>

<p align="center">
  <img alt="Platform" src="https://img.shields.io/badge/Platform-ESP32-00979D?logo=espressif&logoColor=white">
  <img alt="Language" src="https://img.shields.io/badge/Language-Arduino%20C%2B%2B-00979D?logo=arduino&logoColor=white">
  <img alt="Display" src="https://img.shields.io/badge/OLED-SSD1306-blue">
  <img alt="Sensors" src="https://img.shields.io/badge/Sensors-MQ--7%20%7C%20MQ--2-success">
  <img alt="Version" src="https://img.shields.io/badge/Version-v1.4-orange">
  <img alt="Status" src="https://img.shields.io/badge/Status-Active-brightgreen">
</p>

---

## 📖 Mục lục

- [Giới thiệu](#-giới-thiệu)
- [Tính năng](#-tính-năng)
- [Phần cứng](#-phần-cứng)
- [Sơ đồ chân](#-sơ-đồ-chân)
- [Thư viện cần cài](#-thư-viện-cần-cài)
- [Nguyên lý hoạt động](#-nguyên-lý-hoạt-động)
- [Ngưỡng cảnh báo](#-ngưỡng-cảnh-báo)
- [Cài đặt & nạp code](#-cài-đặt--nạp-code)
- [Serial Monitor](#-serial-monitor)
- [Tuỳ chỉnh nhanh](#-tuỳ-chỉnh-nhanh)
- [Lưu ý kỹ thuật](#-lưu-ý-kỹ-thuật)
- [Hướng phát triển](#-hướng-phát-triển)
- [License](#-license)

---

## 🧠 Giới thiệu

**Carbon Guardian** là dự án đo và cảnh báo khí trong không khí sử dụng:
- **MQ-7** để theo dõi **CO**
- **MQ-2** để theo dõi **Gas/LPG**
- **OLED SSD1306** hiển thị dữ liệu realtime
- **LED + Buzzer** cảnh báo khi nguy hiểm

Dự án phù hợp cho học tập IoT/Embedded, demo phòng thí nghiệm, hoặc giám sát môi trường cơ bản trong nhà.

---

## ✨ Tính năng

- Đọc dữ liệu từ 2 cảm biến khí (MQ-7 và MQ-2)
- Hiển thị trên OLED:
  - CO (ppm), Gas (ppm)
  - Mức từng kênh: `SAFE / WARN / DANGER`
  - Mức tổng hợp `OVERALL`
- Hiệu chuẩn tự động lúc khởi động (INIT)
- Lọc nhiễu bằng **EMA**
- Chống nhảy mức bằng **hysteresis**
- Cảnh báo chỉ khi `OVERALL = DANGER`:
  - LED ON
  - Buzzer kêu nhịp **100ms ON / 100ms OFF**

---

## 🧰 Phần cứng

- ESP32
- MQ-7 Gas Sensor Module
- MQ-2 Gas Sensor Module
- OLED SSD1306 I2C 128x64 (địa chỉ `0x3C`)
- LED + đi��n trở hạn dòng
- Active buzzer
- Dây nối, breadboard, nguồn ổn định

---

## 🔌 Sơ đồ chân

| Thành phần | Chân ESP32 |
|---|---|
| MQ-7 (AO) | **GPIO 35** |
| MQ-2 (AO) | **GPIO 34** |
| LED | **GPIO 25** |
| Buzzer (active) | **GPIO 26** |
| OLED SDA | **GPIO 21** |
| OLED SCL | **GPIO 22** |

> Lưu ý: GPIO 34/35 là chân input-only (phù hợp đọc ADC).

---

## 📚 Thư viện cần cài

Cài bằng Library Manager trong Arduino IDE:

- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `Wire` (đi kèm core)

---

## ⚙️ Nguyên lý hoạt động

### 1) INIT Calibration (khởi động)
- Lấy `INIT_SAMPLES = 80` mẫu (~16 giây, chu kỳ 200ms)
- Tính trung bình để tạo:
  - `R0_MQ7`
  - `R0_MQ2`
- Sau khi đủ mẫu → vào chế độ đo realtime

### 2) Đo và xử lý
- Đọc ADC từ MQ-7 và MQ-2
- Tính điện trở cảm biến `Rs`
- Chuyển đổi sang ppm theo hàm curve
- Lọc mượt bằng EMA (`ALPHA = 0.15`)

### 3) Phân loại mức
- Áp ngưỡng `WARN/DANGER`
- Dùng hysteresis để giảm dao động trạng thái khi giá trị gần ngưỡng

### 4) Cảnh báo
- `OVERALL = max(levelCO, levelGas)`
- Chỉ báo động khi `OVERALL == DANGER`

---

## 🚨 Ngưỡng cảnh báo

| Chỉ số | WARN | DANGER |
|---|---:|---:|
| CO | 20 ppm | 80 ppm |
| GAS | 800 ppm | 2500 ppm |

---

## 🚀 Cài đặt & nạp code

1. Mở file `.ino` trong Arduino IDE  
2. Chọn board ESP32 phù hợp  
3. Chọn đúng cổng COM  
4. Cài đủ thư viện ở trên  
5. Upload code  
6. Mở Serial Monitor với baudrate **115200**

---

## 🖥️ Serial Monitor

Ví dụ log:
```text
[DATA] CO: 12.3 (SAFE) | GAS: 950.1 (WARN) | OVERALL: WARN | raw7=1234 raw2=1456
```

Thông tin log gồm:
- Giá trị CO/GAS (ppm)
- Level từng kênh
- Level tổng OVERALL
- Giá trị ADC thô

---

## 🛠️ Tuỳ chỉnh nhanh

Bạn có thể chỉnh các biến sau trong code:

- Hiệu chuẩn:
  - `INIT_SAMPLES`
- Lọc:
  - `ALPHA`
- Ngưỡng:
  - `CO_WARN`, `CO_DANGER`
  - `GAS_WARN`, `GAS_DANGER`
- Hysteresis:
  - `HYS_CO`, `HYS_GAS`
- Hệ số curve:
  - `MQ7_CO_A`, `MQ7_CO_B`
  - `MQ2_LPG_A`, `MQ2_LPG_B`

---

## ⚠️ Lưu ý kỹ thuật

- MQ-series phù hợp cho **giám sát tương đối/cảnh báo xu hướng**, không thay thế thiết bị đo công nghiệp đã hiệu chuẩn chuẩn khí.
- Độ chính xác phụ thuộc nguồn cấp, module, nhiệt độ/độ ẩm, bố trí mạch và môi trường.
- Phiên bản hiện tại tối ưu cho tính ổn định hiển thị + cảnh báo thực tế.

---

## 🔮 Hướng phát triển

- Gửi dữ liệu lên MQTT/Blynk/ThingsBoard
- Lưu lịch sử đo (SD card/cloud)
- Cảnh báo đa mức (WARN beep chậm, DANGER beep nhanh)
- Thêm cảm biến nhiệt/ẩm để bù sai số
- Nút “Recalibrate” tại chỗ

---

## 📄 License

Khuyến nghị dùng **MIT License** để dễ chia sẻ và tái sử dụng.

---

## 🤝 Đóng góp

Mọi contribution đều được chào đón:
- Tối ưu thuật toán lọc
- Bổ sung thêm các linh kiện khác
- Cải tiến độ chính xác
- Nâng cấp giao diện hiển thị / dashboard
- Thêm tính năng IoT
