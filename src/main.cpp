#include <WiFi.h>
#include <ArduinoHA.h>

// ===================== CONFIGURAÇÃO =====================
#define WIFI_SSID        "Wokwi-GUEST"
#define WIFI_PASSWORD    ""

#define BROKER_ADDR      IPAddress(10, 1, 0, 109)
#define BROKER_USERNAME  "grotta"
#define BROKER_PASSWORD  "admin123"

// Pinos de hardware
#define RELAY_PIN         14   // aciona o relé que atende o telefone
#define AVAIL_LED_PIN     27   // indica se o sistema está habilitado

// ===================== OBJETOS GLOBAIS ==================
WiFiClient client;
HADevice device("ESP32_Interfone_IoT");
HAMqtt   mqtt(client, device);

// Switches expostos ao Home Assistant
HASwitch interfoneEnable("interfone_enable");   // Liga / desliga o sistema
HASwitch interfoneRingSim("interfone_ring_sim"); // Simula toque do interfone

bool sistemaHabilitado = false;
bool acaoEmProgresso   = false;

// ========================================================
// CALLBACKS
// --------------------------------------------------------
void onEnableCommand(bool state, HASwitch* sender) {
  sistemaHabilitado = state;
  digitalWrite(AVAIL_LED_PIN, state);
  sender->setState(state);
  Serial.println(state ? "[Sistema] HABILITADO" : "[Sistema] DESABILITADO");
}

void onRingSimCommand(bool state, HASwitch* sender) {
  if (!state) {                   // só reage quando liga o switch
    return;                       // ignore OFF
  }
  if (!sistemaHabilitado) {
    Serial.println("[Interfone] Ignorado: sistema desabilitado");
    sender->setState(false);
    return;
  }
  if (acaoEmProgresso) {
    Serial.println("[Interfone] Ação já em andamento");
    sender->setState(false);
    return;
  }

  acaoEmProgresso = true;
  Serial.println("[Interfone] Toque SIMULADO → Atendendo...");

  digitalWrite(RELAY_PIN, HIGH);  // ativa relé (atende)
  delay(800);                     // pequena pausa simulando atendimento

  Serial.println("[DFPlayer] (sim) Tocar DTMF *1");
  delay(1200);                    // duração do áudio (simulado)

  digitalWrite(RELAY_PIN, LOW);   // desliga relé
  Serial.println("[Interfone] Processo concluído\n");

  acaoEmProgresso = false;
  sender->setState(false);        // retorna switch ao OFF
}

// ========================================================
// SETUP
// --------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando ESP32 Interfone...");

  pinMode(RELAY_PIN, OUTPUT);      digitalWrite(RELAY_PIN, LOW);
  pinMode(AVAIL_LED_PIN, OUTPUT);  digitalWrite(AVAIL_LED_PIN, LOW);

  // Conecta Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("\nWi-Fi conectado");

  // Configura dispositivo Home Assistant
  device.setName("ESP32 Interfone");
  device.enableSharedAvailability();
  device.enableLastWill();

  // Registra switches MQTT
  interfoneEnable.setName("Habilitar Interfone");
  interfoneEnable.onCommand(onEnableCommand);

  interfoneRingSim.setName("Simular Toque de Interfone");
  interfoneRingSim.onCommand(onRingSimCommand);

  // Inicia MQTT
  mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
  Serial.println("MQTT iniciado, aguardando comandos do Home Assistant.");
}

// ========================================================
// LOOP PRINCIPAL
// --------------------------------------------------------
void loop() {
  mqtt.loop();
}
