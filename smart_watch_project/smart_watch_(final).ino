#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

SoftwareSerial BTSerial(10, 11);
// OLED ekran boyutu
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool state = false;
unsigned long lastReceivedTime = 0;
const unsigned long timeoutDuration = 70000; // 70 saniye (70.000 ms)

int arayuzSayac = 0;

void setup() 
{
  Serial.begin(9600);
  BTSerial.begin(9600);
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
  if (BTSerial.available())
  {
    if(!state)
    {
      oledYazdir("Baglanti Basarili!");
      state = true;
      delay(1000);
    }
    String gelenVeri = BTSerial.readStringUntil('\n');
    Serial.println("Gelen saat: " + gelenVeri);
    if(gelenVeri=="b"){gelenVeri="...";arayuzSayac+=1;}
    gelenVeri.replace("\"", "");  // Eğer veride çift tırnak varsa sil

    if(arayuzSayac==0)
    {
      oledYazdir(gelenVeri);
    }
    else if(arayuzSayac == 1)
    {
      arayuzIki(gelenVeri);
    }
    else if(arayuzSayac == 2)
    {
      arayuzUc(gelenVeri);
    }
    else if(arayuzSayac == 3)
    {
      arayuzDort(gelenVeri);
    }  
    else{arayuzSayac=0;}

    lastReceivedTime = millis();  // Son veri zamanını güncelle
  }
  if (state && millis() - lastReceivedTime > timeoutDuration)
  {
    oledYazdir("Baglanti Koptu!");
    state = false;
    delay(1000);
    oledYazdir("Akilli Saat V1");
  }
  if(state){oledBluetoothSimgesi();}
}

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
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(cursorX, cursorY);
  display.println(mesaj);
  display.display();
}

void arayuzIki(String mesaj)
{
  int charWidth = 6 * 2; 
  int charHeight = 8 * 2;
  int textPixelWidth = mesaj.length() * charWidth;
  int textPixelHeight = charHeight;
  int cursorX = (SCREEN_WIDTH - textPixelWidth) / 2;
  int cursorY = (SCREEN_HEIGHT - textPixelHeight) / 2;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(cursorX, cursorY);
  display.println(mesaj);
  display.display();
}

void arayuzUc(String mesaj)
{
  int hour = mesaj.substring(0, 2).toInt();
  int minute = mesaj.substring(3, 5).toInt();
  display.clearDisplay();

  int centerX = SCREEN_WIDTH / 2;   // 64
  int centerY = SCREEN_HEIGHT / 2;  // 16
  int radiusX = 25;  // yatay yarıçap
  int radiusY = 15;  // dikey yarıçap

  for (int angle = 0; angle < 360; angle++) {
    float rad = radians(angle);
    int x = centerX + cos(rad) * radiusX;
    int y = centerY + sin(rad) * radiusY;
    display.drawPixel(x, y, SSD1306_WHITE);
  }

  // Saat kolu
  float hourAngle = ((hour % 12) + minute / 60.0) * 30.0 - 90.0;
  float hourRad = radians(hourAngle);
  int hourX = centerX + cos(hourRad) * (radiusX * 0.5);
  int hourY = centerY + sin(hourRad) * (radiusY * 0.5);
  display.drawLine(centerX, centerY, hourX, hourY, SSD1306_WHITE);

  // Dakika kolu
  float minuteAngle = (minute * 6.0) - 90.0;
  float minuteRad = radians(minuteAngle);
  int minX = centerX + cos(minuteRad) * (radiusX * 0.8);
  int minY = centerY + sin(minuteRad) * (radiusY * 0.8);
  display.drawLine(centerX, centerY, minX, minY, SSD1306_WHITE);

  display.display();
}

void arayuzDort(String mesaj)
{
  int hour = mesaj.substring(0, 2).toInt();
  int minute = mesaj.substring(3, 5).toInt();

  display.clearDisplay();

  // Analog saat için merkez koordinatlar
  int centerX = 32;   // Sol tarafta
  int centerY = SCREEN_HEIGHT / 2;
  int radiusX = 25;
  int radiusY = 15;

  // Elips çiz (analog saat)
  for (int angle = 0; angle < 360; angle++) {
    float rad = radians(angle);
    int x = centerX + cos(rad) * radiusX;
    int y = centerY + sin(rad) * radiusY;
    display.drawPixel(x, y, SSD1306_WHITE);
  }

  // Saat kolu
  float hourAngle = ((hour % 12) + minute / 60.0) * 30.0 - 90.0;
  float hourRad = radians(hourAngle);
  int hourX = centerX + cos(hourRad) * (radiusX * 0.5);
  int hourY = centerY + sin(hourRad) * (radiusY * 0.5);
  display.drawLine(centerX, centerY, hourX, hourY, SSD1306_WHITE);

  // Dakika kolu
  float minuteAngle = (minute * 6.0) - 90.0;
  float minuteRad = radians(minuteAngle);
  int minX = centerX + cos(minuteRad) * (radiusX * 0.8);
  int minY = centerY + sin(minuteRad) * (radiusY * 0.8);
  display.drawLine(centerX, centerY, minX, minY, SSD1306_WHITE);

  // Dijital saat (sağda)
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Ekranın sağ tarafında konumlandır
  int textX = 80;
  int textY = (SCREEN_HEIGHT - 8) / 2;  // 16 piksel yükseklik

  display.setCursor(textX, textY);
  display.print(mesaj);

  display.display();
}

void oledBluetoothSimgesi()
{
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Sağ üst köşe için referans noktası (az biraz içeri alınmış)
  int baseX = SCREEN_WIDTH - 10; // 128 - 10 = 118
  int baseY = 2;                 // 2 piksel aşağıdan başlasın

  // Küçük Bluetooth simgesi çizimi (12x7 civarı)
  display.drawLine(baseX, baseY, baseX, baseY + 10, SSD1306_WHITE); // Dikey çizgi

  display.drawLine(baseX, baseY + 5, baseX + 4, baseY + 2, SSD1306_WHITE); // Üst çapraz
  display.drawLine(baseX, baseY + 5, baseX + 4, baseY + 8, SSD1306_WHITE); // Alt çapraz

  display.drawLine(baseX, baseY, baseX + 4, baseY + 3, SSD1306_WHITE);     // Üst dış çapraz
  display.drawLine(baseX, baseY + 10, baseX + 4, baseY + 7, SSD1306_WHITE); // Alt dış çapraz

  display.display();
}