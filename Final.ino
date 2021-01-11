#include "HX711.h"
#define LOADCELL_DOUT_PIN  6
#define LOADCELL_SCK_PIN  7

#include<Wire.h>
#include "RTClib.h"

#include <LiquidCrystal_I2C.h>

#include <SPI.h>
#include <SD.h>

//////load cell////
HX711 scale;
float calibration_factor = 185000; //-7050 worked for my 440lb max scale setup
float berat;
int state = 1;
//////////////////

////////RTC///////
RTC_DS1307 RTC;
/////////////////

//////Ultrasonic//
int trigPin = 9;    // Trigger
int echoPin = 8;    // Echo
long duration, cm, inches;
/////////////////

/////LCD display//
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
byte save[] = {
  B00000,
  B11110,
  B11111,
  B11111,
  B11111,
  B10011,
  B11111,
  B00000
};

byte noSave[] = {
  B00000,
  B11110,
  B11111,
  B10101,
  B11011,
  B10101,
  B11111,
  B00000
};
/////////////////

///////SDcard///
File myFile;
const int chipSelect = 4;
int button = A2;
///////////////


//////millis////
unsigned long time_now = 0;
int period = 15000;
////////////////

void setup() {
  // Load cell
    Serial.begin(9600);
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale();
    scale.tare(); //Reset the scale to 0
    long zero_factor = scale.read_average(); //Get a baseline reading
    Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(zero_factor);
  // 

  //RTC
  Wire.begin();
  RTC.begin(); // load the time from your computer.
  if (! RTC.isrunning())  {
    Serial.println("RTC is NOT running!");// This will reflect the time that your sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  //

  //Ultrasonic
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
  //

  //LCD display
    lcd.init();                      // initialize the lcd 
    lcd.init();
    lcd.backlight();
    lcd.createChar(0, save);
    lcd.createChar(1, noSave);
    lcd.setCursor(3,0);
    lcd.print("Mr.Garbage");
    delay(500);
    lcd.setCursor(1,1);
    lcd.print("Dump Maximizer");
    delay(1000);
    lcd.clear();
  //

  //SDcard
    Serial.print("Initializing SD card...");
    pinMode(SS, OUTPUT);
    if (!SD.begin(chipSelect)) {
      Serial.println("initialization failed!");
      return;
    }
    Serial.println("initialization done.");
  //
}

void loop() {

  
    if (!SD.begin(chipSelect)) {
      Serial.println("initialization failed!");
      state = 0;
      //lcd.write(1);
      //return;
    }
    else state = 1;

  //RTC
    DateTime now = RTC.now();
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.year(), DEC);
    Serial.print(" , ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print(" , ");
    //delay(1000);
    //


  
  // Load cell
    scale.set_scale(calibration_factor); //Adjust to this calibration factor
    Serial.print("Berat: ");
    berat = scale.get_units()*0.4535923;
    Serial.print(berat, 2);
    Serial.print(" kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
    Serial.print(" , ");
  //

  // Ultrasonic
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);

    cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
    Serial.print("space: ");
    Serial.print(cm);
    Serial.print(" cm");
    Serial.println();
  //

  //LCD display
    lcd.setCursor(0,0);
    lcd.print("Berat:");
    if(berat < 0) berat = 0;;
    lcd.print(berat, 2);
    lcd.print("kg");

    lcd.setCursor(0,1);
    lcd.print("space:");
    lcd.print(cm);
    lcd.print("cm ");
    
  //

  //SD card
  if(digitalRead(button) == HIGH && state == 1) {

   if(millis() >= time_now + period) {
      time_now += period;
      //Serial.println("Bisa");
   
    
   myFile = SD.open("test.txt", FILE_WRITE);
    // if the file opened okay, write to it:
   if (myFile) {


      Serial.print("Inserting data...");

      myFile.print(now.month(), DEC);
      myFile.print('/');
      myFile.print(now.day(), DEC);
      myFile.print('/');
      myFile.print(now.year(), DEC);
      myFile.print(" , ");
      myFile.print(now.hour(), DEC);
      myFile.print(':');
      myFile.print(now.minute(), DEC);
      myFile.print(':');
      myFile.print(now.second(), DEC);
      myFile.print(" , ");
  
      myFile.print(berat, 2);
      myFile.print(" , ");
      myFile.print(" kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
      myFile.print(" , ");
  
      myFile.print(cm);
      myFile.print(" , ");
      myFile.print(" cm");
      myFile.println();
    
      // close the file:
      myFile.close();
      Serial.println("done.");
      lcd.setCursor(12,1);
      lcd.write(0);
      //lcd.write(0);
      //lcd.print(" ");
    } 
    else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
      lcd.setCursor(12,1);
      lcd.write(1);
    }
    
   }
    
  }
  else if(digitalRead(button) == LOW && state == 1) {
      lcd.setCursor(12,1);
      lcd.print(" ");
  }
  else {
      lcd.setCursor(12,1);
      lcd.write(1);
  }

  delay(1000);
}
