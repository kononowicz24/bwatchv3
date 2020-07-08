//k24 07-07-2020: the below code is the modification of the following:
#ifndef __OLED_H
#define __OLED_H
//
// oled_turbo
//
// Copyright (c) 2018 BitBank Software, Inc.
// Written by Larry Bank
// project started 5/3/2018
// bitbank@pobox.com
//
// This experimental code is meant to push the SSD1306 OLED controller to the limits by bit-banging the I2C
// protocol. The code is designed for speed only and does not conform to the I2C spec. This
// may not work on all devices which use the SSD1306 / SSD1106.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#define FONT_NORMAL 0
#define FONT_SMALL 1
#define FONT_LARGE 2

//
// Comment out this line to gain 1K of RAM and not use a backing buffer
//
#define USE_BACKBUFFER

#include "oled_fonts.h"
// some globals
static int iScreenOffset; // current write offset of screen data
#ifdef USE_BACKBUFFER
static unsigned char ucScreen[1024]; // local copy of the image buffer
#endif
static byte oled_addr;
#define MAX_CACHE 32
static byte bCache[MAX_CACHE] = {0x40}; // for faster character drawing
static byte bEnd = 1;
static void oledWriteCommand(unsigned char c);

#define DIRECT_PORT
#define I2CPORT PORTC
// A bit set to 1 in the DDR is an output, 0 is an INPUT
#define I2CDDR DDRC

// Pin or port numbers for SDA and SCL
#define BB_SDA 1
#define BB_SCL 0

#if  F_CPU > 8000000L
 #define I2C_CLK_LOW() I2CPORT &= ~(1 << BB_SCL) //compiles to cbi instruction taking 2 clock cycles, extending the clock pulse
#else
 #define I2C_CLK_LOW() I2CPORT = bOld //setting a port instruction takes 1 clock cycle
#endif 

//
// Transmit a byte and ack bit
//
static inline void i2cByteOut(byte b)
{
byte i;
byte bOld = I2CPORT & ~((1 << BB_SDA) | (1 << BB_SCL));
     for (i=0; i<8; i++)
     {
         bOld &= ~(1 << BB_SDA);
         if (b & 0x80)
            bOld |= (1 << BB_SDA);
         I2CPORT = bOld;
         I2CPORT |= (1 << BB_SCL);
         I2C_CLK_LOW();
         b <<= 1;
     } // for i
// ack bit
  I2CPORT = bOld & ~(1 << BB_SDA); // set data low
  I2CPORT |= (1 << BB_SCL); // toggle clock
  I2C_CLK_LOW();
} /* i2cByteOut() */

void i2cBegin(byte addr)
{
   I2CPORT |= ((1 << BB_SDA) + (1 << BB_SCL));
   I2CDDR |= ((1 << BB_SDA) + (1 << BB_SCL));
   I2CPORT &= ~(1 << BB_SDA); // data line low first
   I2CPORT &= ~(1 << BB_SCL); // then clock line low is a START signal
   i2cByteOut(addr << 1); // send the slave address
} /* i2cBegin() */

void i2cWrite(byte *pData, byte bLen)
{
byte i, b;
byte bOld = I2CPORT & ~((1 << BB_SDA) | (1 << BB_SCL));

   while (bLen--)
   {
      b = *pData++;
      if (b == 0 || b == 0xff) // special case can save time
      {
         bOld &= ~(1 << BB_SDA);
         if (b & 0x80)
            bOld |= (1 << BB_SDA);
         I2CPORT = bOld;
         for (i=0; i<8; i++)
         {
            I2CPORT |= (1 << BB_SCL); // just toggle SCL, SDA stays the same
            I2C_CLK_LOW();
         } // for i    
     }
     else // normal byte needs every bit tested
     {
        for (i=0; i<8; i++)
        {
          
         bOld &= ~(1 << BB_SDA);
         if (b & 0x80)
            bOld |= (1 << BB_SDA);

         I2CPORT = bOld;
         I2CPORT |= (1 << BB_SCL);
         I2C_CLK_LOW();
         b <<= 1;
        } // for i
     }
// ACK bit seems to need to be set to 0, but SDA line doesn't need to be tri-state
      I2CPORT &= ~(1 << BB_SDA);
      I2CPORT |= (1 << BB_SCL); // toggle clock
      I2CPORT &= ~(1 << BB_SCL);
   } // for each byte
} /* i2cWrite() */

//
// Send I2C STOP condition
//
void i2cEnd()
{
  I2CPORT &= ~(1 << BB_SDA);
  I2CPORT |= (1 << BB_SCL);
  I2CPORT |= (1 << BB_SDA);
  //I2CDDR &= ~((1 << BB_SDA) | (1 << BB_SCL)); // let the lines float (tri-state)
} /* i2cEnd() */

// Wrapper function to write I2C data on Arduino
static void I2CWrite(int iAddr, unsigned char *pData, int iLen)
{
  //i2cBegin(oled_addr);
  i2cWrite(pData, iLen);
  //i2cEnd();
} /* I2CWrite() */

static void oledCachedFlush(void)
{
       I2CWrite(oled_addr, bCache, bEnd); // write the old data
#ifdef USE_BACKBUFFER
       memcpy(&ucScreen[iScreenOffset], &bCache[1], bEnd-1);
       iScreenOffset += (bEnd - 1);
#endif
       bEnd = 1;
} /* oledCachedFlush() */

static void oledCachedWrite(byte *pData, byte bLen)
{

   if (bEnd + bLen > MAX_CACHE) // need to flush it
   {
       oledCachedFlush(); // write the old data
   }
   memcpy(&bCache[bEnd], pData, bLen);
   bEnd += bLen;
  
} /* oledCachedWrite() */
//
// Initializes the OLED controller into "page mode"
//
void oledInit(byte bAddr, int bFlip, int bInvert)
{
unsigned char uc[4];
unsigned char oled_initbuf[]={0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,
      0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,
      0xaf,0x20,0x02};

  oled_addr = bAddr;
  I2CDDR &= ~(1 << BB_SDA);
  I2CDDR &= ~(1 << BB_SCL); // let them float high
  I2CPORT |= (1 << BB_SDA); // set both lines to get pulled up
  I2CPORT |= (1 << BB_SCL);
  
  I2CWrite(oled_addr, oled_initbuf, sizeof(oled_initbuf));
  if (bInvert)
  {
    uc[0] = 0; // command
    uc[1] = 0xa7; // invert command
    I2CWrite(oled_addr, uc, 2);
  }
  if (bFlip) // rotate display 180
  {
    uc[0] = 0; // command
    uc[1] = 0xa0;
    I2CWrite(oled_addr, uc, 2);
    uc[1] = 0xc0;
    I2CWrite(oled_addr, uc, 2);
  }
} /* oledInit() */
//
// Sends a command to turn off the OLED display
//
void oledShutdown()
{
    oledWriteCommand(0xaE); // turn off OLED
}

// Send a single byte command to the OLED controller
static void oledWriteCommand(unsigned char c)
{
unsigned char buf[2];

  buf[0] = 0x00; // command introducer
  buf[1] = c;
  I2CWrite(oled_addr, buf, 2);
} /* oledWriteCommand() */

static void oledWriteCommand2(unsigned char c, unsigned char d)
{
unsigned char buf[3];

  buf[0] = 0x00;
  buf[1] = c;
  buf[2] = d;
  I2CWrite(oled_addr, buf, 3);
} /* oledWriteCommand2() */

//
// Sets the brightness (0=off, 255=brightest)
//
void oledSetContrast(unsigned char ucContrast)
{
  oledWriteCommand2(0x81, ucContrast);
} /* oledSetContrast() */

//
// Send commands to position the "cursor" (aka memory write address)
// to the given row and column
//
static void oledSetPosition(int x, int y)
{
  oledWriteCommand(0xb0 | y); // go to page Y
  oledWriteCommand(0x00 | (x & 0xf)); // // lower col addr
  oledWriteCommand(0x10 | ((x >> 4) & 0xf)); // upper col addr
  iScreenOffset = (y*128)+x;
}

//
// Write a block of pixel data to the OLED
// Length can be anything from 1 to 1024 (whole display)
//
static void oledWriteDataBlock(unsigned char *ucBuf, int iLen)
{
unsigned char ucTemp[129];

  ucTemp[0] = 0x40; // data command
  memcpy(&ucTemp[1], ucBuf, iLen);
  I2CWrite(oled_addr, ucTemp, iLen+1);
  // Keep a copy in local buffer
#ifdef USE_BACKBUFFER
  memcpy(&ucScreen[iScreenOffset], ucBuf, iLen);
  iScreenOffset += iLen;
#endif
}

// Set (or clear) an individual pixel
// The local copy of the frame buffer is used to avoid
// reading data from the display controller
int oledSetPixel(int x, int y, unsigned char ucColor)
{
int i;
unsigned char uc, ucOld;

  i = ((y >> 3) * 128) + x;
  if (i < 0 || i > 1023) // off the screen
    return -1;
#ifdef USE_BACKBUFFER
  uc = ucOld = ucScreen[i];
#else
  uc = ucOld = 0;
#endif

  uc &= ~(0x1 << (y & 7));
  if (ucColor)
  {
    uc |= (0x1 << (y & 7));
  }
  if (uc != ucOld) // pixel changed
  {
    oledSetPosition(x, y>>3);
    oledWriteDataBlock(&uc, 1);
#ifdef USE_BACKBUFFER
    ucScreen[i] = uc;
#endif
  }
  return 0;
} /* oledSetPixel() */

//
// Invert font data
//
void InvertBytes(byte *pData, byte bLen)
{
byte i;
   for (i=0; i<bLen; i++)
   {
      *pData = ~(*pData);
      pData++;
   }
} /* InvertBytes() */

//
// Load a 128x64 1-bpp Windows bitmap
// Pass the pointer to the beginning of the BMP file
// First pass version assumes a full screen bitmap
//
int oledLoadBMP(byte *pBMP)
{
int16_t i16;
int iOffBits, q, y, j; // offset to bitmap data
int iPitch;
byte x, z, b, *s;
byte dst_mask;
byte ucTemp[16]; // process 16 bytes at a time
byte bFlipped = false;

  i16 = pgm_read_word(pBMP);
  if (i16 != 0x4d42) // must start with 'BM'
     return -1; // not a BMP file
  i16 = pgm_read_word(pBMP + 18);
  if (i16 != 128) // must be 128 pixels wide
     return -1;
  i16 = pgm_read_word(pBMP + 22);
  if (i16 != 64 && i16 != -64) // must be 64 pixels tall
     return -1;
  if (i16 == 64) // BMP is flipped vertically (typical)
     bFlipped = true;
  i16 = pgm_read_word(pBMP + 28);
  if (i16 != 1) // must be 1 bit per pixel
     return -1;
  iOffBits = pgm_read_word(pBMP + 10);
  iPitch = 16;
  if (bFlipped)
  { 
    iPitch = -16;
    iOffBits += (63 * 16); // start from bottom
  }

// rotate the data and send it to the display
  for (y=0; y<8; y++) // 8 lines of 8 pixels
  {
     oledSetPosition(0, y);
     for (j=0; j<8; j++) // do 8 sections of 16 columns
     {
         s = &pBMP[iOffBits + (j*2) + (y * iPitch*8)]; // source line
         memset(ucTemp, 0, 16); // start with all black
         for (x=0; x<16; x+=8) // do each block of 16x8 pixels
         {
            dst_mask = 1;
            for (q=0; q<8; q++) // gather 8 rows
            {
               b = pgm_read_byte(s + (q * iPitch));
               for (z=0; z<8; z++) // gather up the 8 bits of this column
               {
                  if (b & 0x80)
                      ucTemp[x+z] |= dst_mask;
                  b <<= 1;
               } // for z
               dst_mask <<= 1;
            } // for q
            s++; // next source byte
         } // for x
         oledWriteDataBlock(ucTemp, 16);
     } // for j
  } // for y
} /* oledLoadBMP() */

//
// Draw a string of normal (8x8), small (6x8) or large (16x32) characters
// At the given col+row
//
int oledWriteString(int x, int y, char *szMsg, int iSize, int bInvert)
{
int i, iLen, iFontOff;
unsigned char c, *s, ucTemp[16];

    iLen = strlen(szMsg);
    oledSetPosition(x, y);
    if (iSize == FONT_NORMAL) // 8x8 font
    {
       if (iLen*8 + x > 128) iLen = (128 - x)/8; // can't display it
       if (iLen < 0)return -1;

       for (i=0; i<iLen; i++)
       {
         c = (unsigned char)szMsg[i];
         iFontOff =  (int)c * 8;
         // we can't directly use the pointer to FLASH memory, so copy to a local buffer
         memcpy_P(ucTemp, &ucFont[iFontOff], 8);
         if (bInvert) InvertBytes(ucTemp, 8);
         oledCachedWrite(ucTemp, 8);
//         oledWriteDataBlock(ucTemp, 8); // write character pattern
       }
    oledCachedFlush(); // write any remaining data
    }
#ifndef __AVR_ATtiny85__
    if (iSize == FONT_LARGE) // 16x32 font
    {
      if (iLen*16+x > 128) iLen = (128-x)/16;
      if (iLen < 0) return -1;
      for (i=0; i<iLen; i++)
      {
          s = (unsigned char *)&ucBigFont[(unsigned char)szMsg[i]*64];
          // we can't directly use the pointer to FLASH memory, so copy to a local buffer
          oledSetPosition(x+(i*16), y);
          memcpy_P(ucTemp, s, 16);
          if (bInvert) InvertBytes(ucTemp, 16);
          oledWriteDataBlock(ucTemp, 16); // write character pattern
          oledSetPosition(x+(i*16), y+1);
          memcpy_P(ucTemp, s+16, 16);
          if (bInvert) InvertBytes(ucTemp, 16);
          oledWriteDataBlock(ucTemp, 16); // write character pattern
          if (y <= 5)
          {
             oledSetPosition(x+(i*16), y+2);
             memcpy_P(ucTemp, s+32, 16);
             if (bInvert) InvertBytes(ucTemp, 16);
             oledWriteDataBlock(ucTemp, 16); // write character pattern
          }
          if (y <= 4)
          {
             oledSetPosition(x+(i*16), y+3);
             memcpy_P(ucTemp, s+48, 16);
             if (bInvert) InvertBytes(ucTemp, 16);
             oledWriteDataBlock(ucTemp, 16); // write character pattern
          }
       }
    }
#endif
    if (iSize == FONT_SMALL) // 6x8 font
    {
       if (iLen*6 + x > 128) iLen = (128 - x)/6; // can't display it
       if (iLen < 0)return -1;

       for (i=0; i<iLen; i++)
       {
         // we can't directly use the pointer to FLASH memory, so copy to a local buffer
         memcpy_P(ucTemp, &ucSmallFont[(unsigned char)szMsg[i]*6], 6);
         if (bInvert) InvertBytes(ucTemp, 6);
//         oledWriteDataBlock(ucTemp, 6); // write character pattern
         oledCachedWrite(ucTemp, 6);
       }
    oledCachedFlush(); // write any remaining data      
    }
  return 0;
} /* oledWriteString() */

//
// Fill the frame buffer with a byte pattern
// e.g. all off (0x00) or all on (0xff)
//
void oledFill(unsigned char ucData)
{
int x, y;
unsigned char temp[16];

  memset(temp, ucData, 16);
  for (y=0; y<8; y++)
  {
    oledSetPosition(0,y); // set to (0,Y)
    for (x=0; x<8; x++)
    {
      oledWriteDataBlock(temp, 16); 
    } // for x
  } // for y
#ifdef USE_BACKBUFFER
   memset(ucScreen, ucData, 1024);
#endif
} /* oledFill() */

void oledDrawLine(int x1, int y1, int x2, int y2)
{
  int temp, i;
  int dx = x2 - x1;
  int dy = y2 - y1;
  int error;
  byte *p, *pStart, mask, bOld, bNew;
  int xinc, yinc;
  int y, x;
  
  if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 > 127 || x2 > 127 || y1 > 63 || y2 > 63)
     return;

  if(abs(dx) > abs(dy)) {
    // X major case
    if(x2 < x1) {
      dx = -dx;
      temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    }

    y = y1;
    dy = (y2 - y1);
    error = dx >> 1;
    yinc = 1;
    if (dy < 0)
    {
      dy = -dy;
      yinc = -1;
    }
    p = pStart = &ucScreen[x1 + ((y >> 3) << 7)]; // point to current spot in back buffer
    mask = 1 << (y & 7); // current bit offset
    for(x=x1; x1 <= x2; x1++) {
      *p++ |= mask; // set pixel and increment x pointer
      error -= dy;
      if (error < 0)
      {
        error += dx;
        if (yinc > 0)
           mask <<= 1;
        else
           mask >>= 1;
        if (mask == 0) // we've moved outside the current row, write the data we changed
        {
           oledSetPosition(x, y>>3);
           oledWriteDataBlock(pStart,  (int)(p-pStart)); // write the row we changed
           x = x1+1; // we've already written the byte at x1
           y1 = y+yinc;
           p += (yinc > 0) ? 128 : -128;
           pStart = p;
           mask = 1 << (y1 & 7);
        }
        y += yinc;
      }
    } // for x1    
   if (p != pStart) // some data needs to be written
   {
     oledSetPosition(x, y>>3);
     oledWriteDataBlock(pStart, (int)(p-pStart));
   }
  }
  else {
    // Y major case
    if(y1 > y2) {
      dy = -dy;
      temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    } 

    p = &ucScreen[x1 + ((y1 >> 3) * 128)]; // point to current spot in back buffer
    bOld = bNew = p[0]; // current data at that address
    mask = 1 << (y1 & 7); // current bit offset
    dx = (x2 - x1);
    error = dy >> 1;
    xinc = 1;
    if (dx < 0)
    {
      dx = -dx;
      xinc = -1;
    }
    for(x = x1; y1 <= y2; y1++) {
      bNew |= mask; // set the pixel
      error -= dx;
      mask <<= 1; // y1++
      if (mask == 0) // we're done with this byte, write it if necessary
      {
        if (bOld != bNew)
        {
          p[0] = bNew; // save to RAM
          oledSetPosition(x, y1>>3);
          oledWriteDataBlock(&bNew, 1);
        }
        p += 128; // next line
        bOld = bNew = p[0];
        mask = 1; // start at LSB again
      }
      if (error < 0)
      {
        error += dy;
        if (bOld != bNew) // write the last byte we modified if it changed
        {
          p[0] = bNew; // save to RAM
          oledSetPosition(x, y1>>3);
          oledWriteDataBlock(&bNew, 1);         
        }
        p += xinc;
        x += xinc;
        bOld = bNew = p[0];
      }
    } // for y
    if (bOld != bNew) // write the last byte we modified if it changed
    {
      p[0] = bNew; // save to RAM
      oledSetPosition(x, y2>>3);
      oledWriteDataBlock(&bNew, 1);        
    }
  } // y major case
} /* oledDrawLine() */

#endif