/*
  Sensori.h - Header per inizializzare sensori, vecchia funzione "setup()" 
              e costruire stringa JSON, vecchia funzione "loop()"
*/

#ifndef SENSORI_H
#define SENSORI_H

#include <dht11.h>
#include <Arduino.h>
#include "Config.h"

dht11 DHT11;

void initSensori() {
  // Configurazione fotoresistore
  pinMode(PhotocecllPin, INPUT);

  // Configurazione Steam sensor
  pinMode(STEAM_PIN, INPUT);
}

double dewPointFast(double celsius, double humidity) {
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}

String telemetriaJSON() {
  int chk = DHT11.read(DHT11PIN);         // Lettura dal sensore DHT11
  int steamValue = analogRead(STEAM_PIN); // Lettura dallo steam sensor
  int light = analogRead(PhotocecllPin);  // Lettura dal fotoresistore

  String payload = "";

  if (chk == DHTLIB_OK) { // Lettura andata a buon fine
    float t = (float)DHT11.temperature;
    float h = (float)DHT11.humidity;
    float dp = dewPointFast(t, h); // Punto di rugiada

  payload = "{\"temperature\":" + String(t, 1) + 
            ", \"humidity\":" + String(h, 1) + 
            ", \"dewPoint\":" + String(dp, 2) +
            ", \"steam\":" + String(steamValue) +
            ", \"light\":" + String(light) + "}";
  } else {
    Serial.println("Errore lettura DHT11!");
  }
  return payload;
}

#endif