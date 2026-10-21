#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// ==== LoRa Setup (Adafruit Feather M0 LoRa / RFM95, antenna on U.FL) ====
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
#define LORA_FREQUENCY 915E6  // must match the receiver

// LoRa.begin() leaves the PA at 17 dBm on PA_BOOST; the RFM95W's PA_BOOST pin reaches
// +20 dBm via PA_DAC (the library raises OCP to 140 mA for us above 17 dBm).
//
// CLOSE-RANGE WARNING: the SX1276 is rated for +10 dBm max RF input, as both an
// absolute maximum and an operating limit (datasheet Tables 3 and 4) -- above that,
// "stresses may cause permanent device failure". At 20 dBm a working pair delivers
// roughly +8 dBm into the receiver at 10 cm, and exceeds the +10 dBm rating below
// ~8 cm. So for bench tests closer than ~0.5 m, drop this to 2-5 dBm; keep 20 dBm
// for actual range/antenna comparisons at a distance.
#define LORA_TX_POWER_DBM 15

// ==== Battery Voltage (JST LiPo connector / VBAT rail) ====
// Feather boards route VBAT through an onboard 2:1 voltage divider to pin A7,
// so the ADC only ever sees half the actual battery voltage -- multiply by 2 to undo it.
#define VBAT_PIN A7

float readBatteryVoltage();

uint32_t packetCount = 0;

void setup() {
  Serial.begin(115200);

  LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);
  while (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("LoRa init failed, retrying...");
    delay(1000);
  }
  LoRa.setTxPower(LORA_TX_POWER_DBM);

  Serial.print("LoRa radio ready! (board 3: bare RSSI-test transmitter, ");
  Serial.print(LORA_TX_POWER_DBM);
  Serial.println(" dBm)");

  analogReadResolution(12);
}

void loop() {
  float batteryVoltage = readBatteryVoltage();
  packetCount++;

  // Packet format (leading board ID "3" identifies this RSSI-test transmitter
  // to the shared receiver): "3,<packetCount>,<batteryVoltage>"
  String packet = String("3,") + String(packetCount) + "," + String(batteryVoltage, 2);

  LoRa.beginPacket();
  LoRa.print(packet);
  LoRa.endPacket();

  Serial.print("Sent via LoRa: ");
  Serial.println(packet);

  delay(1000);
}

float readBatteryVoltage() {
  int raw = analogRead(VBAT_PIN);
  return raw * (3.3f / 4095.0f) * 2.0f;
}
