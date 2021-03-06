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

#include "txtsys.h"
#include "screen.h"
#include "keyb.h"
#include "menu.h"
#include <stdlib.h>
#include <string.h>

textsystem::textsystem(const char *title, menuopt_callback_proc pr) {
    if (title) {
        this->title = new char[strlen(title) + 1];
        strcpy(this->title, title);
    } else
        this->title = NULL;

    numlines = 0;
    max_length = 0;
    lines = NULL;
    mproc = pr;
    xoffs = yoffs = disp_xoffs = disp_yoffs = 0;
    ystart = (title) ? FONT_HEIGHT + 15 : 0;
    shownlines = ((SCREEN_HEIGHT - ystart) / FONT_HEIGHT) + 1;
}

textsystem::~textsystem() {
    int i;

    if (lines && numlines) {
        for (i = 0; i < numlines; i++)
            if (lines[i])
                delete[] lines[i];
        delete[] lines;
    }
    if (title)
        delete[] title;
}

void textsystem::addline(const char *line) {
    int olen;

    char **tmp = new char *[numlines + 1];
    if (!tmp)
        return;

    if (lines) {
        memcpy(tmp, lines, sizeof(char *) * numlines);
        delete[] lines;
    }

    if (line && (strlen(line) > 0)) {
        tmp[numlines] = new char[strlen(line) + 1];
        strcpy(tmp[numlines], line);
    } else
        tmp[numlines] = NULL;

    lines = tmp;
    numlines++;

    if (line) {
        olen = scr_formattextlength(0, 0, line);
        if (olen < 0)
            olen = 0;
    } else
        olen = 0;

    if (max_length < olen)
        max_length = olen;
}

void textsystem::run() {
    bool ende = false;
    SDLKey key = SDLK_UNKNOWN;

    do {
        (void) key_readkey();

        draw();

        key = key_sdlkey();

        switch (key_sdlkey2conv(key, false)) {
        case up_key:
            if (yoffs >= FONT_HEIGHT)
                yoffs -= FONT_HEIGHT;
            else
                yoffs = 0;
            break;
        case down_key:
            if (yoffs + (shownlines * FONT_HEIGHT) < (numlines * FONT_HEIGHT))
                yoffs += FONT_HEIGHT;
            else
                yoffs = (numlines - shownlines + 1) * FONT_HEIGHT;
            break;
        case break_key:
            ende = true;
            break;
        case left_key:
            if (xoffs >= FONTWID)
                xoffs -= FONTWID;
            else
                xoffs = 0;
            break;
        case right_key:
            if (xoffs <= max_length - SCREEN_WIDTH - FONTWID)
                xoffs += FONTWID;
            else
                xoffs = max_length - SCREEN_WIDTH;
            break;
        default:
            switch (key) {
            case SDLK_PAGEUP:
                if (yoffs >= shownlines * FONT_HEIGHT)
                    yoffs -= shownlines * FONT_HEIGHT;
                else
                    yoffs = 0;
                break;
            case SDLK_SPACE:
            case SDLK_PAGEDOWN:
                if ((yoffs / FONT_HEIGHT) + (shownlines * 2) <= numlines)
                    yoffs += shownlines * FONT_HEIGHT;
                else
                    yoffs = (numlines - shownlines + 1) * FONT_HEIGHT;
                break;
            case SDLK_HOME:
                yoffs = 0;
                break;
            case SDLK_END:
                yoffs = (numlines - shownlines + 1) * FONT_HEIGHT;
                break;
            case SDLK_RETURN:
            case SDLK_ESCAPE:
                ende = true;
                break;
            default:
                break;
            }
            break;
        }

    } while (!ende);
}

void textsystem::draw() {
    char pointup[2], pointdown[2], pointleft[2], pointright[2];

    pointup[0] = fontptrup;
    pointup[1] = 0;
    pointdown[0] = fontptrdown;
    pointdown[1] = 0;
    pointleft[0] = fontptrleft;
    pointleft[1] = 0;
    pointright[0] = fontptrright;
    pointright[1] = 0;

    if (mproc)
        (*mproc)(NULL);

    if (title)
        scr_writetext_center(5, title);

    if (disp_yoffs < yoffs) {
        disp_yoffs += ((yoffs - disp_yoffs + 3) / 4) + 1;
        if (disp_yoffs > yoffs)
            disp_yoffs = yoffs;
    } else if (disp_yoffs > yoffs) {
        disp_yoffs -= ((disp_yoffs - yoffs + 3) / 4) + 1;
        if (disp_yoffs < yoffs)
            disp_yoffs = yoffs;
    }

    if (disp_xoffs < xoffs) {
        disp_xoffs += ((xoffs - disp_xoffs) / 4) + 1;
        if (disp_xoffs > xoffs)
            disp_xoffs = xoffs;
    } else if (disp_xoffs > xoffs) {
        disp_xoffs -= ((disp_xoffs - xoffs) / 4) + 1;
        if (disp_xoffs < xoffs)
            disp_xoffs = xoffs;
    }

    scr_setclipping(0, ystart, SCREEN_WIDTH, SCREEN_HEIGHT);
    for (int k = 0; k <= shownlines; k++)
        if (k + (disp_yoffs / FONT_HEIGHT) < numlines) {
            if (lines[k + (disp_yoffs / FONT_HEIGHT)])
                scr_writeformattext(-disp_xoffs, k * FONT_HEIGHT + ystart - (disp_yoffs % FONT_HEIGHT),
                        lines[k + (disp_yoffs / FONT_HEIGHT)]);
        }

    scr_setclipping();

    if (disp_yoffs > 0)
        scr_writetext(SCREEN_WIDTH - FONTWID, 34, pointup);
    if ((disp_yoffs / FONT_HEIGHT) + shownlines < numlines)
        scr_writetext(SCREEN_WIDTH - FONTWID, SCREEN_HEIGHT - FONT_HEIGHT, pointdown);

    if (disp_xoffs > 0)
        scr_writetext(FONTWID, 5, pointleft);
    if (disp_xoffs < max_length - SCREEN_WIDTH)
        scr_writetext(SCREEN_WIDTH - FONTWID, 5, pointright);

    scr_swap();
    dcl_wait();
}
