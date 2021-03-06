// Simple application to control Panasonic and Daikin heat pumps / airconditioning devices
// * Panasonic CKP series remote number A75C2317/CWA75C2317, which replaces A75C559/CWA75C559 and A75C2295
// at least according to ebay!
//
// Almost all of this is adapted from the excellent: https://github.com/ToniA/arduino-xpl-heatpump-controller
// With some excellent help from: https://github.com/shirriff/Arduino-IRremote
//
// Connect an IR led (with 1k resistor in series)
// between GND and digital pin 3
//

//IRsend irsend;

// Infrared LED on digital PIN 3 (needs a PWM pin)
// Connect with 1 kOhm resistor in series to GND
#define IR_LED_PIN        3

// *************************************
// Timing constants adn codes
//**************************************

// Panasonic CKP timing constants
#define PANASONIC_AIRCON1_HDR_MARK   3400
#define PANASONIC_AIRCON1_HDR_SPACE  3500
#define PANASONIC_AIRCON1_BIT_MARK   800
#define PANASONIC_AIRCON1_ONE_SPACE  2700
#define PANASONIC_AIRCON1_ZERO_SPACE 1000
#define PANASONIC_AIRCON1_MSG_SPACE  14000

// Panasonic CKP codes
#define PANASONIC_AIRCON1_MODE_AUTO  0x06 // Operating mode
#define PANASONIC_AIRCON1_MODE_HEAT  0x04
#define PANASONIC_AIRCON1_MODE_COOL  0x02
#define PANASONIC_AIRCON1_MODE_DRY   0x03
#define PANASONIC_AIRCON1_MODE_FAN   0x01
#define PANASONIC_AIRCON1_MODE_ONOFF 0x00 // Toggle ON/OFF
#define PANASONIC_AIRCON1_MODE_KEEP  0x08 // Do not toggle ON/OFF
#define PANASONIC_AIRCON1_FAN_AUTO   0xF0 // Fan speed
#define PANASONIC_AIRCON1_FAN1       0x20
#define PANASONIC_AIRCON1_FAN2       0x30
#define PANASONIC_AIRCON1_FAN3       0x40
#define PANASONIC_AIRCON1_FAN4       0x50
#define PANASONIC_AIRCON1_FAN5       0x60
#define PANASONIC_AIRCON1_VS_AUTO    0xF0 // Vertical swing
#define PANASONIC_AIRCON1_VS_UP      0x90
#define PANASONIC_AIRCON1_VS_MUP     0xA0
#define PANASONIC_AIRCON1_VS_MIDDLE  0xB0
#define PANASONIC_AIRCON1_VS_MDOWN   0xC0
#define PANASONIC_AIRCON1_VS_DOWN    0xD0
#define PANASONIC_AIRCON1_HS_AUTO    0x08 // Horizontal swing
#define PANASONIC_AIRCON1_HS_MANUAL  0x00

// Panasonic DKE timing constants
#define PANASONIC_AIRCON2_HDR_MARK   3500
#define PANASONIC_AIRCON2_HDR_SPACE  1800
#define PANASONIC_AIRCON2_BIT_MARK   420
#define PANASONIC_AIRCON2_ONE_SPACE  1350
#define PANASONIC_AIRCON2_ZERO_SPACE 470
#define PANASONIC_AIRCON2_MSG_SPACE  10000

// Panasonic DKE codes
#define PANASONIC_AIRCON2_MODE_AUTO  0x00 // Operating mode
#define PANASONIC_AIRCON2_MODE_HEAT  0x40
#define PANASONIC_AIRCON2_MODE_COOL  0x30
#define PANASONIC_AIRCON2_MODE_DRY   0x20
#define PANASONIC_AIRCON2_MODE_FAN   0x60
#define PANASONIC_AIRCON2_MODE_OFF   0x00 // Power OFF
#define PANASONIC_AIRCON2_MODE_ON    0x01
#define PANASONIC_AIRCON2_TIMER_CNL  0x08
#define PANASONIC_AIRCON2_FAN_AUTO   0xA0 // Fan speed
#define PANASONIC_AIRCON2_FAN1       0x30
#define PANASONIC_AIRCON2_FAN2       0x40
#define PANASONIC_AIRCON2_FAN3       0x50
#define PANASONIC_AIRCON2_FAN4       0x60
#define PANASONIC_AIRCON2_FAN5       0x70
#define PANASONIC_AIRCON2_VS_AUTO    0x0F // Vertical swing
#define PANASONIC_AIRCON2_VS_UP      0x01
#define PANASONIC_AIRCON2_VS_MUP     0x02
#define PANASONIC_AIRCON2_VS_MIDDLE  0x03
#define PANASONIC_AIRCON2_VS_MDOWN   0x04
#define PANASONIC_AIRCON2_VS_DOWN    0x05
#define PANASONIC_AIRCON2_HS_AUTO    0x0D // Horizontal swing
#define PANASONIC_AIRCON2_HS_MIDDLE  0x06
#define PANASONIC_AIRCON2_HS_LEFT    0x09
#define PANASONIC_AIRCON2_HS_MLEFT   0x0A
#define PANASONIC_AIRCON2_HS_MRIGHT  0x0B
#define PANASONIC_AIRCON2_HS_RIGHT   0x0C


// ************************************************
// Signal building
//*************************************************

// CKP code signal start
// Send the Panasonic CKP code

void sendPanasonicCKP(byte operatingMode, byte fanSpeed, byte temperature, byte swingV, byte swingH)
{
  byte sendBuffer[4];

  // Fan speed & temperature, temperature needs to be 27 in FAN mode
  if (operatingMode == PANASONIC_AIRCON1_MODE_FAN || operatingMode == (PANASONIC_AIRCON1_MODE_FAN | PANASONIC_AIRCON1_MODE_KEEP ))
  {
    temperature = 27;
  }

  sendBuffer[0] = fanSpeed | (temperature - 15);

  // Power toggle & operation mode
  sendBuffer[1] = operatingMode;

  // Swings
  sendBuffer[2] = swingV | swingH;

  // Always 0x36
  sendBuffer[3]  = 0x36;

  // Send the code
  sendPanasonicCKPraw(sendBuffer);
}

// Send the Panasonic CKP raw code

void sendPanasonicCKPraw(byte sendBuffer[])
{
  // 40 kHz PWM frequency
  enableIROut(40);

  // Header, two first bytes repeated
  mark(PANASONIC_AIRCON1_HDR_MARK);
  space(PANASONIC_AIRCON1_HDR_SPACE);

  for (int i=0; i<2; i++) {
    sendIRByte(sendBuffer[0], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[0], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[1], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[1], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);

    mark(PANASONIC_AIRCON1_HDR_MARK);
    space(PANASONIC_AIRCON1_HDR_SPACE);
  }

  // Pause

  mark(PANASONIC_AIRCON1_BIT_MARK);
  space(PANASONIC_AIRCON1_MSG_SPACE);

  // Header, two last bytes repeated

  mark(PANASONIC_AIRCON1_HDR_MARK);
  space(PANASONIC_AIRCON1_HDR_SPACE);

  for (int i=0; i<2; i++) {
    sendIRByte(sendBuffer[2], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[2], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[3], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[3], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);

    mark(PANASONIC_AIRCON1_HDR_MARK);
    space(PANASONIC_AIRCON1_HDR_SPACE);
  }

  mark(PANASONIC_AIRCON1_BIT_MARK);
  space(0);
}

// Send the Panasonic CKP On/Off code
//
// CKP does not have discrete ON/OFF commands, but this can be emulated by using the timer
// The side-effects of using the timer are:
// * one minute delay before the power state changes
// * the 'TIMER' led (orange) is lit

void sendPanasonicCKPOnOff(boolean powerState)
{
  byte ON_msg[] = { 0x7F, 0x38, 0xBF, 0x38, 0x10, 0x3D, 0x80, 0x3D, 0x09, 0x34, 0x80, 0x34 };  // ON at 00:10,  time now 00:09, no OFF timing
  byte OFF_msg[] = { 0x10, 0x38, 0x80, 0x38, 0x7F, 0x3D, 0xBF, 0x3D, 0x09, 0x34, 0x80, 0x34 }; // OFF at 00:10, time now 00:09, no ON timing
  byte *sendBuffer;

  if ( powerState == true ) {
    sendBuffer = ON_msg;
  } else {
    sendBuffer = OFF_msg;
  }

  // 40 kHz PWM frequency
  enableIROut(40);

  for (int i=0; i<6; i++) {
    mark(PANASONIC_AIRCON1_HDR_MARK);
    space(PANASONIC_AIRCON1_HDR_SPACE);

    sendIRByte(sendBuffer[i*2 + 0], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[i*2 + 0], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[i*2 + 1], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);
    sendIRByte(sendBuffer[i*2 + 1], PANASONIC_AIRCON1_BIT_MARK, PANASONIC_AIRCON1_ZERO_SPACE, PANASONIC_AIRCON1_ONE_SPACE);

    mark(PANASONIC_AIRCON1_HDR_MARK);
    space(PANASONIC_AIRCON1_HDR_SPACE);

    if ( i < 5 ) {
      mark(PANASONIC_AIRCON1_BIT_MARK);
      space(PANASONIC_AIRCON1_MSG_SPACE);
    }
  }

  mark(PANASONIC_AIRCON1_BIT_MARK);
  space(0);
}

// CKP code end

//DKE code start

// Send the Panasonic DKE code

void sendPanasonicDKE(byte operatingMode, byte fanSpeed, byte temperature, byte swingV, byte swingH)
{
  byte DKE_template[] = { 0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06, 0x02, 0x20, 0xE0, 0x04, 0x00, 0x48, 0x2E, 0x80, 0xA3, 0x0D, 0x00, 0x0E, 0xE0, 0x00, 0x00, 0x01, 0x00, 0x06, 0xA2 };
  byte checksum = 0xF4;

  DKE_template[13] = operatingMode;
  DKE_template[14] = temperature << 1;
  DKE_template[16] = fanSpeed | swingV;
  DKE_template[17] = swingH;

  // Checksum

  for (int i=0; i<26; i++) {
    checksum += DKE_template[i];
  }

  DKE_template[26] = checksum;

  // 40 kHz PWM frequency
  enableIROut(40);

  // Header
  mark(PANASONIC_AIRCON2_HDR_MARK);
  space(PANASONIC_AIRCON2_HDR_SPACE);

  // First 8 bytes
  for (int i=0; i<8; i++) {
    sendIRByte(DKE_template[i], PANASONIC_AIRCON2_BIT_MARK, PANASONIC_AIRCON2_ZERO_SPACE, PANASONIC_AIRCON2_ONE_SPACE);
  }

  // Pause
  mark(PANASONIC_AIRCON2_BIT_MARK);
  space(PANASONIC_AIRCON2_MSG_SPACE);

  // Header
  mark(PANASONIC_AIRCON2_HDR_MARK);
  space(PANASONIC_AIRCON2_HDR_SPACE);

  // Last 19 bytes
  for (int i=8; i<27; i++) {
    sendIRByte(DKE_template[i], PANASONIC_AIRCON2_BIT_MARK, PANASONIC_AIRCON2_ZERO_SPACE, PANASONIC_AIRCON2_ONE_SPACE);
  }

  mark(PANASONIC_AIRCON2_BIT_MARK);
  space(0);
}

// Send the Midea code

void sendMidea(byte operatingMode, byte fanSpeed, byte temperature)
{
  byte sendBuffer[3] = { 0x4D, 0x00, 0x00 }; // First byte is always 0x4D

  byte temperatures[] = {0, 8, 12, 4, 6, 14, 10, 2, 3, 11, 9, 1, 5, 13 };

  byte OffMsg[] = {0x4D, 0xDE, 0x07 };
  byte FPMsg[]  = {0xAD, 0xAF, 0xB5 };

  if (operatingMode == MIDEA_AIRCON1_MODE_OFF)
  {
    sendMidearaw( OffMsg );
  }
  else if (operatingMode == MIDEA_AIRCON1_MODE_FP)
  {
    sendMidearaw( FPMsg );
  }
  else
  {
    sendBuffer[1] = ~fanSpeed;

    if ( operatingMode == MIDEA_AIRCON1_MODE_FAN )
    {
      sendBuffer[2] = MIDEA_AIRCON1_MODE_DRY | 0x07;
    }
    else
    {
      sendBuffer[2] = operatingMode | temperatures[temperature-17];
    }

    // Send the code
    sendMidearaw(sendBuffer);
  }
}

// Send the Midea raw code

void sendMidearaw(byte sendBuffer[])
{
  // 40 kHz PWM frequency
  enableIROut(40);

  // Header
  mark(MIDEA_AIRCON1_HDR_MARK);
  space(MIDEA_AIRCON1_HDR_SPACE);

  // Six bytes, every second byte is a bitwise not of the previous byte
  for (int i=0; i<3; i++) {
    sendIRByte(sendBuffer[i], MIDEA_AIRCON1_BIT_MARK, MIDEA_AIRCON1_ZERO_SPACE, MIDEA_AIRCON1_ONE_SPACE);
    sendIRByte(~sendBuffer[i], MIDEA_AIRCON1_BIT_MARK, MIDEA_AIRCON1_ZERO_SPACE, MIDEA_AIRCON1_ONE_SPACE);
  }

  // Pause

  mark(MIDEA_AIRCON1_BIT_MARK);
  space(MIDEA_AIRCON1_MSG_SPACE);

  // Header, two last bytes repeated

  mark(MIDEA_AIRCON1_HDR_MARK);
  space(MIDEA_AIRCON1_HDR_SPACE);

  // Six bytes, every second byte is a bitwise not of the previous byte
  for (int i=0; i<3; i++) {
    sendIRByte(sendBuffer[i], MIDEA_AIRCON1_BIT_MARK, MIDEA_AIRCON1_ZERO_SPACE, MIDEA_AIRCON1_ONE_SPACE);
    sendIRByte(~sendBuffer[i], MIDEA_AIRCON1_BIT_MARK, MIDEA_AIRCON1_ZERO_SPACE, MIDEA_AIRCON1_ONE_SPACE);
  }

  // End mark

  mark(MIDEA_AIRCON1_BIT_MARK);
  space(0);
}

//DKE code end

//****************************************************************
// Send a byte over IR - tell arduino how to send built code
//****************************************************************

void sendIRByte(byte sendByte, int bitMarkLength, int zeroSpaceLength, int oneSpaceLength)
{
  for (int i=0; i<8 ; i++)
  {
    if (sendByte & 0x01)
    {
      mark(bitMarkLength);
      space(oneSpaceLength);
    }
    else
    {
      mark(bitMarkLength);
      space(zeroSpaceLength);
    }

    sendByte >>= 1;
  }
}

// 'mark', 'space' and 'enableIROut' have been taken
// from Ken Shirriff's IRRemote library:
// https://github.com/shirriff/Arduino-IRremote

void mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  (TCCR2A |= _BV(COM2B1)); // Enable pin 3 PWM output
  delayMicroseconds(time);
}

void space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  (TCCR2A &= ~(_BV(COM2B1))); // Disable pin 3 PWM output
  delayMicroseconds(time);
}

void enableIROut(int khz) {
  pinMode(IR_LED_PIN, OUTPUT);
  digitalWrite(IR_LED_PIN, LOW); // When not sending PWM, we want it low

  const uint8_t pwmval = F_CPU / 2000 / (khz);
  TCCR2A = _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS20);
  OCR2A = pwmval;
  OCR2B = pwmval / 3;
}


//******************************************************
// Receive instructions form external source (serial port/esp module/pin state)
//******************************************************
// parse the input so appropriate code is built and can be sent

//void sendCKPCmd(xPL_Message * message)
//{
//  int param = 0;
//  byte i;



// Turn the fucker off

//  byte powerMode     = false;
//  byte operatingMode = PANASONIC_AIRCON1_MODE_HEAT | PANASONIC_AIRCON1_MODE_KEEP;
//  byte fanSpeed      = PANASONIC_AIRCON1_FAN_AUTO;
//  byte temperature   = 24;
//  byte swingV        = PANASONIC_AIRCON1_VS_UP;
//  byte swingH        = PANASONIC_AIRCON1_HS_AUTO;
//
//}

// Listen on serial
void setup()
{
  Serial.begin(9600);
  //IR Demo send a cmd to panasonic heatpump
 Serial.println("IR Demo send a cmd to Panasonic heatpump");
 Serial.println("Please input any data to the Serial Interface in order to start the Demo");
 Serial.println("");
}


//Send it all
void loop() {
  #define CKP_on
//  #define CKP_off
//  #define DKE_on
//  #define DKE_off
//  #define DAI_on
//  #define DAI_off

  #ifdef CKP_on
  if (Serial.read() != -1) {
    Serial.println("Set heatpump in Hot mode, auto fan swing with temp at 24 and turn on.");
      //Need some shit in here for listening to on or off switch eg
      // if pin in 'state' do on, else if pin in 'state' off do off, else start again...
      // Sensible defaults for the heat pump mode
      byte powerMode     = true;
      byte operatingMode = PANASONIC_AIRCON1_MODE_HEAT | PANASONIC_AIRCON1_MODE_KEEP;
      byte fanSpeed      = PANASONIC_AIRCON1_FAN_AUTO;
      byte temperature   = 24;
      byte swingV        = PANASONIC_AIRCON1_VS_UP;
      byte swingH        = PANASONIC_AIRCON1_HS_AUTO;     
    sendPanasonicCKP(operatingMode, fanSpeed, temperature, swingV, swingH);
    delay(3000); // Sleep 3 seconds between the messages
    sendPanasonicCKPOnOff(powerMode);
    }
  #endif 
  
  #ifdef CKP_off
  if (Serial.read() != -1) {
    Serial.println("Set heatpump in Hot mode, auto fan swing with temp at 24 and turn on.");
      //Need some shit in here for listening to on or off switch eg
      // if pin in 'state' do on, else if pin in 'state' off do off, else start again...
      // Sensible defaults for the heat pump mode
      byte powerMode     = false;
      byte operatingMode = PANASONIC_AIRCON1_MODE_HEAT | PANASONIC_AIRCON1_MODE_KEEP;
      byte fanSpeed      = PANASONIC_AIRCON1_FAN_AUTO;
      byte temperature   = 24;
      byte swingV        = PANASONIC_AIRCON1_VS_UP;
      byte swingH        = PANASONIC_AIRCON1_HS_AUTO;     
    sendPanasonicCKP(operatingMode, fanSpeed, temperature, swingV, swingH);
    delay(3000); // Sleep 3 seconds between the messages
    sendPanasonicCKPOnOff(powerMode);
    }
  #endif  
 
 #ifdef DKE_on
  if (Serial.read() != -1) {
    Serial.println("Set heatpump in Hot mode, auto fan swing with temp at 24 and turn on."); 
      byte operatingMode = PANASONIC_AIRCON2_TIMER_CNL;
      // byte operatingMode = PANASONIC_AIRCON2_MODE_ON;
      byte fanSpeed      = PANASONIC_AIRCON2_FAN_AUTO;
      byte temperature   = 23;
      byte swingV        = PANASONIC_AIRCON2_VS_UP;
      byte swingH        = PANASONIC_AIRCON2_HS_AUTO;
    sendPanasonicDKE(operatingMode, fanSpeed, temperature, swingV, swingH);  
  } 
 #endif 
 
 #ifdef DKE_off
  if (Serial.read() != -1) {
    Serial.println("Set heatpump in Hot mode, auto fan swing with temp at 24 and turn on."); 
      byte operatingMode = PANASONIC_AIRCON2_TIMER_CNL;
      // byte operatingMode = PANASONIC_AIRCON2_MODE_OFF;
      byte fanSpeed      = PANASONIC_AIRCON2_FAN_AUTO;
      byte temperature   = 23;
      byte swingV        = PANASONIC_AIRCON2_VS_UP;
      byte swingH        = PANASONIC_AIRCON2_HS_AUTO;
    sendPanasonicDKE(operatingMode, fanSpeed, temperature, swingV, swingH);
  }  
 #endif
 
 #ifdef DAI_on

 #endif 
 
 #ifdef DAI_off

 #endif
}


// Who needs options right? Just turn the heat pump on or off


//  for (i=0; i < message->command_count; i++)
//  {
//    param = atoi(message->command[i].value);
//
//    if (strcmp(message->command[i].name, "power") == 0 )
//    {
//      switch (param)
//      {
//        case 1:
//          powerMode = true;
//          break;
//      }
//    }
//    else if (strcmp(message->command[i].name, "mode") == 0 )
//    {
//      operatingMode &= PANASONIC_AIRCON1_MODE_KEEP;
//
//      switch (param)
//      {
//        case 1:
//          operatingMode |= PANASONIC_AIRCON1_MODE_AUTO;
//          break;
//        case 2:
//          operatingMode |= PANASONIC_AIRCON1_MODE_HEAT;
//          break;
//        case 3:
//          operatingMode |= PANASONIC_AIRCON1_MODE_COOL;
//          break;
//        case 4:
//          operatingMode |= PANASONIC_AIRCON1_MODE_DRY;
//          break;
//        case 5:
//          operatingMode |= PANASONIC_AIRCON1_MODE_FAN;
//          break;
//      }
//    }
//    else if (strcmp(message->command[i].name, "fan") == 0 )
//    {
//      switch (param)
//      {
//        case 1:
//          fanSpeed = PANASONIC_AIRCON1_FAN_AUTO;
//          break;
//        case 2:
//          fanSpeed = PANASONIC_AIRCON1_FAN1;
//          break;
//        case 3:
//          fanSpeed = PANASONIC_AIRCON1_FAN2;
//          break;
//        case 4:
//          fanSpeed = PANASONIC_AIRCON1_FAN3;
//          break;
//        case 5:
//          fanSpeed = PANASONIC_AIRCON1_FAN4;
//          break;
//        case 6:
//          fanSpeed = PANASONIC_AIRCON1_FAN5;
//          break;
//      }
//    }
//    else if (strcmp(message->command[i].name, "temperature") == 0 )
//    {
//      if ( param >= 15 && param <= 31)
//      {
//        temperature = param;
//      }
//    }
//// Vertical swing has a broken plastic part, but wtf - eh, Won't work without the command right?
//
//    else if (strcmp(message->command[i].name, "vswing") == 0 )
//    {
//      switch (param)
//      {
//        case 1:
//          swingV = PANASONIC_AIRCON1_VS_AUTO;
//          break;
//        case 2:
//          swingV = PANASONIC_AIRCON1_VS_UP;
//          break;
//        case 3:
//          swingV = PANASONIC_AIRCON1_VS_MUP;
//          break;
//        case 4:
//          swingV = PANASONIC_AIRCON1_VS_MIDDLE;
//          break;
//        case 5:
//          swingV = PANASONIC_AIRCON1_VS_MDOWN;
//          break;
//        case 6:
//          swingV = PANASONIC_AIRCON1_VS_DOWN;
//          break;
//      }
//    }
//// No H-swing on Sarah's heatpump - best trial with it in as 'filler' otherewise CRC check may fail (*lacks understanding*
//    else if (strcmp(message->command[i].name, "hswing") == 0 )
//    {
//      switch (param)
//      {
//        case 1:
//          swingH = PANASONIC_AIRCON1_HS_AUTO;
//          break;
//        case 2:
//          swingH = PANASONIC_AIRCON1_HS_MANUAL;
//          break;
//      }
//    }
//  }
//
//  sendPanasonicCKP(operatingMode, fanSpeed, temperature, swingV, swingH);
//  delay(3000); // Sleep 3 seconds between the messages
//  sendPanasonicCKPOnOff(powerMode);
//}


/*




The following is the xPl stuff we don't need





*/




// parse incoming xPL message
// To test with xpl-perl:
//
// xpl-sender -m xpl-cmnd -t xpl-arduino.heatpumpctrl -c aircon.ckp power=0 mode=2 fan=3 temperature=23
// xpl-sender -m xpl-cmnd -t xpl-arduino.heatpumpctrl -c aircon.dke power=0 mode=2 fan=3 temperature=23
// xpl-sender -m xpl-cmnd -t xpl-arduino.heatpumpctrl -c aircon.midea power=0 mode=2 fan=3 temperature=23


//void AfterParseAction(xPL_Message * message)
//{
//  if (xpl.TargetIsMe(message))
//    {
//      if (message->IsSchema_P(PSTR("aircon"), PSTR("dke")))
//      {
//        sendDKECmd(message);
//      }
//      else if (message->IsSchema_P(PSTR("aircon"), PSTR("ckp")))
//      {
//        sendCKPCmd(message);
//      }
//      else if (message->IsSchema_P(PSTR("aircon"), PSTR("midea")))
//      {
//        sendMideaCmd(message);
//      }
//
//      Serial.println("Got xPL message:");
//      Serial.println(message->toString());
//      Serial.println("");
//    }
//}
//
//// The setup
//
//void setup()
//{
//  // Initialize serial
//  Serial.begin(9600);
//  Serial.print("Starting... ");
//
//  // Ethernet shield reset trick
//  // Need to cut the RESET lines (also from ICSP header) and connect an I/O to RESET on the shield
//
//  pinMode(ETHERNET_RST, OUTPUT);
//  digitalWrite(ETHERNET_RST, HIGH);
//  delay(50);
//  digitalWrite(ETHERNET_RST, LOW);
//  delay(50);
//  digitalWrite(ETHERNET_RST, HIGH);
//  delay(100);
//
//  // generate or read the already generated MAC address
//  generateMAC();
//
//  // initialize the Ethernet adapter
//  Ethernet.begin(macAddress, ip);
//
//  // initialize the xPL UDP port
//  Udp.begin(xpl.udp_port);
//
//  // initialize xPL
//  xpl.SendExternal = &SendUdPMessage;                                   // pointer to the send callback
//  xpl.AfterParseAction = &AfterParseAction;                             // pointer to a post parsing action callback
//  xpl.SetSource_P(PSTR("xpl"), PSTR("arduino"), PSTR("heatpumpctrl"));  // parameters for hearbeat message
//
//  Serial.println("Started");
//}
//
//// The loop
//// * heartbeats
//// * xPL message processing
//
//void loop()
//{
//  // xPL heartbeat management
//  xpl.Process();
//
//  // process incoming xPL packets
//  int packetSize = Udp.parsePacket();
//  if(packetSize)
//  {
//    char xPLMessageBuff[XPL_MESSAGE_BUFFER_MAX];
//
//    // read the packet into packetBufffer
//    Udp.read(xPLMessageBuff, XPL_MESSAGE_BUFFER_MAX);
//
//    // parse message
//    xpl.ParseInputMessage(xPLMessageBuff);
//  }
//}
//
//// Random MAC based on:
//// http://files.pcode.nl/arduino/EthernetPersistentMAC.ino
//// A5 is the entropy PIN for random MAC generation, leave it unconnected
//
//void generateMAC()
//{
//  randomSeed(analogRead(ENTROPY_PIN));
// 
//  // Uuncomment to generate a new MAC
//  //EEPROM.write(E2END - 8, 0x00);
//  //EEPROM.write(E2END - 7, 0x00);
//  
//  // We store the MAC address in the last 8 bytes of the EEPROM using E2END to determine it's size
//  // The first of those two bytes are checked for the magic values 0x80 and 0x23 (a reference to 802.3)
//
//  if ((EEPROM.read(E2END - 8) == 0x80) && (EEPROM.read(E2END - 7) == 0x23))
//  {
//    Serial.println("Reading MAC address from EEPROM");
//    for (int i = 0; i <= 5; i++)
//    {
//       macAddress[i] = EEPROM.read(E2END - 6 + i);
//    }
//  }
//  else
//  {
//    Serial.println("Writing new random MAC address to EEPROM");
//    
//    EEPROM.write(E2END - 8, 0x80);
//    EEPROM.write(E2END - 7, 0x23);
//    for (int i = 0; i <= 5; i++)
//    {
//      // Skip the Organisationally Unique Identifier (OUI) 
//      // Randomize only the Network Interface Controller specific part
//      if (i >= 3)
//      {
//        macAddress[i] = random(0, 255);
//      }
//      EEPROM.write(E2END - 6 + i, macAddress[i]);
//    }
//  }
//  snprintf(macstr, 18, "%02X:%02X:%02X:%02X:%02X:%02X", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
//
//  // Print out the MAC address
//  Serial.print("MAC: ");
//  Serial.println(macstr);
//}
