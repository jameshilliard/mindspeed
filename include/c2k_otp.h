#ifndef _C2K_OTP_H
#define _C2K_OTP_H

#include <common.h>

/*
 * Reads from the OTP.
 *
 * @offset:	The offset to read from, in bits.
 * @read_data:	The buffer to read data into.
 * @size:	The number of *bytes* to read.
 *
 * @return	0 if successful, non-zero otherwise
 */
int otp_read(u32 offset, u8 *read_data, int size);

/*
 * Writes to the OTP.
 *
 * Take care: although this method takes a u8 array for the
 * data being written, each element represents only one bit
 * in the OTP. If set to 1, the OTP is flashed at that bit.
 * If set to 0, it is not. It is unknown what happens if it
 * is set to a value other than 0 or 1.
 *
 * @offset:	The offset to write to, in bits.
 * @prog_data:	A *bit* array to write data from.
 * @size:	The number of *bits* to write.
 */
void otp_write(u32 offset, u8 *prog_data, int size);

#endif
