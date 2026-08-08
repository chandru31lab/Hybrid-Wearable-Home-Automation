
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "****************"
#define BLYNK_TEMPLATE_NAME "Hybrid Home Automation"
#define BLYNK_AUTH_TOKEN "******************"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <SPI.h>
#include <LoRa.h>

// ---------- WiFi ----------
char ssid[] = "********";
char pass[] = "***********";

// ---------- WiFi / Blynk config ----------
#define WIFI_CONNECT_TIMEOUT_MS  8000   // max wait for WiFi at boot
bool blynkConnected = false;

// ---------- Relay Pins ----------
#define LIGHT1  5          // Active LOW
#define LIGHT2  18         // Active LOW
#define FAN     19         // Active LOW
#define ALARM1  12        // Active HIGH
#define ALARM2  4         // Active HIGH

// ---------- Blynk Virtual Pins ----------
#define VPIN_LIGHT1      V0
#define VPIN_LIGHT2      V1
#define VPIN_HELP_STATUS V2
#define VPIN_MED_ALERT   V3
#define VPIN_FAN         V4

// ---------- LoRa Pins ----------
#define LORA_SCK   25
#define LORA_MISO  33
#define LORA_MOSI  32
#define LORA_SS    26
#define LORA_RST   14
#define LORA_DIO0  27
#define LORA_FREQ  433E6

// ---------- Alert Timing ----------
#define ALERT_ALARM_DURATION_MS  6000   // 5 sec alarm on ALARM1

bool          alertActive = false;
unsigned long alertStart  = 0;

// ---------- Relay Functions ----------
void lightOn(int pin)  { digitalWrite(pin, LOW); }
void lightOff(int pin) { digitalWrite(pin, HIGH); }

void fanOn()  { digitalWrite(FAN, LOW); }
void fanOff() { digitalWrite(FAN, HIGH); }

void alarmOn(int pin)  { digitalWrite(pin, HIGH); }
void alarmOff(int pin) { digitalWrite(pin, LOW); }

// ---------- Safe Blynk write (only if connected) ----------
void blynkWrite(int vpin, int val) {
  if (blynkConnected && Blynk.connected()) Blynk.virtualWrite(vpin, val);
}
void blynkWriteStr(int vpin, const char* val) {
  if (blynkConnected && Blynk.connected()) Blynk.virtualWrite(vpin, val);
}

// ---------- Blynk Controls ----------
BLYNK_WRITE(VPIN_LIGHT1) {
  if (param.asInt()) lightOn(LIGHT1);
  else lightOff(LIGHT1);
}

BLYNK_WRITE(VPIN_LIGHT2) {
  if (param.asInt()) lightOn(LIGHT2);
  else lightOff(LIGHT2);
}

BLYNK_WRITE(VPIN_MED_ALERT) {
  if (param.asInt()) alarmOn(ALARM2);
  else alarmOff(ALARM2);
}

BLYNK_WRITE(VPIN_FAN) {
  if (param.asInt()) fanOn();
  else fanOff();
}

// ---------- Handle VC-02 Commands (UNCHANGED) ----------
void handleHexCommand(uint16_t cmd) {
  Serial.printf("Handling Command: 0x%04X\n", cmd);

  switch (cmd) {
    case 0xA111: lightOn(LIGHT1);  blynkWrite(VPIN_LIGHT1, 1); break;
    case 0xA118: lightOff(LIGHT1); blynkWrite(VPIN_LIGHT1, 0); break;
    case 0xA119: lightOn(LIGHT2);  blynkWrite(VPIN_LIGHT2, 1); break;
    case 0xA112: lightOff(LIGHT2); blynkWrite(VPIN_LIGHT2, 0); break;

    case 0xA113:
      lightOn(LIGHT1); lightOn(LIGHT2);
      blynkWrite(VPIN_LIGHT1, 1);
      blynkWrite(VPIN_LIGHT2, 1);
      break;

    case 0xA114:
      lightOff(LIGHT1); lightOff(LIGHT2);
      blynkWrite(VPIN_LIGHT1, 0);
      blynkWrite(VPIN_LIGHT2, 0);
      break;

    case 0xA120: fanOn();  blynkWrite(VPIN_FAN, 1); break;
    case 0xA121: fanOff(); blynkWrite(VPIN_FAN, 0); break;

    case 0xA122:
      lightOff(LIGHT1); lightOff(LIGHT2);
      fanOff();
      alarmOff(ALARM1); alarmOff(ALARM2);
      blynkWrite(VPIN_LIGHT1, 0);
      blynkWrite(VPIN_LIGHT2, 0);
      blynkWrite(VPIN_FAN, 0);
      break;

    case 0xA123:
      lightOn(LIGHT1); lightOn(LIGHT2); fanOn();
      blynkWrite(VPIN_LIGHT1, 1);
      blynkWrite(VPIN_LIGHT2, 1);
      blynkWrite(VPIN_FAN, 1);
      break;

    case 0xA117:
      alarmOn(ALARM1);
      blynkWriteStr(VPIN_HELP_STATUS, "Emergency");
      delay(5000);
      alarmOff(ALARM1);
      blynkWriteStr(VPIN_HELP_STATUS, "Safe");
      break;

    default:
      Serial.println("Unknown Command");
      break;
  }
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(LIGHT1, OUTPUT);
  pinMode(LIGHT2, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(ALARM1, OUTPUT);
  pinMode(ALARM2, OUTPUT);

  lightOff(LIGHT1);
  lightOff(LIGHT2);
  fanOff();
  alarmOff(ALARM1);
  alarmOff(ALARM2);

  // ── Non-blocking WiFi connect ──────────────────────────────────
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, pass);
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_CONNECT_TIMEOUT_MS) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    // Connect Blynk without blocking — 4 sec timeout
    Blynk.config(BLYNK_AUTH_TOKEN);
    blynkConnected = Blynk.connect(4000);
    if (blynkConnected) {
      Serial.println("Blynk connected.");
      Blynk.virtualWrite(VPIN_HELP_STATUS, "Safe");
    } else {
      Serial.println("Blynk connect failed — running offline.");
    }
  } else {
    Serial.println("\nWiFi not available — running fully offline.");
  }

  // ── LoRa ──────────────────────────────────────────────────────
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa init FAILED");
    while (1);
  }

  Serial.println("Main System Ready (LoRa + Blynk optional)");
}

// ---------- Loop ----------
void loop() {

  // ── Run Blynk only if connected ───────────────────────────────
  if (blynkConnected) {
    if (Blynk.connected()) {
      Blynk.run();
    } else {
      // Lost connection mid-run — try reconnect once, don't block
      blynkConnected = Blynk.connect(2000);
    }
  }

  // ── 5 sec alert timeout (non-blocking) ───────────────────────
  if (alertActive && millis() - alertStart >= ALERT_ALARM_DURATION_MS) {
    alarmOff(ALARM1);
    blynkWriteStr(VPIN_HELP_STATUS, "Safe");
    alertActive = false;
    Serial.println("Alert alarm ended — status: Safe");
  }

  // ── LoRa receive ──────────────────────────────────────────────
  int packetSize = LoRa.parsePacket();
  if (packetSize == 3) {
    uint8_t msb         = LoRa.read();
    uint8_t lsb         = LoRa.read();
    uint8_t alertStatus = LoRa.read();   // 1 = alert, 0 = normal

    uint16_t cmd = (msb << 8) | lsb;

    Serial.printf("LoRa RX -> HEX: 0x%04X | ALERT: %d\n", cmd, alertStatus);

    if (cmd != 0x0000) {
      handleHexCommand(cmd);
    }

    // Alert = 1 → trigger ALARM1 for 5 sec + update Blynk if online
    if (alertStatus == 1 && !alertActive) {
      alarmOn(ALARM1);
      blynkWriteStr(VPIN_HELP_STATUS, "Emergency");
      alertStart  = millis();
      alertActive = true;
      Serial.println("Alert received — ALARM ON for 5 sec");
    }
  }
}
