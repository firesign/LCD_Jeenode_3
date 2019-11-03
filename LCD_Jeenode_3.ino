//  LCD Jeenode for kitchen
//  RTC disabled Nov 3 2019

#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library
MCUFRIEND_kbv tft;

#include <SoftwareSerial.h>

SoftwareSerial mySerial(50, 51); // RX, TX

#include <Fonts/OpenSans_Semibold10pt7b.h>
#include <Fonts/OpenSans_Light50pt7b.h>
#include <Fonts/OpenSans_Light36pt7b.h>
#include <Fonts/OpenSans_Light24pt7b.h>

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410

char time_buf[10];

// RTC  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

int oldMinute, oldHour, hr_24, hr_12;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void setup()
{
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  mySerial.begin(9600);

  uint16_t ID = tft.readID();
  if (ID == 0xD3) ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(2);  // rotate 180 degrees

  tft.fillScreen(RED); // red background

  // RTC setup ------------------------------------

  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");


  // Display setup --------------------------------
  // SHED setup

  showmsgXY(20, 38, 1, &OpenSans_Semibold10pt7b, "OUTSIDE");
  showmsgXY(70, 175, 1, &OpenSans_Semibold10pt7b, "Relative");
  showmsgXY(70, 200, 1, &OpenSans_Semibold10pt7b, "Humidity");
  tft.fillCircle(225, 65, 6, WHITE); // degree symbol shed temp
  tft.fillCircle(225, 65, 4, RED);
  showmsgXY(230, 106, 1, &OpenSans_Light24pt7b, "c"); // celcius symbol
  showmsgXY(268, 184, 1, &OpenSans_Light24pt7b, "%");

  // GARAGE setup
  tft.fillRect(0, 235, 320, 105, BLACK);  // middle black bkgnd
  tft.fillRect(130, 310, 200, 30, WHITE); // white slice
  tft.fillCircle(122, 255, 4, WHITE); // degree symbol garage temp
  tft.fillCircle(122, 255, 3, BLACK);
  showmsgXY(127, 290, 1, &OpenSans_Light24pt7b, "c"); // celcius symbol
  showmsgXY(265, 287, 1, &OpenSans_Light24pt7b, "%");
  tft.fillRect(19, 254, 92, 39, BLACK); // coverup for garage temp
  tft.fillRect(199, 254, 59, 39, BLACK); // coverup for garage humidity
  showmsgXY(40, 330, 1, &OpenSans_Semibold10pt7b, "GARAGE");
  showmsgXYblack(138, 330, 1, &OpenSans_Semibold10pt7b, "DOOR");
}


void loop(void)
{

  byte LTemp, HTemp, LHum, HHum, tempLow, tempHigh, humLow,
       humHigh, coLow, coHigh, gdLow, gdHigh,
       wbTHigh, wbTLow, wbHHigh, wbHLow, unknown1, unknown2;

  if (mySerial.available()) {
    int c = mySerial.parseInt();

    // SHED -------------------------------------------------------------------

    if (c == 5) {
      Serial.println("Shed report:");
      LTemp = mySerial.parseInt();
      HTemp = mySerial.parseInt();
      LHum = mySerial.parseInt();
      HHum = mySerial.parseInt();
      byte bvLow = mySerial.parseInt();
      byte bvMid = mySerial.parseInt();
      byte bvHigh = mySerial.parseInt();
      byte bvThrowaway = mySerial.parseInt();
      byte spvLow = mySerial.parseInt();
      byte spvMid = mySerial.parseInt();
      byte spvHigh = mySerial.parseInt();
      byte spvThrowaway = mySerial.parseInt();

      while (mySerial.available())
        mySerial.read();

      // Shed temp ************
      float temperature = (LTemp + (256 * HTemp)) / 10.0;
      int humidity = (LHum + (256 * HHum)) / 10;

      // If data seems out of range, don't print any data for Shed

      if (temperature > -25 && temperature < 40 && temperature != 0.0) {
        Serial.print("Temperature: ");
        Serial.println(temperature, 1);
        tft.fillRect(18, 52, 195, 78, RED); // coverup for shed temp
        char cstr[16];
        tft.setTextColor(WHITE);
        tft.setCursor(20, 125);
        tft.setFont(&OpenSans_Light50pt7b);
        tft.setTextSize(1);
        tft.print(dtostrf(temperature, 4, 1, cstr));

        // timestamp
        //sprintf(time_buf, "%02u:%02u", hr_12, minute());
        //showmsgXY(115, 38, 1, &OpenSans_Semibold10pt7b, "@");
        //tft.fillRect(135, 20, 60, 23, RED);
        //showmsgXY(139, 38, 1, &OpenSans_Semibold10pt7b, time_buf);
      }

      if (humidity > 0 && humidity <= 100 && temperature != 0.0) {
        // Shed Humidity ***************
        Serial.print("Humidity: ");
        Serial.println(humidity);
        char cstrb[6];
        if (humidity > 10) {                    // position adjust for values less than 10
          tft.fillRect(182, 146, 123, 61, RED); // coverup for shed humidity
          showmsgXY(181, 205, 1, &OpenSans_Light36pt7b, itoa(humidity, cstrb, 10));
          showmsgXY(268, 184, 1, &OpenSans_Light24pt7b, "%");
        } else {
          tft.fillRect(182, 146, 123, 61, RED); // coverup for shed humidity
          showmsgXY(210, 205, 1, &OpenSans_Light36pt7b, itoa(humidity, cstrb, 10));
          showmsgXY(268, 184, 1, &OpenSans_Light24pt7b, "%");
        }

        Serial.println();
      }

      // GARAGE -------------------------------------------------------------------
    } else if (c == 10) {
      Serial.println("Garage report:");
      tempLow = mySerial.parseInt();
      tempHigh = mySerial.parseInt();
      humLow = mySerial.parseInt();
      humHigh = mySerial.parseInt();
      wbTLow = mySerial.parseInt();
      wbTHigh = mySerial.parseInt();
      wbHLow = mySerial.parseInt();
      wbHHigh = mySerial.parseInt();
      coLow = mySerial.parseInt();
      coHigh = mySerial.parseInt();
      unknown1 = mySerial.parseInt();
      unknown2 = mySerial.parseInt();
      gdLow = mySerial.parseInt();
      gdHigh = mySerial.parseInt();

      while (mySerial.available())
        mySerial.read();

      // Garage temp ************
      float temperature = (tempLow + (256 * tempHigh)) / 10.0;
      int humidity = (humLow + (256 * humHigh)) / 10;

      // If data seems out of range, don't print any data for Garage
      if (temperature > -15 && temperature < 35) {
        Serial.print("Temperature: ");
        Serial.println(temperature, 1);
        tft.fillRect(19, 254, 92, 39, BLACK); // coverup for garage temp
        char cstrc[16];
        tft.setTextColor(WHITE);
        tft.setCursor(19, 290);
        tft.setFont(&OpenSans_Light24pt7b);
        tft.setTextSize(1);
        tft.print(dtostrf(temperature, 4, 1, cstrc));
      }
      // Garage Humidity ************
      if (humidity > 0 && humidity < 100) {
        Serial.print("Humidity: ");
        Serial.println(humidity);
        tft.fillRect(199, 254, 59, 39, BLACK);  // coverup for garage humidity
        char cstrd[6];
        if (humidity > 10) {                    // position adjust for values less than 10
          showmsgXY(202, 290, 1, &OpenSans_Light24pt7b, itoa(humidity, cstrd, 10));
        } else {
          showmsgXY(220, 290, 1, &OpenSans_Light24pt7b, itoa(humidity, cstrd, 10));
        }
        // Garage door ************
        if (gdLow == 1) {
          Serial.print("Garage Door: CLOSED");
          tft.fillRect(203, 315, 81, 19, WHITE); // coverup for garage door state
          showmsgXYblack(203, 330, 1, &OpenSans_Semibold10pt7b, "CLOSED");
          Serial.println(" ");
        } else if (gdLow == 0) {
          Serial.print("Garage Door: OPEN");
          tft.fillRect(203, 315, 81, 19, WHITE); // coverup for garage door state
          showmsgXYblack(203, 330, 1, &OpenSans_Semibold10pt7b, "OPEN");
          Serial.println(" ");
        }
      }
      Serial.println();
    } else {
      while (mySerial.available())
        mySerial.read();
    }

  }

  // Real Time Clock -------------------------------------------------------------

  //char time_buf[10];
  char day_buf[2];

  // 12-hour clock
  hr_24 = hour();
  if (hr_24 == 0) {
    hr_12 = 12;
  } else if (hr_24 == 12) {
    hr_12 = 12;
  }
  else hr_12 = hr_24 % 12;

  // update hour and minute once every 60 seconds
//  if (minute() != oldMinute) {        // erase old minute
//    tft.fillRect(15, 366, 246, 77, RED);  // erase
//    oldMinute = minute();
//    // print new minute
//    sprintf(time_buf, "%02u:%02u", hr_12, minute());
//    showmsgXY(15, 440, 1, &OpenSans_Light50pt7b, time_buf);
//
//    // AM/PM
//    if (hr_24 < 12) {
//      // print "AM"
//      char hours12 = "AM";
//      tft.fillRect(270, 404, 35, 26, RED);  // erase
//      tft.setFont(&OpenSans_Semibold10pt7b);
//      tft.setCursor(274, 422);
//      tft.setTextColor(WHITE);
//      tft.print("AM");
//    }
//    else {
//      // print "PM"
//      char hours12 = "PM";
//      tft.fillRect(274, 404, 35, 26, RED);  // erase
//      tft.setFont(&OpenSans_Semibold10pt7b);
//      tft.setCursor(270, 422);
//      tft.setTextColor(WHITE);
//      tft.print("PM");
//    }
//  }
}

// FUNCTIONS --------------------------------------------------------------
// White text
void showmsgXY(int x, int y, int sz, const GFXfont * f, const char *msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(WHITE);
  tft.setTextSize(sz);
  tft.print(msg);
  //delay(1000);
}

// Black text
void showmsgXYblack(int x, int y, int sz, const GFXfont * f, const char *msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(BLACK);
  tft.setTextSize(sz);
  tft.print(msg);
  //delay(1000);
}

// Red text
void showmsgXYred(int x, int y, int sz, const GFXfont * f, const char *msg)
{
  int16_t x1, y1;
  uint16_t wid, ht;
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(RED);
  tft.setTextSize(sz);
  tft.print(msg);
  //delay(1000);
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
