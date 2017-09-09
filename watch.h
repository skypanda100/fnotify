//
// Created by zhengdongtian on 17-9-9.
//

#ifndef FNOTIFY_WATCH_H
#define FNOTIFY_WATCH_H

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "def.h"

void handle_watch(s_notify *ntf);
void watch();

#endif //FNOTIFY_WATCH_H
