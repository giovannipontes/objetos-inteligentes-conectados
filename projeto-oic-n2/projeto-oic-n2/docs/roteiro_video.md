# Roteiro do Vídeo de Apresentação — Projeto OIC N2
**Duração:** máx. 5–6 min · **Formato:** gravação de tela + narração · **Se por celular: paisagem**

> O professor exige mostrar: título/integrantes/data, descrição + ODS, **sensor**, **atuador**, **MQTT**,
> **API**, **banco de dados** e **dashboard**.

## 1. Abertura (0:00–0:40)
- Título: *Sistema Inteligente de Monitoramento da Qualidade do Ar em Ambientes Internos utilizando IoT*
- Integrantes: Gustavo Giacometti, Giovanni Pontes, Wallace Santana · **data de criação**
- Problema: passamos +80% do tempo em ambientes fechados; CO₂ alto afeta saúde e produtividade
- **ODS 3 (Saúde e Bem-Estar)** e **ODS 11 (Cidades e Comunidades Sustentáveis)**

## 2. Arquitetura (0:40–1:30)
Mostrar o diagrama:
`Sensores/Atuador (ESP32) → MQTT (Mosquitto) → Node-RED → InfluxDB Cloud → Grafana`
Explicar que cada elemento está em uma máquina diferente (broker, Node-RED, banco, dashboard).

## 3. Sensor — Dispositivo 1 (1:30–2:30)
- Abrir o Wokwi do **dispositivo de sensores** (ESP32 + MQ-135 + DHT22)
- Mostrar o **Serial Monitor** publicando o JSON e **girar o potenciômetro** (simula o MQ-135) → CO₂ varia

## 4. Atuador — Dispositivo 2 (2:30–3:10)
- Abrir o Wokwi do **atuador** (ESP32 + LED + buzzer)
- Mostrar que, quando o ar fica **crítico**, o Node-RED publica `LIGAR` e o **LED/buzzer aciona**
  (ventilação/alerta); ao normalizar, desliga

## 5. MQTT + Node-RED + API (3:10–4:00)
- Mostrar o fluxo no Node-RED: nó **MQTT in**, a função **Regras de Negócio**, o nó de **comando**, a
  chamada da **API WAQI** e o nó **InfluxDB**
- Explicar a classificação (bom/moderado/crítico) e o comando ao atuador

## 6. Banco de dados em nuvem (4:00–4:30)
- Abrir o **InfluxDB Cloud → Data Explorer** e mostrar os dados gravados (measurement `leituras`)

## 7. Dashboard no Grafana (4:30–5:30)
- Abrir o **Grafana** com os painéis (gauge + histórico de CO₂, temperatura e umidade)
- **Girar o potenciômetro no Wokwi** e mostrar o **gauge mudando de cor** e a curva subindo em tempo real
- Comentar a leitura gerencial (faixas de cor, tendência)

## 8. Encerramento (5:30–6:00)
- Resultados: classificação correta, atuação preventiva, dados em nuvem
- Mostrar rapidamente o **repositório no GitHub**
- Conclusão e agradecimento

---

### Checklist de gravação
- [ ] Wokwi (sensor) + Wokwi (atuador) abertos
- [ ] Node-RED (editor) aberto
- [ ] InfluxDB Cloud (Data Explorer) aberto
- [ ] Grafana (dashboard) aberto em Last 5 min / auto-refresh
- [ ] Repositório GitHub aberto
- [ ] Narração ensaiada · vídeo ≤ 6 min · paisagem (se celular)
- [ ] Subir no YouTube e colar o link no artigo e no README
