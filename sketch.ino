#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define DHTPIN 2       // Pin data DHT22 terhubung ke pin 2
#define DHTTYPE DHT22  // Menggunakan sensor DHT22

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Alamat I2C 0x27, LCD 16x2

// Karakter khusus
byte termometer[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01110
};

byte tetesan[8] = {
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b10001,
  0b01110
};

byte derajat[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte animasi[4][8] = {
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111}
};

int displayMode = 0;
unsigned long lastUpdate = 0;
int animStep = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  // Buat karakter khusus
  lcd.createChar(0, termometer);
  lcd.createChar(1, tetesan);
  lcd.createChar(2, derajat);
  lcd.createChar(4, animasi[0]);
  
  // Animasi pembuka
  tampilkanAnimasiPembuka();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Update setiap 1600ms (seperti kode asli)
  if (currentMillis - lastUpdate >= 1600) {
    lastUpdate = currentMillis;
    
    float kelembaban = dht.readHumidity();
    float suhu = dht.readTemperature();
    
    if (isnan(kelembaban) || isnan(suhu)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor Error!");
      return;
    }
    
    // Ganti mode tampilan setiap update (hanya 2 mode)
    displayMode = (displayMode + 1) % 2;
    
    switch(displayMode) {
      case 0:
        tampilkanModeBiasa(suhu, kelembaban);
        break;
      case 1:
        tampilkanModeBar(suhu, kelembaban);
        break;
    }
    
    // Output ke Serial Monitor
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" °C, Kelembaban: ");
    Serial.print(kelembaban);
    Serial.println(" %");
  }
  
  // Animasi bar (update lebih cepat)
  if (displayMode == 1 && (currentMillis - lastUpdate >= 500)) {
    updateAnimasiBar(dht.readTemperature());
  }
}

void tampilkanAnimasiPembuka() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("DIGITAL");
  lcd.setCursor(2, 1);
  lcd.print("THERMOMETER");
  delay(1000);
  
  // Animasi loading
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.write(255); // Karakter blok penuh
    delay(100);
  }
  
  lcd.clear();
}

void tampilkanModeBiasa(float suhu, float kelembaban) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(0); // Ikon termometer
  lcd.print(" Suhu: ");
  lcd.print(suhu, 1);
  lcd.write(2); // Simbol derajat
  lcd.print("C");
  
  lcd.setCursor(0, 1);
  lcd.write(1); // Ikon tetesan
  lcd.print(" Kelemb: ");
  lcd.print(kelembaban, 1);
  lcd.print("%");
}

void tampilkanModeBar(float suhu, float kelembaban) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(suhu, 1);
  lcd.write(2);
  lcd.print("C");
  
  lcd.setCursor(0, 1);
  lcd.print("Level: ");
  
  // Posisi awal bar
  animStep = 0;
  updateAnimasiBar(suhu);
}

void updateAnimasiBar(float suhu) {
  // Update animasi bar
  lcd.createChar(4, animasi[animStep]);
  animStep = (animStep + 1) % 4;
  
  // Buat bar berdasarkan suhu
  int barLength = map(suhu, 0, 50, 0, 8);
  lcd.setCursor(8, 1);
  for (int i = 0; i < barLength; i++) {
    lcd.write(255); // Karakter blok penuh
  }
  
  if (barLength < 8) {
    lcd.setCursor(8 + barLength, 1);
    lcd.write(4); // Karakter animasi
  }
}