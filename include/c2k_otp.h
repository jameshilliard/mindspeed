#ifndef _C2K_OTP_H
#define _C2K_OTP_H

#include <common.h>

int otp_read(u32 s_addr, u8 *read_data, int size) ;
int otp_write(u32 offset, u8 *prog_data, int size);

#endif
