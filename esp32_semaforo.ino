#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Configurações WiFi
const char* ssid = "SEU_WIFI_SSID";
const char* password = "SUA_SENHA_WIFI";

// Configurações MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic_command = "senai928/semaforo/comando";
const char* mqtt_topic_status = "senai928/semaforo/status";

// Pinos dos LEDs do semáforo
const int LED_RED = 2;
const int LED_YELLOW = 4;
const int LED_GREEN = 5;

// Pino do botão físico (opcional)
const int BUTTON_PIN = 0;

// Estados do semáforo
enum SemaphoreState {
  STATE_OFF,
  STATE_RED,
  STATE_YELLOW,
  STATE_GREEN,
  STATE_AUTOMATIC
};

// Variáveis globais
WiFiClient espClient;
PubSubClient client(espClient);
SemaphoreState currentState = STATE_OFF;
SemaphoreState previousState = STATE_OFF;
unsigned long lastAutoChange = 0;
unsigned long autoInterval = 3000; // 3 segundos para cada cor no modo automático
int autoStep = 0; // 0=Verde, 1=Amarelo, 2=Vermelho
bool buttonPressed = false;
unsigned long lastButtonPress = 0;

void setup() {
  Serial.begin(115200);
  
  // Configurar pinos
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Inicializar com todas as luzes apagadas
  updateLights();
  
  // Conectar WiFi
  setupWiFi();
  
  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Serial.println("Sistema inicializado!");
  Serial.println("Comandos disponíveis:");
  Serial.println("- red: Luz vermelha");
  Serial.println("- yellow: Luz amarela");
  Serial.println("- green: Luz verde");
  Serial.println("- automatic: Modo automático");
  Serial.println("- off: Desligar");
}

void loop() {
  // Manter conexão MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  // Verificar botão físico
  checkButton();
  
  // Processar modo automático
  if (currentState == STATE_AUTOMATIC) {
    handleAutomaticMode();
  }
  
  // Atualizar luzes se o estado mudou
  if (currentState != previousState) {
    updateLights();
    sendStatusUpdate();
    previousState = currentState;
  }
  
  delay(50);
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando ao WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado ao broker MQTT!");
      
      // Inscrever-se no tópico de comandos
      client.subscribe(mqtt_topic_command);
      Serial.print("Inscrito no tópico: ");
      Serial.println(mqtt_topic_command);
      
      // Enviar status inicial
      sendStatusUpdate();
      
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Parse JSON
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, message);
  
  String command = doc["command"];
  processCommand(command);
}

void processCommand(String command) {
  Serial.print("Processando comando: ");
  Serial.println(command);
  
  if (command == "red") {
    currentState = STATE_RED;
  } else if (command == "yellow") {
    currentState = STATE_YELLOW;
  } else if (command == "green") {
    currentState = STATE_GREEN;
  } else if (command == "automatic") {
    currentState = STATE_AUTOMATIC;
    autoStep = 0;
    lastAutoChange = millis();
  } else if (command == "off") {
    currentState = STATE_OFF;
  } else {
    Serial.println("Comando não reconhecido!");
  }
}

void handleAutomaticMode() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastAutoChange >= autoInterval) {
    lastAutoChange = currentTime;
    
    switch (autoStep) {
      case 0: // Verde
        setLights(false, false, true);
        autoStep = 1;
        Serial.println("Automático: Verde");
        break;
      case 1: // Amarelo
        setLights(false, true, false);
        autoStep = 2;
        autoInterval = 1500; // Amarelo fica menos tempo
        Serial.println("Automático: Amarelo");
        break;
      case 2: // Vermelho
        setLights(true, false, false);
        autoStep = 0;
        autoInterval = 3000; // Volta ao tempo normal
        Serial.println("Automático: Vermelho");
        break;
    }
    sendStatusUpdate();
  }
}

void updateLights() {
  switch (currentState) {
    case STATE_OFF:
      setLights(false, false, false);
      Serial.println("Estado: Desligado");
      break;
    case STATE_RED:
      setLights(true, false, false);
      Serial.println("Estado: Vermelho");
      break;
    case STATE_YELLOW:
      setLights(false, true, false);
      Serial.println("Estado: Amarelo");
      break;
    case STATE_GREEN:
      setLights(false, false, true);
      Serial.println("Estado: Verde");
      break;
    case STATE_AUTOMATIC:
      Serial.println("Estado: Automático");
      // As luzes são controladas pela função handleAutomaticMode()
      break;
  }
}

void setLights(bool red, bool yellow, bool green) {
  digitalWrite(LED_RED, red ? HIGH : LOW);
  digitalWrite(LED_YELLOW, yellow ? HIGH : LOW);
  digitalWrite(LED_GREEN, green ? HIGH : LOW);
}

void sendStatusUpdate() {
  if (client.connected()) {
    DynamicJsonDocument doc(1024);
    
    doc["timestamp"] = millis();
    doc["source"] = "esp32";
    doc["state"] = getStateString(currentState);
    
    // Status das luzes
    JsonObject status = doc.createNestedObject("status");
    status["red"] = digitalRead(LED_RED);
    status["yellow"] = digitalRead(LED_YELLOW);
    status["green"] = digitalRead(LED_GREEN);
    
    // Informações adicionais
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["free_heap"] = ESP.getFreeHeap();
    
    String payload;
    serializeJson(doc, payload);
    
    client.publish(mqtt_topic_status, payload.c_str());
    Serial.print("Status enviado: ");
    Serial.println(payload);
  }
}

String getStateString(SemaphoreState state) {
  switch (state) {
    case STATE_OFF: return "off";
    case STATE_RED: return "red";
    case STATE_YELLOW: return "yellow";
    case STATE_GREEN: return "green";
    case STATE_AUTOMATIC: return "automatic";
    default: return "unknown";
  }
}

void checkButton() {
  bool buttonState = digitalRead(BUTTON_PIN) == LOW;
  unsigned long currentTime = millis();
  
  if (buttonState && !buttonPressed && (currentTime - lastButtonPress > 500)) {
    buttonPressed = true;
    lastButtonPress = currentTime;
    
    // Ciclar entre os estados com o botão físico
    switch (currentState) {
      case STATE_OFF:
        currentState = STATE_GREEN;
        break;
      case STATE_GREEN:
        currentState = STATE_YELLOW;
        break;
      case STATE_YELLOW:
        currentState = STATE_RED;
        break;
      case STATE_RED:
        currentState = STATE_AUTOMATIC;
        break;
      case STATE_AUTOMATIC:
        currentState = STATE_OFF;
        break;
    }
    
    Serial.println("Botão pressionado - Estado alterado");
  } else if (!buttonState) {
    buttonPressed = false;
  }
}

// Função para debug via Serial
void serialEvent() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
}
