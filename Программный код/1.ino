#include "WebServerESP.h"
#include "LED.h"
#include <ESP32Servo.h>

// --- Пины подключения ---
#define DISPENSER_SERVO_PIN  13
#define DISK_SERVO_PIN       15

#define STEPPER_1_STEP_PIN   21
#define STEPPER_1_DIR_PIN    19
#define STEPPER_2_STEP_PIN   23
#define STEPPER_2_DIR_PIN    22
#define STEPPER_3_STEP_PIN   5
#define STEPPER_3_DIR_PIN    18

#define LIMIT_SWITCH_1      12
#define LIMIT_SWITCH_2      16
#define LIMIT_SWITCH_3      4
#define LIMIT_SWITCH_4      17

#define LED_PIN             25

// --- Позиции сервомоторов ---
const int dispenserPositions[] = {0, 35, 70, 105, 135};
const int diskPositions[] = {180, 140, 110, 70, 40};

// --- Структура шагового двигателя ---
struct Stepper {
    byte step_pin;
    byte dir_pin;
    bool direction;
    int steps_ml;
};

// --- Объявление шаговых двигателей ---
Stepper steppers[] = {
    {STEPPER_1_STEP_PIN, STEPPER_1_DIR_PIN, 0, 7900},  // Насос 1 (по часовой стрелке)
    {STEPPER_2_STEP_PIN, STEPPER_2_DIR_PIN, 1, 6500}, // Насос 2 (против часовой стрелки)
    {STEPPER_3_STEP_PIN, STEPPER_3_DIR_PIN, 1, 7700} // Насос 3 (по часовой стрелке)
};

const byte limitSwitches[] = {LIMIT_SWITCH_1, LIMIT_SWITCH_2, LIMIT_SWITCH_3, LIMIT_SWITCH_4};

// --- Константы ---
const int stepsPerMl = 7700; // 1 мл = n шагов
const int numRecipes = 6;
const int recipes[numRecipes][3] = {
    {50, 0, 0}, 
    {0, 10, 0},
    {0, 0, 40},
    {80, 20, 0},
    {30, 0, 50},
    {35, 10, 45},
};

const float speed_ml_per_second = 3.0;
int stepDelayMicros;

// --- Переменные ---
enum State {WAIT_REQUEST, MOVE_TO_FILL, POUR_DRINKS, DISPENSE_CUPS};
State state = WAIT_REQUEST;
String orderQueue = "";
int currentDrinkIndex = 0;
Servo dispenserServo;
Servo diskServo;

// --- Функции управления ---
void calculateStepDelay() {
    stepDelayMicros = (500000 / (speed_ml_per_second * stepsPerMl));
}

void moveServo(Servo &servo, int angle) {
    servo.write(angle);
    delay(500); // Ждём, пока серво займёт положение
}

void moveServoSlow(Servo &servo, int targetAngle, int delayMs) {
    int currentAngle = servo.read();
    if (currentAngle < targetAngle) {
        for (int angle = currentAngle; angle <= targetAngle; angle++) {
            servo.write(angle);
            delay(delayMs);
        }
    } else {
        for (int angle = currentAngle; angle >= targetAngle; angle--) {
            servo.write(angle);
            delay(delayMs);
        }
    }
}

void servosZeros(){
  moveServo(dispenserServo, dispenserPositions[0]);
  moveServo(diskServo, diskPositions[0]);
  }

bool checkCupPresence(int cupIndex) {
    return !digitalRead(limitSwitches[cupIndex]);
}

void moveStepper(Stepper &motor, int ml, int cupIndex) {
    int steps = ml * stepsPerMl;
    bool actualDirection = motor.direction ? steps > 0 : steps < 0;
    digitalWrite(motor.dir_pin, actualDirection ? HIGH : LOW);
    
    for (int m = 0; m < ml; m++) {
        for (int i = 0; i < motor.steps_ml; i++) {
            if (!checkCupPresence(cupIndex)) {
                while (!checkCupPresence(cupIndex)) {
                    delay(100);
                }
            }
            digitalWrite(motor.step_pin, HIGH);
            delayMicroseconds(stepDelayMicros);
            digitalWrite(motor.step_pin, LOW);
            delayMicroseconds(stepDelayMicros);
        }
    }

    digitalWrite(motor.dir_pin, !actualDirection);
    for (int i = 0; i < 500; i++) {
        digitalWrite(motor.step_pin, HIGH);
        delayMicroseconds(stepDelayMicros);
        digitalWrite(motor.step_pin, LOW);
        delayMicroseconds(stepDelayMicros);
    }
}

void pourDrink(int recipeIndex, int cupIndex) {
    for (int i = 0; i < 3; i++) {
        moveStepper(steppers[i], recipes[recipeIndex][i], cupIndex);
    }
}

// --- Обработчики состояний ---

void handleWaitRequest() {
  ledGreen();
  orderQueue = waitForCommand();
  currentDrinkIndex = 0;
  state = MOVE_TO_FILL;
  ledRed();
}

void handleMoveToFill() {
    delay(1000);
    if (orderQueue.length() > 0) {
        moveServo(dispenserServo, dispenserPositions[currentDrinkIndex + 1]);
        state = POUR_DRINKS;
    } else {
        state = DISPENSE_CUPS;
    }
}

void handlePourDrinks() {
    int recipeIndex = orderQueue[currentDrinkIndex] - '0' - 1;
    pourDrink(recipeIndex, currentDrinkIndex);
    currentDrinkIndex++;
    if (currentDrinkIndex < orderQueue.length()) {
        state = MOVE_TO_FILL;
    } else {
        state = DISPENSE_CUPS;
    }
}

void handleDispenseCups() {
    moveServo(dispenserServo, dispenserPositions[0]);
    byte drinks = orderQueue.length() - 1;
    for (int i = 0; i < drinks + 1; i++) {
        moveServoSlow(diskServo, diskPositions[4 - drinks + i], 50); // Плавное движение
        ledGreen();
        if (checkCupPresence(drinks - i)) {
            while (checkCupPresence(drinks - i)) {
                delay(100);
            }
            ledRed();
        }
        delay(2000);
    }
    state = WAIT_REQUEST;
    servosZeros();
}

// --- Сетап ---

void setup() {
    Serial.begin(115200);
    startServer();
    setupLedStrip(LED_PIN);
    ledBlue_blink();

    dispenserServo.attach(DISPENSER_SERVO_PIN, 500, 2500);
    diskServo.attach(DISK_SERVO_PIN, 500, 2500);

    for (Stepper &motor : steppers) {
        pinMode(motor.step_pin, OUTPUT);
        pinMode(motor.dir_pin, OUTPUT);
    }

    for (byte pin : limitSwitches) {
        pinMode(pin, INPUT_PULLUP);
    }

    calculateStepDelay();
    servosZeros();
}

// --- Основной цикл ---

void loop() {
    switch (state) {
        case WAIT_REQUEST:
            handleWaitRequest();
            break;
        case MOVE_TO_FILL:
            handleMoveToFill();
            break;
        case POUR_DRINKS:
            handlePourDrinks();
            break;
        case DISPENSE_CUPS:
            handleDispenseCups();
            break;
    }
}
