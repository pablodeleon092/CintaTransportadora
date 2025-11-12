import serial
from pyModbusTCP.client import ModbusClient
import time

# --- Configuración ---
ARDUINO_PORT = 'COM3'  # ¡¡IMPORTANTE: Cambia esto al puerto COM de tu Arduino!!
BAUD_RATE = 9600
FACTORY_IO_HOST = 'localhost' # O la IP donde corre Factory I/O
FACTORY_IO_PORT = 502

# --- Direcciones Modbus (Salidas a Factory I/O) ---
# Estas deben coincidir con tu configuración en el driver de Factory I/O
MOTOR_COIL = 0       # Coil 0 para la cinta transportadora
AUTO_STOP_LIGHT_COIL = 1 # Coil 1 para la luz indicadora

# --- Direcciones Modbus (Entradas desde Factory I/O) ---
# Se asume que el sensor está mapeado a la PRIMERA entrada discreta
SENSOR_INPUT = 0     # Discrete Input 0 para el sensor de final de línea

# --- Conexión al Arduino ---
try:
    ser = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=0.1) # Timeout más corto
    print(f"Conectado a Arduino en {ARDUINO_PORT}")
    time.sleep(2) # Esperar a que el Arduino se reinicie
    ser.flushInput() # Limpiar cualquier dato basura del inicio
except serial.SerialException as e:
    print(f"Error al conectar con Arduino en {ARDUINO_PORT}: {e}")
    print("Verifica el puerto COM y los permisos.")
    exit()

# --- Conexión a Factory I/O ---
client = ModbusClient(host=FACTORY_IO_HOST, port=FACTORY_IO_PORT, auto_open=True, debug=False)
print(f"Intentando conectar a Factory I/G en {FACTORY_IO_HOST}:{FACTORY_IO_PORT}...")

# --- Bucle principal (Puente Bi-direccional) ---
print("Iniciando puente bi-direccional. Presiona CTRL+C para salir.")

last_sensor_state = False # Variable para detectar cambios en el sensor

while True:
    try:
        # 1. Asegurarse de que el cliente Modbus esté conectado
        if not client.is_open:
            print("Cliente Modbus desconectado. Reconectando...")
            client.open()
            time.sleep(2)
            if not client.is_open:
                print("No se pudo reconectar. Verificando en el próximo ciclo.")
                time.sleep(1)
                continue

        # --- TAREA 1: LEER de Factory I/O (Sensor) y ESCRIBIR a Arduino ---
        
        # Leer el estado del sensor (Discrete Input 0)
        sensor_values = client.read_discrete_inputs(SENSOR_INPUT, 1)
        
        if sensor_values:
            current_sensor_state = sensor_values[0] # Es una lista [True] o [False]
            
            # Si el estado del sensor cambió, enviarlo al Arduino
            if current_sensor_state != last_sensor_state:
                command = f"SENSOR:{int(current_sensor_state)}\n"
                ser.write(command.encode('utf-8'))
                print(f"[Modbus] Leyendo SENSOR ({SENSOR_INPUT}) = {current_sensor_state} -> [Arduino] {command.strip()}")
                last_sensor_state = current_sensor_state
        else:
            print("Error al leer SENSOR_INPUT de Factory I/O")

        # --- TAREA 2: LEER de Arduino (Motor/Luz) y ESCRIBIR a Factory I/O ---
        
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            
            if not line:
                continue

            print(f"[Arduino] -> {line}")

            # Procesar la línea recibida
            if line.startswith("MOTOR:"):
                try:
                    val = bool(int(line.split(':')[1]))
                    print(f"[Modbus] Escribiendo MOTOR_COIL ({MOTOR_COIL}) = {val}")
                    client.write_single_coil(MOTOR_COIL, val)
                except Exception as e:
                    print(f"Error al procesar/escribir 'MOTOR': {e}")
            
            elif line.startswith("LIGHT:"):
                try:
                    val = bool(int(line.split(':')[1]))
                    print(f"[Modbus] Escribiendo AUTO_STOP_LIGHT_COIL ({AUTO_STOP_LIGHT_COIL}) = {val}")
                    client.write_single_coil(AUTO_STOP_LIGHT_COIL, val)
                except Exception as e:
                    print(f"Error al procesar/escribir 'LIGHT': {e}")

        # Pausa muy corta para el bucle principal
        time.sleep(0.01) 

    except KeyboardInterrupt:
        print("\nDeteniendo el script.")
        break
    except Exception as e:
        print(f"Error inesperado en el bucle principal: {e}")
        time.sleep(5)

# --- Limpieza ---
ser.close()
client.close()
print("Conexiones cerradas.")