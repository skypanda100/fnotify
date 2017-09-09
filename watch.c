//
// Created by zhengdongtian on 17-9-9.
//

#include "watch.h"
#include "notify.h"

extern s_notify *s_notify_p;
extern int s_notify_p_len;

void handle_watch(s_notify *ntf)
{
    char buf[BUF_LEN];
    size_t read_len;
    char *p;
    struct inotify_event *event;

    read_len = read(ntf->notify_fd, buf, BUF_LEN);
    if(read_len == -1)
    {
        printf("read failed\n");
    }

    for(p=buf;p < buf+read_len;)
    {
        event = (struct inotify_event *)p;
        handle_notify(ntf, event);
        p += sizeof(struct inotify_event) + event->len;
    }
}

void watch()
{
    struct timeval tv;
    tv.tv_sec = 1;
    fd_set rd;
    while(1)
    {
        int max_fd = 0;
        FD_ZERO(&rd);
        for(int i = 0;i < s_notify_p_len;i++)
        {
            FD_SET(s_notify_p[i].notify_fd, &rd);
            if(max_fd < s_notify_p[i].notify_fd)
            {
                max_fd = s_notify_p[i].notify_fd;
            }
        }

        int ret = select(max_fd + 1, &rd, NULL, NULL, &tv);

        for(int i = 0; i < s_notify_p_len;i++)
        {
            time_t cmd_time = s_notify_p[i].time;
            time_t now_time = time(NULL);
            if(cmd_time == 0)
            {
                continue;
            }
            else
            {
                if(now_time - cmd_time >= s_notify_p[i].conf.delay)
                {
                    system(s_notify_p[i].conf.cmd);
                    s_notify_p[i].time = 0;
                }
            }
        }

        if(ret == -1)
        {
            continue;
        }
        else if(ret == 0)
        {
            continue;
        }
        else
        {
            for(int i = 0; i < s_notify_p_len;i++)
            {
                if(FD_ISSET(s_notify_p[i].notify_fd, &rd))
                {
                    handle_watch(&(s_notify_p[i]));
                }
            }
        }
    }
}
