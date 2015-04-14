#ifndef _C2K_OTP_H
#define _C2K_OTP_H

#include <common.h>

/* OTP addresses holding the the board id */
#define OTP_OFFSET_BOARD_ID	(512 * 8) /* 32-bit / 4 bytes */

int otp_read(u32 s_addr, u8 *read_data, int size) ;
int otp_write(u32 offset, u8 *prog_data, int size);
int otp_lock(void);

#endif
