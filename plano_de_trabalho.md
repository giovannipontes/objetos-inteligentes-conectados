# Plano de Trabalho — Correções para Reapresentação (Prova Final)

**Projeto:** Sistema Inteligente de Monitoramento da Qualidade do Ar em Ambientes Internos (IoT)
**Disciplina:** Objetos Inteligentes Conectados — Prof. Wallace Rodrigues de Santana
**Grupo:** Gustavo Giacometti, Giovanni Pontes, Wallace Santana

## Contexto
Este plano detalha, de forma técnica, as correções que serão realizadas para atacar os três pontos
apontados na avaliação da N2:
1. Dashboard não reflete o uso de dois dispositivos (−1,0);
2. Uso dos dois dispositivos não justificado — "poderiam estar em um único dispositivo" (−1,0);
3. Materiais e Métodos — falta da calibração do potenciômetro e das regras de negócio (−1,0).

Para cada correção descrevemos **o que muda no firmware, no fluxo do Node-RED, no banco de dados, no
dashboard e no artigo**, no formato **Antes → Depois**, de modo que a mudança seja verificável.

---

# Correção 1 — Justificativa e diferenciação técnica dos dois dispositivos

### Problema apontado
"Não conseguiram justificar o uso dos dois dispositivos, pois todos os componentes poderiam estar em um
único dispositivo."

### Mudança de concepção
O Dispositivo 2 deixa de ser um simples **LED indicador local** e passa a ser um **nó atuador remoto e
bidirecional**: ele recebe comandos e **publica de volta o seu próprio estado**, tornando-se um elemento
independente que **só funciona em rede**. Com isso, os dois dispositivos passam a ser **mutuamente
dependentes da comunicação MQTT** — não podendo ser reduzidos a um único microcontrolador sem perder a
característica distribuída do sistema.

### 1.1. Mudança no firmware (Dispositivo 2 — atuador)
**Antes:** o dispositivo apenas **assinava** o tópico de comando e ligava/desligava o LED localmente.

**Depois:** além de assinar o comando, o dispositivo **publica seu próprio estado** após atuar:
- Novo tópico de publicação: `mackenzie/oic/atuador/status` (valores `ligado` / `desligado`).
- Trecho adicionado na função de acionamento:
  ```cpp
  void aciona(bool ligar) {
    digitalWrite(LED_VENT, ligar ? HIGH : LOW);
    if (ligar) tone(BUZZER, 1000); else noTone(BUZZER);
    // NOVO: confirma o próprio estado de volta ao sistema
    mqtt.publish("mackenzie/oic/atuador/status", ligar ? "ligado" : "desligado");
  }
  ```
- Resultado: o Dispositivo 2 agora é um **cliente MQTT completo** (assina **e** publica), com `clientId`
  próprio (`oic-atuador-...`), distinto do `clientId` do sensor.

### 1.2. Mudança no fluxo / lógica do Node-RED
**Antes:** o estado do atuador gravado no banco vinha do **comando** gerado pela regra (origem: Node-RED).
Fluxo do ramo do atuador:
```
[regras] --comando--> [Comando ao Atuador (mqtt out)]
[regras] --comando--> [monta_atuador (function)] --> [influxdb out: atuador]   (estado derivado do comando)
```

**Depois:** o estado gravado passa a vir da **confirmação do próprio Dispositivo 2** (origem: o atuador
físico/simulado, não o Node-RED). Mudanças nos nós:
- **Removidos:** `monta_atuador` (function) e o `influxdb out` que gravava o estado a partir do comando.
- **Adicionados:**
  - `Status do Atuador (mqtt in)` — assina `mackenzie/oic/atuador/status`;
  - `Estado confirmado (function)` — converte `ligado`/`desligado` em `1`/`0` e marca a origem
    (`fonte = dispositivo2`);
  - `Grava atuador (influxdb out)` — grava no *measurement* `atuador`.
- Fluxo novo do ramo do atuador:
  ```
  [regras] --comando--> [Comando ao Atuador (mqtt out)]  -->  (rede / broker)  -->  Dispositivo 2
  Dispositivo 2  -->  (rede / broker)  -->  [Status do Atuador (mqtt in)] --> [Estado confirmado] --> [influxdb out: atuador]
  ```
- **Lógica nova (function "Estado confirmado"):**
  ```javascript
  var s = (msg.payload || '').toString().toLowerCase();
  var estado = s.indexOf('desl') >= 0 ? 0 : (s.indexOf('lig') >= 0 ? 1 : 0);
  msg.payload = [ { estado: estado }, { fonte: 'dispositivo2' } ];
  return msg;
  ```
- **Efeito:** o dado de atuação que aparece no banco e no dashboard é agora a **confirmação real do
  Dispositivo 2**, e não mais uma dedução interna do Node-RED. Isso prova, no próprio dado, a existência e a
  participação do segundo dispositivo.

> Os demais ramos (sensores → `leituras`; WAQI → `ar_externo`) **permanecem inalterados**.

### 1.3. Mudança no banco de dados (InfluxDB Cloud)
- *Measurement* `atuador`, campo `estado` (1/0) e *tag* `fonte = dispositivo2`.
- **Antes:** valor escrito a partir do comando do Node-RED.
- **Depois:** valor escrito a partir da **mensagem publicada pelo Dispositivo 2**.
- Sem mudança nos *measurements* `leituras` e `ar_externo`.

### 1.4. Justificativa técnica (à prova de refutação)
A separação em dois dispositivos é **necessária e correta**, pelos seguintes motivos:

1. **Exigência do próprio descritivo:** para projetos simulados, o documento determina *"um dispositivo
   deverá conter um ou mais sensores e o outro dispositivo um ou mais atuadores"*. A implementação segue
   exatamente essa regra (Disp.1 = sensores; Disp.2 = atuador).
2. **Separação física real:** o nó sensor fica **no ambiente monitorado** (medindo o ar) e o nó atuador fica
   **junto ao sistema de ventilação/exaustão**, em outro ponto físico. Em uma instalação real, não é possível
   que estejam no mesmo microcontrolador, pois ocupam locais diferentes.
3. **Comunicação bidirecional e desacoplada:** cada dispositivo é um **cliente MQTT independente**, com
   `clientId` próprio; o Disp.2 **assina** comandos e **publica** confirmação. Os dois só se comunicam pelo
   **broker** — não há ligação direta. Concentrar tudo em um único dispositivo eliminaria o padrão
   publish/subscribe entre nós, que é o cerne da arquitetura IoT.
4. **Independência e tolerância a falhas:** o sensor continua publicando mesmo que o atuador esteja
   offline, e vice-versa. Um único dispositivo seria um **ponto único de falha**.
5. **Escalabilidade N:M:** um mesmo Node-RED coordena **vários sensores e vários atuadores** distribuídos
   (ex.: uma central de ventilação reagindo a sensores de várias salas). Essa topologia N:M é **impossível**
   com todos os componentes em um único dispositivo.

> Em simulação, os dois dispositivos **representam** esses nós fisicamente separados; o que se avalia é a
> **arquitetura distribuída**, demonstrada pela comunicação bidirecional desacoplada e pela confirmação de
> estado do atuador.

**Entregável:** firmware do Disp.2 atualizado, `flows.json` atualizado, seção de justificativa no artigo e
diagrama de topologia revisado.

---

# Correção 2 — Dashboard refletindo o uso dos dois dispositivos

### Problema apontado
"O dashboard não reflete o uso de dois dispositivos."

### Mudança (Antes → Depois)
**Antes:** painéis dos sensores + um indicador de atuador (0/1) sem rótulo de dispositivo; a leitura do
atuador vinha do comando.

**Depois:** dashboard reorganizado em **duas seções rotuladas por dispositivo**:

- **Seção "Dispositivo 1 — Sensores"** (fonte: *measurement* `leituras`):
  - Gauge de CO₂ (faixas 1000 e 2000 ppm em verde/amarelo/vermelho);
  - Série temporal de CO₂;
  - Gauge e/ou série de temperatura e umidade.
  - Query exemplo:
    ```sql
    SELECT time, co2 FROM leituras WHERE $__timeFilter(time) ORDER BY time
    ```
- **Seção "Dispositivo 2 — Atuador"** (fonte: *measurement* `atuador`, alimentado pela **confirmação do
  Disp.2**):
  - Painel **Stat** com mapeamento `1 → LIGADO` (vermelho) / `0 → DESLIGADO` (verde);
  - Painel **State timeline** mostrando os períodos em que a ventilação esteve ativa.
  - Query exemplo:
    ```sql
    SELECT time, estado FROM atuador WHERE $__timeFilter(time) ORDER BY time
    ```

Assim, o dashboard exibe **explicitamente** dados originados de **dois dispositivos distintos**: as medições
(Disp.1) e a atuação confirmada (Disp.2).

**Entregável:** dashboard reorganizado no Grafana + novos prints inseridos no artigo (Resultados).

---

# Correção 3 — Materiais e Métodos: calibração do potenciômetro e regras de negócio

### Problema apontado
"Não mostrou como o potenciômetro foi calibrado e nem as regras de negócio (a partir de quais valores a
ventilação é acionada?)."

### O que será adicionado ao artigo
**3.1. Calibração do sensor de gás (simulação).** O MQ-135 é simulado por um potenciômetro. O conversor
analógico-digital (ADC) do ESP32 lê valores de **0 a 4095**, convertidos linearmente para **400 a 5000 ppm**
de CO₂ pela função `map()` (referência: ~400 ppm para ar externo limpo). Tabela de calibração:

| Posição | Leitura ADC (0–4095) | CO₂ estimado (ppm) |
|---|---|---|
| 0%   | 0    | 400  |
| 25%  | 1024 | 1550 |
| 50%  | 2048 | 2700 |
| 75%  | 3072 | 3850 |
| 100% | 4095 | 5000 |

**3.2. Regras de negócio explícitas.** Classificação do ambiente e acionamento:

| Variável | Bom | Moderado | Crítico |
|---|---|---|---|
| CO₂ (ppm) | < 1000 | 1000 – 2000 | > 2000 |

> **A ventilação (atuador) é acionada automaticamente quando o ambiente atinge o estado crítico, ou seja,
> CO₂ acima de 2000 ppm**, e é desligada quando o valor retorna a níveis adequados. Temperatura fora de
> 18–26 °C e umidade fora de 30–60% geram **alerta**, mas não acionam a ventilação.

**Entregável:** subseções "Calibração do sensor" e "Regras de negócio" no artigo, com as duas tabelas acima.

---

# Resumo das mudanças por componente

| Componente | O que muda |
|---|---|
| **Firmware Disp.2** | Passa a publicar o próprio estado em `mackenzie/oic/atuador/status` |
| **Fluxo Node-RED** | Remove gravação do atuador via comando; adiciona ramo `mqtt in` (status do Disp.2) → função → InfluxDB |
| **Banco (InfluxDB)** | *measurement* `atuador` passa a ser alimentado pela confirmação do Disp.2 (tag `fonte`) |
| **Dashboard (Grafana)** | Reorganizado em 2 seções rotuladas (Disp.1 sensores / Disp.2 atuador) + state timeline |
| **Artigo** | Justificativa dos 2 dispositivos + calibração (tabela) + regras explícitas |
| **Tópicos MQTT** | `mackenzie/oic/qualidadear` (sensor), `mackenzie/oic/comando` (comando), **novo:** `mackenzie/oic/atuador/status` |

# Cronograma
| Etapa | Atividade |
|---|---|
| 1 | Atualizar firmware do Disp.2 e o fluxo do Node-RED; validar gravação no InfluxDB |
| 2 | Reorganizar o dashboard no Grafana e capturar novos prints |
| 3 | Reescrever as seções do artigo (justificativa + calibração + regras) e recompilar o PDF |
| 4 | Regravar o trecho do vídeo afetado e atualizar o repositório público |
