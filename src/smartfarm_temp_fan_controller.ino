#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

const char* ssid = "Wifi";       // Nome Wifi
const char* password = "Password";       // Password Wifi
const char* mqtt_server = "192.168.137.1";       // IP Hotspot PC (Docker)
const char* token = "Access_Token";        // Access Token di ThingsBoard


#define DHTPIN 17          // Pin sensore DHT11
#define DHTTYPE DHT11
#define FAN_PIN 18        // Pin ventola

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

// Gestione comandi per la ventola (se ThingsBoard invia effettivamente i comandi e su quale canale MQTT arrivano)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Comando RPC ricevuto [");
  Serial.print(topic);
  Serial.print("]: ");

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Controllo per attivare o disattivare la ventola
  if (message.indexOf("on") != -1 || message.indexOf("true") != -1 || message.indexOf("1") != -1) {
    Serial.println("-> Attivazione ventilatore...");
    digitalWrite(FAN_PIN, HIGH); // Accende la ventola
  } 
  else if (message.indexOf("off") != -1 || message.indexOf("false") != -1 || message.indexOf("0") != -1) {
    Serial.println("-> Spegnimento ventilatore...");
    digitalWrite(FAN_PIN, LOW);  // Spegne la ventola
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connessione a ThingsBoard... ");
    if (client.connect("ESP32_SmartGreenhouse", token, NULL)) {
      Serial.println("CONNESSO!");
      client.subscribe("v1/devices/me/rpc/request/+"); // Ascolta i comandi RPC
      Serial.println("Iscritto ai comandi RPC.");
    } else {
      Serial.print("FALLITO, rc=");
      Serial.print(client.state());
      Serial.println(" Riprovo tra 5 secondi...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("\n--- AVVIO GREENHOUSE KEYESTUDIO ---");

  // Inizializzazione sensore DHT11
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();

  // Inizializzazione ventola
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW); // inizialmente spenta

  // Connessione Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connesso!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// Invio dati ogni 5 secondi
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      String payload = "{\"temperature\":" + String(t, 1) + ",\"humidity\":" + String(h, 1) + "}";
      Serial.print("Inviato: ");
      Serial.println(payload);
      client.publish("v1/devices/me/telemetry", payload.c_str());
    } else {
      Serial.println("Errore lettura DHT11!");
    }
  }
}
