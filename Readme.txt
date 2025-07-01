# Mini DeskPal

Mini DeskPal, öğrencilerin çalışma masalarında onlara arkadaşlık eden, görevlerini hatırlatan ve motivasyon sağlayan Arduino tabanlı masaüstü asistan robotudur. Proje, ESP32, OLED ekran, DFPlayer ve mikrofon gibi bileşenlerle zenginleştirilmiş; Firebase üzerinden görev yönetimi, sesli uyarılar ve göz animasyonları gibi özellikler sunmaktadır.

---

## Özellikler

- **Görev ve Hatırlatıcı Sistemi:** Firebase ile görevler kaydedilip zamanında sesli hatırlatmalar yapılır.  
- **Duygusal Tepki ve Motivasyon:** Öğrencinin ruh haline uygun motivasyon cümleleri sesli olarak sunulur.  
- **OLED Ekran Göz Animasyonları:** Gözler dinamik animasyonlarla gerçekçi ifadeler verir.  
- **Mod Tabanlı Çalışma:** Ders, mola ve serbest modlarıyla farklı ihtiyaçlara uyum sağlar.  
- **Sesle Kontrol ve Mikrofon Desteği:** Sesli komutlar alınabilir ve analiz edilebilir.  
- **Arduino ve ESP32 Tabanlı:** Kolayca geliştirilebilir ve genişletilebilir mimari.

---

## Donanım Bileşenleri

- ESP32-WROOM-32  
- 0.96" OLED Ekran  
- DFPlayer Mini MP3 Çalar  
- MAX9814 Mikrofon Modülü  
- Mini Hoparlör (8Ω, 1W)  
- Arduino Nano  
- SG90 Servo Motorlar (2 adet)

---

## Kurulum

1. Proje dosyalarını klonlayın veya indirin.  
2. Arduino IDE’de `Mini-DeskPal.ino` dosyasını açın.  
3. Gerekli kütüphaneleri yükleyin:  
   - `Firebase ESP32`  
   - `DFPlayer Mini`  
   - `Adafruit SSD1306` ve `GFX`  
4. Firebase projenizi oluşturup `firebaseConfig.h` dosyasına API anahtarlarını ekleyin.  
5. Donanımı şemaya uygun şekilde bağlayın.  
6. Kodu ESP32 ve Arduino Nano’ya yükleyin.  
7. Projeyi çalıştırın ve Firebase üzerinden görev eklemeye başlayın.

---

## Kullanım

- Projeye yeni görevler eklemek için Firebase Web Konsolu veya arayüz kullanılabilir.  
- DeskPal, görev saatlerinde sesli hatırlatmalar yapar ve göz animasyonları ile durumunu gösterir.  
- Ders modunda motivasyon cümleleri ile destek verir.  
- Sesli komutlar ile modu değiştirebilir veya mola verebilirsiniz.

---