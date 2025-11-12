/*
  Proyecto: Control de Cinta Transportadora para Factory I/O (Versión 2)
  Descripción: Simula un panel de control. La lógica de estado está aquí.
  Funciones:
  - Botón de Arranque (pin 2)
  - Botón de Parada Manual (pin 3)
  - Salida a Motor/Cinta (LED en pin 13)
  - Salida a Indicador de Parada Automática (LED en pin 12)
  
  Comunicación:
  - ENTRADA SERIAL: Recibe el estado del sensor desde Python (ej: "SENSOR:1")
  - SALIDA SERIAL: Envía el estado del motor y la luz a Python (ej: "MOTOR:1")
*/

// --- Pines de Entrada ---
#define START_BUTTON_PIN 2
#define STOP_BUTTON_PIN 3
// ¡El pin 4 (SENSOR_PIN) ya no se usa!

// --- Pines de Salida ---
#define MOTOR_PIN 13       // LED integrado, simula el motor de la cinta
#define AUTO_STOP_LED_PIN 12 // LED indicador de parada automática

// --- Estados del sistema ---
enum State {
  STOPPED,
  RUNNING,
  AUTO_STOPPED
};
State currentState = STOPPED;

// --- Variables de estado ---
bool sensorTriggered = false; // Estado del sensor (controlado por Serial)
String serialBuffer = "";     // Buffer para leer datos de Python

// --- Variables para detectar cambios ---
bool lastMotorState = false;
bool lastAutoStopState = false;

// --- Variables para Anti-rebote (Debounce) y temporización sin delay() ---
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // Tiempo de espera para el anti-rebote
bool lastStartButtonState = HIGH;
bool lastStopButtonState = HIGH;
unsigned long lastLoopTime = 0;

void setup() {
  Serial.begin(9600);
  
  // Configurar pines de entrada (solo botones)
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

  // Configurar pines de salida
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(AUTO_STOP_LED_PIN, OUTPUT);

  // Asegurar estado inicial
  digitalWrite(MOTOR_PIN, LOW);
  digitalWrite(AUTO_STOP_LED_PIN, LOW);
  
  serialBuffer.reserve(64); // Reservar espacio para el buffer de entrada
  Serial.println("Arduino listo. Esperando estado de sensor desde Python.");
}

void loop() {
  // --- 1. Revisar entrada serial de Python (Sensor) ---
  checkSerialInput();

  // --- 2. Leer entradas físicas (Botones) ---
  // Se implementa un anti-rebote (debounce) para evitar lecturas falsas.
  // Solo se considera una pulsación si el estado del botón ha cambiado y se ha mantenido estable.
  bool startPressed = false;
  bool stopPressed = false;

  if (millis() - lastDebounceTime > debounceDelay) {
    bool currentStartButtonState = digitalRead(START_BUTTON_PIN);
    if (currentStartButtonState == LOW && lastStartButtonState == HIGH) {
      startPressed = true;
      lastDebounceTime = millis();
    }
    lastStartButtonState = currentStartButtonState;

    bool currentStopButtonState = digitalRead(STOP_BUTTON_PIN);
    if (currentStopButtonState == LOW && lastStopButtonState == HIGH) {
      stopPressed = true;
      lastDebounceTime = millis();
    }
    lastStopButtonState = currentStopButtonState;
  }
  
  // --- 3. Lógica de la máquina de estados ---
  switch (currentState) {
    case STOPPED:
      if (startPressed) {
        currentState = RUNNING;
      }
      break;
      
    case RUNNING:
      if (stopPressed) {
        currentState = STOPPED;
      }
      // Se usa la variable 'sensorTriggered' que viene de Python
      else if (sensorTriggered) { 
        currentState = AUTO_STOPPED;
      }
      break;
      
    case AUTO_STOPPED:
      if (startPressed) {
        currentState = RUNNING;
      }
      else if (stopPressed) {
        currentState = STOPPED;
      }
      // Si el sensor deja de estar presionado (la caja se va)
      // NO hacemos nada. Se necesita 'start' para reiniciar.
      break;
  }

  // --- 4. Actualizar salidas físicas (LEDs) ---
  bool currentMotorState = (currentState == RUNNING);
  bool currentAutoStopState = (currentState == AUTO_STOPPED);

  digitalWrite(MOTOR_PIN, currentMotorState ? HIGH : LOW);
  digitalWrite(AUTO_STOP_LED_PIN, currentAutoStopState ? HIGH : LOW);

  // --- 5. Enviar estado a Python (Motor/Luz) ---
  if (currentMotorState != lastMotorState) {
    Serial.print("MOTOR:");
    Serial.println(currentMotorState);
    lastMotorState = currentMotorState;
  }
  
  if (currentAutoStopState != lastAutoStopState) {
    Serial.print("LIGHT:");
    Serial.println(currentAutoStopState);
    lastAutoStopState = currentAutoStopState;
  }
}

/**
 * @brief Revisa la entrada serial en busca de comandos de Python.
 * Actualiza la variable global 'sensorTriggered'.
 */
void checkSerialInput() {
  // Lee una línea completa del puerto serie para evitar comandos incompletos.
  // Serial.readStringUntil('\n') lee hasta que encuentra un salto de línea o se agota el tiempo.
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Elimina espacios en blanco o caracteres invisibles

    if (command.startsWith("SENSOR:")) {
      // Extrae el valor (0 o 1) después de "SENSOR:"
      int val = command.substring(7).toInt();
      sensorTriggered = (val == 1);
    }
  }
}