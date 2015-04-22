#ifndef __BOARD_ID_H
#define __BOARD_ID_H

#define OPTIMUS_BOARD_ID	0
#define SIDESWIPE_BOARD_ID	1
#define SPACECAST_BOARD_ID	2

int get_board_id(void);
int get_board_id_gpio(void);

#endif /* __BOARD_ID_H */
