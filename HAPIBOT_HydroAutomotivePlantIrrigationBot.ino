#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN 9
#define SS_PIN 10
#define RELAY_PIN 3
#define ECHO_PIN 7 
#define TRIG_PIN 8 
#define MOISTURE_SENSOR_PIN A0 

const int motorPin1 = 2; 
const int motorPin2 = 5; 
const int motorPin3 = 4; 
const int motorPin4 = 6; 
const int waterSensorPin = 6;

const int RFID_PROXIMITY_THRESHOLD = 5; 

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo;

const int numReadings = 10;     
int moistureReadings[numReadings];  
int arrayIndex = 0;              
unsigned long irrigationStartTime = 0;  
int moistureThresholds[] = {20, 30, 40};
int currentThresholdIndex = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  myServo.attach(9); 

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  pinMode(waterSensorPin, INPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, INPUT);

  analogWrite(motorPin1, 127); 
  analogWrite(motorPin2, 127); 
  analogWrite(motorPin3, 127);
  analogWrite(motorPin4, 127); 

  Serial.println("Waiting for RFID tag");

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("RFID tag detected");
    MoveTowardsRFID(); 

    int waterLevel = digitalRead(waterSensorPin);
    Serial.print("Water level: ");
    Serial.println(waterLevel); 

    CheckPlantAndAdjust(waterLevel); 
  } else {
    Serial.println("No RFID tag detected");
  }

  Serial.println("Taking initial moisture readings");
  for (int i = 0; i < numReadings; i++) {
    moistureReadings[i] = ReadMoistureLevel();
    delay(100); 
  }

  int initialMoistureLevel = CalculateAverageMoisture();
  Serial.print("Initial Moisture Level: ");
  Serial.print(initialMoistureLevel);
  Serial.println("%");
}

void loop() {}

void CheckPlantAndAdjust(int waterLevel) {
  int currentMoisture = ReadMoistureLevel();
  Serial.print("Current Moisture Level: ");
  Serial.print(currentMoisture);
  Serial.println("%");

  int currentThreshold = moistureThresholds[currentThresholdIndex];

  if (currentMoisture < currentThreshold) {
    Serial.print("Moisture below ");
    Serial.print(currentThreshold); 
    Serial.println("Moisture is detected to be at %, starting irrigation");
    irrigationStartTime = millis();
    digitalWrite(RELAY_PIN, HIGH); 
  }

  if (digitalRead(RELAY_PIN) == HIGH) {
    moistureReadings[arrayIndex] = currentMoisture;
    arrayIndex = (arrayIndex + 1) % numReadings;

    if (currentMoisture >= 85 && currentMoisture <= 89) { 
      digitalWrite(RELAY_PIN, LOW); 
      CalculateAndPrintIrrigationMetrics();

      currentThresholdIndex = (currentThresholdIndex + 1) % 3; 
    }
  } 
}

void MoveTowardsRFID() {
  int minDistance = 1000; 

  for (int angle = -45; angle <= 45; angle += 15) {
    myServo.write(90 + angle); 
    delay(50); 
    int distance = GetDistance();
    if (distance < minDistance) {
      minDistance = distance;
    }
  }

  if (minDistance < RFID_PROXIMITY_THRESHOLD) { 
    StopMotors();
    Serial.println("Reached RFID tag");
  } else {
    MoveForward();
  }
}

int GetDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  return pulseIn(ECHO_PIN, HIGH) * 0.034 / 2; 
}

void MoveForward() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);
}

void StopMotors() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
}

int ReadMoistureLevel() {
  int sensorValue = analogRead(MOISTURE_SENSOR_PIN); 
  int moisturePercentage = map(sensorValue, 0, 1023, 100, 0); 
  return moisturePercentage;
}

int CalculateAverageMoisture() {
  int sum = 0;
  for (int i = 0; i < numReadings; i++) {
    sum += moistureReadings[i];
  }
  return sum / numReadings;
}

void CalculateAndPrintIrrigationMetrics() {
  int averageSoilMoistureAfter = CalculateAverageMoisture();
  unsigned long irrigationDuration = (millis() - irrigationStartTime) / 60000; 

  Serial.print("Initial Moisture Level: ");
  for (int i = 0; i < numReadings; i++) {
    Serial.print(moistureReadings[i]);
    Serial.print(", ");
  }
  Serial.println();

  Serial.print("Average Moisture Level After Irrigation: ");
  Serial.print(averageMoistureAfter);
  Serial.println("%");

  Serial.print("Average Time To Reach Optimal Moisture Level (Minutes): ");
  Serial.print(moistureThresholds[currentThresholdIndex]);
  Serial.print("% moisture: ");
  Serial.print(irrigationDuration);
  Serial.println(" minutes"); 
}
