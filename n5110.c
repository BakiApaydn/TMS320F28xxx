/* ************************************************************************** */
/*
 * ��������     : ��� ������� ��� ������������ LCD �� Nokia 5110, � ����� ��� ��������� ������.
 * �����        : Xander Gresolio <xugres@gmail.com>
 * ���-�������� : https://github.com/gresolio/N3310Lib
 * ��������     : GPL v3.0
 * �����������  : kobzar aka kobraz ��� http://cxem.net/ maodzedun@gmail.com
 */
/* ************************************************************************** */

/* ************************************************************************** */
#include "stddef.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

#include "init.h"

#if (1==__AVR__)
#include <avr/interrupt.h>
#include "n5110.h"
#endif //(1==__AVR__)

#if (1==__TMS320__)
#include "n5110.h"
#include "5110.h"
//#include "F2802x_Device.h"
#include "include/F2802x_Device.h"     // DSP2802x Headerfile Include File
#endif //(1==__TMS320__)

/* ************************************************************************** */

#if (1==__USE_LCD_5110__)

// Version 1
/* ************************************************************************** */
// j6   | pin   |
// -----+-------+---------------------------------------------------------------
// RST  | GPIO0 |   �����
// CE   | GPIO1 |   ���������
// DC   | GPIO2 |   ������� / ������
// SDIN | GPIO3 |   ���������������� ���� ������
// SCLK | GPIO4 |   ���������������� ���� ������
/* ************************************************************************** */

// Version 2
/* ************************************************************************** */
// j6    | pin       |
// ------+-----------+----------------------------------------------------------
// RST   | GPIO7     |   �����
// CE    | GPIO6     |   ���������
// DC    | GPIO17/33 |   ������� / ������
// SDIN  | GPIO16/32 |   ���������������� ���� ������
// SCLK  | GPIO12    |   ���������������� ���� ������
// LIGHT | GPIO12    |   ���������
/* ************************************************************************** */


/* ************************************************************************** */
// ��������� ��������� ������� ��������
/* ************************************************************************** */
static void LcdSend    ( byte data, LcdCmdData cd );
static void Delay_lcd5510      ( void );
/* ************************************************************************** */


/* ************************************************************************** */
// ���������� ����������
/* ************************************************************************** */
// ��� � ��� 84*48 ��� ��� 504 �����
static byte  LcdCache [ LCD_CACHE_SIZE ];
/* ************************************************************************** */


/* ************************************************************************** */
// ����� �� ��������� ���� �������, � ���� �� ����� ��� ����������,
// ����� �������� ��� ������� ���� ��� ��������� ���������. �����
// ����� ���������� ��� ����� ���� ����� ��������� � ��� �������.
/* ************************************************************************** */
static int   LoWaterMark;   // ������ �������
static int   HiWaterMark;   // ������� �������
/* ************************************************************************** */


/* ************************************************************************** */
// ��������� ��� ������ � LcdCache[]
/* ************************************************************************** */
static int   LcdCacheIdx;
/* ************************************************************************** */


/* ************************************************************************** */
// ���� ��������� ����
/* ************************************************************************** */
static byte  UpdateLcd;
/* ************************************************************************** */

unsigned char ArrayDisplay[LCD_X_RES];



/* ==========================================================================
 * NAME - LCD_PrintToScreen
 * IN   - void
 * OUT  - void
 * RET  - void
   ========================================================================== */
void LCD_PrintToScreen ( void ) {

/*	//Lcd_prints(0, 0, FONT_1X, "~" );
	Lcd_prints(1, 0, FONT_1X, "Hello world! " );
	Lcd_prints(1, 1, FONT_1X, "It's working." );
	Lcd_prints(1, 2, FONT_1X, "             " );
	Lcd_prints(1, 3, FONT_1X, "uschema.com  " );
	Lcd_prints(1, 4, FONT_1X, "             " );
	Lcd_prints(1, 5, FONT_1X, "TMS320F28027." );
*/
	byte nn;
	byte strings[6][14] = {
		"Laboratotian ",
		"PowerInverter",
		"220V 50Hz    ",
		"Based on DSP:",
		"TMS320F28027 ",
		" uschema.com ",
	};

	for (nn=0; nn<6; nn++) {
		Lcd_prints(1, nn, FONT_1X, &strings[nn][0] );
	}

	//Lcd_rect_empty ( 0, 8, 8, LCD_X_RES-1, PIXEL_XOR);
	//Lcd_line( 20, 8, 30, 16, PIXEL_ON );
	Lcd_rect ( 0, 8, 8, LCD_X_RES-1, PIXEL_XOR);
	Lcd_circle ( 2, 4, 1, PIXEL_OFF );

	//memset (ArrayDisplay, 16, LCD_X_RES);
	//ShowDisplayFromBufferByVertical( ArrayDisplay, 1/*K=0..1*/ );
	//Lcd_update();
}



/* ************************************************************************** */
/*
 * ���                   : ShowDisplayFromBuffer
 * ��������              : ���������� ����� ������ �� LCD ������� � �������
 * ��������(�)           : ���
 * ������������ �������� : ���
 */
/* ************************************************************************** */
int ShowDisplayFromBufferByVertical( unsigned char *pArrayDisplay, float zoom )
{
	byte  x_line = 0, y_offset;

	for ( x_line=0; x_line<LCD_X_RES; x_line++ ) {
	  //pArrayDisplay+=x_line;
	  y_offset = LCD_Y_RES - pArrayDisplay[x_line]*zoom;
	  pArrayDisplay[x_line] = 10;
	  Lcd_line (
	    /*x1*/x_line, /*y1*/y_offset,
	    /*x2*/x_line, /*y2*/LCD_Y_RES,
	    PIXEL_ON
	  );
	}
	return 0;
}



/* ************************************************************************** */
/*
 * ���                   : Lcd_init
 * ��������              : ���������� ������������� ����� � SPI ��, ����������� LCD
 * ��������(�)           : ���
 * ������������ �������� : ���
 */
/* ************************************************************************** */
void Lcd_init ( void ) {
#if (1==__AVR__)
    // Pull-up �� ����� ������������ � reset �������
    LCD_PORT |= _BV ( LCD_RST_PIN );

    // ������������� ������ ���� ����� �� �����
    LCD_DDR |= _BV( LCD_RST_PIN ) | _BV( LCD_DC_PIN ) | _BV( LCD_CE_PIN ) | _BV( SPI_MOSI_PIN ) | _BV( SPI_CLK_PIN );

    // ��������������� ��������
    Delay_lcd5510();

    // ������� reset
    LCD_PORT &= ~( _BV( LCD_RST_PIN ) );
    Delay_lcd5510();
    LCD_PORT |= _BV ( LCD_RST_PIN );

    // ���������� SPI:
    // ��� ����������, ������� ��� ������, ����� �������, CPOL->0, CPHA->0, Clk/4
    SPCR = 0x50;

    // ��������� LCD ���������� - ������� ������� �� SCE
    LCD_PORT |= _BV( LCD_CE_PIN );

    // ���������� ������� �������
    LcdSend ( 0x21, LCD_CMD ); // �������� ����������� ����� ������ (LCD Extended Commands)
    LcdSend ( 0xC8, LCD_CMD ); // ��������� ������������� (LCD Vop)
    LcdSend ( 0x06, LCD_CMD ); // ��������� �������������� ������������ (Temp coefficent)
    LcdSend ( 0x13, LCD_CMD ); // ��������� ������� (LCD bias mode 1:48)
    LcdSend ( 0x20, LCD_CMD ); // �������� ����������� ����� ������ � �������������� ��������� (LCD Standard Commands,Horizontal addressing mode)
    LcdSend ( 0x0C, LCD_CMD ); // ���������� ����� (LCD in normal mode)

    // ��������� ������� �������
    Lcd_clear();
    Lcd_update();
#endif //(1==__AVR__)

#if (1==__TMS320__)
    rst_l;           // Reset (2) 0 OXFD = 11111101
    DELAY(10);       // �������� 5us
    rst_h;           // Reset (2) 1

    ce_l;            // ��� �������� (3) 0, �������� 0xFB = 11111011
    DELAY(0);        // �������� 5us
    ce_h;            // ��� �����


    /*
     """ sets the LCD contrast """
     command([0x21, 0x14, value, 0x20, 0x0c])
     */

    // ���������� ������� �������
    // ........................................................................
    // �������� ����������� ����� ������ (LCD Extended Commands)
    // �������� ������
    // 0x21 = 00100001 - Normal
    // 0b00000001 -
    // 0b00000010 -
    // 0b00000100 -
    // 0b00001000 - Mirror Y
    // 0b00010000 - Mirror X
    // 0b00100000 - 1
    // 0b01000000 -
    // 0b10000000 -
    write_com(0x33);

    // ��������� ������������� (LCD Vop)
    // 0xC8 = 11001000 - Normal
    // 0b00000001 -
    // 0b00000010 -
    // 0b00000100 -
    // 0b00001000 -
    // 0b00010000 -
    // 0b00100000 -
    // 0b01000000 -
    // 0b10000000 -
    write_com(0xc0);

    // ��������� �������������� ������������ (Temp coefficent)
    // 0x06 = 00000110 - Normal
    // 0b00000001 -
    // 0b00000010 -
    // 0b00000100 -
    // 0b00001000 -
    // 0b00010000 -
    // 0b00100000 -
    // 0b01000000 -
    // 0b10000000 -
    write_com(0x07);

    // ��������� ������� (LCD bias mode 1:48)
    // 0x13 = 00010011 - Normal
    // 0b00000001 -
    // 0b00000010 -
    // 0b00000100 -
    // 0b00001000 -
    // 0b00010000 - Light
    // 0b00100000 - Light
    // 0b01000000 - Light
    // 0b10000000 - Light
    write_com(0x13);

    // �������� ����������� ����� ������ � �������������� ���������
    // (LCD Standard Commands,Horizontal addressing mode)
    // 0x20 - Normal
    // �������� ����� ������.
    // 0b00000001 -
    // 0b00000010 -
    // 0b00000100 -
    // 0b00001000 - Mirror Y
    // 0b00010000 - Mirror X
    // 0b00100000 -
    // 0b01000000 -
    // 0b10000000 -
    write_com(0x30);

    // ���������� ����� (LCD in normal mode)
    // ������� ����� 12 (0x0c) = 00001100
    // 0b00000001 - Inversion
    // 0b00000010 -
    // 0b00000100 - Normal text
    // 0b00001000 - Normal text
    // 0b00010000 -
    // 0b00100000 -
    // 0b01000000 -
    // 0b10000000 -
    write_com(0x0c); // color Normal
    // write_com(0x0d); // color Inverted
    // ........................................................................

    lcd_clear();

    ce_l;            // ��� �������� 0xFB = 11111011
#endif //(1==__TMS320__)
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   :  Lcd_clear
 * ��������              :  ������� �������. ����� ���������� ��������� LcdUpdate
 * ��������(�)           :  ���
 * ������������ �������� :  ���
 */
/* ************************************************************************** */
void Lcd_clear ( void ) {
    memset ( LcdCache,  0x00,  LCD_CACHE_SIZE );
    
    // ����� ���������� ������ � ������������ ��������
    LoWaterMark = 0;
    HiWaterMark = LCD_CACHE_SIZE - 1;

    // ��������� ����� ��������� ����
    UpdateLcd = TRUE;
}
/* ************************************************************************** */



/* ************************************************************************** */
/*
 * ���                   : Lcd_update
 * ��������              : �������� ��� � ��� �������
 * ��������(�)           : ���
 * ������������ �������� : ���
 */
/* ************************************************************************** */
void Lcd_update (void) {
    int i;

    if ( LoWaterMark < 0 )
        LoWaterMark = 0;
    else if ( LoWaterMark >= LCD_CACHE_SIZE )
        LoWaterMark = LCD_CACHE_SIZE - 1;

    if ( HiWaterMark < 0 )
        HiWaterMark = 0;
    else
    	if ( HiWaterMark >= LCD_CACHE_SIZE )
    		HiWaterMark = LCD_CACHE_SIZE - 1;

    #ifdef CHINA_LCD  // �������� ��� ���������� �� �� ������������� ������������

        byte x,y;

        // 102 x 64 - ������ �������������� ���������� ������ ���������� ��, ��� ���
        // ������ ������ ������������ �� ������� �� ������� ����� �� 3 �������.
        // ������� ������� �������� ���� - ������� � ������ ������ y+1, � �����
        // ������� ����� (����� ���� ���� �������, �������� � ������ ������)
                
        x = LoWaterMark % LCD_X_RES;      // ������������� ��������� ����� x
        LcdSend( 0x80 | x, LCD_CMD );     // ������������ ������ ������� LoWaterMark
        
        y = LoWaterMark / LCD_X_RES + 1;  // ������������� ��������� ����� y+1
        LcdSend( 0x40 | y, LCD_CMD );     // ������������ ������ ������� LoWaterMark

        for ( i = LoWaterMark; i <= HiWaterMark; i++ )
        {
            // �������� ������ � ����� �������
            LcdSend( LcdCache[i], LCD_DATA );
            
            x++;                 // ������ ������������ ���������� x, ����� ������� ������� �� ����� ������
            if (x >= LCD_X_RES)  // ���� ����� �� ������, �� ��������� �� ��������� ������ (x=0; y++)
            {
                // ����� ������, ����� ����� ��������� ������ ����� �������������� ������,
                // �������� ���� ��������� ��������� �����, ����� ��� �������� :)
                x=0;                
                LcdSend( 0x80, LCD_CMD );
                y++;
                LcdSend( 0x40 | y, LCD_CMD );
            }
        }

        LcdSend( 0x21, LCD_CMD );    // �������� ����������� ����� ������
        LcdSend( 0x45, LCD_CMD );    // �������� �������� �� 5 �������� ����� (������������� ������� �������, �������� � ����������)
        LcdSend( 0x20, LCD_CMD );    // �������� ����������� ����� ������ � �������������� ���������

    #else  // �������� ��� ������������� �������

        // ������������� ��������� ����� � ������������ � LoWaterMark
        LcdSend( 0x80 | ( LoWaterMark % LCD_X_RES ), LCD_CMD );
        LcdSend( 0x40 | ( LoWaterMark / LCD_X_RES ), LCD_CMD );

        // ��������� ����������� ����� ������ �������
        for ( i = LoWaterMark; i <= HiWaterMark; i++ ) {
            // ��� ������������� ������� �� ����� ������� �� ������� � ������,
            // ����� ������ ��������������� �������� ������
            LcdSend( LcdCache[i], LCD_DATA );
        }

    #endif

    // ����� ���������� ������ � �������
    LoWaterMark = LCD_CACHE_SIZE - 1;
    HiWaterMark = 0;

    // ����� ����� ��������� ����
    UpdateLcd = FALSE;
}
/* ************************************************************************** */


#if (1==__AVR__)
#endif //(1==__AVR__)
#if (1==__TMS320__)
#endif //(1==__TMS320__)
/* ************************************************************************** */
/*
 * ���                   : LcdSend
 * ��������              : ���������� ������ � ���������� �������
 * ��������(�)           : data -> ������ ��� ��������
 *                         cd   -> ������� ��� ������ (������ enum � n5110.h)
 * ������������ �������� : ���
 */
/* ************************************************************************** */
static void LcdSend ( byte data, LcdCmdData cd ) {
#if (1==__AVR__)
    // �������� ���������� ������� (������ ������� ��������)
    LCD_PORT &= ~( _BV( LCD_CE_PIN ) );

    if ( cd == LCD_DATA ) {
        LCD_PORT |= _BV( LCD_DC_PIN );
    } else {
        LCD_PORT &= ~( _BV( LCD_DC_PIN ) );
    }

    // �������� ������ � ���������� �������
    SPDR = data;

    // ���� ��������� ��������
    while ( (SPSR & 0x80) != 0x80 );

    // ��������� ���������� �������
    LCD_PORT |= _BV( LCD_CE_PIN );
#endif //(1==__AVR__)

#if (1==__TMS320__)
    // �������� ���������� ������� (������ ������� ��������)
    ce_l; // ��� �������� (3) 0, �������� 0xFB = 11111011

    if ( cd == LCD_DATA ) {
    	write_data(data);
    } else {
    	write_com(data);
    }

    // ��������� ���������� �������
    ce_h; // ��� �������� (3) 0, �������� 0xFB = 11111011
#endif //(1==__TMS320__)
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   : LcdContrast
 * ��������              : ������������� ������������� �������
 * ��������(�)           : �������� -> �������� �� 0x00 � 0x7F
 * ������������ �������� : ���
 */
/* ************************************************************************** */
void LcdContrast ( byte contrast ) {
    LcdSend( 0x21, LCD_CMD );              // ����������� ����� ������
    LcdSend( 0x80 | contrast, LCD_CMD );   // ��������� ������ �������������
    LcdSend( 0x20, LCD_CMD );              // ����������� ����� ������, �������������� ���������
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   : Delay
 * ��������              : ��������������� �������� ��� ��������� ������������� LCD
 * ��������(�)           : ���
 * ������������ �������� : ���
 */
/* ************************************************************************** */
static void Delay_lcd5510 ( void ) {
    int i;
    for ( i = -32000; i < 32000; i++ ) { };
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   : LcdGotoXY
 * ��������              : ������������� ������ � ������� x,y ������������ ������������ ������� ������
 * ��������(�)           : x,y -> ���������� ����� ������� �������. ��������: 0,0 .. 13,5
 * ������������ �������� : ������ ������������ �������� � n5110.h
 */
/* ************************************************************************** */
byte LcdGotoXY ( byte x, byte y ) {
    // �������� ������
    if ( x > 13 || y > 5 ) return OUT_OF_BORDER;

    //  ���������� ���������. ��������� ��� ����� � �������� 504 ����
    LcdCacheIdx = x * 6 + y * 84;

    return OK;
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   : LcdChr
 * ��������              : ������� ������ � ������� ������� �������, ����� �������������� ��������� �������
 * ��������(�)           : size -> ������ ������. ������ enum � n5110.h
 *                         ch   -> ������ ��� ������
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 */
/* ************************************************************************** */
byte LcdChr ( LcdFontSize size, byte ch ) {
    byte i, c;
    byte b1, b2;
    int  tmpIdx;

    if ( LcdCacheIdx < LoWaterMark ) {
        // ��������� ������ �������
        LoWaterMark = LcdCacheIdx;
    }

    if ( (ch >= 0x20) && (ch <= 0x7F) ) {
        // �������� � ������� ��� �������� ASCII[0x20-0x7F]
        ch -= 32;
    } else if ( ch >= 0xC0 ) {
        // �������� � ������� ��� �������� CP1251[0xC0-0xFF]
        ch -= 96;
    } else {
        // ��������� ���������� (�� ������ ��� � ������� ��� �������� ������)
        ch = 95;
    }

    if ( size == FONT_1X ) {
        for ( i = 0; i < 5; i++ ) {
            // �������� ��� ������� �� ������� � ���
            LcdCache[LcdCacheIdx++] = pgm_read_byte( &(FontLookup[ch][i]) ) << 1;
        }
    } else if ( size == FONT_2X ) {
        tmpIdx = LcdCacheIdx - 84;

        if ( tmpIdx < LoWaterMark ) {
            LoWaterMark = tmpIdx;
        }

        if ( tmpIdx < 0 )  return OUT_OF_BORDER;

        for ( i = 0; i < 5; i++ ) {
            // �������� ��� ������� �� ������� � ��������� ����������
            c = pgm_read_byte(&(FontLookup[ch][i])) << 1;
            // ����������� ��������
            // ������ �����
            b1 =  (c & 0x01) * 3;
            b1 |= (c & 0x02) * 6;
            b1 |= (c & 0x04) * 12;
            b1 |= (c & 0x08) * 24;

            c >>= 4;
            // ������ �����
            b2 =  (c & 0x01) * 3;
            b2 |= (c & 0x02) * 6;
            b2 |= (c & 0x04) * 12;
            b2 |= (c & 0x08) * 24;

            // �������� ��� ����� � ���
            LcdCache[tmpIdx++] = b1;
            LcdCache[tmpIdx++] = b1;
            LcdCache[tmpIdx + 82] = b2;
            LcdCache[tmpIdx + 83] = b2;
        }

        // ��������� x ���������� �������
        LcdCacheIdx = (LcdCacheIdx + 11) % LCD_CACHE_SIZE;
    }

    if ( LcdCacheIdx > HiWaterMark ) {
        // ��������� ������� �������
        HiWaterMark = LcdCacheIdx;
    }

    // �������������� ������ ����� ���������
    LcdCache[LcdCacheIdx] = 0x00;

    // ���� �������� ������� ��������� LCD_CACHE_SIZE - 1, ��������� � ������
    if(LcdCacheIdx == (LCD_CACHE_SIZE - 1) ) {
        LcdCacheIdx = 0;
        return OK_WITH_WRAP;
    }

    // ����� ������ �������������� ���������
    LcdCacheIdx++;
    return OK;
}


/* ************************************************************************** */
/*
 * ���                   : Lcd_print
 * ��������              : ��� ������� ������������� ��� ������ ������ �� ����������
 * ��������(�)           : size      -> ������ ������. ������ enum � n5110.h
 *                       : dataArray -> ������ ���������� ������ ������� ����� ����������
 *						 : x,y -> ����������
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 * ������                : LcdFStr(0, 0, FONT_1X,(unsigned char*)some_char);
 *                         LcdFStr(0, 0, FONT_1X, &name_of_string_as_array);
 */
/* ************************************************************************** */
byte Lcd_print ( byte x, byte y, LcdFontSize size, byte dataArray[] ) {
	LcdGotoXY(x,y);
    byte tmpIdx=0;
    byte response;
    while( dataArray[ tmpIdx ] != '\0' ) {
        // ������� ������
        response = LcdChr( size, dataArray[ tmpIdx ] );

        // �� ����� ����������� ���� ���������� OUT_OF_BORDER,
        // ������ ����� ���������� ������ �� ������ �������
        if( response == OUT_OF_BORDER)  return OUT_OF_BORDER;

        // ����������� ���������
        tmpIdx++;
    }
    return OK;
}
/* ************************************************************************** */



/* ************************************************************************** */
/*
 * ���                   : Lcd_prints
 * ��������              : ��� ������� ������������� ��� ������ ��������� ������
 * ��������(�)           : size    -> ������ ������. ������ enum � n5110.h
 *                         dataPtr -> ��������� �� ������ ������� ����� ����������
 *						 : x,y -> ����������
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 * ������                : LcdFStr(0, 0, FONT_1X, PSTR("Hello World"));
 *                         LcdFStr(0, 0, FONT_1X, &name_of_string_as_array);
 */
/* ************************************************************************** */
byte Lcd_prints ( byte x, byte y, LcdFontSize size, const byte *dataPtr ) {
    LcdGotoXY(x,y);

	byte c;
    byte response;

    //for ( c = pgm_read_byte(dataPtr); c; ++dataPtr, c=pgm_read_byte(dataPtr) )
    for ( c = pgm_read_byte(dataPtr); c; ++dataPtr, c=pgm_read_byte(dataPtr) )
    {
        // ������� ������
        response = LcdChr( size, c );
        if ( response == OUT_OF_BORDER ) return OUT_OF_BORDER;
    }

    return OK;
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   : Lcd_printf
 * ��������              : ��� ������� ������������� ��� ������ ����� � ��������� �������
 * ��������(�)           : size      -> ������ ������. ������ enum � n5110.h
 *                       : data -> �����
 *						 : accuracy -> ����� ������ ����� �������
 *						 : x,y -> ����������
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 * ������                : LcdFStr(0, 0, FONT_1X, float_var , 2);
 *                         LcdFStr(0, 0, FONT_1X, &name_of_string_as_array);
 */
/* ************************************************************************** */
void Lcd_printf ( byte x, byte y, LcdFontSize size, float data, int accuracy ) {
	Lcd_print ( x, y, size, (unsigned char*)gftoa(data, accuracy) );
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   : Lcd_pixel
 * ��������              : ���������� ������� �� ���������� ����������� (x,y)
 * ��������(�)           : x,y  -> ���������� ���������� �������
 *                         mode -> Off, On ��� Xor. ������ enum � n5110.h
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 */
/* ************************************************************************** */
byte Lcd_pixel ( byte x, byte y, LcdPixelMode mode ) {
    int  index;
    byte  offset;
    byte  data;

    // ������ �� ������ �� �������
    if ( x >= LCD_X_RES || y >= LCD_Y_RES) return OUT_OF_BORDER;

    // �������� ������� � ��������
    index = ( ( y / 8 ) * 84 ) + x;
    offset  = y - ( ( y / 8 ) * 8 );

    data = LcdCache[ index ];

    // ��������� �����

    // ����� PIXEL_OFF
    if ( mode == PIXEL_OFF ) {
        data &= ( ~( 0x01 << offset ) );
    }
    // ����� PIXEL_ON
    else if ( mode == PIXEL_ON ) {
        data |= ( 0x01 << offset );
    }
    // ����� PIXEL_XOR
    else if ( mode  == PIXEL_XOR ) {
        data ^= ( 0x01 << offset );
    }

    // ������������� ��������� �������� � ���
    LcdCache[ index ] = data;

    if ( index < LoWaterMark ) {
        // ��������� ������ �������
        LoWaterMark = index;
    }

    if ( index > HiWaterMark ) {
        // ��������� ������� �������
        HiWaterMark = index;
    }

    return OK;
}
/* ************************************************************************** */



/* ************************************************************************** */
/*
 * ���                   : Lcd_line
 * ��������              : ������ ����� ����� ����� ������� �� ������� (�������� ����������)
 * ��������(�)           : x1, y1  -> ���������� ���������� ������ �����
 *                         x2, y2  -> ���������� ���������� ����� �����
 *                         mode    -> Off, On ��� Xor. ������ enum � n5110.h
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 */
/* ************************************************************************** */
byte Lcd_line ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode ) {
    int dx, dy, stepx, stepy, fraction;
    byte response;

    // dy   y2 - y1
    // -- = -------
    // dx   x2 - x1

    dy = y2 - y1;
    dx = x2 - x1;

    // dy �������������
    if ( dy < 0 ) {
        dy    = -dy;
        stepy = -1;
    } else {
        stepy = 1;
    }

    // dx �������������
    if ( dx < 0 ) {
        dx    = -dx;
        stepx = -1;
    } else {
        stepx = 1;
    }

    dx <<= 1;
    dy <<= 1;

    // ������ ��������� �����
    response = Lcd_pixel( x1, y1, mode );
    if(response)
        return response;

    // ������ ��������� ����� �� �����
    if ( dx > dy ) {
        fraction = dy - ( dx >> 1);
        while ( x1 != x2 )
        {
            if ( fraction >= 0 )
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;

            response = Lcd_pixel( x1, y1, mode );
            if(response)
                return response;

        }
    } else {
        fraction = dx - ( dy >> 1);
        while ( y1 != y2 ) {
            if ( fraction >= 0 ) {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            response = Lcd_pixel( x1, y1, mode );
            if(response)  return response;
        }
    }

    // ��������� ����� ��������� ����
    UpdateLcd = TRUE;

    return OK;
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   : Lcd_circle
 * ��������              : ������ ���������� (�������� ����������)
 * ��������(�)           : x, y   -> ���������� ���������� ������
 *                         radius -> ������ ����������
 *                         mode   -> Off, On ��� Xor. ������ enum � n5110.h
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 */
/* ************************************************************************** */
byte Lcd_circle(byte x, byte y, byte radius, LcdPixelMode mode) {
    signed char xc = 0;
    signed char yc = 0;
    signed char p = 0;

    if ( x >= LCD_X_RES || y >= LCD_Y_RES) return OUT_OF_BORDER;

    yc = radius;
    p = 3 - (radius<<1);

    while (xc <= yc) {
        Lcd_pixel(x + xc, y + yc, mode);
        Lcd_pixel(x + xc, y - yc, mode);
        Lcd_pixel(x - xc, y + yc, mode);
        Lcd_pixel(x - xc, y - yc, mode);
        Lcd_pixel(x + yc, y + xc, mode);
        Lcd_pixel(x + yc, y - xc, mode);
        Lcd_pixel(x - yc, y + xc, mode);
        Lcd_pixel(x - yc, y - xc, mode);
        if (p < 0) p += (xc++ << 2) + 6;
            else p += ((xc++ - yc--)<<2) + 10;
    }

    // ��������� ����� ��������� ����
    UpdateLcd = TRUE;
    return OK;
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   : Lcd_rect  (rectangle)
 * ��������              : ������ ���� ����������� �������������
 * ��������(�)           : baseX  -> ���������� ���������� x (������ ����� ����)
 *                         baseY  -> ���������� ���������� y (������ ����� ����)
 *                         height -> ������ (� ��������)
 *                         width  -> ������ (� ��������)
 *                         mode   -> Off, On ��� Xor. ������ enum � n5110.h
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 */
/* ************************************************************************** */
byte Lcd_rect ( byte baseX, byte baseY, byte height, byte width, LcdPixelMode mode )
{
    byte tmpIdxX,tmpIdxY,tmp;
    byte response;

    // �������� ������
    if ( ( baseX >= LCD_X_RES) || ( baseY >= LCD_Y_RES) ) return OUT_OF_BORDER;

    if ( height > baseY )
        tmp = 0;
    else
        tmp = baseY - height + 1;

    // ��������� �����
    for ( tmpIdxY = tmp; tmpIdxY <= baseY; tmpIdxY++ )
    {
        for ( tmpIdxX = baseX; tmpIdxX < (baseX + width); tmpIdxX++ )
        {
            response = Lcd_pixel( tmpIdxX, tmpIdxY, mode );
            if(response)
                return response;

        }
    }

    // ��������� ����� ��������� ����
    UpdateLcd = TRUE;
    return OK;
}
/* ************************************************************************** */



/* ************************************************************************** */
/*
 * ���                   : Lcd_rect_empty
 * ��������              : ������ ������������� �������������
 * ��������(�)           : x1    -> ���������� ���������� x ������ �������� ����
 *                         y1    -> ���������� ���������� y ������ �������� ����
 *                         x2    -> ���������� ���������� x ������� ������� ����
 *                         y2    -> ���������� ���������� y ������� ������� ����
 *                         mode  -> Off, On ��� Xor. ������ enum � n5110.h
 * ������������ �������� : ������ ������������ �������� � n5110lcd.h
 */
/* ************************************************************************** */
byte Lcd_rect_empty ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode ) {
    byte tmpIdx;

    // �������� ������
    if ( ( x1 >= LCD_X_RES) ||  ( x2 >= LCD_X_RES) || ( y1 >= LCD_Y_RES) || ( y2 >= LCD_Y_RES) )
        return OUT_OF_BORDER;

    if ( ( x2 > x1 ) && ( y2 > y1 ) )     {
        // ������ �������������� �����
        for ( tmpIdx = x1; tmpIdx <= x2; tmpIdx++ ) {
            Lcd_pixel( tmpIdx, y1, mode );
            Lcd_pixel( tmpIdx, y2, mode );
        }

        // ������ ������������ �����
        for ( tmpIdx = y1; tmpIdx <= y2; tmpIdx++ ) {
            Lcd_pixel( x1, tmpIdx, mode );
            Lcd_pixel( x2, tmpIdx, mode );
        }

        // ��������� ����� ��������� ����
        UpdateLcd = TRUE;
    }
    return OK;
}
/* ************************************************************************** */


/* ************************************************************************** */
//������ � float
/* ************************************************************************** */
int gpow ( int n, int power ) {
	int res = 1;
	while(power--) res *= n;
	return res;
}
/* ************************************************************************** */


/* ************************************************************************** */
/*
 * ���                   :  gftoa
 * ��������              :  ��������� float � string
 * ��������(�)           :  n - �����, power -  ����� ������ ����� �������
 * ������������ �������� :  string
 */
/* ************************************************************************** */
char *gftoa ( float f, int dec ) {
	static char buf[16];
	char *p = buf + 15;
	int i = f * gpow(10, dec);
	int sign = i < 0 ? -1 : 1;

	i *= sign;
	do {
		*--p = '0' + (i % 10);
		i /= 10;
		if (--dec == 0) *--p = '.';
	}
	while (i != 0);

	if (dec > 0) {
		while (dec-- > 0)
		*--p = '0';
		*--p = '.';
	}

	if (*p == '.') *--p = '0';
	if (sign < 0) *--p = '-';

	return p;
}
/* ************************************************************************** */

#endif //(1==__USE_LCD_5110__)

