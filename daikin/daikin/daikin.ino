//

// From https://odgregg.wordpress.com/2010/10/10/starting-from-scratch/#comments
//Daikin FTXG35FVMAS.
//
//The protocol I observed is 35-byte packets (in 3 bursts) and decoded it as:
//
//B0 = 0x11
//B1 = 0xDA
//B2 = 0x27
//B3 = 0x00
//B4 = 0xC5
//B5 = 0x00
//B6
//b7 = 0
//b6 = 0
//b5 = 0
//b4 = Comfort mode ON
//b3 = 0
//b2 = 0
//b1 = 0
//b0 = 0
//
//B7 = sum(B0..B6) & 0xff
//
//B8 = 0x11
//B9 = 0xda
//B10 = 0x27
//B11 = 0x00
//B12 = 0x42
//B13 = current time LSB
//B14 = current time MSB
//B15 = sum(B8..B14) & 0xff
//
//B16 = 0x11
//B17 = 0xda
//B18 = 0x27
//B19 = 0x00
//B20 = 0x00
//B21
//b7 = 0
//b6+b5+b4 = Mode
//b3 = 1
//b2 = OFF timer set
//b1 = ON timer set
//b0 = Air Conditioner ON
//B22 = (temp * 2)
//B23 = 0x00
//B24 = FAN control
//b7+b6+b5+b4 = Fan speed
//b3+b2+b1+b0 = Swing control up/down
//B25
//b7 = 0
//b6 = 0
//b5 = 0
//b4 = 0
//b3+b2+b1+b0 = Swing control left/right
//B26 = ON time LSB
//B27
//b7+b6+b5+b4 = OFF time LSB
//b3+b2+b1+b0 = ON time MSB
//B28 = OFF time MSB
//
//B29
//b7 = 0
//b6 = 0
//b5 = Quiet mode ON
//b4 = 0
//b3 = 0
//b2 = 0
//b1 = 0
//b0 = Powerful mode ON
//B30 = 0x00
//B31 = 0xc0
//B32
//b7 = 0
//b6 = 0
//b5 = 0
//b4 = 0
//b3 = 0
//b2 = 0
//b1 = Sensor ON
//b0 = 0
//B33 = 0x00
//B34 = sum(B16..B33) & 0xff
//
//Modes: b6+b5+b4
//011 = Cool
//100 = Heat (temp 23)
//110 = FAN (temp not shown, but 25)
//000 = Fully Automatic (temp 25)
//010 = DRY (temp 0xc0 = 96 degrees c)
//
//Fan: b7+b6+b5+b4
//0x30 = 1 bar
//0x40 = 2 bar
//0x50 = 3 bar
//0x60 = 4 bar
//0x70 = 5 bar
//0xa0 = Auto
//0xb0 = Not auto, moon + tree
//
//Swing control up/down:
//0000 = Swing up/down off
//1111 = Swing up/down on
//
//Swing control left/right:
//0000 = Swing left/right off
//1111 = Swing left/right on


// From http://www.8052.com/forum/read/132518
//I have recently done this for my own home AC control project. My Daikin ducted system had an "external control board" option so I ordered one of those for that but my Daikin splits are IR remote control only (or so the installer said).
//
//Note: The following results are for my installed model only, and any other model (or maybe even a later version of the same model) might have differences.
//
//1. The remote control is "stateful". It stores all the AC settings itself and when you press a button it updates the internal state and then sends a packet with the complete state via IR. This means that it is not possible to form a packet that just turns the unit on/off. Every packet sets the full state of everything: on/off, mode, temperature, etc.
//
//2. At the lowest level, the IR was pulsed at approx 38kHz when ON. Duty cycle was about 25%, but you could vary this as you wish. Let the time 1/38000 sec be one "slot".
//
//3. Next level up: A zero bit is 16 slots ON followed by 16 slots OFF. A one bit is 16 slots ON followed by 48 slots OFF.
//
//4. Packet format is:
//
//4.1 Start condition of 128 slots ON, 64 slots OFF.
//
//4.2 Next is 152 bits, as follows (multibit fields are LSB first):
//
//(0-39) = 0x11 0xDA 0x27 0x00 0x00
//(40) = Main power: 0->Off, 1->On
//(41) = On timer: 0->Disabled, 1->Enabled
//(42) = Off timer: 0->Disabled, 1->Enabled
//(43) = 0
//(44-47) = Mode: 0->Auto, 2->Dry, 3->Cool, 4->Heat, 6->Fan
//(48-55) = Temp: temp in degrees * 2, except for dry mode where this value is 0xC0
//(56-63) = 0x00
//(64-67) = Vertical swing: 0x0->Off, 0xF->On
//(68-71) = Fan speed: 0x3->Fan-1, 0x4->Fan-2, 0x5->Fan-3, 0x6->Fan-4, 0x7->Fan-5, 0xA->Auto, 0xB->Night (Dry mode puts the fan in auto)
//(72-75) = Horizontal swing: 0x0->Off, 0xF->On
//(76-79) = 0x0
//(80-91) = On timer time in minutes if set, else 0x000
//(92-103) = Off timer time in minutes if set, else 0x000
//(104) = Powerful mode: 0->Disabled, 1->Enabled
//(105-108) = 0x0
//(109) = Silent mode: 0->Disabled, 1->Enabled
//(110) = 0
//(111) = Home leave mode: 0->Disabled, 1->Enabled
//(112-127) = 0x00 0xC0
//(128) = 0
//(129) = Sensor: 0->Disabled, 1->Enabled
//(130-143) = 0
//(144-151) = Simple checksum of all 144 previous bits
//
//5. Good luck. You might be able to get this to work with a limited number of models. If you need to work with ANY Daikin model then be sure to sign the client to an ongoing "maintenance contract".

// Useful http://jamesstewy.com/blog/post/8/


//info about the heatpump

// # of bytes per command
const int COMMAND_LENGTH = 27;    

unsigned char daikin[COMMAND_LENGTH]     = { 
0x11,0xDA,0x27,0xF0,0x00,0x00,0x00,0x20,
//0    1    2   3    4    5     6   7
0x11,0xDA,0x27,0x00,0x00,0x41,0x1E,0x00,
//8    9   10   11   12    13   14   15
0xB0,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0xE3 };
//16  17    18  19   20    21   22  23   24   25   26
/*


byte 13=mode
b7 = 0
b6+b5+b4 = Mode
b3 = 0
b2 = OFF timer set
b1 = ON timer set
b0 = Air Conditioner ON

Modes: b6+b5+b4
011 = Cool
100 = Heat (temp 23)
110 = FAN (temp not shown, but 25)
000 = Fully Automatic (temp 25)
010 = DRY (temp 0xc0 = 96 degrees c)

byte 14=temp*2

byte 16=Fan
FAN control
b7+b6+b5+b4 = Fan speed
b3+b2+b1+b0 = Swing control up/down

Fan: b7+b6+b5+b4
0×30 = 1 bar
0×40 = 2 bar
0×50 = 3 bar
0×60 = 4 bar
0×70 = 5 bar
0xa0 = Auto
0xb0 = Not auto, moon + tree
Swing control up/down:
0000 = Swing up/down off
1111 = Swing up/down on
Swing control left/right:
0000 = Swing left/right off
1111 = Swing left/right on
*/

// Harvested stuff that sets sending eviron
byte airController_checksum()
{
	byte sum = 0;
	byte i;


	for(i = 0; i <= 6; i++){
		sum += daikin[i];
	}

        daikin[7] = sum &0xFF;
        
        sum=0;
	for(i = 8; i <= 25; i++){
		sum += daikin[i];
        }

        daikin[26] = sum &0xFF;

        
}



void airController_on()
{
	//state = ON;
	daikin[13] |= 0x01;
	airController_checksum();
}

void airController_off()
{
	//state = OFF;
	daikin[13] &= 0xFE;
	airController_checksum();
}

void airController_setAux(byte aux)
{
	daikin[21] = aux;
	airController_checksum();
}

byte airController_getAux()
{
	return daikin[21];
}


void airController_setTemp(byte temp)
{
	daikin[14] = (temp)*2;
	airController_checksum();
}


void airController_setFan(byte fan)
{
	daikin[16] = fan;
	airController_checksum();
}


byte airConroller_getTemp()
{
	return (daikin[14])/2;
}


byte airConroller_getMode()
{

/*
Modes: b6+b5+b4
011 = Cool
100 = Heat (temp 23)
110 = FAN (temp not shown, but 25)
000 = Fully Automatic (temp 25)
010 = DRY (temp 0xc0 = 96 degrees c)
*/

	return (daikin[13])>>4;

}


void airController_setMode(byte mode)
{
	daikin[13]=mode<<4 | airConroller_getState();
	airController_checksum();
}


byte airConroller_getState()
{
	return (daikin[13])&0x01;
}

byte airConroller_getFan()
{
	return (daikin[16]);
}

// Sensible defaults (to make it simple)
byte temp = 24
//byte fan = 
byte mode = Heat

//void restartac () {
//  
//            if(airConroller_getState()==1) {
//              
//              airController_off(); 
//              irstate.aux=airController_getAux();
//              irstate.temp=airConroller_getTemp();
//              irstate.fan= airConroller_getFan();
//              irstate.mode=airConroller_getMode();
//             
//              irsend.sendDaikin(daikin, 8,0); 
//              delay(29);
//              irsend.sendDaikin(daikin, 19,8); 
//              
//              delay (10000);
//              
//              airController_on(); 
//              airController_setAux(0); 
//              airController_setTemp(irstate.temp);
//              airController_setFan(irstate.fan);
//              airController_setMode(irstate.mode);
//             
//              irsend.sendDaikin(daikin, 8,0); 
//              delay(29);
//              irsend.sendDaikin(daikin, 19,8); 
//
//            }
//}

//Above hacked to:
//              airController_on(); 
//              airController_setAux(0); 
//              airController_setTemp(irstate.temp);
//              airController_setFan(irstate.fan);
//              airController_setMode(irstate.mode);
//             
//              irsend.sendDaikin(daikin, 8,0); 
//              delay(29);
//              irsend.sendDaikin(daikin, 19,8); 

