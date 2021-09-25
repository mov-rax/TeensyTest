#include <Arduino.h>
#include <SPI.h>
//#include <Adafruit_GFX.h>
//#include <MCUFRIEND_kbv.h>
#include <string>
#include "magic.h"

// A4 - LCD_RST (RESET SIGNAL)
// A3 - LCD_CS (CHIP SELECT)
// A2 - LCD_RS (COMMAND/DATA SELECT)

// 2 - LCD_D0 (LCD DATA Bit0) 
// 3 - LCD_D1 (LCD DATA Bit1)
// 4 - LCD_D2 (LCD DATA Bit2)
// 5 - LCD_D3 (LCD DATA Bit3)
// 6 - LCD_D4 (LCD DATA Bit4)
// 7 - LCD_D5 (LCD DATA Bit5)
// 8 - LCD_D6 (LCD DATA Bit6)
// 9 - LCD_D7 (LCD DATA Bit7)


void setup()
{
    //pinMode(A4, OUTPUT); pinMode(A3, OUTPUT); pinMode(A2, OUTPUT);
    Serial.println("Yeet");
    auto b = pog(4,2);
}

void loop()
{
    // put your main code here, to run repeatedly:
}