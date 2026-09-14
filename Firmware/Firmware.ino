#include <Wire.h>
#include <RTClib.h>
#include <TinyGPS++.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Preferences.h>
#include <math.h>

#define DEG2RAD(d) ((d) * M_PI / 180.0) 
#define RAD2DEG(r) ((r) * 180.0 / M_PI)

#define SW1 4
#define SW2 5
#define SW3 6
#define SW4 8
#define SW5 9
#define SQW_RTC 10  // INT#/SQW on DS3231
#define SCLK 12     // SCLK on E Ink screen connector
#define CS 13       // CS on E Ink screen connector
#define I2C_SCL 14
#define I2C_SDA 15
#define SDI 16         // SDI on E Ink screen
#define BUSY 19        // BUSY on E Ink screen
#define RES 20         // RES on E Ink screen
#define DC 21          // DC on E Ink screen
#define vibromotor 24  // base on transistor
#define TX_GPS 27
#define RX_GPS 28

const double FAJR_ANGLE = 18.0;
const double ISHA_ANGLE = 18.0;
const double ASR_FACTOR = 2;

struct PrayerTimes {
  double fajr, sunrise, dhuhr, asr, maghrib, isha;
};

double julianDay(int year, int month, int day) {
  if(month <= 2) { 
    year -= 1;
    month += 12;
  }
  int A = year / 100;
  int B = 2 - A + (A / 4);
  return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + B - 1524.5;
}

void sunPosition(double jd, double &declination, double &eqTimeMinutes) {
  double D = jd - 2451545.0;
  double g = fmod(357.529 + 0.98560028 * D, 360.0);
  double q = fmod(280.459 + 0.98564736 * D, 360.0);
  double L = fmod(q + 1.915 * sin(DEG2RAD(g)) + 0.020 * sin(DEG2RAD(2 * g)), 360.0);

  double e = 23.439 - 0.00000036 * D;

  double RA = RAD2DEG(atan2(cos(DEG2RAD(e)) * sin(DEG2RAD(L)), cos(DEG2RAD(L)))) / 15.0;
  RA = fmod(RA, 24.0);
  if (RA < 0) RA += 24.0; 

  declination = RAD2DEG(asin(sin(DEG2RAD(e)) * sin(DEG2RAD(L))));

  double eqTimeHours = (q / 15.0) - RA;

  if(eqTimeHours > 12) eqTimeHours -= 24;
  if(eqTimeHours < -12) eqTimeHours += 24;
  eqTimeMinutes = eqTimeHours * 60.0;
}

double hourAngle(double angle, double latitude, double declination) {
  double lat = DEG2RAD(latitude);
  double decl = DEG2RAD(declination);
  double a = DEG2RAD(angle);

  double cosH = (-sin(a) - sin(lat) * sin(decl)) / (cos(lat) * cos(decl));

  if (cosH > 1.0) cosH = 1.0;
  if (cosH < -1.0) cosH = -1.0;

  return RAD2DEG(acos(cosH)) / 15.0;
}

double asrHourAngle(double factor, double latitude, double declination) {
  double lat = DEG2RAD(latitude);
  double decl = DEG2RAD(declination);

  double a = -atan(1.0 / (factor + tan(fabs(lat - decl))));
  double cosH = (sin(a) - sin(lat) * sin(decl)) / (cos(lat) * cos(decl));

  if (cosH > 1.0) cosH = 1.0;
  if (cosH < -1.0) cosH = -1.0;

  return RAD2DEG(acos(cosH)) / 15.0;
}

PrayerTimes calculatePrayerTimes(int year, int month, int day, double latitude, double longitude, double timeZoneOffsetHours) {
  double jd = julianDay(year, month, day);

  double declination, eqTimeMinutes;
  sunPosition(jd, declination, eqTimeMinutes);
  double eqTimeHours = eqTimeMinutes / 60.0;

  double dhuhr = 12.0 + timeZoneOffsetHours - (longitude / 15.0) - eqTimeHours;

  double sunHA = hourAngle(0.833, latitude, declination);
  double fajrHA = hourAngle(FAJR_ANGLE, latitude, declination);
  double ishaHA = hourAngle(ISHA_ANGLE, latitude, declination);
  double asrHA = asrHourAngle(ASR_FACTOR, latitude, declination);

  PrayerTimes t;
  t.fajr = dhuhr - fajrHA;
  t.sunrise = dhuhr - sunHA;
  t.dhuhr = dhuhr;
  t.asr = dhuhr + asrHA;
  t.maghrib = dhuhr + sunHA;
  t.isha = dhuhr + ishaHA;

  return t;
}

void printTime(double decimalHours, char* buf, size_t bufSize) {
  if (decimalHours < 0) decimalHours += 24;
  if (decimalHours >= 24) decimalHours -= 24;
  int h = (int)decimalHours;
  int m = (int)round((decimalHours - h) * 60);
  if (m == 60) { m = 0; h += 1; }
  snprintf(buf, bufSize, "%02d:%02d", h, m);
}

// Objects
RTC_DS3231 rtc; // create an object "rtc" of type "RTC_DS3231"
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);
Preferences prefs;

GxEPD2_BW<GxEPD2_290_T5, GxEPD2_290_T5::HEIGHT> display(
  GxEPD2_290_T5(CS, DC, RES, BUSY)
);

bool coldStart = false;

void showStatus(const char* msg) {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 30);
    display.print(msg);
  } while (display.nextPage());
}

void acquireGpsFixBlocking() {
  unsigned long start = millis();
  bool locked = false;

  while(!locked) {
    while(gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }

    if(gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
      locked = true;

      double lat = gps.location.lat();
      double lng = gps.location.lng();

      rtc.adjust(DateTime(gps.date.year(), gps.date.month(), gps.date.day(), 
                          gps.time.hour(), gps.time.minute(), gps.time.second()));

      prefs.putFloat("lat", lat);
      prefs.putFloat("lng", lng);
      prefs.putBool("setup_done", true);

      Serial.printf("GPS Lock: %.6f, %.6f\n", lat, lng);
      showStatus("GPS Locked!");
      delay(1000);
    }

    if(millis() - start > 300000UL) {
      showStatus("No GPS fix. \nCheck sky view");
      delay(3000);
      start = millis();
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Buttons as inputs
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SW5, INPUT_PULLUP);

  // Vibration + short boot buzz
  pinMode(vibromotor, OUTPUT);
  digitalWrite(vibromotor, HIGH);
  delay(150);
  digitalWrite(vibromotor, LOW);

  // Screen on
  display.init(115200);
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  showStatus("Booting...");

  // I2C + RTC
  Wire.begin(I2C_SDA, I2C_SCL);
  pinMode(SQW_RTC, INPUT_PULLUP);

  if(!rtc.begin()) {
    Serial.println("DS3231 is not recognized!");
    showStatus("ERROR: RTC not found");
  }

  // NVS
  prefs.begin("salahtracker", false); // open a non-dependend on power memory inside of ESP32, false = write allowed
  bool setupComplete = prefs.getBool("setup_done", false);

  // Determine cold start
  bool timeInvalid = rtc.lostPower(); // true if the module lost power since the last check
  bool locationInvalid = !setupComplete;

  if(timeInvalid || locationInvalid){
    coldStart = true;
    Serial.println("COLD START - GPS FIX REQUIRED");
    showStatus("First time setup...\nSearching for GPS");
  } else {
    DateTime now = rtc.now();
    Serial.printf("Current RTC time is: %04d-%02d-%02d %02d:%02d\n", 
                    now.year(), now.month(), now.day(), now.hour(), now.minute());
  }

  gpsSerial.begin(9600, SERIAL_8N1, RX_GPS, TX_GPS);

  if(coldStart) {
    acquireGpsFixBlocking();
  }
  
  double tz = 5.0; // UTC +5

  PrayerTimes pt = calculatePrayerTimes(now.year(), now.month(), now.day(), lat, lng, tz);

  char fajrBuf[6], dhuhrBuf[6], asrBuf[6], maghribBuf[6], ishaBuf[6];
  printTime(pt.fajr, fajrBuf, sizeof(fajrBuf));
  printTime(pt.dhuhr, dhuhrBuf, sizeof(dhuhrBuf));
  printTime(pt.asr, asrBuf, sizeof(asrBuf));
  printTime(pt.maghrib, maghribBuf, sizeof(maghribBuf));
  printTime(pt.isha, ishaBuf, sizeof(ishaBuf));

  char buf(128);
  snprintf(buf, sizeof(buf), 
            "Fajr: %s\nDhuhr: %s\nAsr: %s\nMaghrib: %s\nIsha: %s", fajrBuf, dhuhrBuf, asrBuf, maghribBuf, ishaBuf)
  showStatus(buf);
}

void loop() {
  // put your main code here, to run repeatedly:
}
