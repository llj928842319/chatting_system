#ifndef _COLOR_OUTPUT_H
#define _COLOR_OUTPUT_H

#include <stdio.h>
#include <time.h>


#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "scanbox.h"
#include "epolll.h"

#define COLOR_DEFAULT "\033[m"   //复位所有颜色
#define COLOR_RED_BG     "\033[41m"
#define COLOR_GREEN_BG   "\033[42m"
#define COLOR_YELLOW_BG   "\033[43m"
#define COLOR_BLUE_BG   "\033[44m"
#define COLOR_PURPLE_BG   "\033[45m"
#define COLOR_LIGHTBLUE_BG   "\033[46m"
#define COLOR_WHITE_BG   "\033[47m"
#define COLOR_RED   "\033[91m"
#define COLOR_GREEN   "\033[92m"
#define COLOR_YELLOW   "\033[93m"
#define COLOR_BLUE   "\033[94m"
#define COLOR_PURPLE   "\033[95m"
#define COLOR_LIGHTBLUE   "\033[96m"
#define COLOR_WHITE   "\033[97m"

#define set_cursor(x,y) printf("\033[%d;%dH", x, y)

void newMessage(NMSG_PTR nmsg);


#endif
