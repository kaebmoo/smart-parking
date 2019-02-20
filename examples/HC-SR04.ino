// Hardware Wemos D1
// Ultrasonic HC-SR04

#define trigPin D7
#define echoPin D6

void setup()
{
  Serial.begin (115200);
  delay(10);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


}

void loop()
{
  // put your main code here, to run repeatedly:
  long duration, distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;

  if (distance <= 15) {

  }
  if (distance >= 300 || distance <= 0) {
    Serial.println("Out of range");
  }
}
