#include "color_output.h"

void newMessage(char *from, char *message) {
	static int rol = 1;
	time_t t;
	struct tm *timeinfo;
	time(&t);
	char *mytime = NULL;
	timeinfo = localtime(&t);
	mytime = asctime(timeinfo);
	mytime[strlen(mytime)-1] = '\0';

	set_cursor(rol++, 0);
	printf(COLOR_BLUE " [%s]" COLOR_DEFAULT, mytime);
	printf(COLOR_GREEN "[%s] 说" COLOR_DEFAULT, from);
	printf(COLOR_RED " [%s]" COLOR_DEFAULT "\n", message);
}

