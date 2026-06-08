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
const long TOTAL_TRAVEL_STEPS = 160000;

const long CENTER_POS = TOTAL_TRAVEL_STEPS / 2;

// Pasos para alejarse del switch
const long BACKOFF_STEPS = 1500;

// --------------------
// Motor
// --------------------
AccelStepper stepper(
    AccelStepper::DRIVER,
    STEP_PIN,
    DIR_PIN
);

bool homed = false;
bool errorState = false;

// -----------------------------------
// Prototipos
// -----------------------------------
void performHoming();
void processCommand(String cmd);

// -----------------------------------
// Setup
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
// Loop principal
// -----------------------------------
void loop()
{
    stepper.run();

    // ----------------------------
    // Protección HOME inesperado
    // ----------------------------
    if (
        homed &&
        !errorState &&
        digitalRead(HOME_SWITCH) == LOW &&
        stepper.currentPosition() > 200
    )
    {
        Serial.println("ERROR: UNEXPECTED HOME");

        errorState = true;

        // Cancelar movimiento actual
        stepper.stop();

        while(stepper.distanceToGo() != 0)
        {
            stepper.run();
        }

        Serial.println("AUTO REHOME");

        performHoming();

        errorState = false;

        Serial.println("RECOVERED");
    }

    // ----------------------------
    // Procesamiento Serial
    // ----------------------------
    if (Serial.available())
    {
        String cmd =
            Serial.readStringUntil('\n');

        cmd.trim();

        processCommand(cmd);
    }
}

// -----------------------------------
// Homing
// -----------------------------------
void performHoming()
{
    Serial.println("HOMING...");

    // --------------------------------
    // Si arrancó tocando el switch
    // --------------------------------
    if (digitalRead(HOME_SWITCH) == LOW)
    {
        Serial.println("HOME ALREADY PRESSED");

        stepper.move(BACKOFF_STEPS);

        while (stepper.distanceToGo() != 0)
        {
            stepper.run();
        }
    }

    // --------------------------------
    // Aproximación rápida
    // --------------------------------
    stepper.setSpeed(-1500);

    while (digitalRead(HOME_SWITCH) == HIGH)
    {
        stepper.runSpeed();
    }

    stepper.setSpeed(0);

    delay(50);

    // --------------------------------
    // Alejarse
    // --------------------------------
    stepper.move(BACKOFF_STEPS);

    while (stepper.distanceToGo() != 0)
    {
        stepper.run();
    }

    delay(50);

    // --------------------------------
    // Aproximación lenta
    // --------------------------------
    stepper.setSpeed(-400);

    while (digitalRead(HOME_SWITCH) == HIGH)
    {
        stepper.runSpeed();
    }

    stepper.setSpeed(0);

    delay(50);

    // --------------------------------
    // Definir origen
    // --------------------------------
    stepper.setCurrentPosition(0);

    homed = true;

    Serial.println("HOME FOUND");

    // --------------------------------
    // Ir al centro
    // --------------------------------
    stepper.moveTo(CENTER_POS);

    while (stepper.distanceToGo() != 0)
    {
        stepper.run();
    }

    Serial.println("CENTER REACHED");
}

// -----------------------------------
// Comandos
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

        if (target < 0)
        {
            target = 0;
        }

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
