#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

SoftwareSerial BTSerial(10, 11);
// OLED ekran boyutu
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool state = false;

// YENİ DİNAMİK SAAT VE BAĞLANTI DEĞİŞKENLERİ
unsigned int currentHour = 0;
unsigned int currentMinute = 0;
unsigned long previousMillis = 0;
const long interval = 1000;
unsigned long connectionStartTime = 0; // Bağlantı süresini takip etmek için eklendi

unsigned long lastReceivedTime = 0;
const unsigned long timeoutDuration = 70000; // 70 saniye (70.000 ms)

// **********************************
// FONKSİYON PROTOTİPLERİ
void oledYazdir(String mesaj);
void arayuzSaat();
void oledBluetoothSimgesi();
// **********************************

void setup() 
{
Wire.begin(); // I2C iletişimi başlatıldı
Wire.setClock(100000); // 3.3V Pro Mini kararlılığı için hız düşürüldü

Serial.begin(9600);
BTSerial.begin(9600); // HC-05 ayarı ile uyumlu 9600 Baud
// OLED ekran başlatma
if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
{
Serial.println("SSD1306 ekran başlatılamadı!");
while (true);
}

oledYazdir("Akilli Saat V1");
delay(2000);
oledYazdir("Hos Geldiniz!!!");
delay(2000);
oledYazdir("Baglanti Bekleniyor..");
}

void loop()
{
unsigned long currentMillis = millis();

// 1. BLUETOOTH VERİ ALMA VE GÜNCELLEME
if (BTSerial.available())
{
if(!state)
{
oledYazdir("Baglanti Basarili!");
state = true;
connectionStartTime = millis(); // Bağlantı başladığı anı kaydet
delay(1000);
}

String gelenVeri = BTSerial.readStringUntil('\r'); // Mobil uygulamadan gelen karakteri oku

// VERİ TEMİZLEME
gelenVeri.trim(); 
gelenVeri.replace("\"", ""); 

Serial.println("Gelen saat: " + gelenVeri);

// Saat Bilgisi Format Kontrolü ve Güncelleme
if (gelenVeri.length() >= 5 && gelenVeri.indexOf(':') == 2)
{ 
currentHour = gelenVeri.substring(0, 2).toInt();
currentMinute = gelenVeri.substring(3, 5).toInt();

lastReceivedTime = millis(); // Geçerli veri geldiyse zamanı güncelle
} 
else if(gelenVeri.length() > 0 && gelenVeri != "b") 
{ 
// Saat formatında olmayan diğer mesajlar (bildirimler/metinler)
oledYazdir(gelenVeri);
}
}

// 2. YEREL SAAT İLERLEME VE ÇİZİM (Her saniye çalışır)
if (currentMillis - previousMillis >= interval)
{ 
previousMillis = currentMillis;

// Yerel Saati İlerletme Mantığı
currentMinute++;
if (currentMinute >= 60) { currentMinute = 0; currentHour++; }
if (currentHour >= 24) { currentHour = 0; }

// Yalnızca bağlantı varsa saati çiz
if(state){ arayuzSaat(); } 
}

// 3. BAĞLANTI KOPTU KONTROLÜ
if (state && millis() - lastReceivedTime > timeoutDuration)
{
oledYazdir("Baglanti Koptu!");
state = false;
connectionStartTime = 0; // Bağlantı koptu, süreyi sıfırla
delay(1000);
oledYazdir("Akilli Saat V1");
}

if(state){oledBluetoothSimgesi();}
delay(5); // Kararlılık gecikmesi
}

// **********************************
// YARDIMCI FONKSİYONLAR
// **********************************

void oledYazdir(String mesaj)
{
int charWidth = 6;
int charHeight = 8;
int textPixelWidth = mesaj.length() * charWidth;
int textPixelHeight = charHeight;
int cursorX = (SCREEN_WIDTH - textPixelWidth) / 2;
int cursorY = (SCREEN_HEIGHT - textPixelHeight) / 2;

display.display();
display.clearDisplay();
display.setTextColor(SSD1306_WHITE);
display.setTextSize(1);

display.setCursor(cursorX, cursorY);
display.println(mesaj);
display.display();
}

// YENİ DİNAMİK SAAT ARANÜZÜ
void arayuzSaat()
{
display.clearDisplay();

// 1. ANALOG SAAT (Sol Taraf - Boyutlar Küçültüldü)
int centerX = 32; 
int centerY = SCREEN_HEIGHT / 2;
int radiusX = 12; // Yarıçap küçültüldü
int radiusY = 12; // Yarıçap küçültüldü

// Analog Elips çiz
for (int angle = 0; angle < 360; angle++) {
float rad = radians(angle);
int x = centerX + cos(rad) * radiusX;
int y = centerY + sin(rad) * radiusY;
display.drawPixel(x, y, SSD1306_WHITE);
}

// Saat ve Dakika Kollarını Hesapla
float hourAngle = ((currentHour % 12) + currentMinute / 60.0) * 30.0 - 90.0;
float hourRad = radians(hourAngle);
int hourX = centerX + cos(hourRad) * (radiusX * 0.7); // Kol uzunluğu ayarlandı
int hourY = centerY + sin(hourRad) * (radiusY * 0.7);
display.drawLine(centerX, centerY, hourX, hourY, SSD1306_WHITE);

float minuteAngle = (currentMinute * 6.0) - 90.0;
float minuteRad = radians(minuteAngle);
int minX = centerX + cos(minuteRad) * (radiusX * 0.9); // Kol uzunluğu ayarlandı
int minY = centerY + sin(minuteRad) * (radiusY * 0.9);
display.drawLine(centerX, centerY, minX, minY, SSD1306_WHITE);

// 2. DİJİTAL SAAT (Sağ Taraf - Büyük Font)
display.setTextColor(SSD1306_WHITE);
display.setTextSize(2); 

// HH:MM formatını oluştur
String hourStr = (currentHour < 10 ? "0" + String(currentHour) : String(currentHour));
String minuteStr = (currentMinute < 10 ? "0" + String(currentMinute) : String(currentMinute));
String timeStr = hourStr + ":" + minuteStr;

int textX = 70; 
int textY = (SCREEN_HEIGHT - 16) / 2; 

display.setCursor(textX, textY);
display.print(timeStr);

// 3. BAĞLANTI SÜRESİ (SAĞ ALT KÖŞE)
if (state && connectionStartTime > 0) {
    unsigned long uptimeSeconds = (millis() - connectionStartTime) / 1000;
    
    int upMinutes = uptimeSeconds / 60;
    int upSeconds = uptimeSeconds % 60;

    String uptimeStr = "C:" + String(upMinutes) + "d " + String(upSeconds) + "s";
    
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    // Konumlandırma: Ekranın sağ alt köşesi
    int textXUptime = 70; // Dijital saatin altına
    int textYUptime = 25; 

    display.setCursor(textXUptime, textYUptime);
    display.print(uptimeStr);
}

display.display();
}

void oledBluetoothSimgesi()
{
display.setTextColor(SSD1306_WHITE);
display.setTextSize(1);

// Sağ üst köşe için referans noktası (az biraz içeri alınmış)
int baseX = SCREEN_WIDTH - 10; // 128 - 10 = 118
int baseY = 2; // 2 piksel aşağıdan başlasın

// Küçük Bluetooth simgesi çizimi (12x7 civarı)
display.drawLine(baseX, baseY, baseX, baseY + 10, SSD1306_WHITE); // Dikey çizgi

display.drawLine(baseX, baseY + 5, baseX + 4, baseY + 2, SSD1306_WHITE); // Üst çapraz

display.drawLine(baseX, baseY + 5, baseX + 4, baseY + 8, SSD1306_WHITE); // Alt çapraz

display.drawLine(baseX, baseY, baseX + 4, baseY + 3, SSD1306_WHITE); // Üst dış çapraz

display.drawLine(baseX, baseY + 10, baseX + 4, baseY + 7, SSD1306_WHITE); // Alt dış çapraz

display.display();
}