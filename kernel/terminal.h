#ifndef TELNET_H
#define TELNET_H

#include <stddef.h>
#include <css.h>

int ModInitBsc(void);

#define ModAdd2703Device _Ma2703d
int ModAdd2703Device(struct css_schid schid, struct css_senseid *sensebuf);

#define ModAdd1403Device _Ma1403d
int ModAdd1403Device(struct css_schid schid, struct css_senseid *sensebuf);
#define ModInit1403 _Mi1403
int ModInit1403(void);

#define ModInit2703 _Mi2703
int ModInit2703(void);

#define X3270_CMD_SELECT 0x0B
#define X3270_CMD_SELECT_WRITE 0x4B

/* Write without carriage return */
#define X3270_CMD_WRITE_NOCR (CSS_CMD_WRITE)

/* Write with carriage return */
#define X3270_CMD_WRITE_CR 0x09

#define X3270_CMD_READ_BUFFER (CSS_CMD_READ)
#define X3270_CMD_READ_MOD 0x06
#define X3270_CMD_NOP (CSS_CMD_CONTROL)
#define X3270_CMD_WSF 0x11

#define X3270_CMD_SENSE 0x04
#define X3270_CMD_SENSE_ID 0xE4

#define X3270_CMD_TIC 0x08

enum x3270_order {
    X3270_ORDER_START_FIELD = 0x1D,
    X3270_ORDER_SET_BUFFER_ADDR = 0x11,
    X3270_ORDER_INSERT_CURSOR = 0x13,
    X3270_ORDER_ERASE_UNPROTECTED = 0x12
};

enum x3270_color {
    X3270_COLOR_WHITE = 0xF0,
    X3270_COLOR_BLUE = 0xF1,
    X3270_COLOR_RED = 0xF2,
    X3270_COLOR_PINK = 0xF3,
    X3270_COLOR_GREEN = 0xF4,
    X3270_COLOR_TURQUOISE = 0xF5,
    X3270_COLOR_YELLOW = 0xF6,
    X3270_COLOR_BLACK = 0xF7
};

/* Write control code */
enum x3270_wcc {
    X3270_WCC_PARITY = 0x01,
    X3270_WCC_START_PRINT = 0x10,
    X3270_WCC_SOUND_ALARM = 0x20,
    X3270_WCC_UNLOCK_INPUT = 0x40,
    X3270_WCC_RESET_MDT = 0x80
};

#define ModAdd3270Device _Ma3270
int ModAdd3270Device(struct css_schid schid, struct css_senseid *sensebuf);
#define ModInit3270 _Mi3270
int ModInit3270(void);

#endif
