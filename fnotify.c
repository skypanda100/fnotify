#include <stdio.h>
#include <sys/inotify.h>
#include "def.h"
#include "notify.h"
#include "watch.h"

/** conf container **/
s_conf *s_conf_p = NULL;
/** conf container length **/
int s_conf_p_len = 0;
/** watch container **/
s_watch **s_watch_p = NULL;
/** watch container length **/
int *s_watch_p_len = NULL;
/** notify container **/
int *notify_p = NULL;
/** notify container length **/
int notify_p_len = 0;
/** sub dir container **/
char dirs[DIR_MAX][PATH_MAX];
/** sub dir container length **/
int dirs_len = 0;
/** notify flags **/
size_t flags = IN_CREATE | IN_CLOSE_WRITE | IN_MOVE | IN_MOVE_SELF | IN_DELETE | IN_DELETE_SELF;

int main(int argc,char **argv)
{
    char *path[1] = {"/home/program/fnotify"};
    notify(path);

    if(notify_p == NULL || s_watch_p == NULL)
    {
        printf("can not notify\n");
        return 1;
    }

    watch();

    return 0;
}