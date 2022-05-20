#include "scanbox.h"


// nbuf: buffer 的大小
void boxout()
{
    //int x = 0, y = 0;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    set_cursor(w.ws_row - 4, 0);
    printf(COLOR_DEFAULT 
        "╔═════════════════════════════╗\n"
        "║ Say:                        ║\n"
        "╚═════════════════════════════╝\n");
    set_cursor(w.ws_row - 3, 12); 
    
}

void scantext(char buffer[], size_t nbuf){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    set_cursor(w.ws_row - 3, 12);
    memset(buffer, 0, nbuf);
    scanf("%s", buffer);
    set_cursor(w.ws_row - 3, 12);
}


    

