#include <WiFi.h>
#include <FirebaseESP32.h>

// Inställningar för specifik Arduino
const char* company = "testCompany";
const char* building = "testBuilding";

const int antalC = 1; // Antal kolumner
const int antalR = 1; // Antal rader

int analogPins[antalC][antalR] = { {A0} }; // Analog-pinnar för sensorer
int plate[antalC][antalR]; // Array för att lagra sensorvärden
int plateSteps[antalC][antalR] = {0}; // Array för att lagra antal steg
bool activePlate[antalC][antalR]; // Array för att lagra aktiva plattor
int totalSteps = 0; // Totala steg
String pathTotal = "/" + String(company) + "/" + String(building) + "/totalSteps"; // Path till antalet totala steg

// WiFi-inställningar
const char* ssid = "Bahnhof-500051";
const char* password = "PJPRF3HB5T5NL3";

// Firebase Realtime Database-inställningar
#define FIREBASE_HOST "ipp-pathaware-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "oFYol0NVjHHPmzi1KpSvPOXgS7dOT8KXwWJUbRgW"

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < antalC; i++) {
      for (int j = 0; j < antalR; j++) {
          activePlate[i][j] = false;
      }
  }

  // Anslut till Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Ansluter till Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nAnsluten!");

  // Konfigurera Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Hämta befintligt totalSteps värde från Firebase
  if (Firebase.getInt(firebaseData, pathTotal)) {
    if (firebaseData.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
      totalSteps = firebaseData.intData();
    }
  } else {
    Serial.print("Fel vid hämtning av data: ");
    Serial.println(firebaseData.errorReason());
  }
}

void loop() {
  for (int i = 0; i < 120; i++) {
    for (int c = 0; c < antalC; c++) {
      for (int r = 0; r < antalR; r++) {
        plate[c][r] = analogRead(analogPins[c][r]);
        if (plate[c][r] >= 500) { // Kan uppdateras i framtiden för att bättre identifiera ett steg
          Serial.print("C0R0: ");
          Serial.println(plate[c][r]);
          totalSteps++;
          plateSteps[c][r]++;
          activePlate[c][r] = true;
        } else {
          activePlate[c][r] = false;
        }
      }
    }
    delay(500);
  }

  // Skicka data till Firebase
  for (int c = 0; c < antalC; c++) {
    for (int r = 0; r < antalR; r++) {
      String pathPlate = "/" + String(company) + "/" + String(building) + "/C" + String(c) + "/R" + String(r) + "/steps";
      String pathActive = "/" + String(company) + "/" + String(building) + "/C" + String(c) + "/R" + String(r) + "/activePlate";
      int current = 0;

      // Hämta befintligt värde från Firebase
      if (Firebase.getInt(firebaseData, pathPlate)) {
        if (firebaseData.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
          current = firebaseData.intData();
        }
      } else {
        Serial.print("Fel vid hämtning av data: ");
        Serial.println(firebaseData.errorReason());
      }

      current += plateSteps[c][r];

      // Skicka uppdaterat värde till Firebase
      if (Firebase.setInt(firebaseData, pathPlate, current)) {
        Serial.print("Uppdaterat värde skickat: ");
        Serial.print(pathPlate);
        Serial.print(" = ");
        Serial.println(current);
      } else {
        Serial.print("Fel vid uppdatering av Firebase: ");
        Serial.println(firebaseData.errorReason());
      }

      if (Firebase.setInt(firebaseData, pathActive, activePlate[c][r])) {
        Serial.print("Uppdaterat värde skickat: ");
        Serial.print(pathActive);
        Serial.print(" = ");
        Serial.println(activePlate[c][r]);
      } else {
        Serial.print("Fel vid uppdatering av Firebase: ");
        Serial.println(firebaseData.errorReason());
      }

      if (Firebase.setInt(firebaseData, pathTotal, totalSteps)) {
        Serial.print("Uppdaterat värde skickat: ");
        Serial.print(pathTotal);
        Serial.print(" = ");
        Serial.println(totalSteps);
      } else {
        Serial.print("Fel vid uppdatering av Firebase: ");
        Serial.println(firebaseData.errorReason());
      }

      plateSteps[c][r] = 0;
    }
  }
}