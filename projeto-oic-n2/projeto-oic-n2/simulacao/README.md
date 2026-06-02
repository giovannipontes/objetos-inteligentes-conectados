# Simulação no Wokwi

Os arquivos de simulação são os mesmos da pasta `codigo-fonte/`:

- `sketch.ino` — firmware do ESP32
- `diagram.json` — circuito (ESP32 + DHT22 + potenciômetro simulando o MQ-135)
- `libraries.txt` — bibliotecas necessárias

## Passos
1. Acesse https://wokwi.com → **New Project** → **ESP32**.
2. Copie `sketch.ino` e `diagram.json` para o projeto.
3. Clique em **Start the simulation**.
4. Gire o potenciômetro para variar a leitura de CO₂ e observe as publicações MQTT no Serial Monitor.

> Link do projeto no Wokwi: _adicionar aqui após publicar_
