#pragma once

#include <string>
#include <string.h>
#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

namespace util {
std::string swkbdInput(SwkbdType type, int len, std::string header = "Input text", std::string subtext = "", std::string init_text = "");
void showbits(char* mess, short x);
}
