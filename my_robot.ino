// Generated Arduino code
// Translated from custom robot language
// High-level robot control functions

// Custom robot control functions
void avanzar() {
  // Move forward - both motors forward
  digitalWrite(3, HIGH);  // Left motor forward
  digitalWrite(4, LOW);
  digitalWrite(5, HIGH);  // Right motor forward
  digitalWrite(6, LOW);
}

void retroceder() {
  // Move backward - both motors backward
  digitalWrite(3, LOW);   // Left motor backward
  digitalWrite(4, HIGH);
  digitalWrite(5, LOW);   // Right motor backward
  digitalWrite(6, HIGH);
}

void girar_izquierda() {
  // Turn left - left motor backward, right motor forward
  digitalWrite(3, LOW);   // Left motor backward
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);  // Right motor forward
  digitalWrite(6, LOW);
}

void girar_derecha() {
  // Turn right - left motor forward, right motor backward
  digitalWrite(3, HIGH);  // Left motor forward
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);   // Right motor backward
  digitalWrite(6, HIGH);
}

void detener() {
  // Stop all motors
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
}

void encender_led() {
  // Turn on LED
  digitalWrite(13, HIGH);
}

void apagar_led() {
  // Turn off LED
  digitalWrite(13, LOW);
}

int leer_sensor() {
  // Read sensor from analog pin A0
  return analogRead(A0);
}

void setup() {
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop() {
  avanzar();
  delay(2000);
  int hola = 0;
  girar_derecha();
  delay(1000);
  if (hola hola 0) {
    girar_derecha();
    girar_izquierda();
    funcion_feka();
  }
  retroceder();
  delay(1500);
  detener();
}
