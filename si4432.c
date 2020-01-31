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

#define CS_SI0_HIGH     palSetPad(GPIOB, 2)
#define CS_SI1_HIGH     palSetPad(GPIOB, 10)
#define CS_PE_HIGH      palSetPad(GPIOB, 11)

#define CS_SI0_LOW     palClearPad(GPIOB, 2)
#define CS_SI1_LOW     palClearPad(GPIOB, 10)
#define CS_PE_LOW      palClearPad(GPIOB, 11)

// Display
#define CS_LOW          palClearPad(GPIOB, 6)
#define CS_HIGH         palSetPad(GPIOB, 6)


#define SI0_PIN         2
#define SI1_PIN         10
#define PE_PIN          11


static void ssp_wait_slot(void)
{
  while ((SPI1->SR & 0x1800) == 0x1800)
    ;
}

static void ssp_senddata(uint8_t x)
{
  *(uint8_t*)(&SPI1->DR) = x;
  while (SPI1->SR & SPI_SR_BSY)
    ;
}

static uint8_t ssp_sendrecvdata(uint8_t x)
{
    while (!(SPI1->SR & SPI_SR_TXE));
    // clear OVR
    while (SPI1->SR & SPI_SR_RXNE) (void)SPI1->DR;

    *(uint8_t*)(&SPI1->DR) = x;
    while (!(SPI1->SR & SPI_SR_RXNE));
    return SPI1->DR;
}

static void ssp_databit8(void)
{
  SPI1->CR2 = (SPI1->CR2 & 0xf0ff) | 0x0700;
//LPC_SSP1->CR0 = (LPC_SSP1->CR0 & 0xf0) | SSP_DATABIT_8;
}


const int SI_nSEL[3] = { 2,10, 0}; // #3 is dummy!!!!!!

int SI4432_Sel = 0;         // currently selected SI4432

// Use protected with with
// chMtxLock(&mutex_ili9341);
// chMtxUnlock(&mutex_ili9341);



void SI4432_Write_Byte(byte ADR, byte DATA )
{
  CS_HIGH;                  // ensure display is not selected
  palClearPad(GPIOB, SI_nSEL[SI4432_Sel]);
  ADR |= 0x80 ; // RW = 1
//  digitalWrite(SI_SCLK, LOW);
//  ssp_databit8();
  ssp_senddata( ADR );
  ssp_senddata( DATA );
  palSetPad(GPIOB, SI_nSEL[SI4432_Sel]);
}

byte SI4432_Read_Byte( byte ADR )
{
  byte DATA ;
  CS_HIGH;                  // ensure display is not selected
  palClearPad(GPIOB, SI_nSEL[SI4432_Sel]);
//  ssp_databit8();
  ssp_senddata( ADR );
  DATA = ssp_sendrecvdata( 0 );
  palSetPad(GPIOB, SI_nSEL[SI4432_Sel]);
  return DATA ;
}
