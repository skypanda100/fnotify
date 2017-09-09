//
// Created by zhengdongtian on 17-9-9.
//

#include "notify.h"

extern s_watch **s_watch_p;
extern int *s_watch_p_len;
extern int *notify_p;
extern int notify_p_len;
extern size_t flags;
extern char dirs[DIR_MAX][PATH_MAX];
extern int dirs_len;

static void list_dir(char* path, int depth)
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

void notify(char **path)
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
