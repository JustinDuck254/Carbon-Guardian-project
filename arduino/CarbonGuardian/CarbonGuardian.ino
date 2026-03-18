/*
 * Carbon Guardian - Smoke & CO Detector
 *
 * Hardware:
 *   - ESP32 (or ESP8266 with single ADC workaround)
 *   - MQ2  sensor : detects smoke, LPG, and combustible gases
 *   - MQ7  sensor : detects Carbon Monoxide (CO)
 *   - Active buzzer
 *
 * Default wiring (ESP32):
 *   MQ2  AOUT  -> GPIO 34  (ADC1_CH6)
 *   MQ7  AOUT  -> GPIO 35  (ADC1_CH7)
 *   Buzzer     -> GPIO 26
 *   All VCC    -> 3.3 V (or 5 V rail with level-shifter on analog pins)
 *   All GND    -> GND
 *
 * Threshold tuning:
 *   ADC returns 0-4095 on ESP32 (12-bit).
 *   Adjust MQ2_THRESHOLD and MQ7_THRESHOLD after calibrating in clean air.
 *
 * Serial output: 115200 baud
 */

// ─── Pin definitions ────────────────────────────────────────────────────────
#define MQ2_PIN      34   // Analog input for MQ2 (smoke / gas)
#define MQ7_PIN      35   // Analog input for MQ7 (Carbon Monoxide)
#define BUZZER_PIN   26   // Digital output for active buzzer

// ─── Detection thresholds (0 – 4095) ────────────────────────────────────────
// Lower value  = more sensitive.
// Adjust these after running a calibration in clean air (see loop comments).
#define MQ2_THRESHOLD   1500  // Smoke / combustible gas alarm level
#define MQ7_THRESHOLD   1800  // CO alarm level

// ─── Timing ─────────────────────────────────────────────────────────────────
#define SAMPLE_INTERVAL_MS  1000  // How often to read sensors (ms)

// ─── Global state ────────────────────────────────────────────────────────────
bool alarmActive = false;

// ─── Forward declarations ────────────────────────────────────────────────────
void triggerAlarm(bool state);
void printStatus(int mq2Val, int mq7Val, bool smoke, bool co);

// ────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for USB serial on some boards */ }

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // ensure buzzer is off at start

  // MQ2_PIN and MQ7_PIN are input-only ADC pins on ESP32 – no pinMode needed.

  Serial.println("=== Carbon Guardian booting ===");
  Serial.println("Sensor warm-up: 20 seconds...");

  // MQ sensors need ~20 s warm-up for stable readings after power-on.
  for (int i = 20; i > 0; i--) {
    Serial.print(i);
    Serial.print("... ");
    delay(1000);
  }
  Serial.println("\nReady. Monitoring started.");
}

// ────────────────────────────────────────────────────────────────────────────
void loop() {
  int mq2Val = analogRead(MQ2_PIN);
  int mq7Val = analogRead(MQ7_PIN);

  bool smokeDetected = (mq2Val >= MQ2_THRESHOLD);
  bool coDetected    = (mq7Val >= MQ7_THRESHOLD);

  bool shouldAlarm = smokeDetected || coDetected;

  if (shouldAlarm != alarmActive) {
    triggerAlarm(shouldAlarm);
    alarmActive = shouldAlarm;
  }

  printStatus(mq2Val, mq7Val, smokeDetected, coDetected);

  delay(SAMPLE_INTERVAL_MS);
}

// ────────────────────────────────────────────────────────────────────────────
// Turn the active buzzer on or off.
// ────────────────────────────────────────────────────────────────────────────
void triggerAlarm(bool state) {
  digitalWrite(BUZZER_PIN, state ? HIGH : LOW);
  if (state) {
    Serial.println("!!! ALARM TRIGGERED !!!");
  } else {
    Serial.println("--- Alarm cleared ---");
  }
}

// ────────────────────────────────────────────────────────────────────────────
// Print a single-line status report to the Serial monitor.
// ────────────────────────────────────────────────────────────────────────────
void printStatus(int mq2Val, int mq7Val, bool smoke, bool co) {
  Serial.print("MQ2=");
  Serial.print(mq2Val);
  Serial.print(" (");
  Serial.print(smoke ? "SMOKE!" : "OK");
  Serial.print(")  |  MQ7=");
  Serial.print(mq7Val);
  Serial.print(" (");
  Serial.print(co ? "CO!" : "OK");
  Serial.println(")");
}
