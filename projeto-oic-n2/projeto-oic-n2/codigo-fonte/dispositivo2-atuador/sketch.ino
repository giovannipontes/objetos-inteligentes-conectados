/*
 * Dispositivo 2 - ATUADOR
 * Sistema Inteligente de Monitoramento da Qualidade do Ar - Projeto OIC (N2)
 *
 * ESP32 que ASSINA o tópico de comando MQTT e aciona um sistema de
 * ventilação/alerta (LED + buzzer) quando o ambiente é classificado como crítico.
 * O comando é enviado pelo Node-RED a partir das regras de negócio.
 */
#include <WiFi.h>
#include <PubSubClient.h>

#define LED_VENT  2        // LED vermelho = ventilacao/alerta acionado
#define BUZZER    4        // buzzer de alerta sonoro

const char* WIFI_SSID  = "Wokwi-GUEST";
const char* WIFI_PASS  = "";
const char* MQTT_BROKER = "test.mosquitto.org";
const int   MQTT_PORT   = 1883;
const char* TOPICO_CMD  = "mackenzie/oic/comando";   // recebe LIGAR / DESLIGAR

WiFiClient espClient;
PubSubClient mqtt(espClient);

void aciona(bool ligar) {
  digitalWrite(LED_VENT, ligar ? HIGH : LOW);
  if (ligar) tone(BUZZER, 1000); else noTone(BUZZER);
  Serial.println(ligar ? "Ventilacao/alerta LIGADO" : "Ventilacao/alerta DESLIGADO");
}

void callback(char* topico, byte* payload, unsigned int tamanho) {
  String msg;
  for (unsigned int i = 0; i < tamanho; i++) msg += (char)payload[i];
  Serial.print("Comando recebido: "); Serial.println(msg);
  if (msg.indexOf("DESLIGAR") >= 0) aciona(false);
  else if (msg.indexOf("LIGAR") >= 0) aciona(true);
}

void conectaWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println(" conectado!");
}

void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    String clientId = "oic-atuador-" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println(" conectado!");
      mqtt.subscribe(TOPICO_CMD);
    } else {
      Serial.print(" falhou, rc="); Serial.print(mqtt.state());
      Serial.println(" tentando em 2s");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_VENT, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  conectaWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(callback);
}

void loop() {
  if (!mqtt.connected()) conectaMQTT();
  mqtt.loop();
}
