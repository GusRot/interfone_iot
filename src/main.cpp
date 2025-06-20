#include <WiFi.h>
#include <ArduinoHA.h>

/* ---------- CONFIG ---------- */
#define WIFI_SSID        "Wokwi-GUEST"
#define WIFI_PASSWORD    ""

#define BROKER_ADDR      IPAddress(10, 1, 0, 109)
#define BROKER_USERNAME  "grotta"
#define BROKER_PASSWORD  "admin123"

/* GPIOs */
#define RELAY_PIN        14        // relé
#define AVAIL_LED_PIN    27        // LED azul (sistema habilitado)
#define RING_LED_PIN     32        // LED laranja  – simula toque do interfone
#define DTMF_LED_PIN     33        // LED rosa     – só quando habilitado

/* ---------- MQTT / HA ---------- */
WiFiClient client;
HADevice device("ESP32_Interfone_IoT");
HAMqtt   mqtt(client, device);

HASwitch swEnable("interfone_enable");
HASwitch swRing  ("interfone_ring_sim");

bool sistemaHabilitado = false;

/* ---------- Funções utilitárias ---------- */
void blinkLED(uint8_t pin, uint16_t ms) {
  digitalWrite(pin, HIGH);
  delay(ms);
  digitalWrite(pin, LOW);
}

/* ---------- TOQUES ---------- */
void tocarRing() {
  Serial.println("[Interfone] *RING*");
  blinkLED(RING_LED_PIN, 1000);
  delay(400);
  blinkLED(RING_LED_PIN, 1000);
  delay(400);
}

void tocarDTMF() {
  Serial.println("[DTMF] * → 1");
  blinkLED(DTMF_LED_PIN, 700);
  delay(300);
  blinkLED(DTMF_LED_PIN, 700);
  delay(300);
}

/* ---------- Rotina de atendimento ---------- */
void atenderEEnviarDTMF() {
  Serial.println("[Relé] Atendendo…");
  digitalWrite(RELAY_PIN, HIGH);
  delay(1600);

  tocarDTMF();

  digitalWrite(RELAY_PIN, LOW);
  Serial.println("[Relé] Desligado \n");
}

/* ---------- CALLBACKS MQTT ---------- */
void onEnableCmd(bool st, HASwitch* s) {
  sistemaHabilitado = st;
  digitalWrite(AVAIL_LED_PIN, st);
  s->setState(st);
  Serial.println(st ? "[Sistema] HABILITADO" : "[Sistema] DESABILITADO");
}

void onRingCmd(bool st, HASwitch* s) {
  if (!st) return;          // reage somente ao ON
  s->setState(false);

  /* Toque sempre acontece */
  tocarRing();

  /* Se o sistema estiver habilitado, executa */
  if (!sistemaHabilitado) {
    Serial.println("[Interfone] Tocou, mas sistema desabilitado - ignorado");
    return;
  }

  atenderEEnviarDTMF();
}

/* ---------- SETUP ---------- */
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(AVAIL_LED_PIN, OUTPUT);
  pinMode(RING_LED_PIN, OUTPUT);
  pinMode(DTMF_LED_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { Serial.print('.'); delay(400); }
  Serial.println("\nWi-Fi ok");

  device.setName("ESP32 Interfone");
  device.enableSharedAvailability();
  device.enableLastWill();

  swEnable.setName("Habilitar Interfone");
  swEnable.onCommand(onEnableCmd);

  swRing.setName("Simular Toque de Interfone");
  swRing.onCommand(onRingCmd);

  mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
  Serial.println("MQTT pronto — controle via HA");
}

void loop() {
  mqtt.loop();
}
