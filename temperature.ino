
/***************************************************************************
                     Copyright 2008 Gravitech
                        All Rights Reserved
****************************************************************************/

/***************************************************************************
 File Name: I2C_7SEG_Temperature.pde
 Hardware: Arduino Diecimila with 7-SEG Shield
 Description:
   This program reads I2C data from digital thermometer and display it on 7-Segment
 Change History:
   03 February 2008, Gravitech - Created
****************************************************************************/

#include <Wire.h> 
//#include <String.h>
 
#define BAUD (9600)    /* Serial baud define */
#define _7SEG (0x38)   /* I2C address for 7-Segment */
#define THERM (0x49)   /* I2C address for digital thermometer */
#define EEP (0x50)     /* I2C address for EEPROM */
#define RED (3)        /* Red color pin of RGB LED */
#define GREEN (5)      /* Green color pin of RGB LED */
#define BLUE (6)       /* Blue color pin of RGB LED */

#define COLD (23)      /* Cold temperature, drive blue LED (23c) */
#define HOT (26)       /* Hot temperature, drive red LED (27c) */

const byte NumberLookup[16] =   {0x3F,0x06,0x5B,0x4F,0x66,
                                 0x6D,0x7D,0x07,0x7F,0x6F, 
                                 0x77,0x7C,0x39,0x5E,0x79,0x71};

/* Function prototypes */
void Cal_temp (int&, byte&, byte&, bool&, bool&);
void Dis_7SEG (int, byte, byte, bool, bool, bool);
void Send7SEG (byte, byte);
void SerialMonitorPrint (byte, int, bool);
void UpdateRGB (byte);

/***************************************************************************
 Function Name: setup
 Purpose: 
   Initialize hardwares.
****************************************************************************/

void setup() 
{ 
  Serial.begin(BAUD);
  Wire.begin();        /* Join I2C bus */
  pinMode(RED, OUTPUT);    
  pinMode(GREEN, OUTPUT);  
  pinMode(BLUE, OUTPUT);   
  delay(500);          /* Allow system to stabilize */
} 

/***************************************************************************
 Function Name: loop
 Purpose: 
   Run-time forever loop.
****************************************************************************/
 
void loop() 
{ 
  int Decimal;
  byte Temperature_H, Temperature_L, counter, counter2;
  bool IsPositive;
  bool isF = false; // default Celsius
  bool changeColor = false;
  bool isLightOn = false;
  bool isStandBy = false;
  
  /* Configure 7-Segment to 12mA segment output current, Dynamic mode, 
     and Digits 1, 2, 3 AND 4 are NOT blanked */
     
  Wire.beginTransmission(_7SEG);   
  byte val = 0; 
  Wire.write(val);
  val = B01000111;
  Wire.write(val);
  Wire.endTransmission();
  
  /* Setup configuration register 12-bit */
     
  Wire.beginTransmission(THERM);  
  val = 1;  
  Wire.write(val);
  val = B01100000;
  Wire.write(val);
  Wire.endTransmission();
  
  /* Setup Digital THERMometer pointer register to 0 */
     
  Wire.beginTransmission(THERM); 
  val = 0;  
  Wire.write(val);
  Wire.endTransmission();
  
  /* Test 7-Segment */
  for (counter=0; counter<8; counter++)
  {
    Wire.beginTransmission(_7SEG);
    Wire.write(1);
    for (counter2=0; counter2<4; counter2++)
    {
      Wire.write(1<<counter);
    }
    Wire.endTransmission();
    delay (250);
  }

  int colorNumber = 0;
  while (1)
  {
    if (Serial.available() > 0) {
      char tmp = Serial.read();
      if (tmp == 't' || tmp == 'T') {
        isF = !isF;
      }

      if (tmp == 'c' || tmp == 'C') {
        changeColor = true;
      } 

      if (tmp == 'l' || tmp == 'L') {
        isLightOn = !isLightOn;
      }

      if (tmp == 's' || tmp == 'S') {
        isStandBy = !isStandBy;
      }
    }
    Wire.requestFrom(THERM, 2);
    Temperature_H = Wire.read();
    Temperature_L = Wire.read();
    
    /* Calculate temperature */
    Cal_temp (Decimal, Temperature_H, Temperature_L, IsPositive, isF);
    
    /* Display temperature on the serial monitor. 
       Comment out this line if you don't use serial monitor.*/
    SerialMonitorPrint (Temperature_H, Decimal, IsPositive, isF);

    if (isLightOn) {
      if (changeColor) {
      /* change color number*/
      if (colorNumber == 2) {
        colorNumber = 0;
      } else {
        colorNumber ++;
      }
        changeColor = false;
      }
      /* Update RGB LED.*/
      SetRGB (colorNumber);
    } else {
      setColor(0, 0, 0); // turn off the light
    }

    
        /* Display temperature on the 7-Segment */
        Dis_7SEG (Decimal, Temperature_H, Temperature_L, IsPositive, isF, isStandBy);
    
    
    delay (1000);        /* Take temperature read every 1 second */
  }
} 

/***************************************************************************
 Function Name: Cal_temp
 Purpose: 
   Calculate temperature from raw data.
****************************************************************************/
void Cal_temp (int& Decimal, byte& High, byte& Low, bool& sign, bool& isF)
{
  if ((High&B10000000)==0x80)    /* Check for negative temperature. */
    sign = 0;
  else
    sign = 1;
    
  High = High & B01111111;      /* Remove sign bit */
  Low = Low & B11110000;        /* Remove last 4 bits */
  Low = Low >> 4; 
  Decimal = Low;
  Decimal = Decimal * 625;      /* Each bit = 0.0625 degree C */
  //Serial.println(High);
  //Serial.println(Decimal);
  //Serial.println(Decimal, BIN);
  double temp = High + Decimal / 10000.0;
  
  
  if (isF) {
    // convert C to F
   
    
    temp = 1.8 * temp + 32;
    High = (int)temp;
    Decimal = (int)((temp - High) * 10000);
    
  }
  //Serial.println(High);
  //Serial.println(Decimal);

  if (sign == 0)                /* if temperature is negative */
  {
    High = High ^ B01111111;    /* Complement all of the bits, except the MSB */
    Decimal = Decimal ^ 0xFF;   /* Complement all of the bits */
  } 
   


  //Serial.println(High, DEC);
  //Serial.println(Decimal, DEC);
}

/***************************************************************************
 Function Name: Dis_7SEG
 Purpose: 
   Display number on the 7-segment display.
****************************************************************************/
void Dis_7SEG (int Decimal, byte High, byte Low, bool sign, bool isF, bool isStandby)
{
  if(!isStandby){
  byte Digit = 4;                 /* Number of 7-Segment digit */
  byte Number;                    /* Temporary variable hold the number to display */
  
  if (sign == 0)                  /* When the temperature is negative */
  {
    Send7SEG(Digit,0x40);         /* Display "-" sign */
    Digit--;                      /* Decrement number of digit */
  }
  
  if (High > 99)                  /* When the temperature is three digits long */
  {
    Number = High / 100;          /* Get the hundredth digit */
    Send7SEG (Digit,NumberLookup[Number]);     /* Display on the 7-Segment */
    High = High % 100;            /* Remove the hundredth digit from the TempHi */
    Digit--;                      /* Subtract 1 digit */    
  }
  
  if (High > 9)
  {
    Number = High / 10;           /* Get the tenth digit */
    Send7SEG (Digit,NumberLookup[Number]);     /* Display on the 7-Segment */
    High = High % 10;            /* Remove the tenth digit from the TempHi */
    Digit--;                      /* Subtract 1 digit */
  }
  
  Number = High;                  /* Display the last digit */
  Number = NumberLookup [Number]; 
  if (Digit > 1)                  /* Display "." if it is not the last digit on 7-SEG */
  {
    Number = Number | B10000000;
  }
  Send7SEG (Digit,Number);
  Digit--;                        /* Subtract 1 digit */
  
  if (Digit > 0)                  /* Display decimal point if there is more space on 7-SEG */
  {
    Number = Decimal / 1000;
    if (Number >= 10) {
      //Send7SEG (Digit+1,Number);
      Number = Number % 10;
    }
    Send7SEG (Digit,NumberLookup[Number]);
    Digit--;
  }

  if (Digit > 0)                 /* Display "c" if there is more space on 7-SEG */
  {
    if (isF) {
      Send7SEG (Digit,0x71);
    } else {
      Send7SEG (Digit,0x58);
    }
    Digit--;
  }
  
  if (Digit > 0)                 /* Clear the rest of the digit */
  {
    Send7SEG (Digit,0x00);    
  }  
  }
  else{
    Send7SEG(4, 0x00);
    Send7SEG(3, 0x00);
    Send7SEG(2, 0x00);
    Send7SEG(1, 0x00);
  }
}

/***************************************************************************
 Function Name: Send7SEG
 Purpose: 
   Send I2C commands to drive 7-segment display.
****************************************************************************/

void Send7SEG (byte Digit, byte Number)
{
  Wire.beginTransmission(_7SEG);
  Wire.write(Digit);
  Wire.write(Number);
  Wire.endTransmission();
}

/***************************************************************************
 Function Name: SetRGB
 Purpose: 
   Set the RGB light to different color
****************************************************************************/

void SetRGB (int colorNumber)
{
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);        /* Turn off all LEDs. */
  
  if (colorNumber == 0)
  {
    // RED
    setColor(255,0,0);
  }
  else if (colorNumber == 1)
  {
    // YELLOW
        setColor(255,80,0);
  }
  else 
  {
    // GREEN
        setColor(0, 255,0);
  }
}

/***************************************************************************
 Function Name: SerialMonitorPrint
 Purpose: 
   Print current read temperature to the serial monitor.
****************************************************************************/
void SerialMonitorPrint (byte Temperature_H, int Decimal, bool IsPositive, bool IsF)
{
    //Serial.print("The temperature is ");
    if (!IsPositive)
    {
      Serial.print("-");
    }
    Serial.print(Temperature_H, DEC);
    Serial.print(".");
    if(Decimal < 10){
      Serial.print("000");
    }
    else if(Decimal < 100){
      Serial.print("00");
    }
    else if(Decimal < 1000){
      Serial.print("0");
    }
    Serial.print(Decimal, DEC);
    
    if(IsF){
      Serial.print(" F");
    }
    else{
      Serial.print(" C");
    }
    
    Serial.print("\n");
}

/***************************************************************************
 Function Name: setColor
 Purpose: 
   set a rgb color
****************************************************************************/
void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(RED, red);
  analogWrite(GREEN, green);
  analogWrite(BLUE, blue);  
}


