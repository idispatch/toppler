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

#include "game.h"
#include "archi.h"
#include "menu.h"
#include "decl.h"
#include "sound.h"
#include "level.h"
#include "configuration.h"
#include "highscore.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include <stdexcept>
#if 0
#include <eula.h>
#endif

#if ENABLE_NLS == 1
#include <libintl.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#ifdef __BLACKBERRY__
#else
static void printhelp(void) {
    printf(
            _("\n\tOptions:\n\n  -f\tEnable fullscreen mode\n  -s\tSilence, disable all sound\n  -dX\tSet debug level to X  (default: %i)\n"),
            config.debug_level());
}

static bool parse_arguments(int argc, char *argv[]) {
    for (int t = 1; t < argc; t++) {
        if (!strncmp(argv[t], "-f", 2))
            config.fullscreen(true);
        else if (!strncmp(argv[t], "-s", 2))
            config.nosound(true);
        else if (strstr(argv[t], "-d") == argv[t]) {
            char parm = argv[t][2];
            if (parm >= '0' && parm <= '9') {
                printf(_("Debug level is now %c.\n"), parm);
                config.debug_level(parm - '0');
            } else
                printf(_("Illegal debug level value, using default.\n"));
        } else {
            printhelp();
            return false;
        }
    }
    return true;
}
#endif

static void startgame(void) {
    lev_findmissions();
    gam_init();
    men_init();
    snd_init();
    snd_playTitle();
    men_main();
    snd_stopTitle();
    lev_done();
    snd_done();
    gam_done();
}

static void QuitFunction(void) {
    SDL_Quit();
}

int main(int argc, char *argv[]) {
#if 0
    if(!show_hint_dialog()) {
        return 1;
    }
#endif
    dataarchive = new archive(open_data_file("toppler.dat"));
#if ENABLE_NLS == 1
    setlocale(LC_MESSAGES, "");
    setlocale(LC_CTYPE, "");

    DIR *dir = opendir("locale");
    bindtextdomain("toppler", dir == NULL ? LOCALEDIR : "locale");
    closedir(dir);
    textdomain("toppler");
#endif
#ifdef __BLACKBERRY__
#else
    printf(_("Nebulus version %s"), VERSION);
    printf("\n");

    printf("hsc init\n");
#endif
    hsc_init();
#ifdef __BLACKBERRY__
#else
    if (parse_arguments(argc, argv)) {
#endif
        SDL_InitSubSystem(SDL_INIT_VIDEO);
#ifdef __BLACKBERRY__
        SDL_ShowCursor(SDL_DISABLE);
#else
        SDL_WM_SetCaption(_("Nebulus"), NULL);
        int mouse = SDL_ShowCursor(config.fullscreen() ? 0 : 1);
#endif
        tt_has_focus = true;
        atexit(QuitFunction);
        srand(time(0));
        startgame();
#ifdef __BLACKBERRY__
#else
        printf(_("Thanks for playing!\n"));
        SDL_ShowCursor(mouse);
#endif
        SDL_Quit();
#ifdef __BLACKBERRY__
#else
    }
#endif
    return 0;
}
