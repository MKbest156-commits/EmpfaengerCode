/*
====================================================================
AMC Team – CanSat Österreich 2026
LoRa Receiver (ESP32-C3 + RFM95W / SX1276) – CSV Telemetrie-Empfang
====================================================================

Projekt/Team:
  - AMC Team (CanSat 2026)
  - Zweck: Empfang der Messdaten vom CanSat über LoRa und Ausgabe am PC

Was macht dieses Programm?
  - Initialisiert das LoRa-Modul (RFM95W / SX1276) mit 868 MHz (EU) und SyncWord 0xF3
  - Wartet im loop() auf eingehende LoRa-Pakete
  - Liest das komplette Paket als Text (CSV-Zeile) ein
  - Gibt die empfangene CSV-Zeile + RSSI (Signalstärke) im Serial Monitor aus

Warum RSSI?
  - RSSI ist ein grober Hinweis auf Empfangsqualität / Entfernung / Abschattung
  - Werte z.B. um -40 dBm = sehr stark, -110 dBm = sehr schwach (kommt auf Setup an)

Verdrahtung (ESP32-C3 <-> RFM95W Modul):
  - Hinweis: Zusätzlich zu den unten genannten Pins braucht das LoRa-Modul SPI!
    (SCK/MISO/MOSI sind je nach ESP32-C3-Board unterschiedlich – wir nutzen hier die
     Standard-SPI-Pins des Boards bzw. die, die in der Board-Definition hinterlegt sind.)

  RFM95W Pin   ->  ESP32-C3 Pin
  --------------------------------
  NSS / CS     ->  GPIO 3     (ss)
  RESET        ->  GPIO 2     (rst)
  DIO0         ->  GPIO 1     (dio0)
  VCC          ->  3.3V       (NICHT 5V!)
  GND          ->  GND

  SPI (Board-abhängig):
  - SCK  -> SCK-Pin des Boards
  - MISO -> MISO-Pin des Boards
  - MOSI -> MOSI-Pin des Boards

Wichtige Einstellungen:
  - Frequenz: 868E6 (Europa)
  - SyncWord: 0xF3  (muss beim Sender identisch sein, sonst "hören" wir nix)
  - Baudrate: 115200

Autor:
  - AMC Team
  - Stand: 15.1.2026
====================================================================
*/

#include <SPI.h>
#include <LoRa.h>

// ------------------------------
// LoRa-Pins (RFM95W / SX1276)
// ------------------------------
#define ss   3   // NSS / CS: Chip Select vom LoRa-Modul
#define rst  2   // Reset-Pin vom LoRa-Modul
#define dio0 1   // DIO0: Interrupt-Pin (RX/TX Done)

// ------------------------------
// Setup: läuft einmal beim Start
// ------------------------------
void setup() {
  // Serielle Ausgabe zum PC (Serial Monitor)
  Serial.begin(115200);

  // Manche Boards brauchen das, bis USB-Serial "da" ist
  while (!Serial);

  Serial.println("LoRa Receiver (AMC CanSat 2026)");

  // LoRa-Bibliothek: sagt ihr, welche Pins wir verwenden
  LoRa.setPins(ss, rst, dio0);

  // LoRa starten (868 MHz für EU). Wenn’s nicht klappt -> weiter probieren.
  // Typische Fehlerursachen:
  //  - falsche Pins
  //  - LoRa-Modul nicht mit 3.3V versorgt
  //  - SPI-Leitungen falsch / nicht verbunden
  Serial.print("LoRa init @ 868 MHz");
  while (!LoRa.begin(868E6)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" OK");

  // SyncWord setzen: verhindert, dass wir fremde LoRa-Sender "sehen"
  // (MUSS exakt gleich sein wie beim Sender)
  LoRa.setSyncWord(0xF3);
  Serial.println("SyncWord set to 0xF3");

  Serial.println("Ready. Waiting for packets...");
}

// ------------------------------
// Loop: läuft ständig
// ------------------------------
void loop() {
  // Prüfen, ob ein Paket angekommen ist
  int packetSize = LoRa.parsePacket();
  if (!packetSize) {
    // kein Paket -> sofort raus (spart Zeit)
    return;
  }

  // Paket ist da: kompletten Payload als String lesen
  // (Der Sender schickt eine CSV-Zeile als Text.)
  String line = LoRa.readString();

  // RSSI ausgeben: Signalstärke des empfangenen Pakets
  Serial.print("RSSI=");
  Serial.print(LoRa.packetRssi());
  Serial.print(" | ");

  // CSV-Zeile ausgeben (kann man am PC direkt loggen / copy-pasten)
  Serial.println(line);
}
