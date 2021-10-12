#ifndef IBM3270_H
#define IBM3270_H

#define IBM3270_CMD_SELECT 0x0B
#define IBM3270_CMD_SELECT_WRITE 0x4B

/* Write without carriage return */
#define IBM3270_CMD_WRITE_NOCR (CSS_CMD_WRITE)

/* Write with carriage return */
#define IBM3270_CMD_WRITE_CR 0x09

#define IBM3270_CMD_RDBUF (CSS_CMD_READ)
#define IBM3270_CMD_RMOD 0x06
#define IBM3270_CMD_NOP (CSS_CMD_CONTROL)
#define IBM3270_CMD_WSF 0x11

#define IBM3270_CMD_SENSE 0x04
#define IBM3270_CMD_SENSE_ID 0xE4

#define IBM3270_CMD_TIC 0x08

int ibm3270_init(void);

#endif