//
// Created by zhengdongtian on 17-9-9.
//

#ifndef FNOTIFY_DEF_H
#define FNOTIFY_DEF_H

#include <limits.h>

#define BUF_LEN 1024
#define DEPTH 64
#define DIR_MAX 256

typedef struct watch
{
    int wd;
    char wpath[PATH_MAX];
}s_watch;

typedef struct conf
{
    char path[PATH_MAX];
    char cmd[PATH_MAX];
    int delay;
}s_conf;
#endif //FNOTIFY_DEF_H
