// Generated Arduino code
// Translated from custom language

int ledPin = 13;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

void loop() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
  delay(1000);
}
