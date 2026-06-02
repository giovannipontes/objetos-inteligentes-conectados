# Sistema Inteligente de Monitoramento da Qualidade do Ar em Ambientes Internos (IoT)

Projeto da disciplina **Objetos Inteligentes Conectados (OIC)** — Universidade Presbiteriana Mackenzie.
Monitora **CO₂, temperatura e umidade** de ambientes internos com dois dispositivos IoT (sensor + atuador),
transmite os dados via **MQTT**, processa **regras de negócio** no **Node-RED**, grava em um banco de
**séries temporais em nuvem (InfluxDB Cloud)** e exibe **dashboards** no **Grafana**, além de **acionar um
atuador** (ventilação/alerta) quando o ar fica crítico.

## Arquitetura

```
[Disp. 1: ESP32 + MQ-135 + DHT22] ──publica──┐
                                              ├─► MQTT (Mosquitto) ─► Node-RED ─► InfluxDB Cloud ─► Grafana
[Disp. 2: ESP32 + LED/buzzer (atuador)] ◄─────┘        (regras de negócio + API WAQI + comando ao atuador)
```

| Camada | Tecnologia |
|---|---|
| Sensoriamento | MQ-135 (CO₂/gases), DHT22 (temperatura/umidade) |
| Atuação | ESP32 + LED + buzzer (ventilação/alerta) |
| Microcontrolador | ESP32 |
| Comunicação | MQTT — broker **Eclipse Mosquitto** (`test.mosquitto.org`) |
| Processamento | **Node-RED** (regras de negócio) |
| API externa | **WAQI** (qualidade do ar externa) |
| Banco em nuvem | **InfluxDB Cloud** (série temporal) |
| Dashboards | **Grafana** (ferramenta externa) |
| Simulação | **Wokwi** |

## Estrutura do repositório

```
projeto-oic-n2/
├── codigo-fonte/
│   ├── dispositivo1-sensores/   # ESP32 + MQ-135 + DHT22 (sketch.ino, diagram.json, libraries.txt)
│   └── dispositivo2-atuador/    # ESP32 + LED/buzzer (atuador via MQTT)
├── node-red/flows.json          # Fluxo: regras + comando ao atuador + gravação no InfluxDB + WAQI
├── simulacao/                   # Instruções de reprodução no Wokwi
├── artigo/                      # Artigo técnico (LaTeX + PDF)
└── docs/                        # Guias (InfluxDB/Grafana, roteiro do vídeo, execução)
```

## Regras de negócio

| Variável | Bom | Moderado / Crítico |
|---|---|---|
| CO₂ (ppm) | < 1000 | 1000–2000 (moderado) / > 2000 (crítico) |
| Temperatura (°C) | 18 – 26 (conforto) | fora da faixa gera alerta |
| Umidade (%) | 30 – 60 (conforto) | fora da faixa gera alerta |

Quando o ambiente é classificado como **crítico**, o Node-RED publica o comando `LIGAR` no tópico
`mackenzie/oic/comando`, e o **atuador** aciona a ventilação/alerta.

## Como executar

### 1. Dispositivos (Wokwi)
Crie **dois** projetos ESP32 em https://wokwi.com e cole os arquivos de `dispositivo1-sensores/` e
`dispositivo2-atuador/` (sketch.ino + diagram.json + libraries.txt). Inicie as simulações.

### 2. Node-RED
```
npm install -g node-red
cd ~/.node-red && npm install node-red-contrib-influxdb
node-red
```
Importe `node-red/flows.json`, configure o **token** do InfluxDB no nó "InfluxDB Cloud" e faça **Deploy**.
Veja `docs/setup_influxdb_grafana.md`.

### 3. Banco em nuvem (InfluxDB Cloud) e Dashboards (Grafana)
Siga `docs/setup_influxdb_grafana.md`: criar bucket `qualidade_ar`, token, e conectar o Grafana ao
InfluxDB (SQL) com 2+ painéis.

## Links
- **Vídeo (YouTube):** _adicionar link_
- **Repositório:** _este repositório_

## Autores
- Gustavo Giacometti
- Giovanni Pontes
- Wallace Santana

Orientador: Prof. Wallace Rodrigues de Santana — UPM, Faculdade de Computação e Informática.
