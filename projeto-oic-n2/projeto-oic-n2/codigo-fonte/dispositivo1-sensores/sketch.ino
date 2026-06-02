/*
 * Sistema Inteligente de Monitoramento da Qualidade do Ar em Ambientes Internos
 * Universidade Presbiteriana Mackenzie - Projeto OIC (N2)
 *
 * ESP32 + MQ-135 (qualidade do ar) + DHT22 (temperatura/umidade)
 * Publica os dados em formato JSON via MQTT no broker Eclipse Mosquitto.
 *
 * No Wokwi o MQ-135 e simulado por um potenciometro ligado ao pino analogico,
 * representando a variacao da saida analogica do sensor de gas.
 */
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ---------- Configuracoes ----------
#define DHTPIN     15
#define DHTTYPE    DHT22
#define MQ135PIN   34          // entrada analogica (potenciometro simula o MQ-135)
#define INTERVALO  5000        // intervalo de publicacao (ms)

const char* WIFI_SSID  = "Wokwi-GUEST";   // rede aberta do simulador Wokwi
const char* WIFI_PASS  = "";
const char* MQTT_BROKER = "test.mosquitto.org";
const int   MQTT_PORT   = 1883;
const char* MQTT_TOPIC  = "mackenzie/oic/qualidadear";   // topico do grupo

// Limites das regras de negocio (CO2 em ppm)
const int CO2_BOM      = 1000;   // < 1000 ppm  -> bom
const int CO2_MODERADO = 2000;   // 1000-2000   -> moderado / > 2000 -> critico

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient mqtt(espClient);
unsigned long ultimaLeitura = 0;

void conectaWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println(" conectado!");
}

void conectaMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    String clientId = "oic-mackenzie-" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou, rc="); Serial.print(mqtt.state());
      Serial.println(" tentando novamente em 2s");
      delay(2000);
    }
  }
}

// Converte a leitura analogica (0-4095) em uma estimativa de CO2 em ppm
int estimaCO2(int leituraAnalogica) {
  return map(leituraAnalogica, 0, 4095, 400, 5000);
}

// Classifica o ambiente conforme as regras de negocio
String classifica(int co2) {
  if (co2 < CO2_BOM)            return "bom";
  else if (co2 < CO2_MODERADO) return "moderado";
  else                         return "critico";
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  conectaWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  if (!mqtt.connected()) conectaMQTT();
  mqtt.loop();

  if (millis() - ultimaLeitura < INTERVALO) return;
  ultimaLeitura = millis();

  float temperatura = dht.readTemperature();
  float umidade     = dht.readHumidity();
  int   co2         = estimaCO2(analogRead(MQ135PIN));
  String status     = classifica(co2);

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Falha na leitura do DHT22");
    return;
  }

  // Monta o JSON com os dados coletados
  StaticJsonDocument<200> doc;
  doc["co2"]         = co2;
  doc["temperatura"] = temperatura;
  doc["umidade"]     = umidade;
  doc["status"]      = status;

  char buffer[200];
  serializeJson(doc, buffer);
  mqtt.publish(MQTT_TOPIC, buffer);
  Serial.print("Publicado -> "); Serial.println(buffer);
}
