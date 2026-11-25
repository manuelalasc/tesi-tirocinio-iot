/*
  Attuatori.h - Header per inizializzare attuatori, vecchia funzione "setup()" 
*/

#ifndef ATTUATORI_H
#define ATTUATORI_H

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include "Config.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void initAttuatori() {
  // Configurazione LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  // Configurazione PWM per il motore
  pinMode(MotorPin1, OUTPUT);
  pinMode(MotorPin2, OUTPUT);
  ledcSetup(1, 1200, 8);        // Imposta frequenza LEDC Channel 1 a 1200Hz, PWM risoluzione a 8 bit, quindi velocità del motore regolata su 256 livelli, da 0 (fermo) a 255 (massima velocità)
  ledcAttachPin(MotorPin1, 1);  // Collega fisicamente LEDC Channel 1 appena configurato al pin gpio19
  ledcSetup(3, 1200, 8);        // Stessa cosa per pin 18
  ledcAttachPin(MotorPin2, 3);
  
  ledcWrite(1, 0);
  ledcWrite(3, 0);

  // Configurazione LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Led inizialmente spento
}

#endif