// Configuração do MQTT (HiveMQ WebSocket)
const MQTT_BROKER_HOST = 'broker.hivemq.com';
const MQTT_BROKER_PORT = 8884;
const MQTT_TOPIC_COMMAND = 'senai928/semaforo/comando';
const MQTT_TOPIC_STATUS = 'senai928/semaforo/status';

let client;
let isConnected = false;

// Estados das luzes
const lights = {
    red: document.querySelector('.red'),
    yellow: document.querySelector('.yellow'),
    green: document.querySelector('.green')
};

// Botões de controle
const buttons = {
    green: document.querySelector('.buttonGreen'),
    yellow: document.querySelector('.buttonYellow'),
    red: document.querySelector('.buttonRed'),
    automatic: document.querySelector('.buttonAutomatic'),
    off: document.querySelector('.buttonOff')
};

// Status de conexão
let connectionStatus;

// Inicializar aplicação
document.addEventListener('DOMContentLoaded', function() {
    createStatusIndicator();
    setupEventListeners();
    
    // Aguardar um pouco para garantir que a biblioteca Paho carregue
    setTimeout(() => {
        if (typeof Paho !== 'undefined' && Paho.MQTT) {
            initializeMQTT();
        } else if (typeof Messaging !== 'undefined') {
            initializeMQTT();
        } else {
            console.error('Biblioteca Paho MQTT não encontrada');
            updateConnectionStatus(false, 'Biblioteca MQTT não carregada');
            // Ainda permitir uso local sem MQTT
        }
    }, 1000);
});

// Criar indicador de status de conexão
function createStatusIndicator() {
    connectionStatus = document.createElement('div');
    connectionStatus.className = 'connection-status disconnected';
    connectionStatus.innerHTML = '● Conectando...';
    document.body.insertBefore(connectionStatus, document.body.firstChild);
}

// Configurar MQTT
function initializeMQTT() {
    try {
        // Criar cliente MQTT com ID único
        const clientId = `webClient_${Math.random().toString(36).substr(2, 9)}`;
        
        // Verificar se Paho está disponível
        if (typeof Paho !== 'undefined' && Paho.MQTT && Paho.MQTT.Client) {
            client = new Paho.MQTT.Client(MQTT_BROKER_HOST, MQTT_BROKER_PORT, clientId);
        } else if (typeof Messaging !== 'undefined') {
            // Para versões mais antigas da biblioteca
            client = new Messaging.Client(MQTT_BROKER_HOST, MQTT_BROKER_PORT, clientId);
        } else {
            throw new Error('Biblioteca MQTT não encontrada');
        }
        
        // Configurar callbacks
        client.onConnectionLost = onConnectionLost;
        client.onMessageArrived = onMessageArrived;
        
        // Opções de conexão
        const options = {
            onSuccess: onConnect,
            onFailure: onFailure,
            useSSL: true,
            cleanSession: true,
            timeout: 30,
            keepAliveInterval: 60
        };
        
        updateConnectionStatus(false, 'Conectando...');
        client.connect(options);
        
    } catch (error) {
        console.error('Erro ao inicializar MQTT:', error);
        updateConnectionStatus(false, 'Erro na conexão');
    }
}

// Callback quando conectado
function onConnect() {
    console.log('Conectado ao broker MQTT');
    isConnected = true;
    updateConnectionStatus(true);
    
    // Inscrever-se no tópico de status
    client.subscribe(MQTT_TOPIC_STATUS);
    console.log(`Inscrito no tópico: ${MQTT_TOPIC_STATUS}`);
}

// Callback quando falha a conexão
function onFailure(error) {
    console.error('Falha na conexão MQTT:', error);
    isConnected = false;
    updateConnectionStatus(false, 'Falha na conexão');
    
    // Tentar reconectar após 5 segundos
    setTimeout(() => {
        if (!isConnected) {
            console.log('Tentando reconectar...');
            initializeMQTT();
        }
    }, 5000);
}

// Callback quando perde conexão
function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
        console.log('Conexão perdida:', responseObject.errorMessage);
        isConnected = false;
        updateConnectionStatus(false);
    }
}

// Callback quando recebe mensagem
function onMessageArrived(message) {
    console.log('Mensagem recebida:', message.payloadString);
    
    try {
        const data = JSON.parse(message.payloadString);
        updateSemaphoreDisplay(data);
    } catch (error) {
        console.error('Erro ao processar mensagem:', error);
    }
}

// Atualizar status de conexão
function updateConnectionStatus(connected, customMessage = null) {
    if (connectionStatus) {
        if (connected) {
            connectionStatus.className = 'connection-status connected';
            connectionStatus.innerHTML = '● Conectado ao MQTT';
        } else {
            connectionStatus.className = 'connection-status disconnected';
            connectionStatus.innerHTML = customMessage || '● Desconectado';
        }
    }
}

// Configurar event listeners dos botões
function setupEventListeners() {
    buttons.red.addEventListener('click', () => sendCommand('red'));
    buttons.yellow.addEventListener('click', () => sendCommand('yellow'));
    buttons.green.addEventListener('click', () => sendCommand('green'));
    buttons.automatic.addEventListener('click', () => sendCommand('automatic'));
    buttons.off.addEventListener('click', () => sendCommand('off'));
}

// Enviar comando via MQTT
function sendCommand(command) {
    if (!isConnected) {
        console.warn('Não conectado ao MQTT, apenas atualizando display local');
        updateLocalDisplay(command);
        return;
    }
    
    const message = {
        command: command,
        timestamp: new Date().toISOString(),
        source: 'web'
    };
    
    try {
        let mqttMessage;
        if (typeof Paho !== 'undefined' && Paho.MQTT && Paho.MQTT.Message) {
            mqttMessage = new Paho.MQTT.Message(JSON.stringify(message));
        } else if (typeof Messaging !== 'undefined') {
            mqttMessage = new Messaging.Message(JSON.stringify(message));
        } else {
            throw new Error('Biblioteca MQTT não encontrada para enviar mensagem');
        }
        
        mqttMessage.destinationName = MQTT_TOPIC_COMMAND;
        mqttMessage.qos = 1; // Garantir entrega
        client.send(mqttMessage);
        
        console.log('Comando enviado:', command);
        
        // Atualizar display local imediatamente para feedback visual
        updateLocalDisplay(command);
        
    } catch (error) {
        console.error('Erro ao enviar comando:', error);
        // Ainda atualizar o display local
        updateLocalDisplay(command);
    }
}

// Atualizar display local
function updateLocalDisplay(command) {
    // Resetar todas as luzes
    Object.values(lights).forEach(light => {
        light.style.opacity = '0.3';
    });
    
    // Acender a luz correspondente
    switch (command) {
        case 'red':
            lights.red.style.opacity = '1';
            break;
        case 'yellow':
            lights.yellow.style.opacity = '1';
            break;
        case 'green':
            lights.green.style.opacity = '1';
            break;
        case 'off':
            // Todas as luzes ficam apagadas (opacidade 0.3)
            break;
        case 'automatic':
            // Modo automático será controlado pelo ESP32
            break;
    }
}

// Atualizar display do semáforo baseado no status recebido
function updateSemaphoreDisplay(data) {
    if (data.status) {
        // Resetar todas as luzes
        Object.values(lights).forEach(light => {
            light.style.opacity = '0.3';
        });
        
        // Acender luzes baseadas no status
        if (data.status.red) lights.red.style.opacity = '1';
        if (data.status.yellow) lights.yellow.style.opacity = '1';
        if (data.status.green) lights.green.style.opacity = '1';
        
        console.log('Display atualizado com status do ESP32');
    }
}