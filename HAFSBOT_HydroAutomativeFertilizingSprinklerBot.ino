int water;
const int motorPin1 = 2;
const int motorPin2 = 5;
const int motorPin3 = 4;
const int motorPin4 = 6; 

void setup() 
{
  pinMode(3,OUTPUT);
  pinMode(6,INPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  analogWrite(motorPin2, 255);
  analogWrite(motorPin4, 255);
}

void loop() 
{ 
  water = digitalRead(6);  
  if(water == HIGH)
  {
    digitalWrite(3,LOW); 
    digitalWrite(motorPin1, LOW);
  }
  else
  {
    digitalWrite(3,HIGH);
    digitalWrite(motorPin1, HIGH); 
  }
  delay(400);
}