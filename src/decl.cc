/* Tower Toppler - Nebulus
 * Copyright (C) 2000-2006  Andreas R�ver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "decl.h"
#include "configuration.h"

#include <SDL.h>

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#ifndef WIN32
#include <pwd.h>
#endif

static bool wait_overflow = false;
/* Not read from config file */
int curr_scr_update_speed = MENU_DCLSPEED;

int dcl_update_speed(int spd) {
    int tmp = curr_scr_update_speed;
    curr_scr_update_speed = spd;
    return tmp;
}

void dcl_wait(void) {
    static Uint32 last;

    if (SDL_GetTicks() >= last + (Uint32) (55 - (curr_scr_update_speed * 5))) {
        wait_overflow = true;
        last = SDL_GetTicks();
        return;
    }

    wait_overflow = false;
    while ((SDL_GetTicks() - last) < (Uint32) (55 - (curr_scr_update_speed * 5)))
        SDL_Delay(2);
    last = SDL_GetTicks();
}

bool dcl_wait_overflow(void) {
    return wait_overflow;
}

static int current_debuglevel;

void dcl_setdebuglevel(int level) {
    current_debuglevel = level;
}

void debugprintf(int lvl, const char *fmt, ...) {
#ifdef _DEBUG
    if (lvl <= current_debuglevel) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
#endif
}

/* returns true, if file exists, this is not the
 optimal way to do this. it would be better to open
 the dir the file is supposed to be in and look there
 but this is not really portable so this
 */
bool dcl_fileexists(const char * path) {
#ifdef __BLACKBERRY__
    struct stat buf;
    bool result = stat(path, &buf) == 0 && S_ISREG(buf.st_mode);
#ifdef _DEBUG
    fprintf(stderr, "dcl_fileexists: %s [%s]", path, (result ? "ok" : "missing!"));
#endif
    return result;
#else
    FILE *f = fopen(n, "r");
    if (f) {
        fclose(f);
        return true;
    } else {
        return false;
    }
#endif
}

const char * homedir() {

#ifndef WIN32
    return getenv("HOME");
#else
    return "./";
#endif
}

/* checks if home/.toppler exists and creates it, if not */
static void checkdir(void) {
#ifndef WIN32
#ifdef __BLACKBERRY__
    /* Do absolutely nothing */
#else
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/.toppler", homedir());
    DIR *d = opendir(path);
    if (!d) {
        mkdir(path, S_IRWXU);
    }
#endif
#endif
}

FILE *open_data_file(const char *name) {

#ifndef WIN32
#ifdef __BLACKBERRY__
#else
    // look into actual directory
    if (dcl_fileexists(name))
        return fopen(name, OPEN_FOR_READING);
#endif
    // look into the data dir
    char path[MAX_PATH];
    FILE * result;
    snprintf(path, sizeof(path), "%s/../app/native/assets/%s", homedir(), name);
    result = fopen(path, OPEN_FOR_READING);
#ifdef _DEBUG
    fprintf(stderr, "open_data_file: %s [%s]", path, (result ? "ok" : "missing!"));
#endif
    return result;
#else
    if (dcl_fileexists(name))
        return fopen(name, OPEN_FOR_READING);
    return NULL;
#endif
}

bool get_data_file_path(const char * name, char * f, int len) {

#ifndef WIN32
#ifdef __BLACKBERRY__
#else
    // look into actual directory
    if (dcl_fileexists(name)) {
        snprintf(f, len, name);
        return true;
    }
#endif
    // look into the data dir
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/../app/native/assets/%s", homedir(), name);
    if (dcl_fileexists(path)) {
        snprintf(f, len, path);
        return true;
    }
    return false;
#else
    if (dcl_fileexists(name)) {
        snprintf(f, len, name);
        return true;
    }
    return false;
#endif
}

FILE *open_local_config_file(const char *name) {
#ifndef WIN32
    checkdir();
    char path[MAX_PATH];
#ifdef __BLACKBERRY__
    snprintf(path, sizeof(path), "%s/%s", homedir(), name);
#else
    snprintf(path, sizeof(path), "%s/.toppler/%s", homedir(), name);
#endif
    return fopen(path, "r+");
#else
    if (dcl_fileexists(name))
        return fopen(name, "r+");
    return NULL;
#endif
}

FILE *create_local_config_file(const char *name) {

#ifndef WIN32
    checkdir();
    char path[MAX_PATH];
#ifdef __BLACKBERRY__
    snprintf(path, sizeof(path), "%s/%s", homedir(), name);
#else
    snprintf(path, sizeof(path), "%s/.toppler/%s", homedir(), name);
#endif
    return fopen(path, "w");
#else
    return fopen(name, "w");
#endif
}

/* used for tower and mission saving */

FILE *open_local_data_file(const char *name) {

#ifndef WIN32
    checkdir();
    char path[MAX_PATH];
#ifdef __BLACKBERRY__
    snprintf(path, sizeof(path), "%s/%s", homedir(), name);
#else
    snprintf(path, sizeof(path), "%s/.toppler/%s", homedir(), name);
#endif
    return fopen(path, "r");
#else
    return fopen(name, OPEN_FOR_READING);
#endif
}

FILE *create_local_data_file(const char *name) {
#ifndef WIN32
    checkdir();
    char path[MAX_PATH];
#ifdef __BLACKBERRY__
    snprintf(path, sizeof(path), "%s/%s", homedir(), name);
#else
    snprintf(path, sizeof(path), "%s/.toppler/%s", homedir(), name);
#endif
    return fopen(path, "w+");
#else
    fclose(fopen(name, "w+"));
    return fopen(name, "rb+");
#endif
}

static int sort_by_name(const void *a, const void *b) {
    return (strcmp((*((struct dirent **) a))->d_name, ((*(struct dirent **) b))->d_name));
}

int alpha_scandir(const char *dir, struct dirent ***namelist,
        int (*select)(const struct dirent *)) {
    DIR *d;
    struct dirent *entry;
    int i = 0;
    size_t entrysize;

    if ((d = opendir(dir)) == NULL)
        return (-1);

    *namelist = NULL;
    while ((entry = readdir(d)) != NULL) {
        if (select == NULL || (select != NULL && (*select)(entry))) {
            *namelist = (struct dirent **) realloc((void *) (*namelist),
                    (size_t) ((i + 1) * sizeof(struct dirent *)));
            if (*namelist == NULL)
                return (-1);
            entrysize = sizeof(struct dirent) - sizeof(entry->d_name) + strlen(entry->d_name) + 1;
            (*namelist)[i] = (struct dirent *) malloc(entrysize);
            if ((*namelist)[i] == NULL)
                return (-1);
            memcpy((*namelist)[i], entry, entrysize);
            i++;
        }
    }
    if (closedir(d))
        return (-1);

    if (i == 0)
        return (-1);

    qsort((void *) (*namelist), (size_t) i, sizeof(struct dirent *), sort_by_name);

    return (i);
}

#ifdef WIN32

static int
utf8_mbtowc (void * conv, wchar_t *pwc, const unsigned char *s, int n)
{
    unsigned char c = s[0];

    if (c < 0x80) {
        *pwc = c;
        return 1;
    } else if (c < 0xc2) {
        return -1;
    } else if (c < 0xe0) {
        if (n < 2)
        return -2;
        if (!((s[1] ^ 0x80) < 0x40))
        return -1;
        *pwc = ((wchar_t) (c & 0x1f) << 6)
        | (wchar_t) (s[1] ^ 0x80);
        return 2;
    } else if (c < 0xf0) {
        if (n < 3)
        return -2;
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                        && (c >= 0xe1 || s[1] >= 0xa0)))
        return -1;
        *pwc = ((wchar_t) (c & 0x0f) << 12)
        | ((wchar_t) (s[1] ^ 0x80) << 6)
        | (wchar_t) (s[2] ^ 0x80);
        return 3;
    } else if (c < 0xf8 && sizeof(wchar_t)*8 >= 32) {
        if (n < 4)
        return -2;
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                        && (s[3] ^ 0x80) < 0x40
                        && (c >= 0xf1 || s[1] >= 0x90)))
        return -1;
        *pwc = ((wchar_t) (c & 0x07) << 18)
        | ((wchar_t) (s[1] ^ 0x80) << 12)
        | ((wchar_t) (s[2] ^ 0x80) << 6)
        | (wchar_t) (s[3] ^ 0x80);
        return 4;
    } else if (c < 0xfc && sizeof(wchar_t)*8 >= 32) {
        if (n < 5)
        return -2;
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                        && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
                        && (c >= 0xf9 || s[1] >= 0x88)))
        return -1;
        *pwc = ((wchar_t) (c & 0x03) << 24)
        | ((wchar_t) (s[1] ^ 0x80) << 18)
        | ((wchar_t) (s[2] ^ 0x80) << 12)
        | ((wchar_t) (s[3] ^ 0x80) << 6)
        | (wchar_t) (s[4] ^ 0x80);
        return 5;
    } else if (c < 0xfe && sizeof(wchar_t)*8 >= 32) {
        if (n < 6)
        return -2;
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                        && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
                        && (s[5] ^ 0x80) < 0x40
                        && (c >= 0xfd || s[1] >= 0x84)))
        return -1;
        *pwc = ((wchar_t) (c & 0x01) << 30)
        | ((wchar_t) (s[1] ^ 0x80) << 24)
        | ((wchar_t) (s[2] ^ 0x80) << 18)
        | ((wchar_t) (s[3] ^ 0x80) << 12)
        | ((wchar_t) (s[4] ^ 0x80) << 6)
        | (wchar_t) (s[5] ^ 0x80);
        return 6;
    } else
    return -1;
}

size_t mbrtowc (wchar_t * out, const char *s, int n, mbstate_t * st) {
    return utf8_mbtowc(0, out, (const unsigned char *)s, n);
}

#endif
