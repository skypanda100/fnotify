//
// Created by zhengdongtian on 17-9-9.
//

#ifndef FNOTIFY_NOTIFY_H
#define FNOTIFY_NOTIFY_H

#include <sys/inotify.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "def.h"

void handle_notify(s_notify *ntf, struct inotify_event *event);
void notify();

#endif //FNOTIFY_NOTIFY_H
