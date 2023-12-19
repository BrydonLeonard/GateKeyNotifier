#include <Arduino.h>
#include "Notification.h"

int success[] { 1047, 1175, 1319 };
int pendingWifi[] { 1319, 1319, 1319 };
int pendingRegistration[] { 1047, 1047, 1175 };
int pendingMqtt[] { 1047, 1175, 1047 };
int opened[] { 1047, 1568, 1047 };

Notification::Notification(int speakerPin, int redPin, int greenPin, int bluePin) {
    this->speakerPin = speakerPin;
    this->redPin = redPin;
    this->greenPin = greenPin;
    this->bluePin = bluePin;
    this->state = none;
    this->stateChanged = true;
    this->lastLoop = millis();

    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
}

void Notification::setWaitingWifi() {
    if (this->state != waitingWifi) {
        this->stateChanged = true;
    }
    this->state = waitingWifi;
}

void Notification::setWaitingCreds() {
    if (this->state != waitingCreds) {
        this->stateChanged = true;
    }
    this->state = waitingCreds;
}

void Notification::setWaitingMqtt() {
    if (this->state != waitingMqtt) {
        this->stateChanged = true;
    }
    this->state = waitingMqtt;
}

void Notification::setConnected() {
    if (this->state != connected) {
        this->stateChanged = true;
    }
    this->state = connected;
}

void Notification::clearState() {
    if (this->state != none) {
        this->stateChanged = true;
    }
    this->state = none;
}

int gateOpenedSound[] = { 1047, 1047, 1319, 1568, 2093 };
int waitingWifiSound[] = { 1319, 1319, 1319 };
int waitingCredsSound[] = { 1047, 1047, 1175 };
int waitingMqttSound[] = { 1047, 1175, 1047 };
int connectedSound[] = { 1047, 1175, 1319 };
void Notification::gateOpened() {
    setLedColour(0, 255, 50);
    playNotification(gateOpenedSound, 5);

    delay(500); // Keep the light on a little longer
    setLedColour(0, 0, 0);
}

void Notification::loop() {
    if (stateChanged || millis() > this->lastLoop + 2000) {
        switch (this->state) {
            case none:
                break;
            case waitingWifi: 
                setLedColour(100, 0, 0);
                playNotification(waitingWifiSound, 3);
                break;
            case waitingCreds: 
                setLedColour(0, 100, 0);
                playNotification(waitingCredsSound, 3);
                break;
            case waitingMqtt:
                setLedColour(100, 100, 0);
                playNotification(waitingMqttSound, 3);
                break;
            case connected:
                setLedColour(0, 0, 0);
                playNotification(connectedSound, 3);
                clearState();
                break;
        }
        lastLoop = millis();
    }

    stateChanged = false;
}

void Notification::setLedColour(int red, int green, int blue) {
  analogWrite(this->redPin, red);
  analogWrite(this->greenPin, green);
  analogWrite(this->bluePin, blue);
}

void Notification::playNotification(int notification[], int length) {
  for (int i = 0; i < length; i++) {
    tone(this->speakerPin, notification[i], 60);
    delay(120);
  }
}

