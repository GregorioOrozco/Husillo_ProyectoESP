#include <AccelStepper.h>

// --------------------
// Pines
// --------------------
const int STEP_PIN = 26;
const int DIR_PIN = 27;
const int ENABLE_PIN = 25;

const int HOME_SWITCH = 33;

// --------------------
// Configuración mecánica
// --------------------
// AJUSTA ESTE VALOR HASTA QUE EL CENTRO
// QUEDE EXACTAMENTE DONDE QUIERES
const long TOTAL_TRAVEL_STEPS = 150000;

const long CENTER_POS = TOTAL_TRAVEL_STEPS / 2;

// --------------------
// Motor
// --------------------
AccelStepper stepper(
    AccelStepper::DRIVER,
    STEP_PIN,
    DIR_PIN
);

bool homed = false;

// -----------------------------------
// PROTOTIPOS
// -----------------------------------
void performHoming();
void processCommand(String cmd);

// -----------------------------------
// SETUP
// -----------------------------------
void setup()
{
    Serial.begin(115200);

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);

    pinMode(HOME_SWITCH, INPUT_PULLUP);

    stepper.setMaxSpeed(4000);
    stepper.setAcceleration(1500);

    performHoming();

    Serial.println("READY");
}

// -----------------------------------
// LOOP
// -----------------------------------
void loop()
{
    stepper.run();

    // Protección permanente HOME
    if (
        homed &&
        digitalRead(HOME_SWITCH) == LOW &&
        stepper.currentPosition() > 200
    )
    {
        Serial.println("ERROR: HOME SWITCH ACTIVATED");
    }

    if (Serial.available())
    {
        String cmd =
            Serial.readStringUntil('\n');

        cmd.trim();

        processCommand(cmd);
    }
}

// -----------------------------------
// HOMING
// -----------------------------------
void performHoming()
{
    Serial.println("HOMING...");

    // ======================
    // Aproximación rápida
    // ======================

    stepper.setSpeed(-1500);

    while (digitalRead(HOME_SWITCH) == HIGH)
    {
        stepper.runSpeed();
    }

    stepper.setSpeed(0);

    stepper.move(1500);

    while (stepper.distanceToGo() != 0)
    {
        stepper.run();
    }

    stepper.setSpeed(-400);

    while (digitalRead(HOME_SWITCH) == HIGH)
    {
        stepper.runSpeed();
    }

    stepper.setSpeed(0);

    stepper.setCurrentPosition(0);

    homed = true;

    Serial.println("HOME FOUND");

    // ======================
    // Ir al centro
    // ======================

    stepper.moveTo(CENTER_POS);

    while (stepper.distanceToGo() != 0)
    {
        stepper.run();
    }

    Serial.println("CENTER REACHED");
}

// -----------------------------------
// COMANDOS SERIAL
// -----------------------------------
void processCommand(String cmd)
{
    // ------------------
    // HOME
    // ------------------
    if (cmd == "HOME")
    {
        stepper.moveTo(CENTER_POS);

        Serial.println("OK");

        return;
    }

    // ------------------
    // REHOME
    // ------------------
    if (cmd == "REHOME")
    {
        performHoming();

        Serial.println("OK");

        return;
    }

    // ------------------
    // STATUS
    // ------------------
    if (cmd == "STATUS")
    {
        Serial.print("POS:");
        Serial.println(
            stepper.currentPosition()
        );

        return;
    }

    // ------------------
    // MOVE:xxxxx
    // ------------------
    if (cmd.startsWith("MOVE:"))
    {
        long target =
            cmd.substring(5).toInt();

        // Límite mínimo
        if (target < 0)
        {
            target = 0;
        }

        // Límite máximo
        if (target > TOTAL_TRAVEL_STEPS)
        {
            target = TOTAL_TRAVEL_STEPS;
        }

        stepper.moveTo(target);

        Serial.println("OK");

        return;
    }

    Serial.println("UNKNOWN COMMAND");
}
