#include <WiFi.h>
#include <ArduinoHA.h>

// ===================== CONFIGURAÇÃO =====================
#define WIFI_SSID        "Wokwi-GUEST"
#define WIFI_PASSWORD    ""

#define BROKER_ADDR      IPAddress(10, 1, 0, 109)
#define BROKER_USERNAME  "grotta"
#define BROKER_PASSWORD  "admin123"

// Pinos de hardware
#define RELAY_PIN        14   // relé: atende o telefone
#define AVAIL_LED_PIN    27   // LED azul: sistema habilitado
#define BUZZ_PIN         25   // único buzzer
#define BUZZ_CH          0    // Canal LEDC

// ===================== OBJETOS GLOBAIS ==================
WiFiClient client;
HADevice device("ESP32_Interfone_IoT");
HAMqtt   mqtt(client, device);

HASwitch swEnable("interfone_enable");     // Liga/desliga sistema
HASwitch swRing("interfone_ring_sim");      // Simula toque do interfone

bool sistemaHabilitado = false;

// ========================================================
// FUNÇÕES DE SOM -----------------------------------------
void playTone(uint16_t freq, uint16_t durMs) {
  if (freq == 0) {
    ledcWriteTone(BUZZ_CH, 0);
    ledcWrite(BUZZ_CH, 0);
  } else {
    ledcWrite(BUZZ_CH, 128);      // 50% duty para animar ícone
    ledcWriteTone(BUZZ_CH, freq);
  }
  delay(durMs);
}

void tocarRing() {
  Serial.println("[Interfone] *** TOQUE SIMULADO ***");
  playTone(800, 1000);
  playTone(0,   400);
  playTone(800, 1000);
  playTone(0,   400);
}

void tocarDTMF() {
  Serial.println("[DFPlayer] Enviando DTMF *1");
  playTone(941, 700);
  playTone(0,   300);
  playTone(697, 700);
  playTone(0,   300);
}

// ========================================================
//  FUNÇÃO ÚNICA QUE EXECUTA TODO O PROCESSO DO INTERFONE
// --------------------------------------------------------
void simularProcessoInterfone() {
  tocarRing();

  Serial.println("[Relé] Atendendo chamada");
  digitalWrite(RELAY_PIN, HIGH);
  delay(1600);

  tocarDTMF();

  digitalWrite(RELAY_PIN, LOW);
  Serial.println("[Relé] Desligado – processo concluído\n");
}

// ========================================================
// CALLBACKS MQTT -----------------------------------------
void onEnableCmd(bool st, HASwitch* s) {
  sistemaHabilitado = st;
  digitalWrite(AVAIL_LED_PIN, st);
  s->setState(st);
  Serial.println(st ? "[Sistema] HABILITADO" : "[Sistema] DESABILITADO");
}

void onRingCmd(bool st, HASwitch* s) {
  if (!st) return;          // só reage no ON
  s->setState(false);       // volta ao OFF imediato

  if (!sistemaHabilitado) {
    Serial.println("[Interfone] Ignorado: sistema desabilitado");
    return;
  }

  simularProcessoInterfone();  // *** CHAMADA CENTRAL ***
}

// ========================================================
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Interfone – refatorado");

  pinMode(RELAY_PIN, OUTPUT);      digitalWrite(RELAY_PIN, LOW);
  pinMode(AVAIL_LED_PIN, OUTPUT);  digitalWrite(AVAIL_LED_PIN, LOW);

  // Buzzer: canal LEDC
  ledcSetup(BUZZ_CH, 2000, 8);
  ledcAttachPin(BUZZ_PIN, BUZZ_CH);
  playTone(0, 1);

  // Wi‑Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { Serial.print('.'); delay(400);}  
  Serial.println("\nWi‑Fi conectado");

  // Home‑Assistant device
  device.setName("ESP32 Interfone");
  device.enableSharedAvailability();
  device.enableLastWill();

  // Switches
  swEnable.setName("Habilitar Interfone");
  swEnable.onCommand(onEnableCmd);

  swRing.setName("Simular Toque de Interfone");
  swRing.onCommand(onRingCmd);

  mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
  Serial.println("MQTT pronto — controle pelo Home Assistant");
}

// ========================================================
void loop() {
  mqtt.loop();
}
