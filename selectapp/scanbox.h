#ifndef _SCANBOX_H
#define _SCANBOX_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "color_output.h"





void boxout();
void scantext(char buffer[], size_t nbuf);


 #endif