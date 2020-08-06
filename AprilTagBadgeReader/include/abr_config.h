#ifndef ABR_CONFIG_H
#define ABR_CONFIG_H

//#define SKIP_DISPLAY_IMAGES //comment this line to skip displaying images

#define SKIP_DEBUG_PRINT //comment this line to print debug statements

#ifndef SKIP_DEBUG_PRINT
    #define DEBUG_PRINTF(x) configPRINTF(x);
#else
    #define DEBUG_PRINTF(x)
#endif

#endif
