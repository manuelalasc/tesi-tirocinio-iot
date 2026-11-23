#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "WiFi";
const char* password = "Password";
const char* mqtt_server = "MQTT_server_IP";
const char* token = "Access_Token"; 

#define PhotocecllPin 34      // Pin fotoresistore
#define LED_PIN 27            // Pin LED

LiquidCrystal_I2C lcd(0x27, 16, 2);

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

  // Configurazione LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  // Configurazione pin LED
  pinMode(PhotocecllPin,INPUT);
  pinMode(LED_PIN,OUTPUT);

  digitalWrite(LED_PIN, LOW); // Led inizialmente spento

  // Connessione Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connesso!");
  Serial.println(WiFi.localIP());

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

    int light = analogRead(PhotocecllPin);

    String payload = "{\"light\":" + String(light) + "}";
    Serial.print("Inviato: ");
    Serial.println(payload);
    client.publish("v1/devices/me/telemetry", payload.c_str());
  }
}
