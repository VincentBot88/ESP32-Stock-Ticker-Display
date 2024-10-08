#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// Set the I2C LCD address, columns, and rows
const int lcdColumns = 16;
const int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  // 0x27 is a common I2C address; change it if needed

const int greenLED = 12;
const int redLED = 13;
const int buttonPin = 14;  // Button connected to pin 14

const char* ssid = "meshAirsonics_LVK7";
const char* password = "fdhhcc8884";
String payload = "";

String stockSymbols[] = { "AAPL", "AMZN", "TSLA", "MSFT", "PFE", "OXY", "EBAY", "FDX" };
int currentStockIndex = 0;

unsigned long previousMillis = 0;
const long interval = 5000;  // 5 seconds interval

void setup() {
  // Initialize the LCD and backlight
  lcd.init();
  lcd.backlight();

  // Create custom characters (optional, but leaving it here)
  byte upArrow[] = {
    B00000,
    B00000,
    B00100,
    B01110,
    B11111,
    B00000,
    B00000,
    B00000
  };

  byte downArrow[] = {
    B00000,
    B00000,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000,
    B00000
  };

  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);  // Set button pin as input with internal pull-up resistor

  connectWiFi();
}

void connectWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi");

  delay(500);
}

void readPrice(const String& stockName) {
  lcd.clear();
  String httpRequestAddress = "https://finnhub.io/api/v1/quote?symbol=" + stockName + "&token=cqtc4t9r01qvdch31jo0cqtc4t9r01qvdch31jog";
  HTTPClient http;
  int httpCode;

  http.begin(httpRequestAddress);
  httpCode = http.GET();

  if (httpCode > 0) {
    DynamicJsonDocument doc(1024);
    String payload = http.getString();

    deserializeJson(doc, payload);

    float previousClosePrice = doc["pc"];
    float currentPrice = doc["c"];
    float differenceInPricePercent = ((currentPrice - previousClosePrice) / previousClosePrice) * 100.0;
    float differenceInPrice = currentPrice - previousClosePrice;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(stockName + " " + String(currentPrice));
    lcd.setCursor(0, 1);

    // Display the appropriate LED and text
    if (differenceInPrice >= 0) {
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW);
    } else {
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
    }

    lcd.print(String(differenceInPrice, 2) + " ");
    lcd.print(String(differenceInPricePercent, 2) + "%");

  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HTTP request");
    lcd.setCursor(0, 1);
    lcd.print("error");
  }

  http.end();
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to switch the stock automatically
  if (currentMillis - previousMillis >= interval) {
    // Turn off LEDs before switching stocks
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);

    previousMillis = currentMillis;
    currentStockIndex = (currentStockIndex + 1) % (sizeof(stockSymbols) / sizeof(stockSymbols[0]));
    readPrice(stockSymbols[currentStockIndex]);
  }

  // Check if the button is pressed to manually switch the stock
  if (digitalRead(buttonPin) == LOW) {    // Adjusted button logic for better handling
    delay(50);                            // Debounce delay
    if (digitalRead(buttonPin) == LOW) {  // Confirm button is still pressed
      // Turn off LEDs before switching stocks
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, LOW);

      currentStockIndex = (currentStockIndex + 1) % (sizeof(stockSymbols) / sizeof(stockSymbols[0]));
      readPrice(stockSymbols[currentStockIndex]);
      previousMillis = currentMillis;  // Reset the timer to avoid immediate auto-switch
    }
  }
}
