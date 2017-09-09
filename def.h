//
// Created by zhengdongtian on 17-9-9.
//

#ifndef FNOTIFY_DEF_H
#define FNOTIFY_DEF_H

#include <limits.h>

#define BUF_LEN 1024
#define DEPTH 64
#define DIR_MAX 256

typedef struct watch{
    int wd;
    char wpath[PATH_MAX];
}s_watch;

#endif //FNOTIFY_DEF_H
