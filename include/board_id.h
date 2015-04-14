#ifndef __BOARD_ID_H
#define __BOARD_ID_H

#define OPTIMUS_BOARD_ID	0
#define SIDESWIPE_BOARD_ID	1
#define SPACECAST_BOARD_ID	2

#define OPTIMUS_BOARD_ID_OTP	(1 << OPTIMUS_BOARD_ID)
#define SIDESWIPE_BOARD_ID_OTP	(1 << SIDESWIPE_BOARD_ID)
#define SPACECAST_BOARD_ID_OTP	(1 << SPACECAST_BOARD_ID)

int get_board_id(void);
int get_board_id_gpio(void);

#endif /* __BOARD_ID_H */
