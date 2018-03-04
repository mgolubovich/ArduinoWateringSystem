// libraries definition
#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>

// frequency musical notes
#define NOTE_C6  1047
#define NOTE_C3  131
#define NOTE_G3  196

// pins definition
int moistureSensorPin = 0;
int levelSensorPin = 1;
int audioPin = 2;
int soggyLEDPin = 3;
int moistsoilLEDPin = 4;
int drysoilLEDPin = 5;
int pumpLEDPin = 6;
int pumpPin = 7;
int pinDHT22 = 8;
int lightingPin = 9;
int coolerPin = 10;

//   Variables

int startUnixTime;
int levelSensorValue; // stores the level sensor values
int moistureSensorValue; // stores the moisture sensor values
bool isLightOn = false; // light on flag

// Config variables ---Все, что можно менять---

int temperatureExtraCoolerOn = 28; // температура включения доп вентелятора
int temperatureLightOff = 33; // температура аварийного выключения света
int pauseBetweenMoistureCheckInSeconds = 3600; // пауза между замерами в секундах
int delayAfterPump = 30; // задержка после полива до измерения
int pumpOnFor = 5; // полив включается на столько секунд - время работы помпы
int lightOnInHour = 11; // час включения света
int lightOffInHour = 10; // час выключения

// -------------------------------------------

// led system messages
const char *string_table[] =
{
  "VER 1.1",
  "Ma/lo BoDbI ",
  "3EM Cyxo ",
  "3EM CbIPO ",
  "3EM MOKPO ",
  "HACOC BK/I",
  "I7PoPAIII/IBATE/Ib",
  "/IyKA",
  "Please wait!"
};

// objects definition
LiquidCrystal_I2C lcd(0x27, 20, 4);
DS3231 rtc(SDA, SCL);
SimpleDHT22 dht22;

void setup() {
  // serial initialization
  Serial.begin(9600);
  // RTC clock init
  //initRTC(); //uncomment this to set up clocks from PC, comment then unplugged
  rtc.begin();
  // LCD initialization
  lcd.init();
  lcd.backlight();     // with Backlight
  lcd.clear();         // clearscreen

  // Wire initialization
  //Wire.begin();

  // Arduino pins initalization
  pinMode(audioPin, OUTPUT);
  pinMode(soggyLEDPin, OUTPUT);
  pinMode(moistsoilLEDPin, OUTPUT);
  pinMode(drysoilLEDPin, OUTPUT);
  pinMode(pumpLEDPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(lightingPin, OUTPUT);
  pinMode(coolerPin, OUTPUT);

  

  // LCD initial messages
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(string_table[6]);
  lcd.setCursor(0, 1);
  lcd.print(string_table[7]);
  lcd.setCursor(0, 3);
  lcd.print(string_table[0]);
  // initialization delay
  delay(5000);
  // constrained first non-waiting sensors run
  startUnixTime = int(rtc.getUnixTime(rtc.getTime())) - pauseBetweenMoistureCheckInSeconds;

  /*
  testLEDs();
  testCoolerRelay();
  delay(2000);
  testLightingRelay();
  delay(2000);
  */
}


void loop()
{
  showTimeLcd();
  delay(2000);
  
  temperatureCheck();
  delay(2000);
  
  waterSystemCheck();
  delay(2000);    
}

void waterSystemCheck()
{

  if ((startUnixTime + pauseBetweenMoistureCheckInSeconds) < int(rtc.getUnixTime(rtc.getTime())))
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("u3MEP9IEM 6aK|3EM");
    delay(3000);
    // read sensors
    levelSensorValue = analogRead(levelSensorPin);
    moistureSensorValue = analogRead(moistureSensorPin);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("6AK ");
    lcd.print(levelSensorValue);
    delay(4000);
    // if low water level: plays the low level alarm
    if (levelSensorValue > 600) 
    {
      // system messages
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(string_table[1]);
      lcd.print(levelSensorValue);
      // plays the alarm sound
      for (int i = 0; i < 2; i++) {
        tone(audioPin, NOTE_G3, 200);
        delay(3000);
        tone(audioPin, NOTE_C3, 200);
        delay(3000);
        noTone(audioPin);
      }
    }
    else
    {
      if ((moistureSensorValue < 700) && (moistureSensorValue >= 300)) {
        // in case of moist soil
        // system messages
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(string_table[3]);
        lcd.print(moistureSensorValue);
        // lights up the correct LED
        digitalWrite(drysoilLEDPin, LOW);
        digitalWrite(moistsoilLEDPin, HIGH);
        digitalWrite(soggyLEDPin, LOW);
        delay(3000);
      }
      if (moistureSensorValue < 300) {
        // in case of soggy soil
        // system messages
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(string_table[4]);
        lcd.print(moistureSensorValue);
        // lights up the correct LED
        digitalWrite(drysoilLEDPin, LOW);
        digitalWrite(moistsoilLEDPin, LOW);
        digitalWrite(soggyLEDPin, HIGH);
        delay(3000);
      }
  
      // if the soil is dry: turn on the pump for pumpOnFor seconds and repeat if needed
      if (moistureSensorValue >= 700) {
        // in case of dry soil
        // system messages
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(string_table[2]);
        lcd.print(moistureSensorValue);
        // lights up the correct LED
        digitalWrite(drysoilLEDPin, HIGH);
        digitalWrite(moistsoilLEDPin, LOW);
        digitalWrite(soggyLEDPin, LOW);
        // plays the alarm sound
        tone(audioPin, NOTE_C6, 100);
        delay(1000);
        noTone(audioPin);
        while (moistureSensorValue >= 700) {
          // system messages
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(string_table[2]);
          lcd.setCursor(0, 1);
          lcd.print(string_table[5]);
          lcd.setCursor(0, 2);
          lcd.print(pumpOnFor);
          lcd.setCursor(0, 3);
          lcd.print("CEK.");
          // turn the pump on
          digitalWrite(pumpPin, HIGH);
          digitalWrite(pumpLEDPin, HIGH);
          delay(pumpOnFor * 1000);
          // turn the pump off
          //lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("HACOC BbIK/I");
          lcd.setCursor(0, 2);
          lcd.print(delayAfterPump);
          lcd.setCursor(0, 3);
          lcd.print("CEK.");
          digitalWrite(pumpPin, LOW);
          digitalWrite(pumpLEDPin, LOW);
          delay(delayAfterPump * 1000);
          // reads the moisture sensor once more
          moistureSensorValue = analogRead(moistureSensorPin);
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("3EM ");
          lcd.print(moistureSensorValue);
        }
      }
    }
    //reset loops timer
    startUnixTime = int(rtc.getUnixTime(rtc.getTime()));
  }
  //wait
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BODHb DAT4uKu 4EpE3");
  lcd.setCursor(0, 1);
  int allSecondsWait = startUnixTime + pauseBetweenMoistureCheckInSeconds;
  int secondsLast = allSecondsWait - int(rtc.getUnixTime(rtc.getTime()));
  lcd.print(secondsLast);
  lcd.print(" CEK");
}

void lightCycleCheck()
{
  if (rtc.getTime().hour == lightOnInHour && !isLightOn)
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    digitalWrite(lightingPin, HIGH);
    lcd.print(lightOnInHour);
    lcd.print(" hours");
    lcd.setCursor(0, 2);
    lcd.print("turning light on");
    isLightOn = true;
    delay(3000);
  }
  if (rtc.getTime().hour == lightOffInHour && isLightOn)
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    digitalWrite(lightingPin, LOW);
    lcd.print(lightOffInHour);
    lcd.print(" hours");
    lcd.setCursor(0, 2);
    lcd.print("turning light off");
    isLightOn = false;
    delay(3000);
  }
  if (isLightOn)
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("light is on");
    lcd.setCursor(0, 2);
    lcd.print("light off in ");
    lcd.print(lightOffInHour);
    lcd.print(" hour");
    delay(3000);
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("light is off");
    lcd.setCursor(0, 2);
    lcd.print("light on in ");
    lcd.print(lightOnInHour);
    lcd.print(" hour");
    delay(3000);
  }
}

int currentMonth (const char* date) {
  int m;
  // sample input: date = "Dec 26 2009"
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
  switch (date[0]) {
    case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
    case 'F': m = 2; break;
    case 'A': m = date[2] == 'r' ? 4 : 8; break;
    case 'M': m = date[2] == 'r' ? 3 : 5; break;
    case 'S': m = 9; break;
    case 'O': m = 10; break;
    case 'N': m = 11; break;
    case 'D': m = 12; break;
  }
  return m;
}

void testLEDs()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("TECT LED MOKPO");
  digitalWrite(soggyLEDPin, HIGH);
  delay(2000);
  digitalWrite(soggyLEDPin, LOW);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("TECT LED CbIPO");
  digitalWrite(moistsoilLEDPin, HIGH);
  delay(2000);
  digitalWrite(moistsoilLEDPin, LOW);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("TECT LED CYXO");
  digitalWrite(drysoilLEDPin, HIGH);
  delay(2000);
  digitalWrite(drysoilLEDPin, LOW);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("TECT LED HACOC");
  digitalWrite(pumpLEDPin, HIGH);
  delay(2000);
  digitalWrite(pumpLEDPin, LOW);
}

void temperatureCheck()
{
  // read without samples.
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht22.read(pinDHT22, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err="); Serial.println(err); delay(1000);
    return;
  }
  
  if (temperature >= temperatureExtraCoolerOn)
  {
    digitalWrite(coolerPin, HIGH);
  }
  else
  {
    digitalWrite(coolerPin, LOW);
  }
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 2);
  lcd.print(int(temperature));
  lcd.print(" *C");
  lcd.setCursor(0, 3);
  lcd.print(int(humidity));
  lcd.print(" RH%");
  if (temperature >= temperatureLightOff)
  {
    while (temperature >= temperatureLightOff)
    {
      digitalWrite(lightingPin, LOW);
      for (int i = 0; i < 2; i++) 
      {
          tone(audioPin, NOTE_G3, 200);
          delay(2000);
          tone(audioPin, NOTE_C3, 200);
          delay(2000);
          noTone(audioPin);
      }
    }
    if (isLightOn) {digitalWrite(lightingPin, HIGH);}
  }
  delay(2000);
  lightCycleCheck();
}

void initRTC()
{
  char *time;
  char *date;
  String hours;
  String minutes;
  String seconds;
  int month;
  String d;
  String y;
  char *currentTime = __TIME__;
  char *currentDate = __DATE__;
  time = strtok(currentTime, ":");
  hours = time;
  time = strtok(NULL, ":");
  minutes = time;
  time = strtok(NULL, ":");
  seconds = time;
  date = strtok(currentDate, " ");
  date = strtok(NULL, " ");
  d = date;
  month = currentMonth(__DATE__);
  date = strtok(NULL, " ");
  y = date;
  rtc.begin();
  rtc.setDate(d.toInt(), month, y.toInt());
  rtc.setTime(hours.toInt(), minutes.toInt(), seconds.toInt());
}
void showTimeLcd()
{
  lcd.clear();
  lcd.setCursor(0, 2);
  lcd.print(rtc.getTimeStr());
  lcd.setCursor(0, 3);
  lcd.print(rtc.getDateStr());
}

void testLightingRelay()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("TECTuPyEM CBET-PE/IE");
  lcd.setCursor(0, 2);
  lcd.print(" BK/I");
  digitalWrite(lightingPin, HIGH);
  delay(5000);
  lcd.setCursor(0, 2);
  lcd.print("BbIK/I");
  digitalWrite(lightingPin, LOW);
}

void testCoolerRelay()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("TECT BEHT-PE/IE");
  lcd.setCursor(0, 2);
  lcd.print(" BK/I");
  digitalWrite(coolerPin, HIGH);
  delay(5000);
  lcd.setCursor(0, 2);
  lcd.print("BbIK/I");
  digitalWrite(coolerPin, LOW);
}
