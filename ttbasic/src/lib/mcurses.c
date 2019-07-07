/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses.c - mcurses lib
 *
 * Copyright (c) 2011-2015 Frank Meyer - frank(at)fli4l.de
 *
 * Revision History:
 * V1.0 2015 xx xx Frank Meyer, original version
 * V1.1 2017 01 13 ChrisMicro, addepted as Arduino library, MCU specific functions removed
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

//
// 修正 2018/01/30 キーコードの変更（全角文字シフトJIS対応のため）
// 修正 2018/02/14 Arduino(AVR)用SRAM利用消費軽減対応
// 修正 2019/06/05 未実装関数の呼び出しを削除
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVR__
	#include <avr/pgmspace.h>
#else
	#define PROGMEM
	#define PSTR(x)                                 (x)
	#define pgm_read_byte(s)                        (*s)
#endif 

#include "mcurses.h"

#define SEQ_CSI                                 PSTR("\033[")                   // code introducer
#define SEQ_CLEAR                               PSTR("\033[2J")                 // clear screen
#define SEQ_CLRTOBOT                            PSTR("\033[J")                  // clear to bottom
#define SEQ_CLRTOEOL                            PSTR("\033[K")                  // clear to end of line
#define SEQ_DELCH                               PSTR("\033[P")                  // delete character
#define SEQ_NEXTLINE                            PSTR("\033E")                   // goto next line (scroll up at end of scrolling region)
#define SEQ_INSERTLINE                          PSTR("\033[L")                  // insert line
#define SEQ_DELETELINE                          PSTR("\033[M")                  // delete line
#define SEQ_ATTRSET                             PSTR("\033[0")                  // set attributes, e.g. "\033[0;7;1m"
#define SEQ_ATTRSET_REVERSE                     PSTR(";7")                      // reverse
#define SEQ_ATTRSET_UNDERLINE                   PSTR(";4")                      // underline
#define SEQ_ATTRSET_BLINK                       PSTR(";5")                      // blink
#define SEQ_ATTRSET_BOLD                        PSTR(";1")                      // bold
#define SEQ_ATTRSET_DIM                         PSTR(";2")                      // dim
#define SEQ_ATTRSET_FCOLOR                      PSTR(";3")                      // forground color
#define SEQ_ATTRSET_BCOLOR                      PSTR(";4")                      // background color
#define SEQ_INSERT_MODE                         PSTR("\033[4h")                 // set insert mode
#define SEQ_REPLACE_MODE                        PSTR("\033[4l")                 // set replace mode
#define SEQ_RESET_SCRREG                        PSTR("\033[r")                  // reset scrolling region
#define SEQ_LOAD_G1                             PSTR("\033)0")                  // load G1 character set
#define SEQ_CURSOR_VIS                          PSTR("\033[?25")                // set cursor visible/not visible

static uint_fast8_t                             mcurses_scrl_start = 0;         // start of scrolling region, default is 0
static uint_fast8_t                             mcurses_scrl_end = LINES - 1;   // end of scrolling region, default is last line

uint_fast8_t                                    mcurses_is_up = 0;              // flag: mcurses is up
uint_fast8_t                                    mcurses_cury = 0xff;            // current y position of cursor, public (getyx())
uint_fast8_t                                    mcurses_curx = 0xff;            // current x position of cursor, public (getyx())

static void                                     mcurses_puts_P (const char *);

char (*FunctionPointer_getchar)(void);
void  (*FunctionPointer_putchar)(uint_fast8_t ch);

void setFunction_getchar(char (*functionPoitner)(void)) {
	FunctionPointer_getchar = functionPoitner;
}

void setFunction_putchar(void (*functionPoitner)(uint8_t ch)) {
	FunctionPointer_putchar = functionPoitner;
}

static void mcurses_phyio_putc(uint_fast8_t ch) {
	if(FunctionPointer_putchar!=0)	FunctionPointer_putchar(ch);
}

static uint_fast8_t mcurses_phyio_getc(void) {
	if(FunctionPointer_getchar!=0)	return FunctionPointer_getchar();
	else return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a character (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mcurses_putc(uint_fast8_t ch) {
    mcurses_phyio_putc (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a string from flash (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mcurses_puts_P(const char * str) {
    uint_fast8_t ch;
    while ((ch = pgm_read_byte(str)) != '\0')  {
        mcurses_putc(ch);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a 3/2/1 digit integer number (raw)
 *
 * Here we don't want to use sprintf (too big on AVR/Z80) or itoa (not available on Z80)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mcurses_puti(uint_fast8_t i) {
    uint_fast8_t ii;
    if (i >= 10) {
        if (i >= 100) {
            ii = i / 100;
            mcurses_putc (ii + '0');
            i -= 100 * ii;
        }

        ii = i / 10;
        mcurses_putc (ii + '0');
        i -= 10 * ii;
    }

    mcurses_putc (i + '0');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: addch or insch a character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CHARSET_G0      0
#define CHARSET_G1      1

static void mcurses_addch_or_insch(uint_fast8_t ch, uint_fast8_t insert) {
//    static uint_fast8_t  charset = 0xff;
    static uint_fast8_t  insert_mode = FALSE;

    if (insert) {
        if (! insert_mode) {
            mcurses_puts_P (SEQ_INSERT_MODE);
            insert_mode = TRUE;
        }
    } else  {
        if (insert_mode) {
            mcurses_puts_P(SEQ_REPLACE_MODE);
            insert_mode = FALSE;
        }
    }

    mcurses_putc (ch);
    mcurses_curx++;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: set scrolling region (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mysetscrreg(uint_fast8_t top, uint_fast8_t bottom) {
    if (top == bottom) {
        mcurses_puts_P(SEQ_RESET_SCRREG);                                      // reset scrolling region
    } else {
        mcurses_puts_P(SEQ_CSI);
        mcurses_puti(top + 1);
        mcurses_putc(';');
        mcurses_puti(bottom + 1);
        mcurses_putc('r');
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * move cursor (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mymove(uint_fast8_t y, uint_fast8_t x) {
    mcurses_puts_P(SEQ_CSI);
    mcurses_puti(y + 1);
    mcurses_putc(';');
    mcurses_puti(x + 1);
    mcurses_putc('H');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: initialize
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void initscr(void) {
    mcurses_puts_P(SEQ_LOAD_G1);                                               // load graphic charset into G1
    attrset(A_NORMAL);
    clear();
    move(0, 0);
    mcurses_is_up = 1;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void addch(uint_fast8_t ch) {
    mcurses_addch_or_insch (ch, FALSE);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void addstr(const char * str) {
    while (*str) {
        mcurses_addch_or_insch(*str++, FALSE);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void addstr_P(const char * str) {
    uint_fast8_t ch;

    while ((ch = pgm_read_byte(str)) != '\0')  {
        mcurses_addch_or_insch (ch, FALSE);
        str++;
    }
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set attribute(s)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void attrset(uint_fast16_t attr) {
    static uint_fast8_t mcurses_attr = 0xff;                    // current attributes
    uint_fast8_t        idx;

    if (attr != mcurses_attr)  {
        mcurses_puts_P (SEQ_ATTRSET);

        idx = (attr & F_COLOR) >> 8;

        if (idx >= 1 && idx <= 8)  {
            mcurses_puts_P(SEQ_ATTRSET_FCOLOR);
            mcurses_putc(idx - 1 + '0');
        }

        idx = (attr & B_COLOR) >> 12;

        if (idx >= 1 && idx <= 8)  {
            mcurses_puts_P(SEQ_ATTRSET_BCOLOR);
            mcurses_putc(idx - 1 + '0');
        }

        if (attr & A_REVERSE)  {
            mcurses_puts_P(SEQ_ATTRSET_REVERSE);
        }
        if (attr & A_UNDERLINE)  {
            mcurses_puts_P(SEQ_ATTRSET_UNDERLINE);
        }
        if (attr & A_BLINK) {
            mcurses_puts_P(SEQ_ATTRSET_BLINK);
        }
        if (attr & A_BOLD) {
            mcurses_puts_P(SEQ_ATTRSET_BOLD);
        }
        if (attr & A_DIM)  {
            mcurses_puts_P(SEQ_ATTRSET_DIM);
        }
        mcurses_putc('m');
        mcurses_attr = attr;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: move cursor
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void move(uint_fast8_t y, uint_fast8_t x) {
    if (mcurses_cury != y || mcurses_curx != x)  {
        mcurses_cury = y;
        mcurses_curx = x;
        mymove(y, x);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void deleteln(void) {
    mysetscrreg(mcurses_scrl_start, mcurses_scrl_end);                         // set scrolling region
    mymove(mcurses_cury, 0);                                                   // goto to current line
    mcurses_puts_P(SEQ_DELETELINE);                                            // delete line
    mysetscrreg (0, 0);                                                        // reset scrolling region
    mymove(mcurses_cury, mcurses_curx);                                        // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void insertln(void) {
    mysetscrreg (mcurses_cury, mcurses_scrl_end);                              // set scrolling region
    mymove(mcurses_cury, 0);                                                   // goto to current line
    mcurses_puts_P(SEQ_INSERTLINE);                                            // insert line
    mysetscrreg(0, 0);                                                         // reset scrolling region
    mymove(mcurses_cury, mcurses_curx);                                        // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: scroll
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void scroll(void) {
    mysetscrreg (mcurses_scrl_start, mcurses_scrl_end);                        // set scrolling region
    mymove(mcurses_scrl_end, 0);                                               // goto to last line of scrolling region
    mcurses_puts_P (SEQ_NEXTLINE);                                             // next line
    mysetscrreg(0, 0);                                                         // reset scrolling region
    mymove(mcurses_cury, mcurses_curx);                                        // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void clear(void) {
    mcurses_puts_P(SEQ_CLEAR);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to bottom of screen
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void clrtobot(void) {
    mcurses_puts_P(SEQ_CLRTOBOT);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to end of line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void clrtoeol(void) {
    mcurses_puts_P(SEQ_CLRTOEOL);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete character at cursor position
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void delch(void) {
    mcurses_puts_P(SEQ_DELCH);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void insch(uint_fast8_t ch) {
    mcurses_addch_or_insch(ch, TRUE);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set scrolling region
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void setscrreg(uint_fast8_t t, uint_fast8_t b) {
    mcurses_scrl_start = t;
    mcurses_scrl_end = b;
}

void curs_set(uint_fast8_t visibility) {
    mcurses_puts_P(SEQ_CURSOR_VIS);
    if (visibility == 0)  {
        mcurses_putc('l');
    } else {
        mcurses_putc('h');
    }
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read key
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#define MAX_KEYS    (23)
#define KW(k,s) const char k[] PROGMEM=s  // キーワード定義マクロ
KW(k00,"B");  KW(k01,"A");  KW(k02,"D");  KW(k03,"C");  KW(k04,"1~"); 
KW(k05,"3~"); KW(k06,"2~"); KW(k07,"6~"); KW(k08,"5~"); KW(k09,"4~");
KW(k10,"Z");  KW(k11,"11~");KW(k12,"12~");KW(k13,"13~");KW(k14,"14~");
KW(k15,"15~");KW(k16,"17~");KW(k17,"18~");KW(k18,"19~");KW(k19,"20~");
KW(k20,"21~");KW(k21,"23~");KW(k22,"24~");

const char* const function_keys[] PROGMEM = {
  k00,k01,k02,k03,k04,k05,k06,k07,k08,k09,
  k10,k11,k12,k13,k14,k15,k16,k17,k18,k19,
  k20,k21,k22,
};

const char const function_keys_code[] PROGMEM = {
  KEY_DOWN   ,                // Down arrow key
  KEY_UP     ,                // Up arrow key
  KEY_LEFT   ,                // Left arrow key
  KEY_RIGHT  ,                // Right arrow key
  KEY_HOME   ,               // Home key
  KEY_DC     ,               // Delete character key
  KEY_IC     ,               // Ins char/toggle ins mode key
  KEY_NPAGE  ,               // Next-page key
  KEY_PPAGE  ,               // Previous-page key
  KEY_END    ,               // End key
  KEY_BTAB   ,               // Back tab key
  KEY_F1     ,               // Function key F1
  KEY_F2     ,               // Function key F2
  KEY_F3     ,               // Function key F3
  KEY_F4     ,               // Function key F4
  KEY_F5     ,               // Function key F5
  KEY_F6     ,               // Function key F6
  KEY_F7     ,               // Function key F7
  KEY_F8     ,               // Function key F8
  KEY_F9     ,               // Function key F9
  KEY_F10    ,               // Function key F10
  KEY_F11    ,               // Function key F11
  KEY_F12    ,               // Function key F12
};

uint_fast8_t getch(void) {
    char    buf[4];
    uint_fast8_t ch;
    uint_fast8_t idx;

    ch = mcurses_phyio_getc ();

    if (ch == 0x7F) {                                                           // BACKSPACE on VT200 sends DEL char
          ch = KEY_BACKSPACE;                                                   // map it to '\b'
    } else if (ch == '\033') {                                                  // ESCAPE   
        while ((ch = mcurses_phyio_getc ()) == ERR)  {
            ;
        }

        if (ch == '\033') {                                                     // 2 x ESCAPE
            return KEY_ESCAPE;
        } else if (ch == '[') {
            for (idx = 0; idx < 3; idx++) {
                while ((ch = mcurses_phyio_getc ()) == ERR)  {
                    ;
                }
                buf[idx] = ch;
                if ((ch >= 'A' && ch <= 'Z') || ch == '~')  {
                    idx++;
                    break;
                }
            }

            buf[idx] = '\0';

            for (idx = 0; idx < MAX_KEYS; idx++)  {
               if (!strcmp_P(buf,(char*)pgm_read_word(function_keys+idx)))   {
                  ch = pgm_read_byte(function_keys_code+idx);
                  break;
                }
            }

            if (idx == MAX_KEYS)  {
                ch = ERR;
            }
        }  else  {
            ch = ERR;
        }
    }
    return ch;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: endwin
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void endwin(void) {
    move(LINES - 1, 0);                                                        // move cursor to last line
    clrtoeol();                                                                // clear this line
    mcurses_putc('\017');                                                      // switch to G0 set
    curs_set(TRUE);                                                            // show cursor
    mcurses_puts_P(SEQ_REPLACE_MODE);                                          // reset insert mode
    mcurses_is_up = 0;
}
