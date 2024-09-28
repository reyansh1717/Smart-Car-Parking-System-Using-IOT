#include<Wire.h>
#include<LiquidCrystal_I2C.h>
#include<Servo.h>

const int lcd_I2C_address = 0x27;
LiquidCrystal_I2C lcd(lcd_I2C_address, 16, 2);

const int entrySensorPin = 2;
const int exitSensorPin = 12;

const int entryMotorPin = 3;
Servo entryServo;

const int exitMotorPin = 13;
Servo exitServo;

const int parkingSensorPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
int occupiedSpots = 0;
int availableSpots = 8;

unsigned long entryTime = 0;
unsigned long carLeftTime = 0;
unsigned long fullMessageStartTime = 0;

bool carDetectedEntry = false;
bool carDetectedExit = false;
bool parkingLotFull = false;
bool carLeaving = false;

const int fullMessageDuration = 3000; 

const int backlightPin = 10;
int backlightBrightness = 230;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  lcd.begin();
  lcd.backlight();
  analogWrite(backlightPin, backlightBrightness);

  entryServo.attach(entryMotorPin);
  exitServo.attach(exitMotorPin);

  entryServo.write(0);
  exitServo.write(90);

  pinMode(entrySensorPin, INPUT);
  pinMode(exitSensorPin, INPUT);
  for (int i = 0; i < 8; i++)
  {
    pinMode(parkingSensorPins[i], INPUT);
  }

  lcd.setCursor(0, 0);
  lcd.print("Welcome to Parking!");
  Serial.println("Setup complete. Ready for operation.");
}

void loop()
{
  carDetectedEntry = digitalRead(entrySensorPin) == LOW;
  carDetectedExit = digitalRead(exitSensorPin) == LOW;

  // Update occupied and available spots count
  occupiedSpots = 0;
  for (int i = 0; i < 8; i++)
  {
    if (digitalRead(parkingSensorPins[i]) == LOW)
    {
      occupiedSpots++;
    }
  }
  availableSpots = 8 - occupiedSpots;

  if (occupiedSpots == 8)
  {
    if (!parkingLotFull)
    {
      fullMessageStartTime = millis();
      parkingLotFull = true;
      updateDisplay("Parking Lot Full!", "", "");
    }
    else if (millis() - fullMessageStartTime >= fullMessageDuration)
    {
      parkingLotFull = false;
      updateDisplay("Occupied: 8", "Available: 0", "");
    }
  }
  else
  {
    if (carDetectedEntry && availableSpots > 0 && entryTime == 0)
    {
      entryServo.write(90);
      entryTime = millis();
      updateDisplay("Car Entering!", "", "");
      Serial.println("Car entered parking lot.");
    }

    if (millis() - entryTime >= 3000 && entryTime != 0 && !carDetectedEntry)
    {
      entryServo.write(0);
      entryTime = 0;
    }

    if (carDetectedExit)
    {
      exitServo.write(0);
      carLeftTime = millis();
      carLeaving = true;
      updateDisplay("Car Leaving!", "", "");
    }
    else if (carLeaving && millis() - carLeftTime >= 3000)
    {
      carLeaving = false;
      exitServo.write(90);
      updateDisplay("Occupied: " + String(occupiedSpots), "Available: " + String(availableSpots), "");
    }
    else
    {
      exitServo.write(90);
      updateDisplay("Occupied: " + String(occupiedSpots), "Available: " + String(availableSpots), "");
    }
  }
}

void updateDisplay(String line1, String line2, String line3)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2 != "")
  {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  if (line3 != "")
  {
    lcd.setCursor(0, 2);
    lcd.print(line3);
  }
}
