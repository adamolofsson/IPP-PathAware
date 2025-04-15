int ruta0;
int ruta1;

void setup() {
  Serial.begin(9600);
}

void loop() {
  ruta0 = analogRead(A0);
  ruta1 = analogRead(A1);

  if (ruta0 >= 0) {
    Serial.print("Ruta0 aktiverad, värde ");
    Serial.println(ruta0);
  }
  else {
    Serial.println("Ruta0 inaktiv");
  }

  if (ruta1 >= 0) {
    Serial.print("Ruta1 aktiverad, värde ");
    Serial.println(ruta1);
  }
  else {
    Serial.println("Ruta1 inaktiv");
  }
  delay(1000);
}