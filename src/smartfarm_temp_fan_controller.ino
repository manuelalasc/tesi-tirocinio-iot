#include <WiFi.h>
#include <PubSubClient.h>
#include <dht11.h>

const char* ssid = "WiFi";       
const char* password = "Password";       
const char* mqtt_server = "MQTT_server_IP";       
const char* token = "Access_Token";        

#define DHT11PIN 17        // Pin sensore DHT11
#define MotorPin1 19       // Pin (IN+) per ventola 
#define MotorPin2 18       // Pin (IN-) per ventola

dht11 DHT11;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

double dewPointFast(double celsius, double humidity)
{
        double a = 17.271;
        double b = 237.7;
        double temp = (a * celsius) / (b + celsius) + log(humidity/100);
        double Td = (b * temp) / (a - temp);
        return Td;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Comando RPC ricevuto: ");
  Serial.println(message);

  if (message.indexOf("on") != -1 || message.indexOf("true") != -1 || message.indexOf("1") != -1) {
    Serial.println("-> Attivazione ventola...");
    // Rotazione sinistra con potenza = 70
    ledcWrite(1, 70); 
    ledcWrite(3, 0);
  } 
  else if (message.indexOf("off") != -1 || message.indexOf("false") != -1 || message.indexOf("0") != -1) {
    Serial.println("-> Spegnimento ventola...");
    // Stop
    ledcWrite(1, 0);
    ledcWrite(3, 0);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connessione a ThingsBoard... ");
    if (client.connect("ESP32_SmartFarm", token, NULL)) {
      Serial.println("CONNESSO!");
      client.subscribe("v1/devices/me/rpc/request/+");
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

  // Configurazione PWM per il motore
  pinMode(MotorPin1, OUTPUT);
  pinMode(MotorPin2, OUTPUT);
  ledcSetup(1, 1200, 8);        // Imposta frequenza LEDC Channel 1 a 1200Hz, PWM risoluzione a 8 bit, quindi velocità del motore regolata su 256 livelli, da 0 (fermo) a 255 (massima velocità)
  ledcAttachPin(MotorPin1, 1);  // Collega fisicamente LEDC Channel 1 appena configurato al pin gpio19
  ledcSetup(3, 1200, 8);        // Stessa cosa per pin 18
  ledcAttachPin(MotorPin2, 3);
  
  ledcWrite(1, 0);
  ledcWrite(3, 0);

  // Connessione Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connesso!");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    // Lettura dal sensore DHT11
    int chk = DHT11.read(DHT11PIN);

    if (chk == DHTLIB_OK) { // Lettura andata a buon fine
      float t = (float)DHT11.temperature;
      float h = (float)DHT11.humidity;

      float dp = dewPointFast(t, h); // Punto di rugiada

      // Costruzione del payload JSON
      String payload = "{\"temperature\":" + String(t, 1) + 
                        ", \"humidity\":" + String(h, 1) + 
                        ", \"dewPoint\":" + String(dp, 2) + "}";
      Serial.print("Inviato: ");
      Serial.println(payload);
      client.publish("v1/devices/me/telemetry", payload.c_str());
    } else {
      Serial.println("Errore lettura DHT11!");
    }
  }
}
