# Setup InfluxDB Cloud + Grafana Cloud (banco em nuvem + dashboard externo)

> **Por que isso é obrigatório:** o descritivo exige banco **em nuvem do tipo Time Series (InfluxDB)** e dashboard em
> **ferramenta externa (Grafana)** — *dashboard nativo do Node-RED é proibido*. Usando os dois na nuvem, o broker,
> o Node-RED, o banco e o dashboard ficam em **máquinas diferentes**, o que também é exigido.

⚠️ **NÃO comite o token real no GitHub.** No `flows.json` que vai pro repositório, deixe o token como
`SEU_TOKEN_INFLUXDB`. O spec penaliza expor chaves/tokens.

---

## Parte 1 — InfluxDB Cloud (banco Time Series, grátis)

1. Acesse **https://cloud2.influxdata.com/signup** e crie a conta (plano **Free**). Escolha um provedor/região (ex.: AWS / us-east-1).
2. No menu **Load Data → Buckets → Create Bucket**: nome **`qualidade_ar`**.
3. Anote o nome da sua **Organization** (geralmente é o seu e-mail).
4. Em **Load Data → API Tokens → Generate API Token → All Access Token** (ou Read/Write no bucket). **Copie e guarde** o token.
5. Anote a **URL da sua região** (aparece na barra do navegador), ex.: `https://us-east-1-1.aws.cloud2.influxdata.com`.

**Dados que você terá ao final desta parte:** URL, Organization, Bucket (`qualidade_ar`) e Token.

### Configurar o Node-RED para gravar no InfluxDB
1. Pare o Node-RED, e em `~/.node-red` instale o conector:
   ```
   cd ~/.node-red
   npm install node-red-contrib-influxdb
   ```
   Reinicie o Node-RED.
2. (Eu já deixo o nó **influxdb out** no fluxo.) Você só vai abrir o nó de configuração e preencher:
   - **Version:** 2.0 (Flux)
   - **URL:** a URL da sua região
   - **Token:** seu token
   - **Organization:** sua org
   - **Bucket:** `qualidade_ar`
   - **Measurement:** `leituras`

---

## Parte 2 — Grafana Cloud (dashboard externo, grátis)

> ⚠️ Este InfluxDB Cloud é a versão **Serverless (v3)**, que consulta em **SQL** (não Flux).
> No Grafana, conecte usando **Query Language = SQL**.

1. Acesse **https://grafana.com/auth/sign-up/create-user** e crie a conta **Free**
   (instância `https://SEU_USUARIO.grafana.net`).
2. Menu **Connections → Add new connection → InfluxDB → Add new data source**:
   - **Query language:** **SQL**
   - **URL:** `https://us-east-1-1.aws.cloud2.influxdata.com`
   - Em **InfluxDB Details**: **Database** = `qualidade_ar` · **Token** = seu token
   - **Save & Test** (deve dar "datasource is working").
3. Menu **Dashboards → New → New dashboard → Add visualization** (crie **pelo menos 2 painéis**):

   **Painel 1 — CO₂ ao longo do tempo (Time series):**
   ```sql
   SELECT time, co2 FROM leituras WHERE $__timeFilter(time) ORDER BY time
   ```
   **Painel 2 — Temperatura e Umidade (Time series):**
   ```sql
   SELECT time, temperatura, umidade FROM leituras WHERE $__timeFilter(time) ORDER BY time
   ```
   **Painel 3 (opcional) — Última leitura / status (Table ou Stat):**
   ```sql
   SELECT time, co2, status FROM leituras WHERE $__timeFilter(time) ORDER BY time DESC LIMIT 1
   ```
   > Se o macro `$__timeFilter(time)` der erro, troque por `time >= now() - interval '1 hour'`.

4. **Save dashboard** (nome: "Qualidade do Ar - OIC"). Tire o **print** para o artigo (`artigo/dashboard.png`).

---

## Resumo do que me mandar quando terminar a Parte 1
Para eu fechar o `flows.json` final, me passe (ou só confirme que configurou no nó):
- URL do InfluxDB · Organization · Bucket (`qualidade_ar`)
- O token **NÃO precisa me mandar** — você cola direto no nó do Node-RED.
