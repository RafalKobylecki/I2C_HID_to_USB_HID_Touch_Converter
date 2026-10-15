/**
 * @Author: Rafal Kobylecki
 * @Date:   2023-12-14 07:34:17
 * @Last Modified by:   Your name
 * @Last Modified time: 2024-07-05 15:10:45
 */
#include <Arduino.h>
// #include <i2c_t3.h>
#include <Wire.h>
//   #include <i2c_driver_wire.h>

#define DEBUG

#define FAROE 1
#define RAMIA 2
#define BUMBLEBEE 3
#define ZIG_ZAG 4
#define BERLINETTA_CFL_R 5
#define SOUTHPEAK_13_TGL 6
#define MOCKINGBIRD 7
#define FIORANO 8
#define CINAMONBAY 9
#define SOUTHPEAK_13
#define SOUTHPEAK_15
#define eKTH5015 10
#define eKTH5018 11
#define WACOM_G14 14
#define G2TOUCH_G7500 15
#define LX92513 16
#define SW42101A 17

#define LED_PIN 13
#define TS_RESET_PIN 17
#define TS_ENABLE_PIN 16
#define TS_INTERRUPT_PIN 15

#define I2C_OK 0
#define I2C_OTH_ERR 4

int i2c_error = 0;

int C0x = 0;
int C0y = 0;
int C1x = 0;
int C1y = 0;

int multi_x = 92;
int multi_y = 158;

byte c[200];
int data[256];
int data_m[5][100];
//************************************************************* FINGER PRESS DATA TABLE: 16BIT
uint32_t press_data_x[10];
uint32_t press_data_y[10];
uint32_t press_data_xy[10][2];

byte i;
byte finger[11];
byte addr;
int reg;
bool flag = false;
bool readTSOnDemandFlag = false;

int platform = 0;

int c19;

uint8_t i2c_err = 0;

uint8_t _i2c_address_array[128];
bool LX9215B_init = false;
bool WACOM_G14_init = false;
void TP_Detect();
void ReadTouchscreenXXX(int slaAddr, int *pArray, int size, int index);
void ReadTouchscreen(int slaAddr, int reg_addr_l, int reg_addr_h, int *pArray, int size, int index);
void ReadTouchscreen_4B();
void ReadTouchscreen_0A();
void ReadTouchscreen_10();
void ReadTouchscreen_Southpeak_13_TGL();
void Southpeak_TS_Init();
void ReadTouchscreen_eKTH5018();
void ReadTouchscreen_Wacom_G14();
void Read_TS_Wacom_G14();
void ReadTouchscreen_G7500();
void ReadTouchscreen_eKTH5015();
void ReadTouchscreen_Melfas_0x34();
void fingerPressRelease(int fingers, int data[][2], int mx, int my);



void SetupOnDemandReadFlag();

void LXSemicon_LX9215B_init();
void ReadTouchscreen_LXSemicon_LX92515B();
void Wacom_G14_init();

void LXSemicon_SW42101A_init();
void ReadTouchscreen_LXSemicon_SW42101A();


void i2c_scanner_m(TwoWire &wire, uint8_t *tab, uint8_t len);
void i2c_scanner_print_data(uint8_t *tab, uint8_t len);
void setup()
{

  LX9215B_init = false;
#ifdef DEBUG
  Serial1.begin(115200);
  Serial1.println("IVY TECHNOLOGY - E2E");
  Serial1.println("Touchscreen tester V.6.6.6");
#endif
  pinMode(LED_PIN, OUTPUT);
  pinMode(TS_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TS_INTERRUPT_PIN), SetupOnDemandReadFlag, FALLING);
  pinMode(TS_RESET_PIN, OUTPUT);
  digitalWrite(TS_RESET_PIN, HIGH);

  Wire.begin();
  Wire.setClock(400000);
#ifdef DEBUG
  delay(1000);
#endif
  Serial1.println("IVY TECHNOLOGY - E2E");
  Serial1.println("Touchscreen tester V.1.1.0");
  i2c_scanner_m(Wire, _i2c_address_array, 128);
  i2c_scanner_print_data(_i2c_address_array, 128);
  // CHANGE: 2026-10-15
  Wire.begin();
  Wire.setClock(400000);
  // END CHANGE
  TP_Detect();

  Serial1.println("DETECT");
  if (addr < 16)
    Serial1.print("ADDRESS: 0x0");
  else
    Serial1.print("ADDRESS: 0x");

  Serial1.println(addr, HEX);
  // if (addr != 0)
  if (addr != 0x00 && addr != 0xFF)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  TouchscreenUSB.begin();
  Serial1.println("TOUCH ON");
}

void loop()
{
  // digitalWrite(TS_RESET_PIN, LOW);
  // delayMicroseconds(250);
  // digitalWrite(TS_RESET_PIN, HIGH);
  if (addr == 0x0A)
  {
    if (platform == SOUTHPEAK_13_TGL)
    {
      ReadTouchscreen_Southpeak_13_TGL();
    }
    else if (platform == WACOM_G14)
    {
      // Southpeak_TS_Init();
      // ReadTouchscreen_Wacom_G14();
      if (WACOM_G14_init == false)
        Southpeak_TS_Init();
      Read_TS_Wacom_G14();
      // delay(10);
      //   Serial1.println("WACOM G14 FOUND");
    }
    else
    {
      ReadTouchscreen_0A();
    }

  } // RAMIA / BUMBLEBEE / ZIG-ZAG
  if (addr == 0x4B)
  {
    ReadTouchscreen_4B();
  } // FAROE
  // CHANGE : 2024-07-05 15:10:45 REMOVED: 2024-07-05 15:10:45
  // if (addr == 0x40)
  // {
  //   ReadTouchscreen_G7500();
  // }
// END CHANGE
  if (addr == 0x10)
  {
    if (platform == eKTH5018)
    {
      ReadTouchscreen_eKTH5018();
    }
    else if (platform == eKTH5015)
    {
      ReadTouchscreen_eKTH5015();
    }
    else if (platform == G2TOUCH_G7500)
    {
      ReadTouchscreen_G7500();
    }
    else if (platform == LX92513)
    {
      if (LX9215B_init == false)
      LXSemicon_LX9215B_init();
      ReadTouchscreen_LXSemicon_LX92515B();
    }else if(platform == SW42101A){
      LXSemicon_SW42101A_init();
      ReadTouchscreen_LXSemicon_SW42101A();
    }
    else
      ReadTouchscreen_10();
  } //

  if (addr == 0x40)
  {
    ReadTouchscreen_G7500();
  }
  if (addr == 0x09)
  {
    if (LX9215B_init == false)
      LXSemicon_LX9215B_init();
    ReadTouchscreen_LXSemicon_LX92515B();
  }
  if (addr == 0x34)
  {
   ReadTouchscreen_Melfas_0x34();
  } //
}

/* *************************************************************************************************************************************************************
 * check HID frame 30 bytes: read from SLA_ADDR, REG_ADDR_L (0x00), REG_ADDR_H(0x00)
 * e.g.
 *
 *    1E	0 	0 	1	 31 	2 	2 	0 	3 	0 	43	0 	4 	0 	FF	0	  5	  0	  6  	0 	F3	 4	 E4 	29 	5	 61 	0 	0 	0 	0
 * data bytes order: data byte low, data byte high
 * 16bit value: data byte high * 256 + data byte low
 *
 * 2 bytes: HID DESC LENGTH
 * 2 bytes: BCD VERSION
 * 2 bytes: REPORT DESC LENGTH
 * 2 bytes: REPORT REGISTER
 * 2 bytes: INPUT REGISTER
 * 2 bytes: MAX INPUT LENGTH
 * 2 bytes: OUTPUT REGISTER
 * 2 bytes: MAX OUTPUT LENGTH
 * 2 bytes: COMMAND REGISTER
 * 2 bytes: DATA REGISTER
 * 2 bytes: VENDOR ID
 * 2 bytes: PRODUCT ID
 * 2 bytes: VERSION ID
 * 4 bytes: RESERVED
 */

void TP_Detect()
{
  uint8_t i = 0;
  //*************************************************                           FAROE                         *************************************************
  
  // CHANGE: 2026-10-15
  memset(c, 0, sizeof(c));
  // END CHANGE
  
  Wire.requestFrom(0x4B, 3);
  while (Wire.available())
  {
    c[0] = Wire.read(); // 20
    c[1] = Wire.read(); // 0
    c[2] = Wire.read(); // 1
  }
  if (c[0] == 20 and c[1] == 0 and c[2] == 1)
    addr = 0x4B;

  //*************************************************                        LXSEMICON                        *************************************************
  i = 0;
  // CHANGE: 2026-10-15
  memset(c, 0, sizeof(c));
  // END CHANGE
  Wire.beginTransmission(0x09);
  Wire.write(0x01);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(0x09, 30);

  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }
  Wire.endTransmission(true);
  if (c[20] == 0xBD and c[21] == 0x29 and c[22] == 0x03 and c[23] == 0x11)
  {
    addr = 0x09;
#ifdef DEBUG
    Serial1.println("OASIS 14/69R1W");
#endif
    // panel resolution: 3840x2400
    multi_x = 10;
    multi_y = 10;
  }
   //**************************************************************************************************************
  i = 0;
  // CHANGE: 2026-10-15
  memset(c, 0, sizeof(c));
  // END CHANGE
  Wire.beginTransmission(0x34);
  Wire.write(00);
  Wire.write(00);
  Wire.endTransmission();
  Wire.requestFrom(0x34, 30);
  while (Wire.available())
 { 
  c[i] = Wire.read();
  i++;
 }
if (c[20] == 0xD2 and c[21] == 0x1F and c[22] == 0x05 and c[23] == 0xB0)
 {
  addr = 0x34;
#ifdef DEBUG_IN
  Serial1.println("SOUTHPEAK 14/V50T5");
 #endif
       // panel resolution: 1920x1080
      // multi_x = 96;
    // multi_y = 162;
  multi_x = 80;
  multi_y = 79;
}
if (c[20] == 0xD2 and c[21] == 0x1F and c[22] == 0x06 and c[23] == 0x90)
{
  addr = 0x34;
#ifdef DEBUG_IN
  Serial1.println("CYBORG L14/NXKRH");
#endif
  // panel resolution: 1920x1080
  // multi_x = 96;
  // multi_y = 162;
  multi_x = 80;
  multi_y = 80;
}
if (c[20] == 0xD2 and c[21] == 0x1F and c[22] == 0x11 and c[23] == 0xB0)
{
    addr = 0x34;
#ifdef DEBUG_IN
  Serial1.println("OASIS 14/FXP48");
#endif
  // panel resolution: 1920x1080
  // multi_x = 96;
  // multi_y = 162;
  multi_x = 80;
  multi_y = 80;
}



  //*************************************************                 G2TOUCH G7500 ADDR- 0x40                *************************************************

  //   i = 0;
  //   Wire.beginTransmission(0x40);
  //   Wire.write(01);
  //   Wire.write(00);
  //   i2c_error = Wire.endTransmission();

  //   Wire.requestFrom(0x40, 30);
  //   while (Wire.available())
  //   {
  //     c[i] = Wire.read();
  //     i++;
  //   }
  // //   if (c[20] == 0x00 and c[21] == 0x00 and c[22] == 0x00 and c[23] == 0x00)
  // //   {
  // //     addr = 0x40;
  // //     platform = G2TOUCH_G7500;
  // // #ifdef DEBUG
  // //     Serial1.println("HP /L37862-L93/NV140FHM-T01");
  // // #endif
  // //     // panel resolution: 1920x1080
  // //     multi_x = 172;
  // //     multi_y = 304;
  // //   }
  //   Serial1.print("HID DESC: ");
  //   Serial1.print(c[20], HEX);
  //   Serial1.print(" ");
  //   Serial1.print(c[21], HEX);
  //   Serial1.print(" ");
  //   Serial1.print(c[22], HEX);
  //   Serial1.print(" ");
  //   Serial1.print(c[23], HEX);
  //   Serial1.println("");

  //*************************************************   0x0A RAMIA / BUMBLEBEE / ZIG-ZAG / BERLINETTA CFL-R   *************************************************
  
  i = 0;
  // CHANGE: 2026-10-15
  memset(c, 0, sizeof(c));
  // END CHANGE
  Wire.beginTransmission(0x0A);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();
  // Read TS data: 30Bytes from 0x0A SLA ADDR
  Wire.requestFrom(0x0A, 30);
  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }

  if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0x1C and c[23] == 0x48)
  {
#ifdef DEBUG
    Serial1.println("RAMIA");
#endif
    addr = 0x0A;
  }
  else if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0x98 and c[23] == 0x48)
  {
#ifdef DEBUG
    Serial1.println("BUMBLEBEE");
#endif
    addr = 0x0A;
  }
  else if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0xD4 and c[23] == 0x48)
  {
#ifdef DEBUG
    Serial1.println("ZIG_ZAG");
#endif
    addr = 0x0A;
  }
  else if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0x0B and c[23] == 0x49)
  {
#ifdef DEBUG
    Serial1.println("BERLINETTA");
#endif
    addr = 0x0A;
  }
  //   else if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0xA0 and c[23] == 0x49)
  //   {
  // #ifdef DEBUG
  //     Serial1.println("SOUTHPEAK 13 TGL");
  // #endif
  //     Southpeak_TS_Init();
  //     platform = SOUTHPEAK_13_TGL;
  //     addr = 0x0A;
  //   }
  else if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0xA3 and c[23] == 0x49)
  {
#ifdef DEBUG
    Serial1.println("CARNOUSTIE / J5RTR");
#endif
    // Southpeak_TS_Init();
    platform = WACOM_G14;

    // multi_x = 92;
    // multi_y = 158;
    multi_x = 30;
    multi_y = 45;
    addr = 0x0A;
  }
  else if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0xA5 and c[23] == 0x49)
  {
#ifdef DEBUG
    Serial1.println("CARNOUSTIE1 / J5RTR");
#endif
    // Southpeak_TS_Init();
    platform = WACOM_G14;

    // multi_x = 92;
    // multi_y = 158;
    multi_x = 30;
    multi_y = 45;
    addr = 0x0A;
  }
  else if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0xA3 and c[23] == 0x49) // c[22] == 0xA3
  {
#ifdef DEBUG
    Serial1.println("SOUTHPEAK 13 ");
#endif
    // Southpeak_TS_Init();
    platform = WACOM_G14;

    // multi_x = 92;
    // multi_y = 158;
    multi_x = 30;
    multi_y = 45;
    addr = 0x0A;

    // montreux do weryfikacji, sprawdzić na platformie?
  }
  else if (c[20] == 0x6A and c[21] == 0x05 and c[22] == 0x8E and c[23] == 0x49)
  {
#ifdef DEBUG
    Serial1.println("MONTREUX 14 / D2VW9");
#endif
    Southpeak_TS_Init();
    platform = WACOM_G14;

    //     // multi_x = 92;
    //     // multi_y = 158;
   multi_x = 28;
   multi_y = 50;
   addr = 0x0A;
  }
  else
  {
#ifdef DEBUG
    Serial1.println("ELSE");
    Serial1.print("HID DESC: ");
    Serial1.print(c[20], HEX);
    Serial1.print(" ");
    Serial1.print(c[21], HEX);
    Serial1.print(" ");
    Serial1.print(c[22], HEX);
    Serial1.print(" ");
    Serial1.print(c[23], HEX);
    Serial1.println("");
#endif
    // addr = 0x0A;
  }

  //*************************************************                 0x10 FIORANO / MockingBird                  *************************************************
  i = 0;
  // CHANGE: 2026-10-15
  memset(c, 0, sizeof(c));
  // END CHANGE
  bool lxSemiconFound = false;
  // check if LX SEMICON HID ADDR 0x00
  Wire.beginTransmission(0x10);
  Wire.write(00);
  Wire.write(00);
  Wire.endTransmission(false);

  Wire.requestFrom(0x10, 30);
  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }
  

  // *************************************************************************  LX SEMICON HID ADDR 0x00
  if (c[20] == 0xD2 and c[21] == 0x1F and c[22] == 0x01 and c[23] == 0x80)
  {
    addr = 0x10;
    platform = SW42101A;
#ifdef DEBUG
    Serial1.println("ARCHES SW42101A");
#endif
    // panel resolution: 3840x2400
    multi_x = 10;
    multi_y = 10;
  }


  Wire.endTransmission(true);
  // else
  Wire.beginTransmission(0x10);
  Wire.write(01);
  Wire.write(00);
  Wire.endTransmission(false);
  // CHANGE: 2026-10-15
  i=0;
  // CHANGE: 2026-10-15
  memset(c, 0, sizeof(c));
  // END CHANGE
  // END CHANGE
  Wire.requestFrom(0x10, 30);
  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }
  Wire.endTransmission(true);


  // *************************************************************************  LX SEMICON

  if (c[20] == 0xBD and c[21] == 0x29 and c[22] == 0x01 and c[23] == 0x91)
  {
    addr = 0x10;
    platform = LX92513;
#ifdef DEBUG
    Serial1.println("OASIS 16/8HT0J");
#endif
    // panel resolution: 3840x2400
    multi_x = 10;
    multi_y = 10;
  }
  else if (c[20] == 0xBD and c[21] == 0x29 and c[22] == 0x03 and c[23] == 0x33)
  {
    addr = 0x10;
    platform = LX92513;
#ifdef DEBUG
    Serial1.println("TRIBUTO/RK44J");
#endif
    // panel resolution: 3840x2400
    multi_x = 10;
    multi_y = 10;
  } 
    // do usuniecia
  //****************************************************************************
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x82 and c[23] == 0x2A) // if (c[20] == 243 and c[21] == 4 and c[22] == 130 and c[23] == 42)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("MOCKINGBIRD");
#endif
    // panel resolution: 1920x1080
    multi_x = 92;
    multi_y = 158;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xE4 and c[23] == 0x29)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("FIORANO");
#endif
    // panel resolution: 3840x2400
    multi_x = 84;
    multi_y = 134;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xB4 and c[23] == 0x2E)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("CINAMONBAY-16");
#endif
    // panel resolution: 3840x2400
    multi_x = 81;
    multi_y = 134;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x46 and c[23] == 0x28)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("SOUTHPEAK 14 MLK");
#endif
    // panel resolution: 1920x1080
    multi_x = 94;
    multi_y = 165;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x56 and c[23] == 0x28)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("SOUTHPEAK 14");
#endif
    // panel resolution: 1920x1080
    multi_x = 99;
    multi_y = 173;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x85 and c[23] == 0x28)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("NORTH BAY 15P MLK");
#endif
    // panel resolution: 1920x1080
    multi_x = 84;
    multi_y = 152;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x30 and c[23] == 0x28)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("SOUTHPEAK 13");
#endif
    // panel resolution: 1920x1080
    multi_x = 96;
    multi_y = 162;
  }
  else if (c[20] == 0xD2 and c[21] == 0x1F and c[22] == 0x08 and c[23] == 0x80) // D2	1F	8	80
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("SOUTHPEAK 15");
#endif
    // panel resolution: 1920x1080
    multi_x = 96;
    multi_y = 162;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x86 and c[23] == 0x28)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("MONTREUX 13");
#endif
    // panel resolution: 1920x1080
    multi_x = 99;
    multi_y = 174;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x59 and c[23] == 0x2F)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("OASIS 14/GN3CP");
#endif
    // panel resolution: 1920x1200
    multi_x = 92;
    multi_y = 152;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xCE and c[23] == 0x40)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("ARCHES 13/W9RMP");
#endif
    // panel resolution: 1920x1200
    multi_x = 99;
    multi_y = 174;
  }
  // else if (c[20] == 0xD2 and c[21] == 0x1F and c[22] == 0x01 and c[23] == 0x80) // D2	1F	1	80
  //{
  //  addr = 0x10;
//#ifdef DEBUG
//    Serial1.println("Arches 13/J9JTD");
//#endif
    // panel resolution: 1920x1080
//    multi_x = 96;
//    multi_y = 162;
 // }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xA7 and c[23] == 0x2B)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("SOUTHPEAK 14/T066T/JN7TT");
#endif
    // panel resolution: 1920x1080
    multi_x = 110;
    multi_y = 195;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xD6 and c[23] == 0x41)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("ARCHES 14");
#endif
    // panel resolution: 1920x1080
    multi_x = 110;
    multi_y = 195;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x6B and c[23] == 0x2C)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("SOUTHPEAK 15/M5DY2");
#endif
    // panel resolution: 1920x1200
    multi_x = 84;
    multi_y = 152;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x24 and c[23] == 0x2D)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("FIORANO ADL/NTM4F");
#endif
    // panel resolution: 3840x2400
    multi_x = 84;
    multi_y = 134;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xB2 and c[23] == 0x40)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("OASIS 13/PV16M");
#endif
    // panel resolution: 1920x1200
    multi_x = 97;
    multi_y = 160;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xB6 and c[23] == 0x40)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("OASIS 13/PV16M");
#endif
    // panel resolution: 1920x1200
    multi_x = 105;
    multi_y = 167;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xAF and c[23] == 0x40)
  {
    addr = 0x10;
    platform = eKTH5018;
#ifdef DEBUG
    Serial1.println("OASIS 14/V8892");
#endif
    // panel resolution: 1920x1200
    // multi_x = 105;
    // multi_y = 167;
    multi_x = 84;
    multi_y = 136;
  }
   else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x0E and c[23] == 0x41)
  {
    addr = 0x10;
    platform = eKTH5018;
#ifdef DEBUG
    Serial1.println("B133UAK01.0");
#endif
    // panel resolution: 1920x1200
    // multi_x = 105;
    // multi_y = 167;
    multi_x = 287;
    multi_y = 179;
  }
 else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xE2 and c[23] == 0x41)
  {
    addr = 0x10;
    platform = eKTH5015;
#ifdef DEBUG
    Serial1.println("WARPATH B140UAK01");
#endif
    // panel resolution: 1920x1200
    // multi_x = 105;
    // multi_y = 167;
    multi_x = 84;
    multi_y = 136;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x27 and c[23] == 0x43)
  {
    addr = 0x10;
    platform = eKTH5015;
#ifdef DEBUG
    Serial1.println("WARPATH B140UAV04");
#endif
    // panel resolution: 1920x1200
    // multi_x = 105;
    // multi_y = 167;
    multi_x = 84;
    multi_y = 136;
  }
  else if (c[20] == 0x94 and c[21] == 0x2A and c[22] == 0x4D and c[23] == 0xC0)
  {
    addr = 0x10;
    platform = eKTH5015;
#ifdef DEBUG
    Serial1.println("WARPATH NV140FHM-T0A-8940");
#endif
    // panel resolution: 1920x1200
    // multi_x = 105;
    // multi_y = 167;
    multi_x = 84;
    multi_y = 136;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x55 and c[23] == 0x2E)
  {
    addr = 0x10;
    platform = eKTH5015;
#ifdef DEBUG
    Serial1.println("WARPATH B140UAK01.0");
#endif
    // panel resolution: 1920x1200
    // multi_x = 105;
    // multi_y = 167;
    multi_x = 84;
    multi_y = 136;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xAE and c[23] == 0x40)
  {
    addr = 0x10;
    platform = eKTH5018;
#ifdef DEBUG
    Serial1.println("OASIS 13/V09XT");
#endif
    // panel resolution: 1920x1200
    multi_x = 82;
    multi_y = 132;
  }
   else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x59 and c[23] == 0x2F)
   {
    addr = 0x10;
    platform = eKTH5015;
#ifdef DEBUG
    Serial1.println("NV140WUM-T03");
#endif
    // panel resolution: 1920x1200
    multi_x = 82;
    multi_y = 132;
  }
   else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x71 and c[23] == 0x2E)
   {
    addr = 0x10;
    platform = eKTH5015;
#ifdef DEBUG
    Serial1.println("NV133WUM-T02");
#endif
    // panel resolution: 1920x1200
    multi_x = 82;
    multi_y = 132;
  }
   else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x34 and c[23] == 0x2E)
   {
    addr = 0x10;
    platform = eKTH5015;
#ifdef DEBUG
    Serial1.println("HP M73319-N31");
#endif
    // panel resolution: 1920x1200
    multi_x = 104;
    multi_y = 167;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x24 and c[23] == 0x24)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("HP L64592-001/B140HAK03-V3-10");
#endif
    // panel resolution: 1920x1200
    multi_x = 84;
    multi_y = 136;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xA2 and c[23] == 0x40)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("OASIS 14 / PGCWT");
#endif
    // panel resolution: 1920x1200
    multi_x = 104;
    multi_y = 167;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xB8 and c[23] == 0x2F)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("RIESLING /FN16697-001/B140HAN04.D"); // po zaprojektowaniu PCB mnożniki do ponownego sprawdzenia
#endif
    // panel resolution: 1920x1200
    multi_x = 97;
    multi_y = 159;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xBE and c[23] == 0x2F)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("RIESLING /FN16697-001/N140HCA-E5B"); // po zaprojektowaniu PCB mnożniki do ponownego sprawdzenia
#endif
    // panel resolution: 1920x1200
    multi_x = 97;
    multi_y = 159;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x6E and c[23] == 0x40)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("WELLER /FN12551-001/B156HAN09.1"); // po zaprojektowaniu PCB mnożniki do ponownego sprawdzenia
#endif
    // panel resolution: 1920x1200
    multi_x = 83;
    multi_y = 145;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xA0 and c[23] == 0x2F)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("WELLER /FN12531-001/ATNA56YX05"); // po zaprojektowaniu PCB mnożniki do ponownego sprawdzenia
#endif
    // panel resolution: 1920x1200
    multi_x = 83;
    multi_y = 145;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x5B and c[23] == 0x2A)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("HP /L75254-331/B133HAK02-V4");
#endif
    // panel resolution: 1920x1200
    multi_x = 99;
    multi_y = 174;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x73 and c[23] == 0x40)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("HAYDEN /FN15176-001/NE160QDM-NY6");
#endif
    multi_x = 96;
    multi_y = 158;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x95 and c[23] == 0x40)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("MONTREUX 14 MLK/08M1N/MCJ60");
#endif
    // panel resolution: 1920x1080
    multi_x = 110;
    multi_y = 195;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x69 and c[23] == 0x2A)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("HP /L75255-N93/R140NWF5-RE");
#endif
    // panel resolution: 1920x1200
    multi_x = 96;
    multi_y = 175;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x44 and c[23] == 0x28)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("NORTH BAY 14 MLK /1V5HH/C8KTC");
#endif
    // panel resolution: 1920x1080
    multi_x = 94;
    multi_y = 165;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0xA7 and c[23] == 0x2B)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("CYBORG L14 /0K4N0/RDT53");
#endif
    // panel resolution: 1920x1080
    multi_x = 94;
    multi_y = 165;
  }
  else if (c[20] == 0xF3 and c[21] == 0x04 and c[22] == 0x50 and c[23] == 0x2E)
  {
    addr = 0x10;
#ifdef DEBUG
    Serial1.println("HP /M73319-J31/N133JCN-EE1-C1");
#endif
    // panel resolution: 1920x1200
    multi_x = 104;
    multi_y = 167;
  }
  //   else if (c[20] == 0x00 and c[21] == 0x00 and c[22] == 0x00 and c[23] == 0x00)
  //   {
  //     // addr = 0x10;
  //     Wire.beginTransmission(0x10);
  //     i2c_error = Wire.endTransmission();

  //     if (i2c_error == 0)
  //       addr = 0x10;
  //     else
  //     {
  //       Wire.beginTransmission(0x40);
  //       i2c_error = Wire.endTransmission();
  //       if (i2c_error == 0)
  //         addr = 0x40;
  //     }
  //     platform = G2TOUCH_G7500;
  // #ifdef DEBUG
  //     Serial1.println("HP /L75255-L91/NV140FHM-T0A");
  // #endif
  //     // panel resolution: 1920x1080
  //     multi_x = 172;
  //     multi_y = 304;
  //   }
  Serial1.print("HID DESC: ");
  Serial1.print(c[20], HEX);
  Serial1.print(" ");
  Serial1.print(c[21], HEX);
  Serial1.print(" ");
  Serial1.print(c[22], HEX);
  Serial1.print(" ");
  Serial1.print(c[23], HEX);
  Serial1.println("");
}
// F3	4	30	28

//-----------------------------------------------------------------------------------------------------------------------------------
void ReadTouchscreen_4B()
{
  uint8_t i = 0;
  int val = 0;
  // Read TouchScreen at 0x4B
  delay(5);
  Wire.requestFrom(0x4B, 20);
  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }

  reg = (c[18] * 256) + c[17];
  if (c[3] == 1 and c[19] == 1)
  {
    TouchscreenUSB.press(c[4], ((c[6] * 256) + c[5]) * 8, ((c[10] * 256) + c[9]) * 8);
    finger[c[4]] = 1;
  }
  else if (c[3] == 1 && c[19] > 1)
  {
    TouchscreenUSB.press(c[4], ((c[6] * 256) + c[5]) * 8, ((c[10] * 256) + c[9]) * 8);
    finger[c[4]] = 1;
    val = c[19];
    // CHANGE 2026-10-15
    for (int index = 2; index < val; index++) // być może do zmiany::  for (int index = 2; index <= val; index++) 
    // END CHANGE
    {
      delay(5);

      Wire.requestFrom(0x4B, 20);
      i = 0;
      while (Wire.available())
      {
        c[i] = Wire.read();
        i++;
      }
      if (reg == (c[18] * 256) + c[17])
      {
        TouchscreenUSB.press(c[4], ((c[6] * 256) + c[5]) * 8, ((c[10] * 256) + c[9]) * 8);
        finger[c[4]] = 1;
      }
    }
  }
  else if (c[3] == 0)
  {
    for (int index = 0; index < 10; index++)
    {
      TouchscreenUSB.release(index);
      finger[index] = 0;
    }
  }
  else if (c[19] != 0)
  {
    for (i = 0; i <= 9; i++)
    {
      if (finger[i] == 0)
        TouchscreenUSB.release(i);
      else
        finger[i] = 0;
    }
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------
void ReadTouchscreen_0A()
{ // RAMIA / BUMBLEBEE/ZIG-ZAG -> WACOM G12+
  uint8_t i = 0;
  Wire.beginTransmission(0x0A);
  Wire.write(03);
  Wire.write(00);
  Wire.endTransmission();
  delay(1);

  Wire.requestFrom(0x0A, 40);
  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }
  if (c[0] == 0)
  {
    Wire.beginTransmission(0x0A);
    Wire.write(04);
    Wire.write(00);
    Wire.endTransmission();
    delay(1);

    Wire.requestFrom(0x0A, 40);
    i = 0;
    while (Wire.available())
    {

      c[i] = Wire.read();
      i++;
    }
    if (c[0] == 0)
    {
      for (i = 1; i <= 39; i++)
        c[i] = 0;
    }
  }

  if (c[4] >= 6)
  {
    do
    {
      Wire.beginTransmission(0x0A);
      Wire.write(03);
      Wire.write(00);
      Wire.endTransmission();
      delay(1);

      Wire.requestFrom(0x0A, 40);
      i = 0;
      while (Wire.available())
      {
        c[i] = Wire.read();
        i++;
        if (i == 4)
          i = 75;
        if (i == 76)
          i = 40;
      }
      if (c[0] == 0)
      {
        Wire.beginTransmission(0x0A);
        Wire.write(04);
        Wire.write(00);
        Wire.endTransmission();
        delay(1);

        Wire.requestFrom(0x0A, 40);
        i = 0;
        while (Wire.available())
        {
          //*******************************************************************************
          c[i] = Wire.read();
          i++;
          if (i == 4)
            i = 75;
          if (i == 76)
            i = 40;
        }
      }
    } while (c[75] != 0);
  }
  else
  {
    for (i = 40; i <= 75; i++)
    {
      c[i] = 0;
    }
  }

  for (i = 6; i <= 69; i = i + 7)
  {
    if (c[i] == 1)
    {
      TouchscreenUSB.press(0, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[0] = 1;
    }
    else if (c[i] == 2)
    {
      TouchscreenUSB.press(1, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[1] = 1;
    }
    else if (c[i] == 3)
    {
      TouchscreenUSB.press(2, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[2] = 1;
    }
    else if (c[i] == 4)
    {
      TouchscreenUSB.press(3, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[3] = 1;
    }
    else if (c[i] == 5)
    {
      TouchscreenUSB.press(4, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[4] = 1;
    }
    else if (c[i] == 6)
    {
      TouchscreenUSB.press(5, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[5] = 1;
    }
    else if (c[i] == 7)
    {
      TouchscreenUSB.press(6, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[6] = 1;
    }
    else if (c[i] == 8)
    {
      TouchscreenUSB.press(7, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[7] = 1;
    }
    else if (c[i] == 9)
    {
      TouchscreenUSB.press(8, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[8] = 1;
    }
    else if (c[i] == 10)
    {
      TouchscreenUSB.press(9, ((c[i + 3] * 256) + c[i + 2]) * 2.8, ((c[i + 5] * 256) + c[i + 4]) * 5);
      finger[9] = 1;
    }
  }
  delay(1);
  for (i = 0; i <= 9; i++)
  {
    if (finger[i] == 0)
      TouchscreenUSB.release(i);
    else
      finger[i] = 0;
  }
  delay(5);
}

void ReadTouchscreen_Wacom_G14()
{
  int index = 0;
  uint8_t finger_cnt = 0;
  /*
   * after initialization , TS controller sends 1-5 21-bits data frames,
   * each dataframe  contains two finger coordinates
   *
   * for 1-2 fingers active controller sends one data frame
   * * for 3-4 fingers active controller sends two data frames
   * * * for 5-6 fingers active controller sends three data frames
   * * * * for 7-8 fingers active controller sends four data frames
   * * * * * for 9-10 fingers active controller sends five data frames
   * e.g 10 fingers detected
   *
   *               15	00	1C	00	0A	05	01	00	8C	03	D8	0C	05	02	00	19	0A	31	08	18	94 00 00 00
   *               15	00	1C	00	00	05	03	00	39	0F	E9	06	05	04	00	C7	12	00	0A	18	94 00 00 00
   *               15	00	1C	00	00	05	05	00	45	13	27	18	05	06	00	76	2D	49	07	18	94 00 00 00
   *               15	00	1C	00	00	05	07	00	47	28	83	05	05	08	00	E8	23	77	05	18	94 00 00 00
   *               15	00	1C	00	00	05	09	00	0F	21	38	0A	05	0A	00	DF	22	D3	14	18	94 00 00 00
   *
   * 5 byte - number of detected fingers
   * 6, 13 byte - finger number
   * 9-10, 16-17 bytes - x position
   * 11-12, 18-19 bytes - y position
   *
   * only first frame contains a field informing about the number of detected fingers
   * in other frames this field always contains the value 0x00
   * if second finger in frame is not detected, bytes 13-19 always equal 0x00
   * if pair of fingers is not detected - TS controller skips sending this frame
   * if any finger is not detected, - TS controller sends frame 24-bytes frame 0x04, 0x00, 0xFF.....0xFF
   *
   */

  Wire.beginTransmission(0x0A);
  Wire.write(0x04); // 0x06
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(0x0A, 24);
  while (Wire.available())
  {
    data[index] = Wire.read();
    index++;
  }
  if (data[0] == 0x15 && data[1] == 0x00 && data[2] == 0x1C)
    finger_cnt = data[4];
  // else
  //   finger_cnt = 0;
  // if (finger_cnt >= 2)
  // {
  if (finger_cnt > 0)
  {
    Wire.beginTransmission(0x0A);
    Wire.write(0x03); // 0x06
    Wire.write(0x00);
    Wire.endTransmission(false);
    Wire.requestFrom(0x0A, 24);
    while (Wire.available())
    {
      data[index] = Wire.read();
      index++;
    }

    delayMicroseconds(2000);
    // Wire.beginTransmission(0x0A);
    // Wire.write(0x05);
    // Wire.write(0x00);
    // Wire.endTransmission(false);
    Wire.requestFrom(0x0A, 24);
    while (Wire.available())
    {
      data[index] = Wire.read();
      index++;
    }
    // }
    // if (finger_cnt >= 4)
    // {
    delayMicroseconds(2000);
    // Wire.beginTransmission(0x0A);
    // Wire.write(0x04);
    // Wire.write(0x00);
    // Wire.endTransmission(false);
    Wire.requestFrom(0x0A, 24);
    while (Wire.available())
    {
      data[index] = Wire.read();
      index++;
    }
    // }
    // if (finger_cnt >= 6)
    // {
    delayMicroseconds(2000);
    // Wire.beginTransmission(0x0A);
    // Wire.write(0x03);
    // Wire.write(0x00);
    // Wire.endTransmission(false);
    Wire.requestFrom(0x0A, 24);
    while (Wire.available())
    {
      data[index] = Wire.read();
      index++;
    }
    // }
    // if (finger_cnt >= 8)
    // {
    delayMicroseconds(2000);
    // Wire.beginTransmission(0x0A);
    // Wire.write(0x02);
    // Wire.write(0x00);
    // Wire.endTransmission(false);
    Wire.requestFrom(0x0A, 24);
    while (Wire.available())
    {
      data[index] = Wire.read();
      index++;
    }
  }
  // for (int j = 0; j < 120; j++)
  // {
  //   Serial1.print(data[j], HEX);
  //   Serial1.print(",");
  //   // if (j % 23 == 0 && j > 0)
  //   //   Serial1.println("");
  // }
  // Serial1.println("");
  // Serial1.println("");
  // //}
  //  finger_cnt = data[4];
  // Serial1.print("FINGERS: ");
  // Serial1.println(finger_cnt);
  //  delay(8);
  if (data[6] == 1)
  {
    press_data_xy[0][0] = (data[8]) + (data[9] * 256);
    press_data_xy[0][1] = (data[10]) + (data[11] * 256);
  }
  if (data[13] == 2)
  {
    press_data_xy[1][0] = (data[15]) + (data[16] * 256);
    press_data_xy[1][1] = (data[17]) + (data[18] * 256);
  }
  if ((data[30] == 3) && data[24] == 0x15 && data[25] == 0x00 && data[26] == 0x1C)
  {
    press_data_xy[2][0] = (data[32]) + (data[33] * 256);
    press_data_xy[2][1] = (data[34]) + (data[35] * 256);
  }
  if ((data[37] == 4) && data[24] == 0x15 && data[25] == 0x00 && data[26] == 0x1C)
  {
    press_data_xy[3][0] = (data[39]) + (data[40] * 256);
    press_data_xy[3][1] = (data[41]) + (data[42] * 256);
  }
  if ((data[54] == 5) && data[48] == 0x15 && data[49] == 0x00 && data[50] == 0x1C)
  {
    press_data_xy[4][0] = (data[56]) + (data[57] * 256);
    press_data_xy[4][1] = (data[58]) + (data[59] * 256);
  }
  if ((data[61] == 6) && data[48] == 0x15 && data[49] == 0x00 && data[50] == 0x1C)
  {
    press_data_xy[5][0] = (data[63]) + (data[64] * 256);
    press_data_xy[5][1] = (data[65]) + (data[66] * 256);
  }
  if ((data[78] == 7) && data[72] == 0x15 && data[73] == 0x00 && data[74] == 0x1C)
  {
    press_data_xy[6][0] = (data[80]) + (data[81] * 256);
    press_data_xy[6][1] = (data[82]) + (data[83] * 256);
  }
  if ((data[85] == 8) && data[72] == 0x15 && data[73] == 0x00 && data[74] == 0x1C)
  {
    press_data_xy[7][0] = (data[87]) + (data[88] * 256);
    press_data_xy[7][1] = (data[89]) + (data[90] * 256);
  }
  if ((data[102] == 9) && data[96] == 0x15 && data[97] == 0x00 && data[98] == 0x1C)
  {
    press_data_xy[8][0] = (data[104]) + (data[105] * 256);
    press_data_xy[8][1] = (data[106]) + (data[107] * 256);
  }
  if ((data[109] == 10) && data[96] == 0x15 && data[97] == 0x00 && data[98] == 0x1C)
  {
    press_data_xy[9][0] = (data[111]) + (data[112] * 256);
    press_data_xy[9][1] = (data[113]) + (data[114] * 256);
  }

  for (int k = 0; k < 10; k++)
  {
    for (int j = 0; j < 2; j++)
    {
      Serial1.print(press_data_xy[k][j], HEX);
      Serial1.print(",");
    }
    Serial1.print(" ;  ");
  }
  Serial1.println("");
  Serial1.println("");

  if (finger_cnt > 0)
  {
    for (int k = 0; k < finger_cnt; k++)
    {
      TouchscreenUSB.press(k, (press_data_xy[k][0] * multi_x) / 10, (press_data_xy[k][1] * multi_y) / 10);
    }
    for (int k = finger_cnt; k < 10; k++)
    {
      TouchscreenUSB.release(k);
    }
    delay(10);
  }
  else
  {
    for (int k = 0; k < 10; k++)
    {
      TouchscreenUSB.release(k);
    }
  }
  delayMicroseconds(15000);
}

void Read_TS_Wacom_G14()
{
  int index = 0;
  uint8_t finger_cnt = 0;
  uint8_t frames = 0;
  /*
   * after initialization , TS controller sends 1-5 21-bits data frames,
   * each dataframe  contains two finger coordinates
   *
   * for 1-2 fingers active controller sends one data frame
   * * for 3-4 fingers active controller sends two data frames
   * * * for 5-6 fingers active controller sends three data frames
   * * * * for 7-8 fingers active controller sends four data frames
   * * * * * for 9-10 fingers active controller sends five data frames
   * e.g 10 fingers detected
   *
   *               15	00	1C	00	0A	05	01	00	8C	03	D8	0C	05	02	00	19	0A	31	08	18	94 00 00 00
   *               15	00	1C	00	00	05	03	00	39	0F	E9	06	05	04	00	C7	12	00	0A	18	94 00 00 00
   *               15	00	1C	00	00	05	05	00	45	13	27	18	05	06	00	76	2D	49	07	18	94 00 00 00
   *               15	00	1C	00	00	05	07	00	47	28	83	05	05	08	00	E8	23	77	05	18	94 00 00 00
   *               15	00	1C	00	00	05	09	00	0F	21	38	0A	05	0A	00	DF	22	D3	14	18	94 00 00 00
   *
   * 5 byte - number of detected fingers
   * 6, 13 byte - finger number
   * 9-10, 16-17 bytes - x position
   * 11-12, 18-19 bytes - y position
   *
   * only first frame contains a field informing about the number of detected fingers
   * in other frames this field always contains the value 0x00
   * if second finger in frame is not detected, bytes 13-19 always equal 0x00
   * if pair of fingers is not detected - TS controller skips sending this frame
   * if any finger is not detected, - TS controller sends frame 24-bytes frame 0x04, 0x00, 0xFF.....0xFF
   *
   */
  Wire.requestFrom(0x0A, 21);
  while (Wire.available())
  {
    data_m[0][index] = Wire.read();
    index++;
    frames = 1;
  }
  if (data_m[0][0] == 0x15 && data_m[0][1] == 0x00 && data_m[0][2] == 0x1C){
    finger_cnt = data_m[0][4];
  }

  if (finger_cnt > 2)
  {
    index = 0;
    frames = 2;
    // delayMicroseconds(1000);
    Wire.requestFrom(0x0A, 21);
    while (Wire.available())
    {
      data_m[1][index] = Wire.read();
      index++;
    }
  }
  if (finger_cnt > 4)
  {
    index = 0;
    frames = 3;
    // delayMicroseconds(1000);
    Wire.requestFrom(0x0A, 21);
    while (Wire.available())
    {
      data_m[2][index] = Wire.read();
      index++;
    }
  }
  if (finger_cnt > 6)
  {
    index = 0;
    frames = 4;
    //  delayMicroseconds(1000);
    Wire.requestFrom(0x0A, 21);
    while (Wire.available())
    {
      data_m[3][index] = Wire.read();
      index++;
    }
  }
  if (finger_cnt > 8)
  {
    index = 0;
    frames = 5;
    //   delayMicroseconds(1000);
    Wire.requestFrom(0x0A, 21);
    while (Wire.available())
    {
      data_m[4][index] = Wire.read();
      index++;
    }
  }

  for (uint8_t i = 0; i < frames; i++)
  {
    if (data_m[i][6] == ((2 * i) + 1))
      TouchscreenUSB.press(((2 * i) + 1), ((data_m[i][9] * 256 + data_m[i][8]) * multi_x) / 10, ((data_m[i][11] * 256 + data_m[i][10]) * multi_y) / 10);
    else
      TouchscreenUSB.release((2 * i) + 1);
    if (data_m[i][13] == ((2 * i) + 2))
      TouchscreenUSB.press(((2 * i) + 2), ((data_m[i][16] * 256 + data_m[i][15]) * multi_x) / 10, ((data_m[i][18] * 256 + data_m[i][17]) * multi_y) / 10);
    else
      TouchscreenUSB.release((2 * i) + 2);
  }
  // CHANGE 2026-10-15
  for (uint8_t i = frames; i < 5; i++)
  {
    TouchscreenUSB.release((2 * i) + 1);
    TouchscreenUSB.release((2 * i) + 2);
  }
  // END CHANGE
  delayMicroseconds(8050);
}

// void fingerPressRelease(uint8_t fingers, uint16_t data[][2], int mx, int my)
// {
//   for (int i = 0; i < fingers; i++)
//   {
//     TouchscreenUSB.press(i, (data[i][0] * mx) / 10, (data[i][1] * my) / 10);
//   }
//   for (int i = fingers; i < 10; i++)
//   {
//     TouchscreenUSB.release(i);
//   }
//   // // if (data[0] == 21 && data[1] == 0 && data[2] == 28)
//   // // {
//   // if (data[6] == 1)
//   //   TouchscreenUSB.press(0, (((data[9] * 256) + data[8]) * multi_x) / 10, (((data[11] * 256) + data[10]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(0);
//   // if (data[13] == 2)
//   //   TouchscreenUSB.press(1, (((data[16] * 256) + data[15]) * multi_x) / 10, (((data[18] * 256) + data[17]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(1);
//   // if (data[27] == 3)
//   //   TouchscreenUSB.press(2, (((data[30] * 256) + data[29]) * multi_x) / 10, (((data[32] * 256) + data[31]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(2);
//   // if (data[34] == 4)
//   //   TouchscreenUSB.press(3, (((data[37] * 256) + data[36]) * multi_x) / 10, (((data[39] * 256) + data[38]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(3);
//   // if (data[48] == 5)
//   //   TouchscreenUSB.press(4, (((data[51] * 256) + data[50]) * multi_x) / 10, (((data[53] * 256) + data[52]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(4);
//   // if (data[55] == 6)
//   //   TouchscreenUSB.press(5, (((data[58] * 256) + data[57]) * multi_x) / 10, (((data[60] * 256) + data[59]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(5);
//   // if (data[69] == 7)
//   //   TouchscreenUSB.press(6, (((data[72] * 256) + data[71]) * multi_x) / 10, (((data[74] * 256) + data[73]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(6);
//   // if (data[76] == 8)
//   //   TouchscreenUSB.press(7, (((data[79] * 256) + data[78]) * multi_x) / 10, (((data[81] * 256) + data[80]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(7);
//   // if (data[90] == 9)
//   //   TouchscreenUSB.press(8, (((data[93] * 256) + data[92]) * multi_x) / 10, (((data[95] * 256) + data[94]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(8);
//   // if (data[97] == 10)
//   //   TouchscreenUSB.press(9, (((data[100] * 256) + data[99]) * multi_x) / 10, (((data[102] * 256) + data[101]) * multi_y) / 10);
//   // else
//   //   TouchscreenUSB.release(9);
//   // // }
//   // // else
//   // // {
//   // //   for (int k = 0; k < 10; k++)
//   // //   {
//   // //     TouchscreenUSB.release(k);
//   // //   }
//   // // }
// }

void ReadTouchscreen_10()
{
  i = 0;
  Wire.requestFrom(0x10, 63);
  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }
  if (c[0] == 63 and c[1] == 0 and c[2] == 1) // Header
  {
    for (i = 3; i <= 56; i = i + 11)
    {

      if (c[i] == 7 or c[i] == 11 or c[i] == 15 or c[i] == 19 or c[i] == 23 or c[i] == 27 or c[i] == 31 or c[i] == 35 or c[i] == 39 or c[i] == 43)
      {
        // TouchscreenUSB.press(   FINGER NR  ,                  X                 ,                 Y                   );
        // TouchscreenUSB.press((c[i] - 7) / 4, ((c[i + 4] * 256) + c[i + 3]) * 9.2, ((c[i + 8] * 256) + c[i + 7]) * 15.8);
        TouchscreenUSB.press((c[i] - 7) / 4, ((((c[i + 4] * 256) + c[i + 3]) * multi_x) / 10), (((c[i + 8] * 256) + c[i + 7]) * multi_y) / 10);
      }

      else if (c[i] == 4 or c[i] == 8 or c[i] == 12 or c[i] == 16 or c[i] == 20 or c[i] == 24 or c[i] == 28 or c[i] == 32 or c[i] == 36 or c[i] == 40)
      {
        TouchscreenUSB.release((c[i] - 4) / 4);
      }
    }
  } // tu normalnie powinien być delay(4)
  if (c[62] > 5) // więcej niż 5 palców
  {
    delayMicroseconds(550);
    i = 0;
    Wire.requestFrom(0x10, 63);
    while (Wire.available())
    {
      c[i] = Wire.read();
      i++;
    }
    if (c[0] == 63 and c[1] == 0 and c[2] == 1) // Header
    {
      for (i = 3; i <= 56; i = i + 11)
      {
        if (c[i] == 7 or c[i] == 11 or c[i] == 15 or c[i] == 19 or c[i] == 23 or c[i] == 27 or c[i] == 31 or c[i] == 35 or c[i] == 39 or c[i] == 43)
        {
          // TouchscreenUSB.press((c[i] - 7) / 4, ((c[i + 4] * 256) + c[i + 3]) * 9.2, ((c[i + 8] * 256) + c[i + 7]) * 15.8);
          TouchscreenUSB.press((c[i] - 7) / 4, ((((c[i + 4] * 256) + c[i + 3]) * multi_x) / 10), (((c[i + 8] * 256) + c[i + 7]) * multi_y) / 10);
        }
        else // AKCEPTUJE BEZ ZMIAN: if (c[i] == 4 or c[i] == 8 or c[i] == 12 or c[i] == 16 or c[i] == 20 or c[i] == 24 or c[i] == 28 or c[i] == 32 or c[i] == 36 or c[i] == 40)
        {
          TouchscreenUSB.release((c[i] - 4) / 4);
        }
      }
    }
    delayMicroseconds(10000); // 5500
  }
  else
    delayMicroseconds(15800); // 7900
}

void ReadTouchscreen_eKTH5018()
{
  i = 0;
  Wire.requestFrom(0x10, 63);
  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }
  if (c[0] == 63 and c[1] == 0 and c[2] == 1) // Header
  {
    for (i = 3; i <= 56; i = i + 11)
    {

      if (c[i] == 135 or c[i] == 139 or c[i] == 143 or c[i] == 147 or c[i] == 151 or c[i] == 155 or c[i] == 159 or c[i] == 163 or c[i] == 167 or c[i] == 171)
      {
        // TouchscreenUSB.press(   FINGER NR  ,                  X                 ,                 Y                   );
        // TouchscreenUSB.press((c[i] - 7) / 4, ((c[i + 4] * 256) + c[i + 3]) * 9.2, ((c[i + 8] * 256) + c[i + 7]) * 15.8);
        TouchscreenUSB.press((c[i] - 135) / 4, ((((c[i + 4] * 256) + c[i + 3]) * multi_x) / 10), (((c[i + 8] * 256) + c[i + 7]) * multi_y) / 10);
      }

      else if (c[i] == 132 or c[i] == 136 or c[i] == 140 or c[i] == 144 or c[i] == 148 or c[i] == 152 or c[i] == 156 or c[i] == 160 or c[i] == 164 or c[i] == 168)
      {
        TouchscreenUSB.release((c[i] - 132) / 4);
      }
    }
  } // tu normalnie powinien być delay(4)
  if (c[62] > 5) // więcej niż 5 palców
  {
    delayMicroseconds(550);
    i = 0;
    Wire.requestFrom(0x10, 63);
    while (Wire.available())
    {
      c[i] = Wire.read();
      i++;
    }
    if (c[0] == 63 and c[1] == 0 and c[2] == 1) // Header
    {
      for (i = 3; i <= 56; i = i + 11)
      {
        if (c[i] == 135 or c[i] == 139 or c[i] == 143 or c[i] == 147 or c[i] == 151 or c[i] == 155 or c[i] == 159 or c[i] == 163 or c[i] == 167 or c[i] == 171)
        {
          // TouchscreenUSB.press((c[i] - 7) / 4, ((c[i + 4] * 256) + c[i + 3]) * 9.2, ((c[i + 8] * 256) + c[i + 7]) * 15.8);
          TouchscreenUSB.press((c[i] - 135) / 4, ((((c[i + 4] * 256) + c[i + 3]) * multi_x) / 10), (((c[i + 8] * 256) + c[i + 7]) * multi_y) / 10);
        }
        else if (c[i] == 132 or c[i] == 136 or c[i] == 140 or c[i] == 144 or c[i] == 148 or c[i] == 152 or c[i] == 156 or c[i] == 160 or c[i] == 164 or c[i] == 168)
        {
          TouchscreenUSB.release((c[i] - 132) / 4);
        }
      }
    }
    delayMicroseconds(10000); // 5500
  }
  else
    delayMicroseconds(15800); // 7900
}

void ReadTouchscreen_eKTH5015()
{
  i = 0;
  Wire.requestFrom(0x10, 63);
  while (Wire.available())
  {
    c[i] = Wire.read();
    i++;
  }
  if (c[0] == 63 and c[1] == 0 and c[2] == 1) // Header
  {
    for (i = 3; i <= 56; i = i + 11)
    {

      if (c[i] == 7 or c[i] == 11 or c[i] == 15 or c[i] == 19 or c[i] == 23 or c[i] == 27 or c[i] == 31 or c[i] == 35 or c[i] == 39 or c[i] == 43)
      {
        // TouchscreenUSB.press(   FINGER NR  ,                  X                 ,                 Y                   );
        // TouchscreenUSB.press((c[i] - 7) / 4, ((c[i + 4] * 256) + c[i + 3]) * 9.2, ((c[i + 8] * 256) + c[i + 7]) * 15.8);
        TouchscreenUSB.press((c[i] - 7) / 4, ((((c[i + 4] * 256) + c[i + 3]) * multi_x) / 10), (((c[i + 8] * 256) + c[i + 7]) * multi_y) / 10);
      }

      else if (c[i] == 4 or c[i] == 8 or c[i] == 12 or c[i] == 16 or c[i] == 20 or c[i] == 24 or c[i] == 28 or c[i] == 32 or c[i] == 36 or c[i] == 40)
      {
        TouchscreenUSB.release((c[i] - 4) / 4);
      }
    }
  } // tu normalnie powinien być delay(4)
  if (c[62] > 5) // więcej niż 5 palców
  {
    delayMicroseconds(550);
    i = 0;
    Wire.requestFrom(0x10, 63);
    while (Wire.available())
    {
      c[i] = Wire.read();
      i++;
    }
    if (c[0] == 63 and c[1] == 0 and c[2] == 1) // Header
    {
      for (i = 3; i <= 56; i = i + 11)
      {
        if (c[i] == 7 or c[i] == 11 or c[i] == 15 or c[i] == 19 or c[i] == 23 or c[i] == 27 or c[i] == 31 or c[i] == 35 or c[i] == 39 or c[i] == 43)
        {
          // TouchscreenUSB.press((c[i] - 7) / 4, ((c[i + 4] * 256) + c[i + 3]) * 9.2, ((c[i + 8] * 256) + c[i + 7]) * 15.8);
          TouchscreenUSB.press((c[i] - 7) / 4, ((((c[i + 4] * 256) + c[i + 3]) * multi_x) / 10), (((c[i + 8] * 256) + c[i + 7]) * multi_y) / 10);
        }

        else if (c[i] == 4 or c[i] == 8 or c[i] == 12 or c[i] == 16 or c[i] == 20 or c[i] == 24 or c[i] == 28 or c[i] == 32 or c[i] == 36 or c[i] == 40)
        {
          TouchscreenUSB.release((c[i] - 4) / 4);
        }
      }
    }
    delayMicroseconds(10000); // 5500
  }
  else
    delayMicroseconds(15800); // 7900
}
// void Southpeak_TS_Init()
// {
//   // Wire.beginTransmission(0x0A);
//   // Wire.write(0x04);
//   // Wire.write(0x00);
//   // Wire.write(0x00);
//   // Wire.write(0x08);
//   // Wire.endTransmission();
//   // delay(2);
//   // Wire.beginTransmission(0x0A);
//   // Wire.write(0x04);
//   // Wire.write(0x00);
//   // Wire.write(0x00);
//   // Wire.write(0x01);
//   // Wire.endTransmission();
//   // delay(30);
//   // Wire.beginTransmission(0x0A);
//   // Wire.write(0x04);
//   // Wire.write(0x00);
//   // Wire.write(0x1C);
//   // Wire.write(0x04);
//   // Wire.endTransmission();

//   Wire.beginTransmission(0x0A);
//   Wire.write(0x04);
//   Wire.write(0x00);
//   Wire.write(0x3E);
//   Wire.write(0x03);
//   Wire.write(0x05);
//   Wire.write(0x00);
//   Wire.write(0x05);
//   Wire.write(0x00);
//   Wire.write(0x0E);
//   Wire.write(0x02);
//   Wire.write(0x00);
//   Wire.endTransmission();
// }

void ReadTouchscreen_Southpeak_13_TGL()
{
  /* for mockingbird, before use ReadTouchscreen_Mockingbird_13_TGL(), please use Southpeak_TS_Init()
   * after initialization , TS controller sends 1-5 24-bits data frames,
   * each dataframe  contains two finger coordinates
   *
   * for 1-2 fingers active controller sends one data frame
   * * for 3-4 fingers active controller sends two data frames
   * * * for 5-6 fingers active controller sends three data frames
   * * * * for 7-8 fingers active controller sends four data frames
   * * * * * for 9-10 fingers active controller sends five data frames
   * e.g 10 fingers detected
   *
   *         15	00	1C	00	0A	05	01	00	18	06	E2	0C	05	02	00	AC	23	99	05	AB	2F	00	00	00
   *         15	00	1C	00	00	05	03	00	B6	0A	24	09	05	04	00	D2	27	3C	08	AB	2F	00	00	00
   *         15	00	1C	00	00	05	05	00	17	1F	22	05	05	06	00	FC	13	64	15	AB	2F	00	00	00
   *         15	00	1C	00	00	05	07	00	48	1A	E0	14	05	08	00	5A	1A	1B	09	AB	2F	00	00	00
   *         15	00	1C	00	00	05	09	00	22	0E	C6	06	05	0A	00	A8	12	13	0A	AB	2F	00	00	00
   *
   * 5 byte - number of detected fingers
   * 6, 13 byte - finger number
   * 9-10, 16-17 bytes - x position
   * 11-12, 18-19 bytes - y position
   *
   * only first frame contains a field informing about the number of detected fingers
   * in other frames this field always contains the value 0x00
   * if second finger in frame is not detected, bytes 13-19 always equal 0x00
   * if pair of fingers is not detected - TS controller skips sending this frame
   * if any finger is not detected, - TS controller sends frame 24-bytes frame 0x04, 0x00, 0xFF.....0xFF
   *
   */
  //
  int mult_x = 1;
  int mult_y = 1;
  int fingers = 0;

  ReadTouchscreenXXX(0x0A, data, 24, 0);
  //  check how many fingers is detected
  if (data[4] != 0x00 && data[4] != 0xFF)
    fingers = data[4];

  if (fingers <= 4)
  {
    delayMicroseconds(1200);
    ReadTouchscreenXXX(0x0A, data, 24, 24);
  }
  else if (fingers <= 6)
  {
    delayMicroseconds(1200);
    ReadTouchscreenXXX(0x0A, data, 24, 48);
  }
  else if (fingers <= 8)
  {
    delayMicroseconds(1200);
    ReadTouchscreenXXX(0x0A, data, 24, 72);
  }
  else if (fingers <= 10)
  {
    delayMicroseconds(1200);
    ReadTouchscreenXXX(0x0A, data, 24, 96);
  }

  for (int index = 0; index < fingers; index++)
  {
    if ((data[6 + (index * 7)]) % 2 == 0)
    {
      TouchscreenUSB.press(index, ((data[17 + (index * 7)] * 256) + data[16 + (index * 7)]) * mult_x, ((data[19 + (index * 7)] * 256) + data[18 + (index * 7)]) * mult_y);
    }
    else
    {
      TouchscreenUSB.press(index, ((data[9 + (index * 7)] * 256) + data[8 + (index * 7)]) * mult_x, ((data[12 + (index * 7)] * 256) + data[11 + (index * 7)]) * mult_y);
    }
  }

  // if (data[6] = 1)
  //   TouchscreenUSB.press(0, ((data[9] * 256) + c[8]) * mult_x, ((c[12] * 256) + c[11]) * mult_y);
  // if (data[6] = 2)
  //   TouchscreenUSB.press(0, ((data[17] * 256) + c[16]) * mult_x, ((c[19] * 256) + c[18]) * mult_y);
  // if (data[6] = 3)
  //   TouchscreenUSB.press(0, ((data[24 + 9] * 256) + c[24 + 8]) * mult_x, ((c[24 + 12] * 256) + c[24 + 11]) * mult_y);
  // if (data[6] = 4)
  //   TouchscreenUSB.press(0, ((data[24 + 17] * 256) + c[24 + 16]) * mult_x, ((c[24 + 19] * 256) + c[24 + 18]) * mult_y);
  // if (data[6] = 5)
  //   TouchscreenUSB.press(0, ((data[48 + 9] * 256) + c[48 + 8]) * mult_x, ((c[48 + 12] * 256) + c[48 + 11]) * mult_y);
  // if (data[6] = 6)
  //   TouchscreenUSB.press(0, ((data[48 + 17] * 256) + c[48 + 16]) * mult_x, ((c[48 + 19] * 256) + c[48 + 18]) * mult_y);
  // if (data[6] = 7)
  //   TouchscreenUSB.press(0, ((data[72 + 9] * 256) + c[72 + 8]) * mult_x, ((c[72 + 12] * 256) + c[72 + 11]) * mult_y);
  // if (data[6] = 8)
  //   TouchscreenUSB.press(0, ((data[72 + 17] * 256) + c[72 + 16]) * mult_x, ((c[72 + 19] * 256) + c[72 + 18]) * mult_y);
  // if (data[6] = 9)
  //   TouchscreenUSB.press(0, ((data[96 + 9] * 256) + c[96 + 8]) * mult_x, ((c[96 + 12] * 256) + c[96 + 11]) * mult_y);
  // if (data[6] = 10)
  //   TouchscreenUSB.press(0, ((data[96 + 17] * 256) + c[96 + 16]) * mult_x, ((c[96 + 19] * 256) + c[96 + 18]) * mult_y);

  for (i = fingers; i < 10; i++)
  {
    TouchscreenUSB.release(i);
  }
  delay(9);
}

void ReadTouchscreen_Melfas_0x34()
{
  // int mult_x = 1;
  // int mult_y = 1;
  // int fingers = 0;
  // Serial1.println("Southpeak15_enter");
  ReadTouchscreen(0x34, 0x00, 0x00, data, 66, 0x00);
  if (data[0] == 66 && data[1] == 0x00 && data[2] == 16)
  {
    // Serial1.println("Southpeak15_frame");
    // fingers = data[3];
    // Serial1.print("fingers ");
    // Serial1.println(fingers);
    // for (i = 0; i < fingers; i++)
    for (i = 0; i < 10; i++)
    {
      // if finger exists
      // if (data[4] == 0 && data[5] == 1)
      //   TouchscreenUSB.press(0, (((data[7] * 256) + data[6]) * multi_x) / 10, (((data[9] * 256) + data[8]) * multi_y) / 10);
      // else if (data[4] == 0 && data[5] == 0)
      //   TouchscreenUSB.release(0);
      // delayMicroseconds(550);
      // if (data[10] == 1 && data[11] == 1)
      //   TouchscreenUSB.press(1, (((data[13] * 256) + data[12]) * multi_x) / 10, (((data[15] * 256) + data[14]) * multi_y) / 10);
      // else if (data[10] == 0 && data[11] == 0)
      //   TouchscreenUSB.release(1);

      if (data[5 + (i * 6)] == 1)
        TouchscreenUSB.press(i, (((data[7 + (i * 6)] * 256) + data[6 + (i * 6)]) * multi_x) / 10, (((data[9 + (i * 6)] * 256) + data[8 + (i * 6)]) * multi_y) / 10);
      else
        TouchscreenUSB.release(i);
      delayMicroseconds(100);
    }
    // if ((fingers > 0 && data[4] == 0) || (i > 0 && data[4 + i * 6] > 0 && data[4 + i * 6] <= fingers))
    // {
    //   TouchscreenUSB.press(i, ((data[7 + (i * 6)] * 256) + data[6 + (i * 6)]) * mult_x, ((data[9 + (i * 6)] * 256) + data[8 + (i * 6)]) * mult_y);
    // }
    // else
    // {
    //   TouchscreenUSB.release(i);
    // }
    //   }
  }
  delayMicroseconds(15800); // 7900
}

//                  (slave_address, pointer_to_data_array, array_size, index)
void ReadTouchscreenXXX(int slaAddr, int *pArray, int size, int index)
{
  //  int index = 0;
  Wire.requestFrom(slaAddr, size);
  while (Wire.available())
  {
    pArray[index] = Wire.read();
    index++;
  }
}

void ReadTouchscreen(int slaAddr, int reg_addr_l, int reg_addr_h, int *pArray, int size, int index)
{
  Wire.beginTransmission(slaAddr);
  Wire.write(reg_addr_l);
  Wire.write(reg_addr_h);
  Wire.endTransmission();
  delay(1);

  Wire.requestFrom(slaAddr, size);
  while (Wire.available())
  {
    pArray[index] = Wire.read();
    index++;
  }

  // Wire.beginTransmission(slaAddr);
  // Wire.write(reg_addr_l);
  // Wire.write(reg_addr_h);
  // Wire.endTransmission();
  // delay(1);
  // Wire.requestFrom(slaAddr, size);
  // while (Wire.available())
  // {
  //   for (int i = index; i < size; i++)
  //   {
  //     pArray[i] = Wire.read();
  //   }
  // }
}

void print_data_table(uint8_t *pArray, int size)
{
  for (i = 0; i <= size; i++)
  {
    Serial1.print(pArray[i], HEX);
    Serial1.print(",");
  }
  Serial1.println("");
}

void SetupOnDemandReadFlag()
{
  readTSOnDemandFlag = true;
}

void ReadTouchscreen_G7500()
{

  int fingers = 0;
  // first data frame
  if (addr == 0x10)
  {
    ReadTouchscreen(0x10, 0x01, 0x00, data, 36, 0);
    delay(1);
    fingers = data[35];
    if (fingers > 5)
      ReadTouchscreen(0x10, 0x01, 0x00, data, 36, 36);
    delay(1);

    if (data[0] == 36 && data[1] == 0 && data[2] == 1) 
    {

      for (int i = 0; i < fingers; i++)
      {
        if (i < 5)
          TouchscreenUSB.press(i, ((((data[6 + (6 * i)] * 256) + (((data[5 + (6 * i)])))) * multi_x) / 10), ((((data[8 + (6 * i)] * 256) + (data[7 + (6 * i)])) * multi_y) / 10));
        else
          TouchscreenUSB.press(i, ((((data[42 + (6 * (i - 5))] * 256) + (((data[41 + (6 * (i - 5))])))) * multi_x) / 10), ((((data[44 + (6 * (i - 5))] * 256) + (data[43 + (6 * (i - 5))])) * multi_y) / 10));
      }
      for (i = (fingers); i < 10; i++)
      {
        TouchscreenUSB.release(i);
      }
    }
    else
    {
      for (int i = 0; i < 10; i++)
      {
        TouchscreenUSB.release(i);
      }
    }
  }
  else if (addr == 0x40)
  {
    // Serial1.println("G7500");
    ReadTouchscreen(0x40, 0x01, 0x00, data, 46, 0);
    delay(1);
    fingers = data[45];
    if (fingers > 5)
      ReadTouchscreen(0x40, 0x01, 0x00, data, 46, 46);
    delay(1);

    if (data[0] == 36 && data[1] == 0 && data[2] == 1)
    {

      for (int i = 0; i < fingers; i++)
      {
        if (i < 5)
          TouchscreenUSB.press(i, ((((data[6 + (7 * i)] * 256) + (((data[5 + (7 * i)])))) * multi_x) / 10), ((((data[8 + (7 * i)] * 256) + (data[7 + (7 * i)])) * multi_y) / 10));
        else
          TouchscreenUSB.press(i, ((((data[52 + (7 * (i - 5))] * 256) + (((data[51 + (6 * (i - 5))])))) * multi_x) / 10), ((((data[54 + (6 * (i - 5))] * 256) + (data[53 + (6 * (i - 5))])) * multi_y) / 10));
      }
      for (i = (fingers); i < 10; i++)
      {
        TouchscreenUSB.release(i);
      }
    }
    else
    {
      for (int i = 0; i < 10; i++)
      {
        TouchscreenUSB.release(i);
      }
    }
  }
  delay(10);
}

void LXSemicon_LX9215B_init()
{
  uint8_t err;
  Wire.beginTransmission(0x10);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x08);
  err = Wire.endTransmission(true);
  delay(10);
  Wire.beginTransmission(0x10);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x01);
  err = Wire.endTransmission(true);
  LX9215B_init = true;
}

void Southpeak_TS_Init()
{
  uint8_t err;
  Wire.beginTransmission(0x0A);
  Wire.write(0x04);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x08);
  err = Wire.endTransmission(true);
  delay(10);
  Wire.beginTransmission(0x0A);
  Wire.write(0x4);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x01);
  err = Wire.endTransmission(true);
  WACOM_G14_init = true;
}
//void LXSemicon_LX9213B_init()
//{
//   uint8_t err;
//   Wire.beginTransmission(0x10);
//   Wire.write(0x22);
//   Wire.write(0x00);
//   Wire.write(0x00);
//   Wire.write(0x08);
//   err = Wire.endTransmission(true);
//   delay(10);
//   Wire.beginTransmission(0x10);
//   Wire.write(0x22);
//   Wire.write(0x00);
//   Wire.write(0x00);
//   Wire.write(0x01);
//   err = Wire.endTransmission(true);
//   LX9215B_init = true;
//}
void ReadTouchscreen_LXSemicon_LX92515B()
{
  uint8_t ii = 0;
  uint8_t fingers = 0;
  if (platform == LX92513)
    Wire.requestFrom(0x10, 0x42);
  else
    Wire.requestFrom(0x09, 0x42);
  while (Wire.available())
  {
    data[ii] = Wire.read();
    ii++;
  }

  if ((data[0] == 0x42 && data[1] == 0x00 && data[2] == 0x10) || (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00))
  {
    fingers = data[63];
    Serial1.print("FINGERS: ");
    Serial1.println(fingers);
    for (uint8_t z = 0; z < 10; z++)
    {
      if ((data[4 + (6 * z)]) == (z + 1))
        TouchscreenUSB.press(z, ((((data[6 + (6 * z)] * 256) + (data[5 + (6 * z)])) * multi_x) / 10), ((((data[8 + (6 * z)] * 256) + (data[7 + (6 * z)])) * multi_y) / 10));
      else if ((data[4 + (6 * z)]) == 0)
        TouchscreenUSB.release(z);
    }
  }
  delay(4);
}

// void ReadTouchscreen_LXSemicon_LX92513B()
// {
//   uint8_t ii = 0;
//   uint8_t fingers = 0;
//   Wire.requestFrom(0x10, 0x42);
//   while (Wire.available())
//   {
//     data[ii] = Wire.read();
//     ii++;
//   }

//   if ((data[0] == 0x42 && data[1] == 0x00 && data[2] == 0x10) || (data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00))
//   {
//     fingers = data[63];
//     Serial1.print("FINGERS: ");
//     Serial1.println(fingers);
//     for (uint8_t z = 0; z < 10; z++)
//     {
//       if ((data[4 + (6 * z)]) == (z + 1))
//         TouchscreenUSB.press(z, ((((data[6 + (6 * z)] * 256) + (data[5 + (6 * z)])) * multi_x) / 10), ((((data[8 + (6 * z)] * 256) + (data[7 + (6 * z)])) * multi_y) / 10));
//       else if ((data[4 + (6 * z)]) == 0)
//         TouchscreenUSB.release(z);
//     }
//   }
//   delay(4);
// }

void i2c_scanner_m(TwoWire &wire, uint8_t *tab, uint8_t len)
{
  uint8_t v = 0;
  uint8_t ret = 0;
  wire.begin();
  for (v = 1; v < (len - 1); ++v)
  {
    wire.beginTransmission(v);
    ret = wire.endTransmission(true);
    if (ret == 0)
      tab[v] = v;
    else
      tab[v] = 0;
  }
  wire.end();
}

void i2c_scanner_print_data(uint8_t *tab, uint8_t len)
{
  uint8_t v = 0;
  Serial1.println("");
  Serial1.println("");
  for (v = 0; v < 128; ++v)
  {
    if (v % 16 == 0 && v != 0)
      Serial1.println("");
    if (tab[v] == 0)
      Serial1.print("--");
    else
    {
      if (tab[v] > 0 && tab[v] < 16)
        Serial1.print("0");
      Serial1.print(tab[v], HEX);
    }
    Serial1.print(" ");
  }
  Serial1.println("");
  Serial1.println("");
}

void LXSemicon_SW42101A_init()
{
  uint8_t err;
  Wire.beginTransmission(0x10);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x08);
  err = Wire.endTransmission(true);
  delay(2);
  Wire.beginTransmission(0x10);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(0x01);
  err = Wire.endTransmission(true);
  delay(20);
  Wire.beginTransmission(0x10);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x10);
  Wire.write(0x04);
  err = Wire.endTransmission(true);
  delay(3300);
  Wire.beginTransmission(0x10);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x38);
  Wire.write(0x02);
  Wire.write(0x23);
  Wire.write(0x00);
  err = Wire.endTransmission(true);
  delay(7);
  Wire.beginTransmission(0x10);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x36);
  Wire.write(0x02);
  Wire.write(0x23);
  Wire.write(0x00);
  err = Wire.endTransmission(true);
  LX9215B_init = true;
  delay(100);
}


void ReadTouchscreen_LXSemicon_SW42101A(){
  int cnt = 0;
  int fingers = 0;
  Wire.requestFrom(0x10, 0x42);
  while (Wire.available())
  {
    data[cnt]=Wire.read();
    cnt++;
  }
  if (data[64]==0)
  {
    for (int i = 0; i < 10; i++) 
    {
      TouchscreenUSB.release(i);
    }
  }else if(data[0]==0x42 && data[64]!=0x00){  //CHANGE 2026-10-15 before if(data[0]==0x42 && data[63]!=0x00)
      fingers = data[64];
    for (int z = 0; z < fingers; z++)
    {
      if ((data[4 + (6 * z)]) == (z + 1))
        TouchscreenUSB.press(z, ((((data[6 + (6 * z)] * 256) + (data[5 + (6 * z)])) * multi_x) / 10), ((((data[8 + (6 * z)] * 256) + (data[7 + (6 * z)])) * multi_y) / 10));
      else if ((data[4 + (6 * z)]) == 0)
        TouchscreenUSB.release(z);
    }
  }
  delay(10);
}