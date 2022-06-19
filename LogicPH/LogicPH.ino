#include <WiFi.h> // Library Wifi
#include <PubSubClient.h> // Library PubSubClient
#include <Wire.h> // Library Wire
#include <LiquidCrystal_I2C.h> // Library LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set inisialisasi lcd

WiFiServer server(80); // Server Default
WiFiClient espClient; // ESPClient
PubSubClient client(espClient); // PubSubClient

#define mqtt_server "i-ot.net" // MQTT Server
#define mqtt_port 1883 // MQTT Port
#define mqtt_username "" // MQTT Username
#define mqtt_password "" // MQTT Password
const int PoPin = 35; //Pin Po

const char *ssid = ""; // nama wifi router
const char *password = ""; // password wifi router

//Inisialisasi variabel global
String stats; //Inisialisasi tipe String
float x, xi, xj, xs, m, y; //Inisialisasi tipe float
int pHRead; //Inisialisasi tipe int

// Fungsi setup
void setup(){
  lcd.init(); // memulai komunikasi serial dengan LCD
  Serial.begin(9600); // memulai komunikasi serial dengan baud rate 9600
  connectWiFi(); // memanggil fungsi connectWiFi
  connectIoT(); // memanggil fungsi connectIoT (i-ot.net)
  LCDStart(); // memanggil fungsi LCDStart
  delay(5000); LCDWelcome(); delay(5000); // memanggil fungsi LCDWelcome
  pinMode(PoPin, INPUT); //Input Data
}

// Fungsi loop
void loop(){ cekpH(); } // dilakukan monitoring secara terus menerus

// Connect WiFi
void connectWiFi(){
  Serial.println("==================================================================================");
  Serial.print("Menghubungkan ke wifi...\nWifi yang dipakai : "); Serial.println(ssid); WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){ delay(500); Serial.print("."); }
  Serial.println(""); Serial.println("WiFi connected"); Serial.println(WiFi.localIP()); delay(1000);
}

// Connect IoT
void connectIoT(){
  client.setServer(mqtt_server, mqtt_port); 
  while (!client.connected()) { Serial.println("\nMenyambungkan ke i-ot.net..."); delay(2000);
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) { Serial.println("Status : Berhasil terkoneksi"); } 
    else { Serial.println("Status : Gagal terkoneksi"); }
  } delay(500);
  Serial.print("==================================================================================\n\n");
}

// Fungsi ReadPH ESP32
void ReadPH(){
  char pHresult[4];client.loop();
  pHRead = analogRead(PoPin);
  x = 3.3/4095.0*pHRead; //Voltage
  xi = 2.5; //VpH 7
  xj = 3.1; //VpH 4
  xs = (xj-xi)/(7-4); //pH Step
  y = 7.00 + ((xi-x)/xs); //pH Result
  delay(5000); //Tunda 5 detik
  dtostrf(y, 4, 2, pHresult); //float -> String
  client.publish("detect",pHresult); //publish nilai pH
}

// Fungsi cekpH
void cekpH(){
  ReadPH(); // Memanggil fungsi ReadPH
  if(y >= 0 && y < 4) { stats = "Asam Kuat"; }
  else if(y >= 4 && y < 6) { stats = "Asam Lemah"; }
  else if(y >= 6 && y < 8) { stats = "Netral"; }
  else if(y >= 8 && y < 10) { stats = "Basa Lemah"; }
  else if(y >= 10 && y <= 14) { stats = "Basa Kuat"; }
  else if(y < 0 && y > 14){ stats = "Error...."; }
  Serial.println("Publish data ke i-ot.net..."); Serial.print("Voltage: "); Serial.print(x); Serial.print(" _ pH Air = "); Serial.print(y); Serial.print(" _ Status = ");  Serial.print(stats); Serial.print("\n\n"); delay(1000);
  LCDHasil(); // Memanggil fungsi LCD Hasil
}

// Fungsi LCDStart
void LCDStart(){
  lcd.clear(); lcd.backlight(); lcd.setCursor(1,0); lcd.print("Memulai"); lcd.setCursor(1,1); lcd.print("Sistem pH..."); delay(1000);
}

// Fungsi LCDWelcome
void LCDWelcome(){
  lcd.clear(); lcd.backlight(); lcd.setCursor(1,0); lcd.print("Welcome to"); lcd.setCursor(1,1); lcd.print("PYMONIPH...."); delay(1000);
}

// Fungsi LCDHasil
void LCDHasil(){
  lcd.clear(); lcd.backlight(); lcd.setCursor(1,0); lcd.print("pH Air : "+ String(y)); lcd.setCursor(1,1); lcd.print("" + String(stats)); delay(5000);
}
