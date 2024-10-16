// Based on https://github.com/idispatch/raster-fonts/blob/master/font-6x8.c
//#include "font.h"

#if 0
#ifndef FONT_H_
#define FONT_H_

typedef struct {
    unsigned char_width;
    unsigned char_height;
    const char * font_name;
    unsigned char first_char;
    unsigned char last_char;
    unsigned char * font_bitmap;
} font_t;

extern const font_t console_fonts[];

#endif /* FONT_H_ */
#endif
/*
#ifdef FONT_FULL_ASCII
	#define FONT_NUMCHARS 256
#else
	#define FONT_NUMCHARS 128
#endif
*/
#define console_font_6x5_char_width 6
#define console_font_6x5_char_height 8
#define console_font_6x5_first_char 31
#define console_font_6x5_last_char 127
#define FONT_NUMCHARS (1 + console_font_6x5_last_char - console_font_6x5_first_char)

//unsigned char console_font_6x5[] = {
const byte console_font_6x5_font_bitmap[FONT_NUMCHARS][8] = {
  {
    /*
     * code=31, hex=0x1F, ascii="^_"
     */
    0x00,  /* 011111 */
    0x00,  /* 011011 */
    0x00,  /* 010101 */
    0x00,  /* 011011 */
    0x00,  /* 011111 */
  },
  {
    /*
     * code=32, hex=0x20, ascii=" "
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=33, hex=0x21, ascii="!"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x38,  /* 001110 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=34, hex=0x22, ascii="""
     */
    0x6C,  /* 011011 */
    0x48,  /* 010010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=35, hex=0x23, ascii="#"
     */
    0x28,  /* 001010 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
  },
  {
    /*
     * code=36, hex=0x24, ascii="$"
     */
    0x20,  /* 001000 */
    0x38,  /* 001110 */
    0x30,  /* 001100 */
    0x70,  /* 011100 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=37, hex=0x25, ascii="%"
     */
    0x64,  /* 011001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x4C,  /* 010011 */
  },
  {
    /*
     * code=38, hex=0x26, ascii="&"
     */
    0x20,  /* 001000 */
    0x50,  /* 010100 */
    0x54,  /* 011101 */
    0x48,  /* 010010 */
    0x34,  /* 001101 */
  },
  {
    /*
     * code=39, hex=0x27, ascii="'"
     */
    0x30,  /* 001100 */
    0x20,  /* 001000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=40, hex=0x28, ascii="("
     */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=41, hex=0x29, ascii=")"
     */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
  },
  {
    /*
     * code=42, hex=0x2A, ascii="*"
     */
    0x28,  /* 001010 */
    0x38,  /* 001110 */
    0x7C,  /* 011111 */
    0x38,  /* 001110 */
    0x28,  /* 001010 */
  },
  {
    /*
     * code=43, hex=0x2B, ascii="+"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=44, hex=0x2C, ascii=","
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 001000 */
    0x20,  /* 010000 */
  },
  {
    /*
     * code=45, hex=0x2D, ascii="-"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=46, hex=0x2E, ascii="."
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x30,  /* 010000 */
  },
  {
    /*
     * code=47, hex=0x2F, ascii="/"
     */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
  },
  {
    /*
     * code=48, hex=0x30, ascii="0"
     */
    0x38,  /* 001110 */
    0x4C,  /* 010011 */
    0x54,  /* 010101 */
    0x64,  /* 011001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=49, hex=0x31, ascii="1"
     */
    0x10,  /* 000100 */
    0x30,  /* 001100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=50, hex=0x32, ascii="2"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x18,  /* 000110 */
    0x40,  /* 011000 */
    0x7C,  /* 011111 */
  },
  {
    /*
     * code=51, hex=0x33, ascii="3"
     */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=52, hex=0x34, ascii="4"
     */
    0x08,  /* 000010 */
    0x18,  /* 000110 */
    0x48,  /* 011010 */
    0x7C,  /* 011111 */
    0x08,  /* 000010 */
  },
  {
    /*
     * code=53, hex=0x35, ascii="5"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=54, hex=0x36, ascii="6"
     */
    0x18,  /* 001110 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=55, hex=0x37, ascii="7"
     */
    0x7C,  /* 011111 */
    0x04,  /* 000001 */
    0x10,  /* 000110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
  },
  {
    /*
     * code=56, hex=0x38, ascii="8"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=57, hex=0x39, ascii="9"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */
    0x30,  /* 001110 */
  },
  {
    /*
     * code=58, hex=0x3A, ascii=":"
     */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=59, hex=0x3B, ascii=";"
     */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x00,  /* 000000 */
    0x30,  /* 001100 */
    0x20,  /* 001000 */
  },
  {
    /*
     * code=60, hex=0x3C, ascii="<"
     */
    0x10,  /* 000100 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=61, hex=0x3D, ascii="="
     */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=62, hex=0x3E, ascii=">"
     */
    0x10,  /* 000100 */
    0x08,  /* 000010 */
    0x04,  /* 000001 */
    0x08,  /* 000010 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=63, hex=0x3F, ascii="?"
     */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x10,  /* 000110 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=64, hex=0x40, ascii="@"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x5C,  /* 010111 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=65, hex=0x41, ascii="A"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
  },
  {
    /*
     * code=66, hex=0x42, ascii="B"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
  },
  {
    /*
     * code=67, hex=0x43, ascii="C"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x40,  /* 010000 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=68, hex=0x44, ascii="D"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
  },
  {
    /*
     * code=69, hex=0x45, ascii="E"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
  },
  {
    /*
     * code=70, hex=0x46, ascii="F"
     */
    0x7C,  /* 011111 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
  },
  {
    /*
     * code=71, hex=0x47, ascii="G"
     */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x5C,  /* 010111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
  },
  {
    /*
     * code=72, hex=0x48, ascii="H"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
  },
  {
    /*
     * code=73, hex=0x49, ascii="I"
     */
    0x38,  /* 001110 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=74, hex=0x4A, ascii="J"
     */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=75, hex=0x4B, ascii="K"
     */
    0x48,  /* 010011 */
    0x50,  /* 010100 */
    0x60,  /* 011000 */
    0x50,  /* 010100 */
    0x48,  /* 010011 */

  },
  {
    /*
     * code=76, hex=0x4C, ascii="L"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x7C,  /* 011111 */
  },
  {
    /*
     * code=77, hex=0x4D, ascii="M"
     */
    0x44,  /* 010001 */
    0x6C,  /* 011011 */
    0x54,  /* 010101 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
  },
  {
    /*
     * code=78, hex=0x4E, ascii="N"
     */
    0x44,  /* 010001 */
    0x64,  /* 011001 */
    0x54,  /* 010101 */
    0x4C,  /* 010011 */
    0x44,  /* 010001 */
  },
  {
    /*
     * code=79, hex=0x4F, ascii="O"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=80, hex=0x50, ascii="P"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
  },
  {
    /*
     * code=81, hex=0x51, ascii="Q"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x48,  /* 010010 */
    0x34,  /* 001101 */
  },
  {
    /*
     * code=82, hex=0x52, ascii="R"
     */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x48,  /* 010010 */
    0x44,  /* 010001 */
  },
  {
    /*
     * code=83, hex=0x53, ascii="S"
     */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=84, hex=0x54, ascii="T"
     */
    0x7C,  /* 011111 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=85, hex=0x55, ascii="U"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=86, hex=0x56, ascii="V"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=87, hex=0x57, ascii="W"
     */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x28,  /* 001010 */
  },
  {
    /*
     * code=88, hex=0x58, ascii="X"
     */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x28,  /* 001010 */
    0x44,  /* 010001 */
  },
  {
    /*
     * code=89, hex=0x59, ascii="Y"
     */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=90, hex=0x5A, ascii="Z"
     */
    0x78,  /* 011110 */
    0x10,  /* 000110 */
    0x20,  /* 001000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
  },
  {
    /*
     * code=91, hex=0x5B, ascii="["
     */
    0x38,  /* 001110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=92, hex=0x5C, ascii="\"
     */
    0x40,  /* 010000 */
    0x20,  /* 001000 */
    0x10,  /* 000100 */
    0x08,  /* 000010 */
    0x04,  /* 000001 */
  },
  {
    /*
     * code=93, hex=0x5D, ascii="]"
     */
    0x38,  /* 001110 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x08,  /* 000010 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=94, hex=0x5E, ascii="^"
     */
    0x10,  /* 000100 */
    0x28,  /* 001010 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=95, hex=0x5F, ascii="_"
     */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0xFC,  /* 111111 */
  },
  {
    /*
     * code=96, hex=0x60, ascii="`"
     */
    0x30,  /* 001100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=97, hex=0x61, ascii="a"
     */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
  },
  {
    /*
     * code=98, hex=0x62, ascii="b"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
  },
  {
    /*
     * code=99, hex=0x63, ascii="c"
     */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x44,  /* 010000 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=100, hex=0x64, ascii="d"
     */
    0x04,  /* 000001 */
    0x04,  /* 000001 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
  },
  {
    /*
     * code=101, hex=0x65, ascii="e"
     */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=102, hex=0x66, ascii="f"
     */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x78,  /* 011110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
  },
  {
    /*
     * code=103, hex=0x67, ascii="g"
     */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=104, hex=0x68, ascii="h"
     */
    0x40,  /* 010000 */
    0x40,  /* 010000 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
  },
  {
    /*
     * code=105, hex=0x69, ascii="i"
     */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
  },
  {
    /*
     * code=106, hex=0x6A, ascii="j"
     */
    0x08,  /* 000010 */
    0x00,  /* 000000 */
    0x18,  /* 000110 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
  },
  {
    /*
     * code=107, hex=0x6B, ascii="k"
     */
    0x40,  /* 010000 */
    0x50,  /* 010110 */
    0x60,  /* 011000 */
    0x50,  /* 010100 */
    0x48,  /* 010010 */
  },
  {
    /*
     * code=108, hex=0x6C, ascii="l"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x18,  /* 000110 */
  },
  {
    /*
     * code=109, hex=0x6D, ascii="m"
     */
    0x00,  /* 000000 */
    0x68,  /* 011010 */
    0x54,  /* 010101 */
    0x54,  /* 010101 */
    0x44,  /* 010001 */
  },
  {
    /*
     * code=110, hex=0x6E, ascii="n"
     */
    0x00,  /* 000000 */
    0x70,  /* 011100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
  },
  {
    /*
     * code=111, hex=0x6F, ascii="o"
     */
    0x00,  /* 000000 */
    0x38,  /* 001110 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=112, hex=0x70, ascii="p"
     */
    0x00,  /* 000000 */
    0x78,  /* 011110 */
    0x44,  /* 010001 */
    0x78,  /* 011110 */
    0x40,  /* 010000 */
  },
  {
    /*
     * code=113, hex=0x71, ascii="q"
     */
    0x00,  /* 000000 */
    0x3C,  /* 001111 */
    0x44,  /* 010001 */
    0x3C,  /* 001111 */
    0x04,  /* 000001 */
  },
  {
    /*
     * code=114, hex=0x72, ascii="r"
     */
    0x00,  /* 000000 */
    0x58,  /* 010110 */
    0x24,  /* 001001 */
    0x20,  /* 001000 */
    0x70,  /* 011100 */
  },
  {
    /*
     * code=115, hex=0x73, ascii="s"
     */
    0x38,  /* 001110 */
    0x40,  /* 010000 */
    0x38,  /* 001110 */
    0x04,  /* 000001 */
    0x38,  /* 001110 */
  },
  {
    /*
     * code=116, hex=0x74, ascii="t"
     */
    0x00,  /* 001000 */
    0x78,  /* 011110 */
    0x20,  /* 001000 */
    0x20,  /* 001000 */
    0x10,  /* 001000 */
  },
  {
    /*
     * code=117, hex=0x75, ascii="u"
     */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x58,  /* 010110 */
    0x28,  /* 001010 */
  },
  {
    /*
     * code=118, hex=0x76, ascii="v"
     */
    0x00,  /* 000000 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x28,  /* 001010 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=119, hex=0x77, ascii="w"
     */
    0x00,  /* 000000 */
    0x44,  /* 010001 */
    0x54,  /* 010101 */
    0x7C,  /* 011111 */
    0x28,  /* 001010 */
  },
  {
    /*
     * code=120, hex=0x78, ascii="x"
     */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x30,  /* 001100 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
  },
  {
    /*
     * code=121, hex=0x79, ascii="y"
     */
    0x00,  /* 000000 */
    0x48,  /* 010010 */
    0x48,  /* 010010 */
    0x38,  /* 001110 */
    0x60,  /* 011000 */
  },
  {
    /*
     * code=122, hex=0x7A, ascii="z"
     */
    0x78,  /* 011110 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
    0x40,  /* 010000 */
    0x78,  /* 011110 */
  },
  {
    /*
     * code=123, hex=0x7B, ascii="{"
     */
    0x18,  /* 000110 */
    0x20,  /* 001000 */
    0x60,  /* 011000 */
    0x20,  /* 001000 */
    0x18,  /* 000110 */
  },
  {
    /*
     * code=124, hex=0x7C, ascii="|"
     */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
    0x00,  /* 000000 */
    0x10,  /* 000100 */
    0x10,  /* 000100 */
  },
  {
    /*
     * code=125, hex=0x7D, ascii="}"
     */
    0x30,  /* 001100 */
    0x08,  /* 000010 */
    0x0C,  /* 000011 */
    0x08,  /* 000010 */
    0x30,  /* 001100 */
  },
  {
    /*
     * code=126, hex=0x7E, ascii="~"
     */
    0x28,  /* 001010 */
    0x50,  /* 010100 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
  },
  {
    /*
     * code=127, hex=0x7F, ascii="^?"
     */
    0x10,  /* 000100 */
    0x38,  /* 001110 */
    0x6C,  /* 011011 */
    0x44,  /* 010001 */
    0x44,  /* 010001 */
    0x7C,  /* 011111 */
    0x00,  /* 000000 */
    0x00,  /* 000000 */
  },
};

