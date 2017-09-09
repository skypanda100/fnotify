//
// Created by zhengdongtian on 17-9-9.
//

#include "notify.h"

extern s_notify *s_notify_p;
extern int s_notify_p_len;

static char dirs[DIR_MAX][PATH_MAX];
static int dirs_len = 0;
static size_t flags = IN_CREATE | IN_CLOSE_WRITE | IN_MOVE | IN_MOVE_SELF | IN_DELETE | IN_DELETE_SELF;

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

void handle_notify(s_notify *ntf, struct inotify_event *event)
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
                for(int i = 0;i < ntf->s_watch_p_len;i++)
                {
                    if(parent_wd == ntf->s_watch_p[i].wd)
                    {
                        sprintf(new_path, "%s/%s", ntf->s_watch_p[i].wpath, event->name);
                        break;
                    }
                }
                int new_wd = inotify_add_watch(ntf->notify_fd, new_path, flags);
                if(new_wd == -1)
                {
                    printf("inotify_add_watch failed[%s]\n", new_path);
                }
                else
                {
                    ntf->s_watch_p_len += 1;
                    s_watch *s_w = (s_watch *)realloc(ntf->s_watch_p, sizeof(s_watch) * ntf->s_watch_p_len);
                    s_w[ntf->s_watch_p_len - 1].wd = new_wd;
                    strcpy(s_w[ntf->s_watch_p_len - 1].wpath, new_path);
                    ntf->s_watch_p = s_w;
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
                    ntf->time = time(NULL);
                }
            }
        }
    }
}

void notify()
{
    for(int i = 0;i < s_notify_p_len;i++)
    {
        int nd = inotify_init();
        if(nd == -1)
        {
            printf("inotify_init failed\n");
            s_notify_p[i].notify_fd = -1;
            s_notify_p[i].s_watch_p = NULL;
            s_notify_p[i].s_watch_p_len = 0;
            continue;
        }

        s_notify_p[i].notify_fd = nd;

        int wd = inotify_add_watch(nd, s_notify_p[i].conf.path, flags);
        if(wd == -1)
        {
            printf("inotify_add_watch failed[%s]\n", s_notify_p[i].conf.path);
            s_notify_p[i].s_watch_p = NULL;
            s_notify_p[i].s_watch_p_len = 0;
        }
        else
        {
            s_watch *s_w = (s_watch *)malloc(sizeof(s_watch));
            s_w->wd = wd;
            strcpy(s_w->wpath, s_notify_p[i].conf.path);

            int valid_dirs_len = 0;
            memset(dirs, 0, sizeof(dirs) / sizeof(char));
            dirs_len = 0;
            list_dir(s_notify_p[i].conf.path, 1);
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
            s_notify_p[i].s_watch_p = s_w;
            s_notify_p[i].s_watch_p_len = valid_dirs_len + 1;
        }
    }
}
