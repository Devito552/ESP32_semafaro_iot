# 🚀 Guia Rápido de Teste

## Como testar a aplicação localmente:

### 1. Teste da Interface Web
1. Abra o arquivo `index.html` no navegador
2. Verifique se o indicador de conexão aparece no canto superior direito
3. Teste os botões do semáforo - as luzes devem acender/apagar visualmente
4. Abra o Console do navegador (F12) para verificar logs

### 2. Deploy no Render
1. Faça commit de todos os arquivos no GitHub
2. Vá para [render.com](https://render.com)
3. Conecte o repositório GitHub
4. Crie um "Static Site" 
5. Configure:
   - Build Command: (deixe vazio)
   - Publish Directory: (deixe vazio)
6. Aguarde o deploy

### 3. Configurar ESP32
1. Instale as bibliotecas no Arduino IDE:
   - PubSubClient
   - ArduinoJson
2. Edite o arquivo `esp32_semaforo.ino`:
   ```cpp
   const char* ssid = "SEU_WIFI";
   const char* password = "SUA_SENHA";
   ```
3. Conecte os LEDs nos pinos 2, 4, 5
4. Faça upload do código

### 4. Testar Comunicação MQTT
1. Abra o Monitor Serial do Arduino IDE
2. Acesse a interface web
3. Clique nos botões - deve ver mensagens no Serial
4. As luzes físicas devem acender conforme comandos

### 5. Solução de Problemas
- Se MQTT não conectar: verifique conexão com internet
- Se LEDs não acendem: verifique as conexões
- Se interface não carrega: teste em navegador diferente
- Se ESP32 não conecta WiFi: verifique SSID e senha

### URLs de Teste
- Local: `file:///caminho/para/index.html`
- Render: `https://seu-app.onrender.com`
- Broker MQTT: `broker.hivemq.com:8884` (WebSocket SSL)
