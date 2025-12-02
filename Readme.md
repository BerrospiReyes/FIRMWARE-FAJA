# 🤖 Firmware ESP32 - Faja Transportadora IoT

Código fuente en C++ (Arduino Framework) para el microcontrolador **ESP32 DevKit V1**. Este firmware es el cerebro del sistema, encargado de la lectura de sensores, control de actuadores y comunicación con la nube.

## 🧠 Funcionalidades
* **Lectura Analógica:** Monitoreo continuo de 3 sensores LDR.
* **Algoritmo de Clasificación:** Lógica local para determinar tamaño de objetos (Pequeño/Mediano/Grande).
* **Actuación:** Control de motor DC mediante Driver L298N (PWM).
* **Conectividad:** Cliente HTTP que reporta telemetría y recibe comandos (Polling).

## 🔌 Diagrama de Conexiones (Pinout)

| Componente | Pin ESP32 | Función |
| :--- | :--- | :--- |
| **Driver L298N (ENA)** | GPIO 26 | Control de Velocidad (PWM) |
| **Driver L298N (IN1)** | GPIO 27 | Dirección A |
| **Driver L298N (IN2)** | GPIO 14 | Dirección B |
| **Sensor LDR 1** | GPIO 34 | Entrada Analógica (S1) |
| **Sensor LDR 2** | GPIO 35 | Entrada Analógica (S2) |
| **Sensor LDR 3** | GPIO 32 | Entrada Analógica (S3) |
| **Láseres** | VIN (5V) | Alimentación Constante |

## 🛠️ Requisitos e Instalación

### Hardware Necesario
* ESP32 DevKit V1
* Driver Puente H L298N
* 3x Módulos Sensor LDR (Fotosensibles)
* 3x Diodos Láser 5V
* Fuente de alimentación externa (Batería 9V/12V)

### Librerías Requeridas
Asegúrate de instalar estas librerías en tu Arduino IDE o PlatformIO:
1.  **ArduinoJson** (v6.x o superior) - Para serializar datos.
2.  **WiFi.h** - Nativa del ESP32.
3.  **HTTPClient.h** - Nativa del ESP32.

### Configuración
1.  Abrir el archivo `.ino`.
2.  Modificar las credenciales de red:
    ```cpp
    const char* ssid = "TU_WIFI";
    const char* password = "TU_CONTRASEÑA";
    ```
3.  Verificar la URL del Backend:
    ```cpp
    String serverUrl = "[https://tu-backend.onrender.com](https://tu-backend.onrender.com)";
    ```
4.  Seleccionar la placa **"DOIT ESP32 DEVKIT V1"** y subir el código.

## 📊 Flujo de Datos
1.  **Input:** Sensores detectan interrupción de luz láser.
2.  **Proceso:** ESP32 calcula el tamaño y actualiza contadores.
3.  **Output (Nube):** Envía JSON `{ "pequenas": 1, ... }`.
4.  **Output (Físico):** Recibe orden `{ "motorCommand": true }` y activa GPIO 26.
