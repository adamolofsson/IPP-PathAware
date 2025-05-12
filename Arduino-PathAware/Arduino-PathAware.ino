#include <WiFi.h>
#include <FirebaseESP32.h>

// Inställningar för specifik Arduino
const char* company = "testCompany";
const char* building = "testBuilding";

const int antalC = 1; // Antal kolumner
const int antalR = 1; // Antal rader

int plate[antalC][antalR]; // Array för att lagra sensorvärden
int plateSteps[antalC][antalR] = {0}; // Array för att lagra antal steg
int analogPins[antalC][antalR] = { {A0} }; // Analog-pinnar för sensorer

// WiFi-inställningar
const char* ssid = "S215G"; // Bahnhof-500051
const char* password = "Vinc1234"; // PJPRF3HB5T5NL3

// Firebase Realtime Database-inställningar
#define FIREBASE_HOST "ipp-pathaware-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "oFYol0NVjHHPmzi1KpSvPOXgS7dOT8KXwWJUbRgW"

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);

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
}

void loop() {
  // Samla in sensorvärden under 60 sekunder
  for (int i = 0; i < 120; i++) {
    for (int c = 0; c < antalC; c++) {
      for (int r = 0; r < antalR; r++) {
        plate[c][r] = analogRead(analogPins[c][r]);
        if (plate[c][r] >= 500) {
          Serial.print("C0R0: ");
          Serial.println(plate[c][r]);
          plateSteps[c][r]++;
        }
      }
    }
    delay(500);
  }

  // Skicka data till Firebase
  for (int c = 0; c < antalC; c++) {
    for (int r = 0; r < antalR; r++) {
      String path = "/" + String(company) + "/" + String(building) + "/C" + String(c) + "/R" + String(r) + "/steps";
      int current = 0;

      // Hämta befintligt värde från Firebase
      if (Firebase.getInt(firebaseData, path)) {
        if (firebaseData.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
          current = firebaseData.intData();
        } else {
          Serial.println("Fel: Datatypen är inte ett heltal!"); // Problem här, resettar datan om den inte lyckas hämta från Firebase
        }
      } else {
        Serial.print("Fel vid hämtning av data: ");
        Serial.println(firebaseData.errorReason());
      }

      // Öka värdet med plateSteps[c][r]
      current += plateSteps[c][r];

      // Skicka det uppdaterade värdet till Firebase
      if (Firebase.setInt(firebaseData, path, current)) {
        Serial.print("Uppdaterat värde skickat: ");
        Serial.print(path);
        Serial.print(" = ");
        Serial.println(current);
      } else {
        Serial.print("Fel vid uppdatering av Firebase: ");
        Serial.println(firebaseData.errorReason());
      }
      plateSteps[c][r] = 0;
    }
  }
}