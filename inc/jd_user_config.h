#include "CodalDmesg.h"

#define JD_DMESG_BUFFER_SIZE 0
#define JD_LOG               DMESG
#define JD_WR_OVERHEAD       45
#define JD_CLIENT            1
#define JD_RAW_FRAME         1

// this is min. erase size
#define JD_FLASH_PAGE_SIZE 4096  // TODO

// probably not so useful on brains...
#define JD_CONFIG_WATCHDOG 0

// #define JD_USB_BRIDGE 1

#define JD_SEND_FRAME_SIZE 1024

#define JD_CONFIG_STATUS 0

#define NUM_DISPLAY_COLS 5
#define NUM_DISPLAY_ROWS 5

#define JD_INSTANCE_NAME 1

#define JD_RX_QUEUE_SIZE 1024

void jdhw_poke(void);
#define JD_WAKE_MAIN() jdhw_poke()

#define JD_GC_KB 4