# 🚦 Semáforo Inteligente IoT

Sistema completo de controle de semáforo utilizando ESP32, comunicação MQTT via HiveMQ e interface web responsiva.

## 📋 Componentes Necessários

### Hardware
- ESP32 (qualquer modelo)
- 3 LEDs (Vermelho, Amarelo, Verde)
- 3 Resistores de 220Ω
- 1 Botão push (opcional)
- 1 Resistor de 10kΩ (pull-up do botão)
- Protoboard e jumpers

### Software
- Arduino IDE
- Bibliotecas: WiFi, PubSubClient, ArduinoJson

## 🔧 Configuração do Hardware

### Conexões do ESP32:
```
LED Vermelho  → Pino 2  (+ Resistor 220Ω)
LED Amarelo   → Pino 4  (+ Resistor 220Ω)  
LED Verde     → Pino 5  (+ Resistor 220Ω)
Botão         → Pino 0  (+ Resistor 10kΩ pull-up)
GND           → GND comum
```

### Esquema de Ligação:
```
ESP32          Componente
Pino 2  ----[220Ω]---- LED Vermelho ---- GND
Pino 4  ----[220Ω]---- LED Amarelo  ---- GND
Pino 5  ----[220Ω]---- LED Verde    ---- GND
Pino 0  ----[Botão]---- GND
         |
      [10kΩ]
         |
       3.3V
```

## ⚙️ Configuração do Software

### 1. Arduino IDE
1. Instale as bibliotecas necessárias:
   - `PubSubClient` by Nick O'Leary
   - `ArduinoJson` by Benoit Blanchon
   
2. Configure a placa ESP32 no Arduino IDE

3. Edite o arquivo `esp32_semaforo.ino`:
   ```cpp
   const char* ssid = "SEU_WIFI_SSID";
   const char* password = "SUA_SENHA_WIFI";
   ```

4. Faça upload do código para o ESP32

### 2. Interface Web
1. Hospede os arquivos `index.html`, `script.js` e `style.css` no Render ou qualquer servidor web
2. Acesse a interface pelo navegador
3. A conexão MQTT será estabelecida automaticamente

## 🌐 Deploy no Render

### Passos para hospedar no Render:
1. Crie uma conta no [Render.com](https://render.com)
2. Conecte seu repositório GitHub
3. Crie um novo "Static Site"
4. Configure:
   - **Build Command**: (deixe vazio)
   - **Publish Directory**: (deixe vazio ou `.`)
5. Deploy automático será feito

### Arquivo de configuração (opcional):
Crie um arquivo `render.yaml`:
```yaml
services:
  - type: web
    name: semaforo-iot
    env: static
    buildCommand: ""
    staticPublishPath: .
```

## 📡 Comunicação MQTT

### Broker: HiveMQ Public
- **Host**: broker.hivemq.com
- **Porta**: 1883 (TCP) / 8884 (WebSocket SSL)
- **Tópicos**:
  - Comandos: `semaforo/comando`
  - Status: `semaforo/status`

### Formato das Mensagens

#### Comando (Web → ESP32):
```json
{
  "command": "red|yellow|green|automatic|off",
  "timestamp": "2025-07-23T10:30:00.000Z",
  "source": "web"
}
```

#### Status (ESP32 → Web):
```json
{
  "timestamp": 1690975800000,
  "source": "esp32",
  "state": "red",
  "status": {
    "red": true,
    "yellow": false,
    "green": false
  },
  "wifi_rssi": -45,
  "free_heap": 280000
}
```

## 🎮 Como Usar

### Interface Web
1. Acesse o site hospedado no Render
2. Verifique o status de conexão (indicador no canto superior direito)
3. Use os botões para controlar o semáforo:
   - **🟢 Verde**: Liga apenas o LED verde
   - **🟡 Amarelo**: Liga apenas o LED amarelo
   - **🔴 Vermelho**: Liga apenas o LED vermelho
   - **🔄 Automático**: Ciclo automático (Verde→Amarelo→Vermelho)
   - **⭕ Desligar**: Apaga todos os LEDs

### Controle Físico (ESP32)
- **Botão**: Cicla entre os modos sequencialmente
- **Monitor Serial**: Digite comandos diretos (`red`, `yellow`, `green`, `automatic`, `off`)

## 🔍 Monitoramento

### Via Serial Monitor:
```
Conectando ao WiFi: SeuWifi
WiFi conectado!
Endereço IP: 192.168.1.100
Conectado ao broker MQTT!
Inscrito no tópico: semaforo/comando
Estado: Verde
Status enviado: {"timestamp":12345,"source":"esp32"...}
```

### Via Interface Web:
- Status de conexão MQTT em tempo real
- Feedback visual imediato dos comandos
- Sincronização automática com o ESP32

## 🚨 Solução de Problemas

### ESP32 não conecta ao WiFi:
- Verifique SSID e senha
- Confirme se o WiFi é 2.4GHz
- Teste o sinal WiFi na localização

### Não conecta ao MQTT:
- Verifique conexão com internet
- Confirme se o HiveMQ está funcionando
- Teste com outro broker MQTT

### LEDs não acendem:
- Verifique as conexões dos pinos
- Confirme valores dos resistores
- Teste LEDs individualmente

### Interface web não funciona:
- Verifique se o navegador suporta WebSocket
- Confirme se a biblioteca Paho foi carregada
- Teste em navegador diferente

## 📊 Recursos Avançados

### Personalizações Possíveis:
1. **Timing do modo automático**: Altere `autoInterval` no código
2. **Novos comandos**: Adicione casos no `processCommand()`
3. **Sensores**: Integre sensores de movimento ou luz
4. **Múltiplos semáforos**: Use tópicos MQTT distintos
5. **Interface móvel**: PWA com manifest.json

### Expansões Futuras:
- Controle por voz (integração com Alexa/Google)
- Dashboard com histórico de estados
- Alertas por email/SMS
- Integração com sistema de tráfego real
- Modo pedestre com botão de requisição

## 📝 Licença

Este projeto é open source e está disponível sob a licença MIT.

## 🤝 Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para:
- Reportar bugs
- Sugerir melhorias
- Enviar pull requests
- Compartilhar suas modificações

## 📞 Suporte

Para dúvidas ou problemas:
1. Verifique a seção "Solução de Problemas"
2. Consulte a documentação das bibliotecas utilizadas
3. Teste com componentes isolados
4. Abra uma issue no repositório

---

**Desenvolvido com ❤️ para aprendizado IoT**
