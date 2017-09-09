//
// Created by zhengdongtian on 17-9-9.
//

#ifndef FNOTIFY_NOTIFY_H
#define FNOTIFY_NOTIFY_H

#include <sys/inotify.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "def.h"

void handle_notify(int fd, struct inotify_event *event);
void notify(char **path);

#endif //FNOTIFY_NOTIFY_H
