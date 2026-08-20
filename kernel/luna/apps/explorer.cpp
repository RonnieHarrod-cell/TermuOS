#include "app.hpp"
#include "../desktop/wm.hpp"
#include "../widgets/window.hpp"
#include "../widgets/label.hpp"
#include "../widgets/button.hpp"

extern "C"
{
#include "../../fs/vfs.h"
#include "../../lib/printf.h"
}

static Window exp_win;
static Label path_lab;
static Label rows[12];
static Button btn_up;
static Button btn_refresh;
static bool exp_ready;

static char exp_path[VFS_PATH_MAX] = "/";
static char row_name[12][VFS_NAME_MAX];
static char row_label[12][VFS_NAME_MAX + 4];
static uint32_t row_type[12];
static int row_count;

static int str_len(const char *s)
{
    int n = 0;
    while (s[n])
        n++;
    return n;
}

static void str_cpy(char *d, const char *s, int max)
{
    int i = 0;
    while (s[i] && i < max - 1)
    {
        d[i] = s[i];
        i++;
    }
    d[i] = '\0';
}

static void path_parent(char *path)
{
    int n = str_len(path);
    if (n <= 1)
    {
        path[0] = '/';
        path[1] = '\0';
        return;
    }
    if (path[n - 1] == '/')
        path[--n] = '\0';
    while (n > 0 && path[n - 1] != '/')
        path[--n] = '\0';
    if (n <= 0)
    {
        path[0] = '/';
        path[1] = '\0';
    }
}

static void path_join(char *path, const char *name)
{
    int n = str_len(path);
    if (n > 0 && path[n - 1] != '/' && n < VFS_PATH_MAX - 1)
        path[n++] = '/';
    for (int i = 0; name[i] && n < VFS_PATH_MAX - 1; i++)
        path[n++] = name[i];
    path[n] = '\0';
}

static void exp_reload(void *user)
{
    (void)user;
    row_count = 0;

    if (!(exp_path[0] == '/' && exp_path[1] == '\0'))
    {
        str_cpy(row_name[0], "..", VFS_NAME_MAX);
        str_cpy(row_label[0], "[..]", VFS_NAME_MAX + 4);
        row_type[0] = VFS_DIR;
        row_count = 1;
    }

    int fd = vfs_open(exp_path, O_RDONLY);
    if (fd >= 0)
    {
        char name[VFS_NAME_MAX];
        for (uint32_t idx = 0; row_count < 12; idx++)
        {
            if (vfs_readdir(fd, idx, name) != 0)
                break;
            if (name[0] == '.' &&
                (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
                continue;

            str_cpy(row_name[row_count], name, VFS_NAME_MAX);

            char full[VFS_PATH_MAX];
            str_cpy(full, exp_path, VFS_PATH_MAX);
            path_join(full, name);

            uint32_t type = VFS_FILE;
            uint64_t size = 0;
            vfs_stat(full, &type, &size);
            row_type[row_count] = type;

            if (type == VFS_DIR)
            {
                int k = 0;
                while (name[k] && k < VFS_NAME_MAX - 1)
                {
                    row_label[row_count][k] = name[k];
                    k++;
                }
                row_label[row_count][k++] = '/';
                row_label[row_count][k] = '\0';
            }
            else
            {
                str_cpy(row_label[row_count], name, VFS_NAME_MAX + 4);
            }
            row_count++;
        }
        vfs_close(fd);
    }

    path_lab.text = exp_path;
    path_lab.mark_dirty();

    for (int i = 0; i < 12; i++)
    {
        rows[i].text = (i < row_count) ? row_label[i] : "";
        rows[i].mark_dirty();
    }
    exp_win.mark_dirty();
}

static void exp_up(void *user)
{
    (void)user;
    path_parent(exp_path);
    exp_reload(nullptr);
}

static void exp_activate(int index)
{
    if (index < 0 || index >= row_count)
        return;

    if (row_type[index] == VFS_DIR)
    {
        if (row_name[index][0] == '.' && row_name[index][1] == '.' &&
            row_name[index][2] == '\0')
            path_parent(exp_path);
        else
            path_join(exp_path, row_name[index]);
        exp_reload(nullptr);
        return;
    }

    kprintf("explorer: file %s (%s)\n", row_name[index], exp_path);
}

void app_explorer_open(void *user)
{
    (void)user;

    if (!exp_ready)
    {
        exp_win.x = 90;
        exp_win.y = 50;
        exp_win.w = 400;
        exp_win.h = 320;
        exp_win.title = "File Explorer";
        exp_win.visible = false;

        path_lab.x = 12;
        path_lab.y = 36;
        path_lab.w = 370;
        path_lab.h = 18;
        path_lab.text = "/";
        exp_win.add(&path_lab);

        for (int i = 0; i < 12; i++)
        {
            rows[i].x = 12;
            rows[i].y = 60 + i * 18;
            rows[i].w = 370;
            rows[i].h = 16;
            rows[i].text = "";
            exp_win.add(&rows[i]);
        }

        btn_up.x = 12;
        btn_up.y = 280;
        btn_up.w = 70;
        btn_up.h = 26;
        btn_up.text = "Up";
        btn_up.on_click = exp_up;
        exp_win.add(&btn_up);

        btn_refresh.x = 90;
        btn_refresh.y = 280;
        btn_refresh.w = 90;
        btn_refresh.h = 26;
        btn_refresh.text = "Refresh";
        btn_refresh.on_click = exp_reload;
        exp_win.add(&btn_refresh);

        exp_ready = true;
        if (g_luna_wm)
            g_luna_wm->add(&exp_win);
    }

    exp_path[0] = '/';
    exp_path[1] = '\0';
    exp_win.visible = true;
    if (g_luna_wm)
        g_luna_wm->raise(&exp_win);
    exp_reload(nullptr);
}

void app_explorer_click(int screen_x, int screen_y)
{
    if (!exp_ready || !exp_win.visible)
        return;
    int lx = screen_x - exp_win.x;
    int ly = screen_y - exp_win.y;
    if (lx < 12 || lx > 380)
        return;
    if (ly < 60 || ly >= 60 + 12 * 18)
        return;
    int row = (ly - 60) / 18;
    exp_activate(row);
}

bool app_explorer_is_open(void)
{
    return exp_ready && exp_win.visible;
}
