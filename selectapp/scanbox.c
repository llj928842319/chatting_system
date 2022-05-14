#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "color_output.h"
#include "scanbox.h"

#define _cursor_PRINTF "\033[%d;%dH"
#define set_cursor(x,y) printf("\033[%d;%dH", x, y)

// nbuf: buffer 的大小
void boxout()
{
    
    //int x = 0, y = 0;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    set_cursor(w.ws_row - 3, 0);
    printf(
        "╔═════════════════════════════╗\n"
        "║ 我想说：                      ║\n"
        "╚═════════════════════════════╝");
     
  
    
    //   printf(
    //     "                                \n"
    //     "                                \n"
    //     "                                ");
 

}
void scantext(char buffer[], size_t nbuf){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    //set_cursor(w.ws_row - 3, 0);
    set_cursor(w.ws_row - 2, 10);
    memset(buffer, 0, nbuf);
    scanf("%s", buffer);
    set_cursor(w.ws_row - 2, 10);
}


    

