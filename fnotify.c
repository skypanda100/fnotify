#include "def.h"
#include "config.h"
#include "notify.h"
#include "watch.h"

s_notify *s_notify_p = NULL;
int s_notify_p_len = 0;

int main(int argc,char **argv)
{
    config("/home/program/fnotify/fnotify.conf");
    notify();
    watch();

    return 0;
}