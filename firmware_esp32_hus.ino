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

const long MIN_POSITION = 1500;
const long MAX_POSITION = 159000;
const long BACKOFF_STEPS = 1500;

// --------------------
// Estados
// --------------------
enum MachineState
{
    STARTUP,

    HOMING_FAST,
    HOMING_BACKOFF,
    HOMING_SLOW,
    GO_CENTER,

    IDLE,
    MOVING,
    STOPPED,

    ERROR_RECOVERY
};

MachineState state = STARTUP;

// --------------------
// Motor
// --------------------
AccelStepper stepper(
    AccelStepper::DRIVER,
    STEP_PIN,
    DIR_PIN
);

// --------------------
// Variables globales
// --------------------
bool homed = false;

long targetPosition = CENTER_POS;
long recoveryTarget = CENTER_POS;

const int QUEUE_SIZE = 100;

long moveQueue[QUEUE_SIZE];

int queueHead = 0;
int queueTail = 0;
int queueCount = 0;

bool paused = false;

long pausedTarget = CENTER_POS;
long lastTarget = CENTER_POS;

bool recovering = false;
// --------------------
// Prototipos
// --------------------
void processCommand(String cmd);
void startHoming();
void updateMachine();
String getStateName();

bool enqueue(long pos)
{
    if(queueCount >= QUEUE_SIZE)
        return false;

    moveQueue[queueTail] = pos;

    queueTail =
        (queueTail + 1) % QUEUE_SIZE;

    queueCount++;

    return true;
}

bool dequeue(long &pos)
{
    if(queueCount == 0)
        return false;

    pos = moveQueue[queueHead];

    queueHead =
        (queueHead + 1) % QUEUE_SIZE;

    queueCount--;

    return true;
}

// --------------------------------
// SETUP
// --------------------------------
void setup()
{
    Serial.begin(115200);

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);

    pinMode(HOME_SWITCH, INPUT_PULLUP);

    stepper.setMaxSpeed(4000);
    stepper.setAcceleration(1500);

    startHoming();

    Serial.println("BOOT");
}

// --------------------------------
// LOOP
// --------------------------------
void loop()
{
    stepper.run();

    updateMachine();

    if (Serial.available())
    {
        String cmd =
            Serial.readStringUntil('\n');

        cmd.trim();

        processCommand(cmd);
    }
}

// --------------------------------
// Iniciar homing
// --------------------------------
void startHoming()
{
    homed = false;

    state = HOMING_FAST;

    Serial.println("HOMING START");
}

// --------------------------------
// Máquina de estados
// --------------------------------
void updateMachine()
{
    switch(state)
    {
        // ------------------------
        // Aproximación rápida
        // ------------------------
        case HOMING_FAST:

            if(digitalRead(HOME_SWITCH) == LOW)
            {
                
                stepper.move(BACKOFF_STEPS);

                state = HOMING_BACKOFF;

                Serial.println("HOME DETECTED");
            }
            else
            {
                stepper.setSpeed(-1500);
                stepper.runSpeed();
            }

            break;

        // ------------------------
        // Alejarse
        // ------------------------
        case HOMING_BACKOFF:

            if(stepper.distanceToGo() == 0)
            {
                state = HOMING_SLOW;

                Serial.println("BACKOFF DONE");
            }
            else
            {
                stepper.setSpeed(1000);
                stepper.runSpeed();
            }

            break;

        // ------------------------
        // Aproximación lenta
        // ------------------------
        case HOMING_SLOW:

            if(digitalRead(HOME_SWITCH) == LOW)
            {
                stepper.setCurrentPosition(0);

                homed = true;

                stepper.moveTo(CENTER_POS);

                state = GO_CENTER;

                Serial.println("HOME FOUND");
            }
            else
            {
                stepper.setSpeed(-400);
                stepper.runSpeed();
            }

            break;

        // ------------------------
        // Ir al centro
        // ------------------------
        case GO_CENTER:

            if(stepper.distanceToGo() == 0)
            {
                if(recovering)
                {
                    recovering = false;

                    targetPosition =
                        recoveryTarget;

                    stepper.moveTo(
                        recoveryTarget
                    );

                    state = MOVING;

                    Serial.println(
                        "RECOVERY COMPLETE"
                    );
                }
                else
                {
                    state = IDLE;

                    Serial.println(
                        "CENTER REACHED"
                    );

                    Serial.println(
                        "READY"
                    );
                }
            }
            else
            {
                stepper.setSpeed(4000);
                stepper.runSpeed();
            }

        break;

        // ------------------------
        // Movimiento normal
        // ------------------------
        case MOVING:

            if(stepper.distanceToGo() == 0)
            {
                Serial.println("MOVE COMPLETE");

                long nextTarget;

                if(dequeue(nextTarget))
                {
                    targetPosition = nextTarget;

                    lastTarget = nextTarget;

                    stepper.moveTo(nextTarget);

                    Serial.print("NEXT:");
                    Serial.println(nextTarget);
                }
                else
                {
                    state = IDLE;

                    Serial.println("QUEUE EMPTY");
                }
            }

            break;

        // ------------------------
        // Error -> recuperar
        // ------------------------
        case ERROR_RECOVERY:

            startHoming();

            break;
        
        case STOPPED:
        break;

        default:
            break;
    }

    // --------------------------------
    // Protección HOME inesperado
    // --------------------------------
    if(
        homed &&
        state != HOMING_FAST &&
        state != HOMING_BACKOFF &&
        state != HOMING_SLOW &&
        state != GO_CENTER &&
        digitalRead(HOME_SWITCH) == LOW &&
        stepper.currentPosition() > 200
    )
    {
        Serial.println("ERROR: UNEXPECTED HOME");

        recoveryTarget =
            stepper.targetPosition();

        recovering = true;

        state = ERROR_RECOVERY;
    }
}

// --------------------------------
// Comandos
// --------------------------------
void processCommand(String cmd)
{
    // --------------------------------
    // No aceptar comandos durante homing
    // --------------------------------
    if(
        state == HOMING_FAST ||
        state == HOMING_BACKOFF ||
        state == HOMING_SLOW ||
        state == GO_CENTER
    )
    {
        Serial.println("BUSY HOMING");
        return;
    }

    // ------------------------
    // STATUS
    // ------------------------
    if(cmd == "STATUS")
    {
        Serial.print("POS:");
        Serial.println(stepper.currentPosition());

        Serial.print("STATE:");
        Serial.println(getStateName());

        Serial.print("QUEUE:");
        Serial.println(queueCount);

        Serial.print("TARGET:");
        Serial.println(targetPosition);

        Serial.print("MIN:");
        Serial.println(MIN_POSITION);

        Serial.print("MAX:");
        Serial.println(MAX_POSITION);

        return;
    }

    // ------------------------
    // STOP
    // ------------------------
    if(cmd == "STOP")
    {
        pausedTarget =
            stepper.targetPosition();

        stepper.stop();

        paused = true;

        state = STOPPED;

        Serial.println("STOPPED");

        return;
    }


    if(cmd == "RUN")
    {
        if(paused)
        {
            paused = false;

            stepper.moveTo(
                pausedTarget
            );

            state = MOVING;

            Serial.println("RUNNING");
        }

        return;
    }
    // ------------------------
    // HOME
    // ------------------------
    if(cmd == "HOME")
    {
        targetPosition = CENTER_POS;

        stepper.moveTo(CENTER_POS);

        state = MOVING;

        Serial.println("OK");

        return;
    }

    // ------------------------
    // REHOME
    // ------------------------
    if(cmd == "REHOME")
    {
        startHoming();

        return;
    }

    // ------------------------
    // MOVE:x
    // ------------------------
    if(cmd.startsWith("MOVE:"))
    {
        long target =
            cmd.substring(5).toInt();

        if(target < MIN_POSITION)
            target = MIN_POSITION;

        if(target > MAX_POSITION)
            target = MAX_POSITION;

        if(state == IDLE)
        {
            targetPosition = target;

            lastTarget = target;

            stepper.moveTo(target);

            state = MOVING;
        }
        else
        {
            if(!enqueue(target))
            {
                Serial.println(
                    "QUEUE FULL"
                );
                return;
            }
        }

        Serial.println("OK");

        return;
    }

    Serial.println("UNKNOWN COMMAND");
}

// --------------------------------
// Nombre del estado
// --------------------------------
String getStateName()
{
    switch(state)
    {
        case IDLE:
            return "IDLE";

        case MOVING:
            return "MOVING";

        case STOPPED:
            return "STOPPED";

        case HOMING_FAST:
        case HOMING_BACKOFF:
        case HOMING_SLOW:
        case GO_CENTER:
            return "HOMING";

        case ERROR_RECOVERY:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}
