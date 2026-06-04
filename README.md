#  Sistema Inteligente de Monitoramento da Qualidade do Ar em Ambientes Internos (IoT)

Projeto da disciplina **Objetos Inteligentes Conectados (OIC)** — Universidade Presbiteriana Mackenzie.

Sistema IoT que monitora **CO₂, temperatura e umidade** de ambientes internos em tempo real, **classifica** a
qualidade do ar, **age automaticamente** (aciona ventilação/alerta) quando o ar fica crítico e disponibiliza
os dados em **dashboards** na nuvem.

---

## Sobre o projeto

Passamos mais de **80% do tempo em ambientes fechados**, onde a concentração de poluentes pode ser várias
vezes maior que a do ar externo. O excesso de **CO₂** prejudica a saúde, o conforto e a produtividade — e
raramente é monitorado em casas, escritórios e salas de aula.

Este projeto resolve isso com um sistema de **Internet das Coisas (IoT)** que mede o ar, decide se o ambiente
está adequado e **atua** para corrigi-lo, fechando a malha **medir → decidir → agir**.

### Objetivos de Desenvolvimento Sustentável (ONU)
- **ODS 3 — Saúde e Bem-Estar:** prevenção contra doenças respiratórias e exposição a poluentes.
- **ODS 11 — Cidades e Comunidades Sustentáveis:** ambientes mais saudáveis e eficientes.

---

## Arquitetura

<img width="480" height="769" alt="arquitetura" src="https://github.com/user-attachments/assets/3372e5fc-b7b6-428a-a6ee-493b7a9f87d3" />


```
[Disp.1: ESP32 + MQ-135 + DHT22]  →  publica leitura ┐
                                                      ├→ MQTT (Mosquitto) → Node-RED → InfluxDB Cloud → Grafana
[Disp.2: ESP32 + LED + Buzzer]    ←  recebe comando  ┘        (regras + API WAQI + comando ao atuador)
```

Cada elemento roda em uma **máquina diferente**: broker, Node-RED, banco e dashboard são independentes.

---

## Os dois dispositivos

### Dispositivo 1 — Sensores
ESP32 com **MQ-135** (CO₂/gases) e **DHT22** (temperatura/umidade). A cada 5 segundos publica os dados em
JSON via MQTT. No Wokwi, o MQ-135 é simulado por um potenciômetro.
<img width="1510" height="765" alt="dispositivo1" src="https://github.com/user-attachments/assets/efa6ab47-4b84-4516-9fb1-c4fdf95491a4" />


### Dispositivo 2 — Atuador
ESP32 com **LED + buzzer** que assina o tópico de comando MQTT. Quando o ambiente fica **crítico**, ele
**liga** a ventilação/alerta; quando normaliza, **desliga**.

<img width="1509" height="771" alt="dispositivo2" src="https://github.com/user-attachments/assets/e8e89410-cf10-4a71-a69c-af0bacdfcca8" />


---

## Como funciona

O **Node-RED** é o cérebro do sistema. Ele recebe as leituras por MQTT, aplica as **regras de negócio**,
consome a **API WAQI** (qualidade do ar externa de São Paulo, como referência), grava tudo no **InfluxDB
Cloud** e publica o comando para o atuador.

<img width="1507" height="744" alt="node" src="https://github.com/user-attachments/assets/538ddd4d-e778-4656-8c20-d0d926f611fa" />


### Regras de negócio
| Variável | Bom | Moderado | Crítico |
|---|---|---|---|
| CO₂ (ppm) | < 1000 | 1000 – 2000 | > 2000 |
| Temperatura (°C) | 18 – 26 (conforto) | — | fora da faixa gera alerta |
| Umidade (%) | 30 – 60 (conforto) | — | fora da faixa gera alerta |

Quando o status é **crítico**, o Node-RED publica `LIGAR` no tópico `mackenzie/oic/comando` e o atuador aciona.

---

## Dashboard (Grafana)

Os dados são visualizados no **Grafana**, conectado ao InfluxDB. Os painéis mostram CO₂, temperatura, umidade,
o status do ambiente e o estado do atuador, com faixas de cor (verde/amarelo/vermelho) e histórico.

<img width="836" height="582" alt="dashboard" src="https://github.com/user-attachments/assets/4e68650b-b1dd-4342-8d2e-f27b26b963ef" />


---

## Tecnologias

| Camada | Tecnologia |
|---|---|
| Sensoriamento | MQ-135, DHT22 |
| Atuação | LED + buzzer (ventilação/alerta) |
| Microcontrolador | ESP32 |
| Comunicação | MQTT — Eclipse Mosquitto |
| Processamento | Node-RED (low-code) |
| API externa | WAQI (World Air Quality Index) |
| Banco em nuvem | InfluxDB Cloud (série temporal) |
| Dashboards | Grafana |
| Simulação | Wokwi |

---

## Estrutura do repositório

```
projeto-oic-n2/
├── codigo-fonte/
│   ├── dispositivo1-sensores/   # ESP32 + MQ-135 + DHT22
│   └── dispositivo2-atuador/    # ESP32 + LED/buzzer
├── node-red/flows.json          # fluxo (regras + API + InfluxDB + comando)
├── simulacao/                   # reprodução no Wokwi
├── artigo/                      # artigo técnico (PDF) + imagens
└── docs/                        # guias e roteiro do vídeo
```

---

## Como executar (resumo)

1. **Wokwi:** crie 2 projetos ESP32 e cole os arquivos de `dispositivo1-sensores/` e `dispositivo2-atuador/`.
2. **Node-RED:** `npm install -g node-red` → `cd ~/.node-red && npm install node-red-contrib-influxdb` → `node-red`. Importe `node-red/flows.json` e configure o token do InfluxDB.
3. **InfluxDB Cloud + Grafana:** siga `docs/setup_influxdb_grafana.md`.

---

## Vídeo de demonstração
https://youtu.be/rEQPCXKOat4

## Autores
- Gustavo Giacometti
- Giovanni Pontes

Prof. Wallace Rodrigues de Santana — UPM, Faculdade de Computação e Informática.
