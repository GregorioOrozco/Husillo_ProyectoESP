#include <AccelStepper.h>

// Pines recomendados para ESP32
const int STEP_PIN = 26;
const int DIR_PIN = 27;
const int ENABLE_PIN = 25;

// Inicialización del motor en modo Driver
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

void setup() {
    Serial.begin(115200);

    pinMode(ENABLE_PIN, OUTPUT);

    // Habilitar driver (TMC2209: LOW = habilitado)
    digitalWrite(ENABLE_PIN, LOW);

    stepper.setMaxSpeed(4000);
    stepper.setAcceleration(2000);

    stepper.setCurrentPosition(0);

}

void loop() {

    stepper.moveTo(-50000);

    while(stepper.distanceToGo() != 0) {
        stepper.run();
    }

    delay(2000);

    stepper.moveTo(50000);

    while(stepper.distanceToGo() != 0) {
        stepper.run();
    }

    delay(2000);
}