#include <SPI.h>//e biblioteca standard care permite Arduino să trimită date pe pinii SPI (D11/D13 etc.)
#include <Adafruit_GFX.h>//Este biblioteca “motor de desen” pentru ecrane: scris text, poziționare cursor, linii, forme
#include <Adafruit_SSD1306.h>//Știe să inițializeze ecranul și să trimită bufferul de pixeli către OLED.
#include <DHT.h>//se ocupă de tot timing-ul și îți dă direct temperature și humidity

// ---------- OLED SPI (7 pini) ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI 11
#define OLED_CLK  13
#define OLED_DC   9
#define OLED_CS   10
#define OLED_RST  8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

// ---------- Senzori ----------
#define DHTPIN 2
#define DHTTYPE DHT22     // AM2302B = DHT22 module
DHT dht(DHTPIN, DHTTYPE);

#define MQ_PIN A0         // MQ-135 A0 -> A0 Arduino
#define LIGHT_PIN 3       // OKY3106 DO1 -> D3 (digital)

// ---------- LED indicator (rosu + verde) ----------
#define LED_RED   4
#define LED_GREEN 5

// Prag pentru calitate aer (ajustezi dupa valorile tale)
const int MQ_THRESHOLD_BAD = 300; // peste = "calitate slaba"

//  senzorul de lumina e inversat: LOW = BRIGHT
const bool LIGHT_ACTIVE_LOW = true;

// LED: false = catod comun (HIGH aprinde). Daca e anod comun -> true
const bool LED_ACTIVE_LOW = false;

// Rate de actualizare (ms)
const unsigned long DHT_PERIOD_MS = 2000;   // DHT22 corect: nu citi prea des
const unsigned long UI_PERIOD_MS  = 250;    // OLED + Serial + LED update

// Valori “curente”
static float t = 0.0;//temperatura
static float h = 0.0;//umiditatea
static bool dhtOK = false;//spune daca citirea dht a reusit 

void ledOff() {
  if (!LED_ACTIVE_LOW) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
  } else {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
  }
}

void ledGood() { // verde
  if (!LED_ACTIVE_LOW) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  } else {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  }
}

void ledBad() { // rosu
  if (!LED_ACTIVE_LOW) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  } else {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  ledOff();

  pinMode(LIGHT_PIN, INPUT);

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    // NU mai printam altceva pe serial (ca sa pastram doar JSON in loop)
    while (1) {}
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("AIR MONITOR");
  display.println("Start...");
  display.display();
  delay(800);
}

void loop() {
  static unsigned long lastDhtMs = 0;
  static unsigned long lastUiMs  = 0;

  unsigned long now = millis();

  // 1) DHT22 la 2 secunde
  if (now - lastDhtMs >= DHT_PERIOD_MS) {
    float nt = dht.readTemperature();
    float nh = dht.readHumidity();
    if (!isnan(nt) && !isnan(nh)) {
      t = nt; h = nh; dhtOK = true;
    } else {
      dhtOK = false;
    }
    lastDhtMs = now;
  }

  // 2) UI + Serial la 250ms
  if (now - lastUiMs >= UI_PERIOD_MS) {
    int mq = analogRead(MQ_PIN);
    int lightRaw = digitalRead(LIGHT_PIN);
    bool isBright = LIGHT_ACTIVE_LOW ? (lightRaw == LOW) : (lightRaw == HIGH);

    bool qualityGood = (mq < MQ_THRESHOLD_BAD);

    // LED
    if (qualityGood) ledGood();
    else ledBad();

    // OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("AIR MONITOR");

    if (dhtOK) {
      display.print("T: "); display.print(t, 1); display.println(" C");
      display.print("H: "); display.print(h, 0); display.println(" %");
    } else {
      display.println("DHT: eroare");
      display.println(" ");
    }

    display.print("MQ: "); display.println(mq);

    display.print("Lum: ");
    display.println(isBright ? "BRIGHT" : "DARK");

    display.println("----------------");
    display.println(qualityGood ? "Calitate buna" : "Calitate slaba");
    display.display();

    // 3) Serial JSON (o singura linie) - robust pentru Java
    // IMPORTANT: nu printa altceva pe Serial in afara de acest JSON
    Serial.print("{\"t\":"); Serial.print(dhtOK ? t : 0.0, 1);
    Serial.print(",\"h\":"); Serial.print(dhtOK ? h : 0.0, 0);
    Serial.print(",\"mq\":"); Serial.print(mq);
    Serial.print(",\"light\":"); Serial.print(isBright ? 1 : 0);
    Serial.print(",\"good\":"); Serial.print(qualityGood ? 1 : 0);
    Serial.println("}");

    lastUiMs = now;
  }
}
