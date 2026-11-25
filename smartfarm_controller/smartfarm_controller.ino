#include <WiFi.h>
#include <PubSubClient.h>
#include "Config.h"
#include "Sensori.h"
#include "Attuatori.h"

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Comando RPC ricevuto: ");
  Serial.println(message);

  // FAN_CONTROLLER
  if (message.indexOf("setFan") != -1) {
    if (message.indexOf("max") != -1) {
      Serial.println("-> Condensa critica: Ventola al MAX!");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RISCHIO CONDENSA");
      lcd.setCursor(0, 1);
      lcd.print("Ventola al MAX");
      
      // Rotazione massima con potenza = 150
      ledcWrite(1, 150); 
      ledcWrite(3, 0);
    }
    else if (message.indexOf("on") != -1 || message.indexOf("true") != -1 || message.indexOf("1") != -1) {
    Serial.println("-> Attivazione ventola...");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALLARME");
    lcd.setCursor(0, 1);
    lcd.print("Avvio ventola");

    // Rotazione sinistra con potenza = 70
    ledcWrite(1, 70); 
    ledcWrite(3, 0);
    } 
    else if (message.indexOf("off") != -1 || message.indexOf("false") != -1 || message.indexOf("0") != -1) {
      Serial.println("-> Spegnimento ventola...");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Spegnimento ventola");

      // Stop
      ledcWrite(1, 0);
      ledcWrite(3, 0);
    }
  }
  // LIGHT_CONTROLLER
  else if (message.indexOf("setLed") != -1) {
    if (message.indexOf("on") != -1 || message.indexOf("true") != -1 || message.indexOf("1") != -1) {
    Serial.println("-> Accensione LED...");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Accensione luci");

    digitalWrite(LED_PIN,HIGH);
    } 
    else if (message.indexOf("off") != -1 || message.indexOf("false") != -1 || message.indexOf("0") != -1) {
      Serial.println("-> Spegnimento LED...");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Spegnimento luci");

      digitalWrite(LED_PIN,LOW);
    }
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

  initSensori();
  initAttuatori();

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

    String payload = telemetriaJSON();

    if (payload != "") {
      Serial.print("Inviato: ");
      Serial.println(payload);
      client.publish("v1/devices/me/telemetry", payload.c_str());
    }
  }
}

