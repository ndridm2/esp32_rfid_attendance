#include <SPI.h>
#include <MFRC522.h>

#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 0 // ESP32 pin GPIO27

const char* ssid = "Skyline";
const char* password = "duaribulimaratus";

const int buzzerPin = 32; // active buzzer
const int ledPin = 33;  // LED pin for success

MFRC522 rfid(SS_PIN, RST_PIN);
String CardId = "";

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    tone(buzzerPin, 1000, 500);
    delay(1000);
  }

  Serial.println("\nConnected to the WIFI network :");
  Serial.println(WiFi.localIP());

  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      // print UID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        CardId += rfid.uid.uidByte[i];
        digitalWrite(ledPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
        delay(700);
      }
      
      // kirim data ke API
      SendCardId(CardId);

      CardId = "";
      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}

void SendCardId(String CardId) {
  Serial.println("Sending card id :");
  Serial.println(CardId);

  if(WiFi.isConnected()){
    HTTPClient http;
    // http.begin("http://192.168.100.57:8000/api/rfid/reads?uid=" + String(CardId));
    http.begin("http://192.168.100.57:8000/api/rfid/attend?uid=" + String(CardId));

    if(http.GET() > 0) {
      Serial.println(http.getString());

      // Jika sukses, aktifkan LED hijau dan bunyikan buzzer
      tone(buzzerPin, 1000, 500);
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      delay(700);
    } else {
      Serial.print("Error code :");
      Serial.println(http.GET());

      // Jika error, aktifkan LED merah dan bunyikan buzzer
      tone(buzzerPin, 1000, 500);
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      delay(700);
    }

    delay(100);
    http.end();
  }

}

