#include "color_output.h"

//这个代码是实现聊天信息彩色输出的

#include "epolll.h"

void newMessage(NMSG_PTR nmsg) {
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
	printf(COLOR_GREEN "[IP:%s PORT: %d] 说" COLOR_DEFAULT, nmsg->name, nmsg->port_name);
	printf(COLOR_RED " [%s]" COLOR_DEFAULT "\n", nmsg->text);
}

