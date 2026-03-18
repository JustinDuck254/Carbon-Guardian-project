#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// ===================== CẤU HÌNH MÀN HÌNH =====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===================== CHÂN KẾT NỐI =====================
#define MQ7_PIN    35
#define MQ2_PIN    34
#define LED_PIN    25
#define BUZZER_PIN 26   // Active buzzer

// ===================== THÔNG SỐ PHẦN CỨNG =====================
#define RL_MQ7        10.0f
#define RL_MQ2         5.0f
#define ADC_MAX     4095.0f
#define VCC_SENSOR     5.0f
#define VCC_ADC        3.3f
#define DIVIDER_RATIO  1.5f

// ===================== KHỞI TẠO BAN ĐẦU (KHÔNG ĐẾM GIỜ WARMUP) =====================
// Lấy INIT_SAMPLES mẫu để tự tính R0 rồi chạy luôn
#define INIT_SAMPLES   80          // 80 mẫu * 200ms ~ 16 giây

// ===================== HỆ SỐ CURVE =====================
// Lưu ý MQ-7 Flying Fish 5V heater cố định => giá trị tương đối
#define MQ7_CO_A    99.042f
#define MQ7_CO_B    -1.518f
#define MQ2_LPG_A  574.25f
#define MQ2_LPG_B   -2.222f

// ===================== LỌC EMA =====================
#define ALPHA  0.15f

// ===================== NGƯỠNG =====================
// Chỉ báo động khi OVERALL = DANGER
#define CO_WARN       20.0f
#define CO_DANGER     80.0f
#define GAS_WARN     800.0f
#define GAS_DANGER  2500.0f

// Hysteresis chống nhảy mức
#define HYS_CO         6.0f
#define HYS_GAS      120.0f

// ===================== BIẾN TOÀN CỤC =====================
float R0_MQ7 = 10.0f;
float R0_MQ2 = 10.0f;

bool calibrated = false;
bool filter_init = false;

float calib_sum_Rs7 = 0;
float calib_sum_Rs2 = 0;
int calib_count = 0;

float ppmCO_filtered = 0;
float ppmGas_filtered = 0;

enum Level { SAFE = 0, WARN = 1, DANGER = 2 };
Level levelCO = SAFE;
Level levelGas = SAFE;

// ===================== TIỆN ÍCH =====================
const char* levelToStr(Level lv) {
  switch (lv) {
    case SAFE: return "SAFE";
    case WARN: return "WARN";
    case DANGER: return "DANGER";
    default: return "UNK";
  }
}

Level getOverall(Level a, Level b) {
  return (a > b) ? a : b;
}

// ===================== TÍNH Rs =====================
float calculateRs(int raw, float RL) {
  float voltage = raw * (VCC_ADC / ADC_MAX) * DIVIDER_RATIO;
  if (voltage < 0.02f) voltage = 0.02f;
  if (voltage >= VCC_SENSOR) voltage = VCC_SENSOR - 0.02f;
  return RL * (VCC_SENSOR - voltage) / voltage;
}

// ===================== TÍNH GIÁ TRỊ =====================
float ppm_MQ7(float Rs) {
  float ratio = Rs / R0_MQ7;
  if (ratio <= 0) return 0;

  float raw = MQ7_CO_A * pow(ratio, MQ7_CO_B);

  // Offset để nền sạch ~0 thay vì ~99
  float adjusted = raw - MQ7_CO_A;
  if (adjusted < 0) adjusted = 0;
  if (adjusted > 10000) adjusted = 10000;
  return adjusted;
}

float ppm_MQ2(float Rs) {
  float ratio = Rs / R0_MQ2;
  if (ratio <= 0) return 0;
  float ppm = MQ2_LPG_A * pow(ratio, MQ2_LPG_B);
  if (ppm < 0) ppm = 0;
  if (ppm > 10000) ppm = 10000;
  return ppm;
}

// ===================== HYSTERESIS =====================
Level classifyWithHys(float v, Level prev, float warn, float danger, float hys) {
  switch (prev) {
    case SAFE:
      if (v >= danger) return DANGER;
      if (v >= warn) return WARN;
      return SAFE;

    case WARN:
      if (v >= danger) return DANGER;
      if (v < (warn - hys)) return SAFE;
      return WARN;

    case DANGER:
      if (v < (danger - hys)) {
        if (v >= warn) return WARN;
        return SAFE;
      }
      return DANGER;
  }
  return SAFE;
}

// ===================== BUZZER: CHỈ DANGER =====================
// DANGER: 100ms ON, 100ms OFF
void updateBuzzerDangerOnly(bool danger) {
  if (!danger) {
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }
  unsigned long t = millis() % 200;
  digitalWrite(BUZZER_PIN, (t < 100) ? HIGH : LOW);
}

// ===================== OLED INIT =====================
void showInit(int count) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Air Detector v1.4");
  display.drawLine(0, 12, 127, 12, WHITE);

  display.setTextSize(2);
  display.setCursor(30, 20);
  display.print("INIT");

  display.setTextSize(1);
  display.setCursor(8, 52);
  display.print("Calibrating ");
  display.print(count);
  display.print("/");
  display.print(INIT_SAMPLES);

  display.display();
}

// ===================== OLED DATA =====================
// Dữ liệu bắt đầu từ y>=18 để tránh vùng vàng ở OLED 2 màu
void showDataOLED(float co, float gas, Level lvCO, Level lvGas) {
  Level overall = getOverall(lvCO, lvGas);

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Air Detector v1.4");
  display.drawLine(0, 12, 127, 12, WHITE);

  display.setCursor(0, 18);
  display.print("CO : ");
  display.print(co, 0);
  display.print(" ppm");
  display.setCursor(92, 18);
  display.print(levelToStr(lvCO));

  display.setCursor(0, 30);
  display.print("GAS: ");
  display.print(gas, 0);
  display.print(" ppm");
  display.setCursor(92, 30);
  display.print(levelToStr(lvGas));

  display.drawLine(0, 42, 127, 42, WHITE);
  display.setCursor(0, 46);
  display.print("STATUS:");

  display.setTextSize(2);
  display.setCursor(48, 48);
  display.print(levelToStr(overall));

  display.display();
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[ERROR] OLED khong tim thay!");
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  Serial.println("=== Air Detector v1.4 ===");
  Serial.println("Init calibration (khong dem gio warmup)...");
}

// ===================== LOOP =====================
void loop() {
  int raw7 = analogRead(MQ7_PIN);
  int raw2 = analogRead(MQ2_PIN);

  float Rs7 = calculateRs(raw7, RL_MQ7);
  float Rs2 = calculateRs(raw2, RL_MQ2);

  // -------- INIT CALIB (không có timer warmup) --------
  if (!calibrated) {
    calib_sum_Rs7 += Rs7;
    calib_sum_Rs2 += Rs2;
    calib_count++;

    if (calib_count >= INIT_SAMPLES) {
      R0_MQ7 = calib_sum_Rs7 / calib_count;
      R0_MQ2 = calib_sum_Rs2 / calib_count;

      if (R0_MQ7 < 0.1f) R0_MQ7 = 0.1f;
      if (R0_MQ2 < 0.1f) R0_MQ2 = 0.1f;

      calibrated = true;

      Serial.println("=== CALIBRATION DONE ===");
      Serial.printf("R0_MQ7 = %.2f kOhm\n", R0_MQ7);
      Serial.printf("R0_MQ2 = %.2f kOhm\n", R0_MQ2);
      Serial.println("========================");
    } else {
      showInit(calib_count);
      if (calib_count % 10 == 0) {
        Serial.printf("[INIT] %d/%d | Rs7=%.2f Rs2=%.2f\n",
                      calib_count, INIT_SAMPLES, Rs7, Rs2);
      }
    }

    delay(200);
    return;
  }

  // -------- TÍNH GIÁ TRỊ --------
  float ppmCO = ppm_MQ7(Rs7);   // nền sạch gần 0
  float ppmGas = ppm_MQ2(Rs2);

  if (!filter_init) {
    ppmCO_filtered = ppmCO;
    ppmGas_filtered = ppmGas;
    filter_init = true;
  } else {
    ppmCO_filtered = ALPHA * ppmCO + (1.0f - ALPHA) * ppmCO_filtered;
    ppmGas_filtered = ALPHA * ppmGas + (1.0f - ALPHA) * ppmGas_filtered;
  }

  // -------- PHÂN LOẠI --------
  levelCO = classifyWithHys(ppmCO_filtered, levelCO, CO_WARN, CO_DANGER, HYS_CO);
  levelGas = classifyWithHys(ppmGas_filtered, levelGas, GAS_WARN, GAS_DANGER, HYS_GAS);

  Level overall = getOverall(levelCO, levelGas);
  bool danger = (overall == DANGER); // CHỈ DANGER mới báo động

  // -------- CẢNH BÁO --------
  digitalWrite(LED_PIN, danger ? HIGH : LOW);
  updateBuzzerDangerOnly(danger);

  // -------- OLED --------
  showDataOLED(ppmCO_filtered, ppmGas_filtered, levelCO, levelGas);

  // -------- SERIAL --------
  Serial.printf("[DATA] CO: %.1f (%s) | GAS: %.1f (%s) | OVERALL: %s | raw7=%d raw2=%d\n",
                ppmCO_filtered, levelToStr(levelCO),
                ppmGas_filtered, levelToStr(levelGas),
                levelToStr(overall), raw7, raw2);

  delay(200);
}