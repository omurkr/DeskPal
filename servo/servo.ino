#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial espSerial(3, 2); // RX, TX

Servo kafaServo;

String gelenKomut = "";
const int servoPin = 9;

String aktifMod = "MODE_serbest"; // Başlangıç modu

unsigned long sonSerbestHareketZamani = 0;
unsigned long serbestHareketAraligi = 0; // Bu rastgele ayarlanacak

unsigned long sonMolaHareketZamani = 0;
unsigned long molaHareketAraligi = 180000; // 3 dakikada bir

unsigned long lastDersHareket = 0;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  kafaServo.attach(servoPin);
  kafaServo.write(80); // Başlangıç pozisyonu
  Serial.println("Nano hazır...");

  // Başlangıç için rastgele aralık belirle
  serbestHareketAraligi = random(30000, 90000); // 30-90 saniye
}

void loop() {
  // ESP'den gelen komutları dinle
  while (espSerial.available()) {
    char c = espSerial.read();
    if (c == '\n') {
      gelenKomut.trim();

      if (gelenKomut == "MODE_serbest") {
        aktifMod = "MODE_serbest";
        Serial.println("Serbest mod aktif.");
        // Her mod geçişinde yeni aralık ata
        serbestHareketAraligi = random(30000, 90000);
      } else if (gelenKomut == "MODE_ders") {
        aktifMod = "MODE_ders";
        Serial.println("Ders mod aktif.");
      } else if (gelenKomut == "MODE_mola") {
        aktifMod = "MODE_mola";
        Serial.println("Mola mod aktif.");
      }
      gelenKomut = "";
    } else {
      gelenKomut += c;
    }
  }

  // Aktif moda göre davranışları çağır
  if (aktifMod == "MODE_serbest") {
    serbestMod();
  } else if (aktifMod == "MODE_ders") {
    dersModu();
  } else if (aktifMod == "MODE_mola") {
    molaModu();
  }
}

void serbestMod() {
  static unsigned long sonSerbestHareketZamani = 0;
  static unsigned long beklemeSuresi = random(15000, 45000); // 15-45 saniye arası rastgele bekleme

  unsigned long simdi = millis();

  if (simdi - sonSerbestHareketZamani > beklemeSuresi) {
    Serial.println("Serbest mod hareketi");

    int merkez = 80;
    int sapma = 30;
    int gecikme = 30;
    int hareketTipi = random(0, 4);  // Artık 4 farklı hareket var

    if (hareketTipi == 0) {
      // === Sağa veya sola yavaş bakma ===
      int yon = random(0, 2); // 0: sola, 1: sağa
      int hedefAci = (yon == 0) ? merkez - sapma : merkez + sapma;

      for (int aci = merkez; aci != hedefAci; aci += (aci < hedefAci ? 1 : -1)) {
        kafaServo.write(aci);
        delay(gecikme);
      }
      delay(400);
      for (int aci = hedefAci; aci != merkez; aci += (aci < merkez ? 1 : -1)) {
        kafaServo.write(aci);
        delay(gecikme);
      }

    } else if (hareketTipi == 1) {
      // === Minik titreşim ===
      int minSalini = random(3, 7);
      for (int i = 0; i < 2; i++) {
        kafaServo.write(merkez + minSalini); delay(120);
        kafaServo.write(merkez - minSalini); delay(120);
        kafaServo.write(merkez + minSalini); delay(120);
        kafaServo.write(merkez - minSalini); delay(120);
      }
      kafaServo.write(merkez);

    } else if (hareketTipi == 2) {
      // === Meraklı eğilme ===
      int hedef = merkez - 10;
      for (int i = merkez; i >= hedef; i--) {
        kafaServo.write(i);
        delay(25);
      }
      delay(300);
      for (int i = hedef; i <= merkez; i++) {
        kafaServo.write(i);
        delay(25);
      }

    } else if (hareketTipi == 3) {
      // === Yavaş sağ-sol gözlemleme ===
      for (int i = merkez - sapma; i <= merkez + sapma; i++) {
        kafaServo.write(i);
        delay(25);
      }
      delay(300);
      for (int i = merkez + sapma; i >= merkez - sapma; i--) {
        kafaServo.write(i);
        delay(25);
      }
    }

    // Yeni rastgele süre belirle
    beklemeSuresi = random(15000, 45000);
    sonSerbestHareketZamani = simdi;
  }
}
void dersModu() {
  unsigned long simdi = millis();

  if (simdi - lastDersHareket > 30000) {
    Serial.println("Ders mod hareketi");
    for (int i = 0; i < 3; i++) {
      kafaServo.write(78);
      delay(200);
      kafaServo.write(82);
      delay(200);
    }
    kafaServo.write(80);
    lastDersHareket = simdi;
  }
}

void molaModu() {
  unsigned long simdi = millis();

  if (simdi - sonMolaHareketZamani > molaHareketAraligi) {
    Serial.println("Mola mod hareketi");

    int merkez = 80;
    int gecikme = 40;

    // === Hareket Tipi Seç ===
    int hareketTipi = random(0, 2); // 0 veya 1

    if (hareketTipi == 0) {
      // === Tip 1: Sağ-sol yavaş salınım ===
      for (int i = 0; i < 2; i++) {
        for (int aci = merkez; aci <= merkez + 20; aci++) {
          kafaServo.write(aci);
          delay(gecikme);
        }
        for (int aci = merkez + 20; aci >= merkez - 20; aci--) {
          kafaServo.write(aci);
          delay(gecikme);
        }
        for (int aci = merkez - 20; aci <= merkez; aci++) {
          kafaServo.write(aci);
          delay(gecikme);
        }
      }

    } else {
      // === Tip 2: Nefes alıyormuş gibi küçük ileri geri eğilme ===
      for (int i = 0; i < 2; i++) {
        kafaServo.write(merkez + 5);
        delay(600);
        kafaServo.write(merkez - 5);
        delay(600);
      }
      kafaServo.write(merkez); // Ortaya dön
    }

    sonMolaHareketZamani = simdi;
  }
}
