/*
  Sound-Reactive LED System with Time Control
  - DS3231 RTC defines active window (08:00–15:00).
  - Potentiometer sets sensitivity.
  - LEDs (PWM) react to sound; LCD shows time/status.
  - After hours: LEDs off, buzzer on (alert).

  Hardware (Arduino UNO/Nano):
    Sound sensor AO -> A1
    Potentiometer  -> A0
    Buzzer         -> D7
    LEDs (PWM)     -> D3, D5, D6, D9, D10, D11  (each with resistor)
    LCD I2C        -> SDA A4, SCL A5  (addr 0x27 or 0x3F)
    DS3231 RTC     -> I2C (SDA A4, SCL A5)

  Libraries: RTClib, LiquidCrystal_I2C, Wire
*/

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// -------- Modules --------
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // change to 0x3F if your LCD doesn't show text

// -------- Pins --------
const int soundAOPin = A1;
const int potPin     = A0;
const int buzzerPin  = 7;

// Use only PWM pins on UNO for smooth brightness
const int ledPins[]  = {3, 5, 6, 9, 10, 11};
const int numLeds    = sizeof(ledPins) / sizeof(ledPins[0]);

// -------- Time Window (08:00–15:00) --------
int startHour   = 8;
int startMinute = 0;
int endHour     = 15;
int endMinute   = 0;

// -------- Vars --------
int soundValue = 0;
int potValue   = 0;
int ledLevel   = 0;
unsigned long lastLCDUpdate = 0;

// --- helpers ---
void print2d(int v) { if (v < 10) lcd.print('0'); lcd.print(v); }

void lcdShowStatus(bool allowed, int hour, int minute) {
  // Update once per second without full clear to reduce flicker
  lcd.setCursor(0, 0);
  lcd.print("Time ");
  print2d(hour);
  lcd.print(":");
  print2d(minute);
  lcd.print("        "); // pad tail

  lcd.setCursor(0, 1);
  if (allowed) {
    lcd.print("DJ Time          "); // pad to clear line
  } else {
    lcd.print("Cutoff Time!     ");
  }
}

void setAllLEDsOff() {
  for (int i = 0; i < numLeds; i++) analogWrite(ledPins[i], 0);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // RTC init
  if (!rtc.begin()) {
    // Basic fault message to serial; still run LEDs as standalone if needed
    Serial.println("RTC not found!");
  }
  // Set RTC once if needed, then re-upload with this line commented:
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Sound Reactive");
  lcd.setCursor(0, 1); lcd.print("System Starting");
  delay(1200);
  lcd.clear();

  // IO init
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    analogWrite(ledPins[i], 0);
  }
  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);

  Serial.println("System Initialized");
}

void loop() {
  // ---- Time ----
  DateTime now = rtc.now();
  int currentHour   = now.hour();
  int currentMinute = now.minute();

  const int currentTimeMins = currentHour * 60 + currentMinute;
  const int startTimeMins   = startHour * 60 + startMinute;
  const int endTimeMins     = endHour   * 60 + endMinute;
  const bool isAllowedTime  = (currentTimeMins >= startTimeMins && currentTimeMins < endTimeMins);

  // ---- Inputs ----
  soundValue = analogRead(soundAOPin);
  potValue   = analogRead(potPin);

  // Debug (optional)
  // Serial.print("Sound: "); Serial.print(soundValue);
  // Serial.print("  Pot: ");  Serial.println(potValue);

  // ---- LCD (1 Hz) ----
  if (millis() - lastLCDUpdate > 1000UL) {
    lcdShowStatus(isAllowedTime, currentHour, currentMinute);
    lastLCDUpdate = millis();
  }

  // ---- Behavior ----
  if (isAllowedTime) {
    // Sensitivity from pot: low pot -> higher sensitivity (smaller threshold)
    int sensitivity = map(potValue, 0, 1023, 100, 800);
    sensitivity = constrain(sensitivity, 50, 1000);

    // Map sound to bar level
    ledLevel = map(soundValue, 0, sensitivity, 0, numLeds);
    ledLevel = constrain(ledLevel, 0, numLeds);

    for (int i = 0; i < numLeds; i++) {
      // Simple bar: LEDs below level are ON (full brightness)
      int brightness = (i < ledLevel) ? 255 : 0;
      analogWrite(ledPins[i], brightness);
    }

    noTone(buzzerPin);  // silent during DJ Time
  } else {
    setAllLEDsOff();
    tone(buzzerPin, 1000); // alert after hours
  }

  delay(80); // small pacing
}
