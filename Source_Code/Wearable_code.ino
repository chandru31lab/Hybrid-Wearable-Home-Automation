
/*
 * Wearable Transmitter Node — ESP32
 * 
 * Features:
 *   - VC02 UART voice command reader
 *   - MPU6050 Fall Detection → local alarm 10 sec → send alert via LoRa
 *   - Cancel Button: press within 10 sec to cancel alarm (no LoRa alert sent)
 *   - Manual Trigger Button: press anytime to force send alert = 1
 * 
 * LoRa Packet (3 bytes):
 *   packet[0] = VC02 high byte
 *   packet[1] = VC02 low byte
 *   packet[2] = alert status: 1 = alert, 0 = normal
 * 
 * Wiring:
 *   MPU6050 SDA    → GPIO 21  |  SCL  → GPIO 22
 *   MPU6050 VCC    → 3.3V     |  GND  → GND
 *   Cancel  Button → GPIO 32  (to GND, active LOW)
 *   Manual  Button → GPIO 25  (to GND, active LOW)
 *   Buzzer         → GPIO 13
 *   LoRa SCK=5, MISO=19, MOSI=27, SS=18, RST=14, DIO0=26
 */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

// ---------------- VC02 UART ----------------
#define VC02_RX 16
#define VC02_TX 17

// ---------------- Buttons & Buzzer ----------------
#define CANCEL_BTN_PIN  32
#define MANUAL_BTN_PIN  25
#define BUZZER_PIN      2

// ---------------- MPU6050 ----------------
#define SDA_PIN 21
#define SCL_PIN 22

// ---------------- LoRa Pins ----------------
#define LORA_SCK   5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS   18
#define LORA_RST  14
#define LORA_DIO0 26
#define LORA_FREQ 433E6

// ---------------- Fall Detection Thresholds ----------------
#define FREE_FALL_THRESHOLD  0.6f
#define IMPACT_THRESHOLD     1.8f

// ---------------- Alarm Config ----------------
#define ALARM_DURATION_MS  10000
#define BEEP_ON_MS         200
#define BEEP_OFF_MS        150
#define BEEP_PAUSE_MS      500

// ================================================================
// Variables
// ================================================================

// VC02 UART
uint16_t vc02HexCmd = 0x0000;
uint8_t  uartBuf[2];
uint8_t  uartIdx   = 0;
uint32_t uartTimer = 0;

// Alert
uint8_t alertStatus = 0;

// Fall detection state
typedef enum { STATE_NORMAL, STATE_FREE_FALL, STATE_ALARM } FallState;
FallState     fallState        = STATE_NORMAL;
unsigned long alarmStart       = 0;
unsigned long lastBuzzerToggle = 0;
int           beepCount        = 0;
bool          buzzerIsOn       = false;

Adafruit_MPU6050 mpu;

// ================================================================
// Buzzer helpers
// ================================================================
void buzzerOn() {
  digitalWrite(BUZZER_PIN, HIGH);
  buzzerIsOn = true;
}

void buzzerOff() {
  digitalWrite(BUZZER_PIN, LOW);
  buzzerIsOn = false;
}

void updateBuzzer() {
  unsigned long now = millis();
  if (buzzerIsOn) {
    if (now - lastBuzzerToggle >= BEEP_ON_MS) {
      buzzerOff();
      lastBuzzerToggle = now;
      beepCount++;
    }
  } else {
    unsigned long off = (beepCount % 3 == 0 && beepCount > 0) ? BEEP_PAUSE_MS : BEEP_OFF_MS;
    if (now - lastBuzzerToggle >= off) {
      buzzerOn();
      lastBuzzerToggle = now;
    }
  }
}

// ================================================================
// Accel magnitude
// ================================================================
float totalAccelG(sensors_event_t &a) {
  float ax = a.acceleration.x / 9.81f;
  float ay = a.acceleration.y / 9.81f;
  float az = a.acceleration.z / 9.81f;
  return sqrtf(ax*ax + ay*ay + az*az);
}

// ================================================================
// Fall Detection
// ================================================================
void readFallDetection() {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);
  float totalG = totalAccelG(accel);

  switch (fallState) {

    case STATE_NORMAL:
      if (totalG < FREE_FALL_THRESHOLD) {
        Serial.println("Free fall started...");
        fallState = STATE_FREE_FALL;
      }
      break;

    case STATE_FREE_FALL:
      if (totalG >= IMPACT_THRESHOLD) {
        Serial.println("Impact! Starting local alarm...");
        buzzerOn();
        lastBuzzerToggle = millis();
        beepCount        = 0;
        alarmStart       = millis();
        fallState        = STATE_ALARM;
      }
      break;

    case STATE_ALARM:
      updateBuzzer();

      // Cancel button pressed within 10 sec → stop alarm, alert = 0
      if (digitalRead(CANCEL_BTN_PIN) == LOW) {
        buzzerOff();
        beepCount   = 0;
        alertStatus = 0;
        fallState   = STATE_NORMAL;
        Serial.println("Alarm CANCELLED. No alert sent.");
        delay(300);
        return;
      }

      // 10 sec elapsed without cancel → alert = 1, send via LoRa
      if (millis() - alarmStart >= ALARM_DURATION_MS) {
        buzzerOff();
        beepCount   = 0;
        alertStatus = 1;
        fallState   = STATE_NORMAL;
        Serial.println("Alarm timeout — alert CONFIRMED, sending via LoRa.");
      }
      break;
  }
}

// ================================================================
// ── ORIGINAL SOURCE FUNCTIONS (UNCHANGED) ───────────────────────
// ================================================================

// ---------------- VC02 UART ----------------
void readVC02_UART() {
  while (Serial2.available()) {
    uartBuf[uartIdx++] = Serial2.read();
    uartTimer = millis();

    if (uartIdx == 2) {
      vc02HexCmd = (uartBuf[0] << 8) | uartBuf[1];

      Serial.print("VC02 HEX CMD: 0x");
      Serial.println(vc02HexCmd, HEX);

      uartIdx = 0;
    }
  }

  if (uartIdx > 0 && millis() - uartTimer > 50)
    uartIdx = 0;
}

// ---------------- Manual Trigger Button ----------------
void readManualButton() {
  if (digitalRead(MANUAL_BTN_PIN) == LOW) {
    alertStatus = 1;
    buzzerOn();
    Serial.println("Manual Trigger PRESSED — alert = 1");
    delay(300);
  }
}

// ---------------- LoRa TX ----------------
void sendLoRaPacket() {
  uint8_t packet[3];

  packet[0] = highByte(vc02HexCmd);
  packet[1] = lowByte(vc02HexCmd);
  packet[2] = alertStatus;

  Serial.print("LoRa TX -> HEX: 0x");
  Serial.print(vc02HexCmd, HEX);
  Serial.print(" | ALERT: ");
  Serial.println(alertStatus);

  LoRa.beginPacket();
  LoRa.write(packet, 3);
  LoRa.endPacket();

  vc02HexCmd  = 0x0000;
  alertStatus = 0;
}

// ================================================================
// Setup & Loop
// ================================================================

void setup() {
  Serial.begin(115200);

  // VC02 UART
  Serial2.begin(9600, SERIAL_8N1, VC02_RX, VC02_TX);

  // Buttons & Buzzer
  pinMode(CANCEL_BTN_PIN, INPUT_PULLUP);
  pinMode(MANUAL_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // MPU6050
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("Initializing MPU6050...");
  if (!mpu.begin()) {
    Serial.println("ERROR: MPU6050 not found! Check wiring.");
    while (1) { delay(500); }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 ready.");

  // LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa init FAILED");
    while (1);
  }

  // Startup beeps
  for (int i = 0; i < 2; i++) { buzzerOn(); delay(80); buzzerOff(); delay(120); }

  Serial.println("Wearable Send Node Started");
}

void loop() {
  readVC02_UART();
  readManualButton();
  readFallDetection();
  sendLoRaPacket();

  delay(100);
}
