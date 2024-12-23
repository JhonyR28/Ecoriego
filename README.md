# ECORIEGO: Sistema de Riego Inteligente
ECORIEGO es un sistema automatizado para optimizar el riego agrícola, diseñado para maximizar la eficiencia en el uso del agua. Este sistema utiliza sensores avanzados, conectividad GPRS y algoritmos dinámicos para gestionar el riego de manera autónoma.

## Características
* Monitoreo de humedad del suelo: Mide la humedad con precisión para evitar riegos innecesarios.
* Sensores integrados:
  - DHT22: Temperatura y humedad ambiental.
  - DS18B20: Temperatura del suelo.
* Relay inteligente: Activa o desactiva la bomba de riego según los niveles de humedad.
* Conectividad GPRS: Envía datos a la nube mediante ThingSpeak.
* Ahorro energético: Implementa modo de sueño profundo para optimizar el consumo de energía.
* Control dinámico: Ajusta los tiempos de riego según las condiciones del suelo.
![image](https://github.com/user-attachments/assets/aa696fe7-33be-496a-bae1-0ef5ec22389f)

## Uso
1. Configuración del hardware:
* Conecta los sensores y el relay al ESP32.
* Configura las credenciales GPRS en secrets.h.
2. Instalación del software:
* Instala las dependencias requeridas.
* Carga el código al ESP32.
3. Operación:
* El sistema mide datos ambientales y de suelo en tiempo real.
* Envía los datos a ThingSpeak para monitoreo remoto.
* Activa o desactiva el riego según los umbrales definidos.
![image](https://github.com/user-attachments/assets/151d951d-3860-4cd7-86bd-c6dbfac1c189)
## Requisitos
* Hardware:
  - ESP32 TTGO T-Call.
  - Sensores DHT22, DS18B20, y de humedad del suelo.
  - Relay.
  - Bomba de agua 12V.
* Software:
  - Biblioteca ThingSpeak.
  - Biblioteca DHT.
  - Biblioteca DallasTemperature.
  - TinyGSM para conectividad GPRS.

![WhatsApp Image 2024-10-03 at 7 12 11 PM](https://github.com/user-attachments/assets/cd29c239-ddc3-45c2-91ba-5a2561e5c6ac)
