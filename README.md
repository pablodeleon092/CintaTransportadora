# Guía del Proyecto: Control de Cinta Transportadora con Arduino y Factory I/O

¡Bienvenido a este proyecto que conecta el mundo físico (hardware) con el mundo industrial virtual!

El objetivo es construir un panel de control real, usando un Arduino y botones, para comandar una cinta transportadora simulada en el software Factory I/O. Usaremos un script de Python como "puente" o "traductor" para que el Arduino y Factory I/O puedan hablar entre sí.

## Materiales Necesarios

### Hardware
*   **1x Placa Arduino** .
*   **1x Cable USB** para conectar el Arduino a la PC.
*   **1x Protoboard** .
*   **1x Sensor distancia** (para "Arranque" y "Parada").
*   **1x LED** (de cualquier color, para la "Luz de Parada Automática").
*   **Cables Jumper** para hacer las conexiones.

### Software
*   **Factory I/O**: El software de simulación industrial.
*   **IDE de Arduino**: Para programar nuestra placa Arduino (descarga gratuita desde `arduino.cc`).
*   **Python 3**: El lenguaje que usaremos como puente (descarga gratuita desde `python.org`).
*   **Bibliotecas de Python**: `pyserial` y `pyModbusTCP`.

---

## Paso 1: Montaje del Circuito

Vamos a montar nuestro panel de control.

> **Nota:** Realizar el codigo necesario para medir la distancia y determinar la presencia de cajas para que el sistema de control pueda accionar la cinta transportadora.


---

##  Paso 2: Configuración del Software


### 2.1. IDE de Arduino

1.  **Abrir el Sketch**: Abrir el archivo .INO que contenga el codigo necesario para que el sistema de control funcione.
2.  **Conectar Arduino**: Conecta tu placa Arduino a la PC con el cable USB.
3.  **Configurar Placa y Puerto**:
    *   Ve a `Herramientas > Placa` y selecciona tu modelo (ej. "Arduino Uno").
    *   Ve a `Herramientas > Puerto` y selecciona el puerto donde aparece tu Arduino (ej. `COM3`, `COM5`, etc.). **¡Anota este puerto, lo necesitarás!**
4.  **Subir el Código**: Haz clic en el botón **Subir** (la flecha `->`) para cargar el programa en el Arduino.

### 2.2. Python y sus Bibliotecas

1.  **Instalar Bibliotecas**: Abre una terminal (CMD, PowerShell, etc.) y ejecuta este comando para instalar las bibliotecas necesarias:
    ```bash
    pip install pyserial pyModbusTCP
    ```
2.  **Configurar el Puerto COM**: Abre el archivo `conexion_arduino_factory.py` con un editor de texto.

    >  **¡MUY IMPORTANTE!** Modifica la línea 5 para que coincida con el puerto COM de tu Arduino que anotaste antes.

    ```python
    # ¡¡IMPORTANTE: Cambia esto al puerto COM de tu Arduino!!
    ARDUINO_PORT = 'COM3'
    ```

### 2.3. Factory I/O

1.  **Cargar Escena**: Abrir Factory I/O y carga tu escena con la cinta transportadora, el sensor y la luz indicadora.
2.  **Configurar Driver**:
    *   Ve a `Archivo > Drivers...` (o presiona `F4`).
    *   En la lista de la izquierda, selecciona **Modbus TCP/IP Server**.
    *   Haz clic en **CONFIGURACIÓN**.
3.  **Mapear Entradas y Salidas**:
    *   **Salidas (Coils)**:
        *   Arrastra el actuador de la **Cinta Transportadora** a la casilla `Coil 0`.
        *   Arrastra la **Luz Indicadora** a la casilla `Coil 1`.
    *   **Entradas (Discrete Inputs)**:
        *   Arrastra el **Sensor** de final de línea a la casilla `DI 0`.
4.  **Conectar**: Cierra la ventana de configuración y haz clic en **CONECTAR**. Debería aparecer un ícono de enlace verde.

---

##  Paso 3: ¡Puesta en Marcha!

Sigue esta secuencia para que todo funcione:

1.  **Verificar Conexiones**: Asegúrarse de que tu Arduino esté conectado al PC y el circuito esté bien montado.
2.  **Ejecutar el Puente Python**: Abrir una terminal, navega a la carpeta del proyecto y ejecuta el script:
    ```bash
    python conexion_arduino_factory.py
    ```
    > visualizaras mensajes como "Conectado a Arduino..." e "Intentando conectar a Factory I/O...". **¡No cierres esta ventana!**
3.  **Iniciar Simulación**: En Factory I/O, pon la simulación en modo **Play** (el botón de triángulo).

**¡Listo!** Ahora podes usar tu codigo en arduino para controlar la cinta transportadora en Factory I/O.

---

## ¿Cómo Funciona? (Explicación)

El script de Python actúa como un cerebro central que traduce los mensajes.

### Flujo 1: Cuando presionas el botón de ARRANQUE

1.  **Arduino**: Detecta la pulsación del botón en el `Pin 2` y cambia su estado interno a `RUNNING`.
2.  **Arduino → Python**: Envía el mensaje `"MOTOR:1"` por el puerto serie (USB).
3.  **Python**: Recibe `"MOTOR:1"` y lo traduce a un comando Modbus.
4.  **Python → Factory I/O**: Envía el comando Modbus para activar la `Coil 0`.
5.  **Factory I/O**: Recibe el comando y enciende el motor de la cinta transportadora.

### Flujo 2: Cuando el sensor de Factory I/O se activa

1.  **Factory I/O**: El sensor virtual detecta un objeto, activando el `DI 0`.
2.  **Factory I/O → Python**: El estado del `DI 0` cambia a `True` y es leído por Python vía Modbus.
3.  **Python → Arduino**: Al detectar el cambio, Python envía el mensaje `"SENSOR:1"` al Arduino.
4.  **Arduino**: Recibe `"SENSOR:1"`, cambia su estado a `AUTO_STOPPED`, apaga el LED del motor (`Pin 13`) y enciende el LED de parada (`Pin 12`).
5.  **Arduino → Python**: Envía los mensajes `"MOTOR:0"` y `"LIGHT:1"` a Python.
6.  **Python → Factory I/O**: Python traduce estos mensajes y envía los comandos Modbus para desactivar la `Coil 0` (motor) y activar la `Coil 1` (luz indicadora).

---

## Solución de Problemas Comunes

*   **Error "Access is denied" o "PermissionError" al ejecutar el script de Python:**
    *   **Causa**: Otro programa (como el Monitor Serie del IDE de Arduino) está usando el puerto COM.
    *   **Solución**: Cierra el Monitor Serie y cualquier otro software que pueda estar conectado al Arduino.

*   **El script de Python dice "Error al conectar con Arduino":**
    *   **Causa**: El puerto COM en `conexion_arduino_factory.py` es incorrecto.
    *   **Solución**: Verifica el puerto correcto en el IDE de Arduino (`Herramientas > Puerto`) y asegúrate de que coincida exactamente en el script.

*   **Los botones no responden o actúan de forma extraña:**
    *   **Causa**: Mal cableado.
    *   **Solución**: Asegúrate de que los botones estén conectados a los pines correctos (2 y 3) y a `GND`. Revisa que los cables hagan buen contacto en la protoboard.

*   **La cinta en Factory I/O no se mueve:**
    *   **Causa 1**: El driver Modbus en Factory I/O no está conectado (no tiene el ícono verde).
    *   **Solución 1**: Ve a `Archivo > Drivers` y haz clic en **CONECTAR**.
    *   **Causa 2**: El script de Python no se está ejecutando.
    *   **Solución 2**: Asegúrate de que la terminal donde ejecutas `conexion_arduino_factory.py` siga abierta y sin errores.