#include <QTRSensors.h>

// --- L298N Motor Driver Pins ---
const uint8_t ENA = 5;  // PWM Left Motor
const uint8_t IN1 = 3;  // Left Dir 1
const uint8_t IN2 = 4;  // Left Dir 2
const uint8_t IN3 = 7;  // Right Dir 1
const uint8_t IN4 = 8;  // Right Dir 2
const uint8_t ENB = 6;  // PWM Right Motor

// --- Hardware Pins ---
const uint8_t BTN_ACTION = 9;   // D9: Short press = Calibrate | Long press = Launch / Stop
const uint8_t BTN_START  = 10;  // D10: Forced HIGH continuously for sensor power

// --- QTR-8A Setup (All 8 Pins) ---
QTRSensors qtr;
const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

// Position target for 8 sensors = 3500 (Range is 0 to 7000)
const int TARGET_POSITION = 3500; 

// --- PD Tuning Parameters for 3mm Line ---
float Kp = 0.095;
float Kd = 1.35;

int lastError = 0;
int lastKnownDirection = 0; // -1 = Left (A0 side), 1 = Right (A7 side)

// Speed Control (0 to 255)
const int BASE_SPEED = 130; 
const int MAX_SPEED  = 180;

bool isRunning = false;

void setup() {
  Serial.begin(9600);

  pinMode(BTN_START, OUTPUT);
  digitalWrite(BTN_START, HIGH);

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  pinMode(BTN_ACTION, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5, A6, A7}, SensorCount);

  halt();
}

void loop() {
  digitalWrite(BTN_START, HIGH);

  if (digitalRead(BTN_ACTION) == HIGH) {
    unsigned long pressStartTime = millis();
    while (digitalRead(BTN_ACTION) == HIGH) delay(10);
    unsigned long pressDuration = millis() - pressStartTime;

    if (pressDuration < 1000) {
      if (!isRunning) {
        digitalWrite(LED_BUILTIN, HIGH);
        for (uint16_t i = 0; i < 200; i++) {
          qtr.calibrate();
          delay(20);
        }
        digitalWrite(LED_BUILTIN, LOW);
      }
    } else {
      isRunning = !isRunning;
      if (!isRunning) halt();
    }
  }

  if (isRunning) {
    runPDTracking();
  } else {
    halt();
  }
}

void runPDTracking() {
  uint16_t position = qtr.readLineBlack(sensorValues);
  int error = position - TARGET_POSITION;

  // 1. Lock Direction Memory
  if (error < -300) lastKnownDirection = -1;      // Line moved to A0/A1 side
  else if (error > 300) lastKnownDirection = 1;   // Line moved to A6/A7 side

  int activeSensors = 0;
  for (uint8_t i = 0; i < SensorCount; i++) {
    if (sensorValues[i] > 350) activeSensors++;
  }

  // 2. CORNER RECOVERY (SWAPPED MOTOR SIGNALS TO MATCH YOUR HARDWARE)
  if (activeSensors == 0 || position <= 100 || position >= 6900) {
    if (lastKnownDirection == -1) {
      // Left side corner -> Spin LEFT hard
      setMotorSpeeds(-140, BASE_SPEED); 
    } else if (lastKnownDirection == 1) {
      // Right side corner -> Spin RIGHT hard
      setMotorSpeeds(BASE_SPEED, -140); 
    }
    return;
  }

  // Standard PD Calculation for normal curves
  int derivative = error - lastError;
  lastError = error;

  int correction = (Kp * error) + (Kd * derivative);

  int leftSpeed  = BASE_SPEED + correction;
  int rightSpeed = BASE_SPEED - correction;

  leftSpeed  = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

  setMotorSpeeds(leftSpeed, rightSpeed);
}

void setMotorSpeeds(int left, int right) {
  if (left >= 0) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); analogWrite(ENA, left);
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); analogWrite(ENA, -left);
  }

  if (right >= 0) {
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENB, right);
  } else {
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); analogWrite(ENB, -right);
  }
}

void halt() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); analogWrite(ENA, 0);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENB, 0);
}
