#include "secrets.h"
#include "ThingSpeak.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "esp_sleep.h"

#define TINY_GSM_MODEM_SIM800 

#define SerialMon Serial
#define SerialAT Serial1

// Tus credenciales GPRS
const char apn[] = "TM"; 
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <Wire.h>
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

long lastMsg = 0;

TinyGsmClient client(modem);

// DHT Sensor
#define DHTPIN 32     
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

// Soil moisture sensor
#define AOUT_PIN 12

const int valAir = 2864;  

const int valWater = 965;

int toPercent(int val) {
  int percent = map(val, valAir, valWater, 0, 100);
  return constrain(percent, 0, 100);
}

// DS18B20 Temperature sensor
const int oneWireBus = 4;     

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// Relay control
#define RELAY_PIN 15  // Pin del relay
#define THRESHOLD 40  // Umbral de humedad en porcentaje (ajustable)

// TTGO T-Call modem configuration
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

unsigned long lastRelayCheck = 0;      // Para controlar el relay
unsigned long lastThingSpeakUpdate = 0; // Para actualizar ThingSpeak
const unsigned long relayInterval = 5000;        // 5 segundos para el control del relay
const unsigned long thingSpeakInterval = 60000;  // 60 segundos para actualizar ThingSpeak
unsigned long activePeriod = 300000;  // 5 minutos activos (en milisegundos) - Inicialmente estático
const unsigned long sleepDuration = 420 * 1000000ULL; // 7 minutos de sueño (en microsegundos) -> para pasarlo a 30 minutos -> 1800

unsigned long startActiveTime;

void setup() {
  Serial.begin(9600);  
  dht.begin();
  sensors.begin();
  
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  
  SerialMon.println("Inicializando el modem...");
  SerialAT.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(6000);
  
  if (!modem.restart()) {
    SerialMon.println("Error: No se pudo reiniciar el modem.");
    while (true);
  }

  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println("Error: No se pudo conectar a GPRS.");
    delay(10000);
    modem.gprsConnect(apn, gprsUser, gprsPass);
  } else {
    SerialMon.println("Conexión GPRS establecida.");
  }

  ThingSpeak.begin(client);

  // Configurar el pin del relay
  pinMode(RELAY_PIN, OUTPUT);

  // Registrar la hora de inicio del período activo
  startActiveTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // Lógica para el control del relay basado en la humedad del suelo
  if (currentMillis - lastRelayCheck >= relayInterval) {
    lastRelayCheck = currentMillis;

    float soilm = analogRead(AOUT_PIN);  
    int soilMoisturePercent = toPercent(soilm);

    Serial.print("Humedad del suelo: ");
    Serial.print(soilMoisturePercent);
    Serial.println("%");

    if (soilMoisturePercent < THRESHOLD) {
      Serial.println("El suelo está seco, activando la bomba.");
      digitalWrite(RELAY_PIN, HIGH);  // Activar la bomba
      activePeriod = 600000;  // Extender el período activo a 10 minutos si el suelo está seco
    } else {
      Serial.println("El suelo está húmedo, desactivando la bomba.");
      digitalWrite(RELAY_PIN, LOW);  // Desactivar la bomba
      activePeriod = 300000;  // Período activo normal de 5 minutos si el suelo está húmedo
    }
  }

  // Lógica para actualizar ThingSpeak
  if (currentMillis - lastThingSpeakUpdate >= thingSpeakInterval) {
    lastThingSpeakUpdate = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Error al leer los datos del sensor DHT.");
      return;
    }

    Serial.print(F("Humedad: "));
    Serial.print(h);
    Serial.print(F("%  Temperatura: "));
    Serial.print(t);
    Serial.println(F("°C"));

    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);

    Serial.print("Temperatura del suelo: ");
    Serial.print(temperatureC);
    Serial.println("ºC");

    float soilm = analogRead(AOUT_PIN);  
    int soilMoisturePercent = toPercent(soilm);

    // Configurar los campos con los valores para ThingSpeak
    ThingSpeak.setField(1, h);
    ThingSpeak.setField(2, t);
    ThingSpeak.setField(3, temperatureC);
    ThingSpeak.setField(4, soilMoisturePercent);

    // Enviar los datos a ThingSpeak
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Actualización de canal exitosa.");
    } else {
      Serial.println("Error al actualizar el canal. Código HTTP: " + String(x));
    }
  }

  // Verificar si ha pasado el período activo dinámico
  if (currentMillis - startActiveTime >= activePeriod) {
    Serial.println("Entrando en modo de sueño profundo durante 7 minutos.");
    esp_sleep_enable_timer_wakeup(sleepDuration); // Configurar el temporizador de sueño
    esp_deep_sleep_start();  // Poner el ESP32 en sueño profundo
  }

  // Pequeña pausa para estabilizar el ciclo
  delay(100);  // Este delay es opcional y puede ajustarse
}
