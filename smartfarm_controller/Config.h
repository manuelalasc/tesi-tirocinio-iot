/*
  Config.h - Header per impostazioni di connessione e pin di sensori e attuatori
*/

#ifndef CONFIG_H // evita problemi nel asoin cui si includa la libreria due volte
#define CONFIG_H

const char* ssid = "WiFi";       
const char* password = "Password";       
const char* mqtt_server = "mqtt server";       
const char* token = "Access_Token";

// Sensori
#define DHT11PIN 17        // Pin sensore DHT11
#define STEAM_PIN 35       // Pin steam sensor
#define PhotocecllPin 34   // Pin fotoresistore

// Attuatori
#define MotorPin1 19       // Pin (IN+) per ventola 
#define MotorPin2 18       // Pin (IN-) per ventola
#define LED_PIN 27         // Pin LED

#endif