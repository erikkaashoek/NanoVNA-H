/* Copyright (c) 2014-2015, TAKAHASHI Tomohiro (TTRFTECH) edy555@gmail.com
 * All rights reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * The software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include "ch.h"
#include "hal.h"
#include "nanovna.h"

#include "si4432.h"

#define CS_SI0_HIGH     palSetPad(GPIOA, GPIOA_RX_SEL)
#define CS_SI1_HIGH     palSetPad(GPIOA, GPIOA_LO_SEL)
#define CS_PE_HIGH      palSetPad(GPIOA, GPIOA_PE_SEL)

#define CS_SI0_LOW     palClearPad(GPIOA, GPIOA_RX_SEL)
#define CS_SI1_LOW     palClearPad(GPIOA, GPIOA_LO_SEL)
#define CS_PE_LOW      palClearPad(GPIOA, GPIOA_PE_SEL)

#define SPI2_CLK_HIGH   palSetPad(GPIOB, GPIOB_SPI2_CLK)
#define SPI2_CLK_LOW    palClearPad(GPIOB, GPIOB_SPI2_CLK)

#define SPI2_SDI_HIGH   palSetPad(GPIOB, GPIOB_SPI2_SDI)
#define SPI2_SDI_LOW    palClearPad(GPIOB, GPIOB_SPI2_SDI)

#define SPI2_SDO    ((palReadPort(GPIOB) & (1<<GPIOB_SPI2_SDO))?1:0)


void shiftOut(uint8_t val)
{
     uint8_t i;

     for (i = 0; i < 8; i++)  {
           if (val & (1 << (7 - i)))
             SPI2_SDI_HIGH;
           else
             SPI2_SDI_LOW;
           SPI2_CLK_HIGH;
           SPI2_CLK_LOW;
     }
}

uint8_t shiftIn(void) {
    uint8_t value = 0;
    uint8_t i;
    for (i = 0; i < 8; ++i) {
      SPI2_CLK_HIGH;
        value |= SPI2_SDO << (7 - i);
        SPI2_CLK_LOW;
    }
    return value;
}

const int SI_nSEL[3] = { GPIOA_RX_SEL, GPIOA_LO_SEL, 0}; // #3 is dummy!!!!!!

int SI4432_Sel = 0;         // currently selected SI4432

#ifdef __SI4432_H__

void SI4432_Write_Byte(byte ADR, byte DATA )
{
  SPI2_CLK_LOW;
  palClearPad(GPIOA, SI_nSEL[SI4432_Sel]);
  ADR |= 0x80 ; // RW = 1
  shiftOut( ADR );
  shiftOut( DATA );
  palSetPad(GPIOA, SI_nSEL[SI4432_Sel]);
}

byte SI4432_Read_Byte( byte ADR )
{
  byte DATA ;
  SPI2_CLK_LOW;
  palClearPad(GPIOA, SI_nSEL[SI4432_Sel]);
  shiftOut( ADR );
  DATA = shiftIn();
  palSetPad(GPIOA, SI_nSEL[SI4432_Sel]);
  return DATA ;
}



void SI4432_Reset()
{
  int count = 0;
  // always perform a system reset (don't send 0x87)
  SI4432_Write_Byte( 0x07, 0x80);
  chThdSleepMilliseconds(10);
  // wait for chiprdy bit
  while (count++ < 100 && ( SI4432_Read_Byte ( 0x04 ) & 0x02 ) == 0) {
    chThdSleepMilliseconds(10);
  }
}

float SI4432_SET_RBW(float WISH)
{
  byte ndec = 5 ;
  byte dwn3 = 0 ;
  byte fils = 1 ;
  float rxosr = 12.5 ;
  float REAL = 2.6 ;   // AS CLOSE AS POSSIBLE TO "WISH" :-)
  // YES, WE KNOW THIS IS SLOW
#if 0 // Too many resolutions, not needed
  if (WISH > 2.6) {
    ndec = 5 ;
    fils = 2 ;
    REAL = 2.8 ;
  }
#endif
  if (WISH > 2.8) {
    ndec = 5 ;
    fils = 3 ;
    REAL = 3.1 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 3.1) {
    ndec = 5 ;
    fils = 4 ;
    REAL = 3.2 ;
  }
  if (WISH > 3.2) {
    ndec = 5 ;
    fils = 5 ;
    REAL = 3.7 ;
  }
  if (WISH > 3.7) {
    ndec = 5 ;
    fils = 6 ;
    REAL = 4.2 ;
  }
  if (WISH > 4.2) {
    ndec = 5 ;
    fils = 7 ;
    REAL = 4.5 ;
  }
  if (WISH > 4.5) {
    ndec = 4 ;
    fils = 1 ;
    REAL = 4.9 ;
  }
  if (WISH > 4.9) {
    ndec = 4 ;
    fils = 2 ;
    REAL = 5.4 ;
  }
  if (WISH > 5.4) {
    ndec = 4 ;
    fils = 3 ;
    REAL = 5.9 ;
  }
  if (WISH > 5.9) {
    ndec = 4 ;
    fils = 4 ;
    REAL = 6.1 ;
  }
  if (WISH > 6.1) {
    ndec = 4 ;
    fils = 5 ;
    REAL = 7.2 ;
  }
  if (WISH > 7.2) {
    ndec = 4 ;
    fils = 6 ;
    REAL = 8.2 ;
  }
  if (WISH > 8.2) {
    ndec = 4 ;
    fils = 7 ;
    REAL = 8.8 ;
  }
  if (WISH > 8.8) {
    ndec = 3 ;
    fils = 1 ;
    REAL = 9.5 ;
  }
#endif
  if (WISH > 9.5) {
    ndec = 3 ;
    fils = 2 ;
    REAL = 10.6 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 10.6) {
    ndec = 3 ;
    fils = 3 ;
    REAL = 11.5 ;
  }
  if (WISH > 11.5) {
    ndec = 3 ;
    fils = 4 ;
    REAL = 12.1 ;
  }
  if (WISH > 12.1) {
    ndec = 3 ;
    fils = 5 ;
    REAL = 14.2 ;
  }
  if (WISH > 14.2) {
    ndec = 3 ;
    fils = 6 ;
    REAL = 16.2 ;
  }
  if (WISH > 16.2) {
    ndec = 3 ;
    fils = 7 ;
    REAL = 17.5 ;
  }
  if (WISH > 17.5) {
    ndec = 2 ;
    fils = 1 ;
    REAL = 18.9 ;
  }
  if (WISH > 18.9) {
    ndec = 2 ;
    fils = 2 ;
    REAL = 21.0 ;
  }
  if (WISH > 21.0) {
    ndec = 2 ;
    fils = 3 ;
    REAL = 22.7 ;
  }
  if (WISH > 22.7) {
    ndec = 2 ;
    fils = 4 ;
    REAL = 24.0 ;
  }
  if (WISH > 24.0) {
    ndec = 2 ;
    fils = 5 ;
    REAL = 28.2 ;
  }
#endif
  if (WISH > 28.2) {
    ndec = 2 ;
    fils = 6 ;
    REAL = 32.2 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 32.2) {
    ndec = 2 ;
    fils = 7 ;
    REAL = 34.7 ;
  }
  if (WISH > 34.7) {
    ndec = 1 ;
    fils = 1 ;
    REAL = 37.7 ;
  }
  if (WISH > 37.7) {
    ndec = 1 ;
    fils = 2 ;
    REAL = 41.7 ;
  }
  if (WISH > 41.7) {
    ndec = 1 ;
    fils = 3 ;
    REAL = 45.2 ;
  }
  if (WISH > 45.2) {
    ndec = 1 ;
    fils = 4 ;
    REAL = 47.9 ;
  }
  if (WISH > 47.9) {
    ndec = 1 ;
    fils = 5 ;
    REAL = 56.2 ;
  }
  if (WISH > 56.2) {
    ndec = 1 ;
    fils = 6 ;
    REAL = 64.1 ;
  }
  if (WISH > 64.1) {
    ndec = 1 ;
    fils = 7 ;
    REAL = 69.2 ;
  }
  if (WISH > 69.2) {
    ndec = 0 ;
    fils = 1 ;
    REAL = 75.2 ;
  }
  if (WISH > 75.2) {
    ndec = 0 ;
    fils = 2 ;
    REAL = 83.2 ;
  }
  if (WISH > 83.2) {
    ndec = 0 ;
    fils = 3 ;
    REAL = 90.0 ;
  }
  if (WISH > 90.0) {
    ndec = 0 ;
    fils = 4 ;
    REAL = 95.3 ;
  }
#endif
  if (WISH > 95.3) {
    ndec = 0 ;
    fils = 5 ;
    REAL = 112.1 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 112.1) {
    ndec = 0 ;
    fils = 6 ;
    REAL = 127.9 ;
  }
  if (WISH > 127.9) {
    ndec = 0 ;
    fils = 7 ;
    REAL = 137.9 ;
  }
  if (WISH > 137.9) {
    ndec = 1 ;
    fils = 4 ;
    REAL = 142.8 ;
  }
  if (WISH > 142.8) {
    ndec = 1 ;
    fils = 5 ;
    REAL = 167.8 ;
  }
  if (WISH > 167.8) {
    ndec = 1 ;
    fils = 9 ;
    REAL = 181.1 ;
  }
  if (WISH > 181.1) {
    ndec = 0 ;
    fils = 15 ;
    REAL = 191.5 ;
  }
  if (WISH > 191.5) {
    ndec = 0 ;
    fils = 1 ;
    REAL = 225.1 ;
  }
  if (WISH > 225.1) {
    ndec = 0 ;
    fils = 2 ;
    REAL = 248.8 ;
  }
  if (WISH > 248.8) {
    ndec = 0 ;
    fils = 3 ;
    REAL = 269.3 ;
  }
  if (WISH > 269.3) {
    ndec = 0 ;
    fils = 4 ;
    REAL = 284.9 ;
  }
#endif
  if (WISH > 284.9) {
    ndec = 0 ;
    fils = 8 ;
    REAL = 335.5 ;
  }
#if 0 // Too many resolutions, not needed
  if (WISH > 335.5) {
    ndec = 0 ;
    fils = 9 ;
    REAL = 361.8 ;
  }
  if (WISH > 361.8) {
    ndec = 0 ;
    fils = 10 ;
    REAL = 420.2 ;
  }
  if (WISH > 420.2) {
    ndec = 0 ;
    fils = 11 ;
    REAL = 468.4 ;
  }
  if (WISH > 468.4) {
    ndec = 0 ;
    fils = 12 ;
    REAL = 518.8 ;
  }
  if (WISH > 518.8) {
    ndec = 0 ;
    fils = 13 ;
    REAL = 577.0 ;
  }
#endif
  if (WISH > 577.0) {
    ndec = 0 ;
    fils = 14 ;
    REAL = 620.7 ;
  }

  if (WISH > 137.9) dwn3 = 1 ;

  byte BW = (dwn3 << 7) | (ndec << 4) | fils ;

  SI4432_Write_Byte(0x1C , BW ) ;

  rxosr = 500.0 * ( 1.0 + 2.0 * dwn3 ) / ( pow(2.0, (ndec-3.0)) * REAL );

  byte integer = (int)rxosr ;
  byte fractio = (int)((rxosr - integer) * 8 ) ;
  byte memory = (integer << 3) | (0x07 & fractio) ;

  SI4432_Write_Byte(0x20 , memory ) ;
  return REAL ;
}


void SI4432_Set_Frequency ( long Freq ) {
  int hbsel;
  long Carrier;
  if (Freq >= 480000000) {
    hbsel = 1;
    Freq = Freq / 2;
  } else {
    hbsel = 0;
  }
  int sbsel = 1;
  int N = Freq / 10000000;
  Carrier = ( 4 * ( Freq - N * 10000000 )) / 625;
  int Freq_Band = ( N - 24 ) | ( hbsel << 5 ) | ( sbsel << 6 );
  SI4432_Write_Byte ( 0x75, Freq_Band );
  SI4432_Write_Byte ( 0x76, (Carrier>>8) & 0xFF );
  SI4432_Write_Byte ( 0x77, Carrier & 0xFF  );
  chThdSleepMilliseconds(2);
}

float SI4432_RSSI(uint32_t i, int s)
{
  int RSSI_RAW;
  // SEE DATASHEET PAGE 61
#ifdef USE_SI4463
  if (SI4432_Sel == 2) {
    RSSI_RAW = Si446x_getRSSI();
  } else
#endif
    RSSI_RAW = (unsigned char)SI4432_Read_Byte( 0x26 ) ;
  float dBm = 0.5 * RSSI_RAW - 120.0 ;
  // Serial.println(dBm,2);
  return dBm ;
}


void SI4432_Sub_Init()
{
  SI4432_Reset();
  // Enable receiver chain
//  SI4432_Write_Byte(0x07, 0x05);
  // Clock Recovery Gearshift Value
  SI4432_Write_Byte(0x1F, 0x00);
  // IF Filter Bandwidth
  SI4432_SET_RBW(10) ;
//  // Register 0x75 Frequency Band Select
//  byte sbsel = 1 ;  // recommended setting
//  byte hbsel = 0 ;  // low bands
//  byte fb = 19 ;    // 430–439.9 MHz
//  byte FBS = (sbsel << 6 ) | (hbsel << 5 ) | fb ;
//  SI4432_Write_Byte(0x75, FBS) ;
  SI4432_Write_Byte(0x75, 0x46) ;
  // Register 0x76 Nominal Carrier Frequency
  // WE USE 433.92 MHz
  // Si443x-Register-Settings_RevB1.xls
//  SI4432_Write_Byte(0x76, 0x62) ;
  SI4432_Write_Byte(0x76, 0x00) ;
  // Register 0x77 Nominal Carrier Frequency
  SI4432_Write_Byte(0x77, 0x00) ;
  // RX MODEM SETTINGS
  SI4432_Write_Byte(0x1C, 0x81) ;
  SI4432_Write_Byte(0x1D, 0x3C) ;
  SI4432_Write_Byte(0x1E, 0x02) ;
  SI4432_Write_Byte(0x1F, 0x03) ;
  // SI4432_Write_Byte(0x20, 0x78) ;
  SI4432_Write_Byte(0x21, 0x01) ;
  SI4432_Write_Byte(0x22, 0x11) ;
  SI4432_Write_Byte(0x23, 0x11) ;
  SI4432_Write_Byte(0x24, 0x01) ;
  SI4432_Write_Byte(0x25, 0x13) ;
  SI4432_Write_Byte(0x2A, 0xFF) ;
  SI4432_Write_Byte(0x2C, 0x28) ;
  SI4432_Write_Byte(0x2D, 0x0C) ;
  SI4432_Write_Byte(0x2E, 0x28) ;


  SI4432_Write_Byte(0x69, 0x60); // AGC, no LNA, fast gain increment


// GPIO automatic antenna switching
  SI4432_Write_Byte(0x0B, 0x12) ;
  SI4432_Write_Byte(0x0C, 0x15) ;
}

#define V0_XTAL_CAPACITANCE 0x64
#define V1_XTAL_CAPACITANCE 0x64



void SI4432_Init()
{


//DebugLine("IO set");
  SI4432_Sel = 0;
  SI4432_Sub_Init();

  SI4432_Sel = 1;
  SI4432_Sub_Init();
//DebugLine("1 init done");

  SI4432_Sel = 0;
  SI4432_Write_Byte(0x07, 0x07);// Enable receiver chain
//  SI4432_Write_Byte(0x09, V0_XTAL_CAPACITANCE);// Tune the crystal
  SI4432_Set_Frequency(433920000);
  SI4432_Write_Byte(0x0D, 0x1F) ; // Set GPIO2 output to ground


  SI4432_Sel = 1;
  SI4432_Write_Byte(0x7, 0x0B); // start TX
//  SI4432_Write_Byte(0x09, V1_XTAL_CAPACITANCE);// Tune the crystal
  SI4432_Set_Frequency(443920000);
  SI4432_Write_Byte(0x6D, 0x1F);//Set full power

  SI4432_Write_Byte(0x0D, 0xC0) ; // Set GPIO2 maximumdrive and clock output
  SI4432_Write_Byte(0x0A, 0x02) ; // Set 10MHz output
}

void SI4432_SetPowerReference(int freq)
{
  SI4432_Sel = 1;         //Select Lo module
  if (freq < 0 || freq > 7 ) {
    SI4432_Write_Byte(0x0D, 0x1F) ; // Set GPIO2 to GND
  } else {
    SI4432_Write_Byte(0x0D, 0xC0) ; // Set GPIO2 maximumdrive and clock output
    SI4432_Write_Byte(0x0A, freq & 0x07) ; // Set GPIO2 frequency
  }
}

//------------PE4302 -----------------------------------------------

// Comment out this define to use parallel mode PE4302

#define PE4302_en 10

void PE4302_init(void) {
  CS_PE_LOW;
}

extern void shiftOut(uint8_t val);

void PE4302_Write_Byte(unsigned char DATA )
{
  SPI2_CLK_LOW;
  shiftOut(DATA);
  CS_PE_HIGH;
  CS_PE_LOW;
}

#endif
