#include<stdio.h>
#include<unistd.h>
#include<sys/inotify.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#define BUF_LEN 1024
#define DEPTH 64
#define DIR_MAX 256

typedef struct watch{
    int wd;
    char wpath[PATH_MAX];
}s_watch;

s_watch **s_watch_p = NULL;
int *s_watch_p_len = NULL;
int *notify_p = NULL;
int notify_p_len = 0;

char dirs[DIR_MAX][PATH_MAX];
int dirs_len = 0;

size_t flags = IN_CREATE | IN_CLOSE_WRITE | IN_MOVE | IN_MOVE_SELF | IN_DELETE | IN_DELETE_SELF;

void handle_notify(int fd, struct inotify_event *event)
{
    if(event->mask & IN_ISDIR)
    {
        if(event->len > 0)
        {
            //ignore hidden file
            if(event->name[0] != '.')
            {
                char new_path[PATH_MAX] = {0};
                int parent_wd = event->wd;
                int watch_index = -1;
                int s_watch_len = 0;
                for(int i = 0;i < notify_p_len;i++)
                {
                    if(notify_p[i] == fd)
                    {
                        s_watch *watch = s_watch_p[i];
                        if(watch != NULL)
                        {
                            s_watch_len = s_watch_p_len[i];
                            for(int j = 0;j < s_watch_len;j++)
                            {
                                if(parent_wd == watch[j].wd)
                                {
                                    sprintf(new_path, "%s/%s", watch[j].wpath, event->name);
                                    watch_index = i;
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                if(watch_index > -1)
                {
                    int new_wd = inotify_add_watch(fd, new_path, flags);
                    if(new_wd == -1)
                    {
                        printf("inotify_add_watch failed[%s]\n", new_path);
                    }
                    else
                    {
                        s_watch *s_w = (s_watch *)realloc(s_watch_p[watch_index], sizeof(s_watch) * (s_watch_len + 1));
                        s_w[s_watch_len].wd = new_wd;
                        strcpy(s_w[s_watch_len].wpath, new_path);
                        s_watch_p[watch_index] = s_w;
                        s_watch_p_len[watch_index] = s_watch_len + 1;
                    }
                }
            }
        }
    }
    else
    {
        if(!(event->mask & IN_CREATE))
        {
            if(event->len > 0)
            {
                //ignore hidden file
                if(event->name[0] != '.')
                {
                    printf("file name = %s\n",event->name);
                }
            }
        }
    }
}

void list_dir(char* path, int depth)
{
    DIR *d;
    struct dirent *file;
    struct stat st;
    char full_path[PATH_MAX] = {0};

    if(!(d = opendir(path)))
    {
        printf("opendir failed[%s]\n", path);
        return;
    }

    while((file = readdir(d)) != NULL)
    {
        if(strncmp(file->d_name, ".", 1) == 0)
        {
            continue;
        }

        memset(full_path, 0, sizeof(full_path) / sizeof(char));
        sprintf(full_path, "%s/%s", path, file->d_name);

        if(stat(full_path, &st) >= 0 && S_ISDIR(st.st_mode) && depth <= DEPTH)
        {
            strcpy(dirs[dirs_len++], full_path);
            list_dir(full_path, depth + 1);
        }
    }
    closedir(d);
}

void init_notify(char **path)
{
    for(int i = 0;i < sizeof(path) / sizeof(char *);i++)
    {
        int nd = inotify_init();
        if(nd == -1)
        {
            printf("inotify_init failed\n");
            continue;
        }

        notify_p = (int *)realloc(notify_p, sizeof(int) * (i + 1));
        notify_p[i] = nd;
        notify_p_len++;
        s_watch_p = (s_watch **)realloc(s_watch_p, sizeof(s_watch *) * (i + 1));
        s_watch_p_len = realloc(s_watch_p_len, sizeof(int) * (i + 1));

        int wd = inotify_add_watch(nd, path[i], flags);
        if(wd == -1)
        {
            printf("inotify_add_watch failed[%s]\n", path[i]);
            s_watch_p[i] = NULL;
            s_watch_p_len[i] = 0;
        }
        else
        {
            s_watch *s_w = (s_watch *)malloc(sizeof(s_watch) * 1);
            s_w->wd = wd;
            strcpy(s_w->wpath, path[i]);

            int valid_dirs_len = 0;
            memset(dirs, 0, sizeof(dirs) / sizeof(char));
            dirs_len = 0;
            list_dir(path[i], 1);
            for(int j = 0; j < dirs_len;j++)
            {
                wd = inotify_add_watch(nd, dirs[j], flags);
                if(wd == -1)
                {
                    printf("inotify_add_watch failed[%s]\n", dirs[j]);
                }
                else
                {
                    valid_dirs_len++;
                    s_w = (s_watch *)realloc(s_w, sizeof(s_watch) * (valid_dirs_len + 1));
                    s_w[valid_dirs_len].wd = wd;
                    strcpy(s_w[valid_dirs_len].wpath, dirs[j]);
                }
            }
            s_watch_p[i] = s_w;
            s_watch_p_len[i] = valid_dirs_len + 1;
        }
    }
}

void handle_select(int fd)
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

void do_select()
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
                handle_select(fd);
            }
        }
    }
}

int main(int argc,char **argv)
{
    char *path[1] = {"/home/zhengdongtian/CLionProjects/fnotify"};
    init_notify(path);

    if(notify_p == NULL || s_watch_p == NULL)
    {
        printf("can not notify\n");
        return 1;
    }

    do_select();

    return 0;
}