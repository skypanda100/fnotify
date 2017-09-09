//
// Created by zhengdongtian on 17-9-9.
//

#include "watch.h"
#include "notify.h"

extern s_watch **s_watch_p;
extern int *s_watch_p_len;
extern int *notify_p;
extern int notify_p_len;
extern size_t flags;
extern char dirs[DIR_MAX][PATH_MAX];
extern int dirs_len;

void handle_watch(int fd)
{
    char buf[BUF_LEN];
    size_t read_len;
    char *p;
    struct inotify_event *event;

    read_len = read(fd, buf, BUF_LEN);
    if(read_len == -1)
    {
        printf("read failed\n");
    }

    for(p=buf;p < buf+read_len;)
    {
        event = (struct inotify_event *)p;
        handle_notify(fd, event);
        p += sizeof(struct inotify_event) + event->len;
    }
}

void watch()
{
    fd_set rd;
    while(1)
    {
        int max_fd = 0;
        FD_ZERO(&rd);
        for(int i = 0;i < notify_p_len;i++)
        {
            FD_SET(notify_p[i], &rd);
            if(max_fd < notify_p[i])
            {
                max_fd = notify_p[i];
            }
        }

        int ret = select(max_fd + 1, &rd, NULL, NULL, NULL);
        if(ret == -1)
        {
            printf("select failed\n");
            continue;
        }
        else if(ret == 0)
        {
            printf("select timeout\n");
            continue;
        }

        for(int i = 0; i < notify_p_len;i++)
        {
            int fd = notify_p[i];
            if(FD_ISSET(fd, &rd))
            {
                handle_watch(fd);
            }
        }
    }
}
