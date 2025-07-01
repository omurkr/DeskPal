#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <FluxGarage_RoboEyes.h>
roboEyes gozler;

#include <DFRobotDFPlayerMini.h>
HardwareSerial mp3Serial(1);
DFRobotDFPlayerMini mp3Calayici;

const int mikrofonPin = 34;

// Wifi bilgileri
const char* wifiAdi = "SUPERONLINE-WiFi_1753";
const char* wifiSifre = "MARVX3YLKYKX";

// NTP server ayarlari
const char* ntpServeri = "pool.ntp.org";
const long gmtOffsetSaniye = 3 * 3600;
const int yazSaatiOffsetSaniye = 0;

// Firebase temel adres
String firebaseAdresi = "https://deskpal-8e7a7-default-rtdb.firebaseio.com";

// Zamanlama ve mod degiskenleri
String aktifMod = "serbest";
unsigned long sonVeriCekmeZamani = 0;
const unsigned long veriCekmeAraligi = 5000;  // 5 saniye

// Mikrofon esik ve kalibrasyon degiskenleri
int esikTaban = 0;
int esikMarj = 400;
int esikDegeri = 0;

unsigned long sonKalibrasyonZamani = 0;
const unsigned long kalibrasyonAraligi = 180000;

bool konusuyorMu = false;
unsigned long sonKonusmaZamani = 0;
const unsigned long konusmaBitisTimeout = 3000;

// Yeni değişkenleri tanımla
static String sonCalinanSaat = "";  // Son sesin çalındığı saat

bool yeniSesBekleniyor = false;
unsigned long sonTetkikZamani = 0;
const unsigned long tetkikCooldown = 10000;

unsigned long sonSesTetiklemeZamani = 0;
const unsigned long sesTetikBekleme = 10000;

// Ses analiz degiskenleri
int valleyThreshold = 1000;
int diffThreshold = 500;
unsigned long recordTime = 2000;

// Mimik Değişkenleri
bool mimikAktif = false;
unsigned long mimikBaslamaZamani = 0;
unsigned long sonMimikZamani = 0;
unsigned long sonrakiMimikSuresi = 15000;  // İlk mimik 15sn sonra gelsin
const unsigned long mimikSuresi = 5000;    // 5 saniye mimik süresi
const unsigned long mimikAralikMin = 30000;  // 30 saniye minimum aralık
const unsigned long mimikAralikMax = 60000;  // 60 saniye maksimum aralık

// Nano ile haberleşme için seri port (RX=4, TX=5)
HardwareSerial nanoSerial(2);

// Fonksiyon prototipleri
void wifiBaglan();
void zamanAyarla();
void ekranBaslat();
void gozleriBaslat();
void mp3CalayiciyiBaslat();
void mikrofonKalibrasyonuYap();
int zamanStringToDakika(const String &zaman);
void firebaseVeriCekVeModBelirle(struct tm zamanBilgisi);
void modDavranisiGerceklestir();
void recordSequence();
void rastgeleMimikGoster();
void serbestMod();
void dersModu();
void molaModu();

void setup() {
  Serial.begin(115200);

  // Nano seri başlatılıyor
  nanoSerial.begin(9600, SERIAL_8N1, 13, 14);

  wifiBaglan();
  zamanAyarla();

  ekranBaslat();
  gozleriBaslat();
  mp3CalayiciyiBaslat();

  mikrofonKalibrasyonuYap();
  sonKalibrasyonZamani = millis();

  struct tm simdikiZaman;
  if (getLocalTime(&simdikiZaman)) {
    firebaseVeriCekVeModBelirle(simdikiZaman);
    sonVeriCekmeZamani = millis();
  }
}

void loop() {
  struct tm simdikiZaman;
  if (getLocalTime(&simdikiZaman)) {
    if (millis() - sonVeriCekmeZamani > veriCekmeAraligi) {
      firebaseVeriCekVeModBelirle(simdikiZaman);
      sonVeriCekmeZamani = millis();
    }
  }

  if (millis() - sonKalibrasyonZamani > kalibrasyonAraligi) {
    mikrofonKalibrasyonuYap();
    sonKalibrasyonZamani = millis();
  }

  modDavranisiGerceklestir();
}

void wifiBaglan() {
  WiFi.begin(wifiAdi, wifiSifre);
  Serial.print("WiFi baglaniyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi baglandi!");
}

void zamanAyarla() {
  configTime(gmtOffsetSaniye, yazSaatiOffsetSaniye, ntpServeri);
  struct tm zamanBilgisi;
  if (!getLocalTime(&zamanBilgisi)) {
    Serial.println("Zaman alinamadi!");
  } else {
    Serial.println("Zaman basariyla alindi!");
  }
}

void ekranBaslat() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED ekran baslatilamadi!");
    while (true);
  }
  Serial.println("OLED ekran hazir.");
}

void gozleriBaslat() {
  gozler.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  gozler.setPosition(DEFAULT);
  gozler.setAutoblinker(ON, 3, 2);
  gozler.setIdleMode(ON, 5, 10);
  Serial.println("RoboEyes baslatildi.");
}

void mp3CalayiciyiBaslat() {
  mp3Serial.begin(9600, SERIAL_8N1, 16, 17);
  if (!mp3Calayici.begin(mp3Serial)) {
    Serial.println("DFPlayer baslatilamadi!");
    while (true);
  }
  Serial.println("DFPlayer baslatildi.");
}

void mikrofonKalibrasyonuYap() {
  Serial.println("Mikrofon kalibrasyonu basliyor...");
  long toplam = 0;
  const int ornekSayisi = 100;

  for (int i = 0; i < ornekSayisi; i++) {
    toplam += analogRead(mikrofonPin);
    delay(10);
  }

  esikTaban = toplam / ornekSayisi;
  esikDegeri = esikTaban + esikMarj;

  Serial.print("Gurultu seviyesi: ");
  Serial.println(esikTaban);
  Serial.print("Esik degeri: ");
  Serial.println(esikDegeri);
}

// Zaman stringini dakika cinsine çeviren yardımcı fonksiyon
int zamanStringToDakika(const String &zaman) {
  int saat = zaman.substring(0, 2).toInt();
  int dakika = zaman.substring(3, 5).toInt();
  return saat * 60 + dakika;
}

void firebaseVeriCekVeModBelirle(struct tm zamanBilgisi) {
  gozler.setMood(DEFAULT); 
  char tarihStr[11];
  strftime(tarihStr, sizeof(tarihStr), "%Y-%m-%d", &zamanBilgisi);

  String tamUrl = firebaseAdresi + "/schedule/" + String(tarihStr) + ".json";
  Serial.println("Firebase'den veri cekiliyor: " + tamUrl);

  HTTPClient http;
  http.begin(tamUrl);
  int httpKodu = http.GET();

  if (httpKodu == 200) {
    String veri = http.getString();
    Serial.println("Firebase verisi:");
    Serial.println(veri);

    StaticJsonDocument<1024> jsonDoc;
    DeserializationError hata = deserializeJson(jsonDoc, veri);

    if (hata) {
      Serial.print("JSON parsing hatasi: ");
      Serial.println(hata.f_str());
      http.end();
      return;
    }

    char simdikiSaat[6];
    strftime(simdikiSaat, sizeof(simdikiSaat), "%H:%M", &zamanBilgisi);
    String simdi = String(simdikiSaat);
    simdi.trim();

    int simdiInt = zamanStringToDakika(simdi);

    String yeniMod = "serbest";

    for (JsonPair anahtarDeger : jsonDoc.as<JsonObject>()) {
      String zamanAraligi = anahtarDeger.key().c_str();
      String modAdi = anahtarDeger.value().as<String>();

      int tireIndex = zamanAraligi.indexOf('-');
      if (tireIndex == -1) continue;

      String baslangicSaat = zamanAraligi.substring(0, tireIndex);
      String bitisSaat = zamanAraligi.substring(tireIndex + 2);

      baslangicSaat.trim();
      bitisSaat.trim();

      int baslangicInt = zamanStringToDakika(baslangicSaat);
      int bitisInt = zamanStringToDakika(bitisSaat);

      if (simdiInt >= baslangicInt && simdiInt < bitisInt) {
        yeniMod = modAdi;
        break;
      }
    }

    if (yeniMod != aktifMod) {
      Serial.println("Mod degisti: " + aktifMod + " -> " + yeniMod);

      // Nano'ya mod bilgisini gönder
      String nanoKomut = "MODE_" + yeniMod;
      nanoSerial.println(nanoKomut);

      if (aktifMod == "serbest" && yeniMod == "ders") {
        Serial.println("🔊 Serbest -> Ders sesi çalınıyor");
        mp3Calayici.playFolder(1, random(1, 6));
      }
      else if (aktifMod == "ders" && yeniMod == "mola") {
        Serial.println("🔊 Ders -> Mola sesi çalınıyor");
        mp3Calayici.playFolder(2, random(1, 6));
      }
      else if (aktifMod == "mola" && yeniMod == "ders") {
        Serial.println("🔊 Mola -> Ders sesi çalınıyor");
        mp3Calayici.playFolder(3, random(1, 6));
      }
      else if (aktifMod == "ders" && yeniMod == "serbest") {
        Serial.println("🔊 Ders -> Serbest sesi çalınıyor");
        mp3Calayici.playFolder(4, random(1, 6));
      }

      aktifMod = yeniMod;
      sonCalinanSaat = simdi;
    }
    else {
      Serial.println("Mod degismedi: " + aktifMod);
    }

  } else {
    Serial.print("HTTP Hatasi: ");
    Serial.println(httpKodu);
  }

  http.end();
}

void modDavranisiGerceklestir() {
  if (aktifMod == "serbest") {
    serbestMod();
  } else if (aktifMod == "ders") {
    dersModu();
  } else if (aktifMod == "mola") {
    molaModu();
  }
}

void recordSequence() {
  int micValue = analogRead(mikrofonPin);
  int peakCount = 0;
  int valleyCount = 0;
  int lastValue = micValue;
  unsigned long startTime = millis();
  unsigned long soundDuration = 0;

  while (millis() - startTime < recordTime) {
    micValue = analogRead(mikrofonPin);
    int diff = abs(micValue - lastValue);

    if ((micValue > esikDegeri) && (diff > diffThreshold)) {
      peakCount++;
      lastValue = micValue;
      soundDuration += 10;
    } else if ((micValue < valleyThreshold) && (diff > diffThreshold)) {
      valleyCount++;
      lastValue = micValue;
    }

    delay(10);
  }

  Serial.print("Tepe: "); Serial.println(peakCount);
  Serial.print("Cukur: "); Serial.println(valleyCount);
  Serial.print("Sure: "); Serial.println(soundDuration);

  if (soundDuration < 10 || soundDuration > 30) {
    Serial.println("Gecersiz ses.");
    return;
  }

  if ((peakCount >= 2 && peakCount <= 6) && (valleyCount >= 2 && valleyCount <= 15)) {
    Serial.println("Komut algilandi!");
    int randomFile = random(1, 10);
    Serial.print("Caliniyor: 06/00"); Serial.println(randomFile);
    mp3Calayici.playFolder(6, randomFile);
  } else {
    Serial.println("Gecerli komut degil.");
  }
}

void rastgeleMimikGoster() {
  unsigned long simdi = millis();

  // Eğer aktif değilse ve aralık geçtiyse yeni mimik başlat
  if (!mimikAktif && simdi - sonMimikZamani > sonrakiMimikSuresi) {
    byte mimik = random(0, 1);
    switch (mimik) {
      case 0: gozler.setMood(HAPPY); break;
    }
    mimikAktif = true;
    mimikBaslamaZamani = simdi;
  }

  // Mimik aktifse ve süresi dolduysa kapat
  if (mimikAktif && simdi - mimikBaslamaZamani > mimikSuresi) {
    gozler.setMood(DEFAULT);
    mimikAktif = false;
    sonMimikZamani = simdi;
    sonrakiMimikSuresi = random(mimikAralikMin, mimikAralikMax);
  }
}

void serbestMod() {
  gozler.update();  // Göz animasyonları her zaman çalışmalı
  gozler.setIdleMode(ON, 5, 10);
  rastgeleMimikGoster();
  pinMode(mikrofonPin, INPUT);

  int mikrofonDeger = analogRead(mikrofonPin);

  // Konuşma başladıysa
  if (mikrofonDeger > esikDegeri) {
    konusuyorMu = true;
    sonKonusmaZamani = millis();
  }
  // Konuşma bitmişse (sessizlik belirli süre devam ederse)
  else if (konusuyorMu && millis() - sonKonusmaZamani > konusmaBitisTimeout) {
    konusuyorMu = false;
    yeniSesBekleniyor = true;
    Serial.println("Konusma bitti, yeni ses bekleniyor...");
  }

  // Konuşma devam ediyorsa veya tetikleme bekleme süresi dolmadıysa fonksiyondan çık
  if (konusuyorMu || (millis() - sonTetkikZamani < tetkikCooldown)) {
    return;
  }

  // Yeni ses tetikleme için bekleniyorsa
  if (yeniSesBekleniyor) {
    unsigned long baslangic = millis();
    bool kisaSesAlgilandi = false;

    // 300ms içinde kısa ses var mı diye dinle
    while (millis() - baslangic < 300) {
      mikrofonDeger = analogRead(mikrofonPin);
      if (mikrofonDeger > esikDegeri) {
        kisaSesAlgilandi = true;
        break;
      }
      delay(5);
    }

    if (kisaSesAlgilandi) {
      Serial.println("Yeni kisa ses algilandi, isleme aliniyor...");
      recordSequence();  // Ses kaydı ve analiz fonksiyonunu çağır
      yeniSesBekleniyor = false;
      sonTetkikZamani = millis();
    }
  }
}

void dersModu() {
  gozler.update();
  // Gözleri biraz hareket ettir, ancak küçük sınırlar içinde
  static unsigned long lastMoveTime = 0;
  const unsigned long moveInterval = 2000;  // 2 saniyede bir pozisyon değiştir

  if (millis() - lastMoveTime > moveInterval) {
    gozler.setPosition(DEFAULT);
    gozler.setMood(DEFAULT); 
    gozler.setAutoblinker(ON, 3, 2);
    gozler.setIdleMode(OFF);
    pinMode(mikrofonPin, INPUT_PULLDOWN);
    // Mikrofon aktif olmasın, sesle ilgilenmeyelim (gerekirse başka kısımlarda ayarla)
    lastMoveTime = millis();
  }
}

void molaModu() {
  gozler.update();  // Gözler sürekli güncellenmeli
  gozler.setIdleMode(ON, 5, 10);
  pinMode(mikrofonPin, INPUT);

  unsigned long simdi = millis();
}
