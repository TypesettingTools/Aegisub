/*
 * Copyright (C) 2006 Evgeniy Stepanov <eugeni.stepanov@gmail.com>
 *
 * This file is part of libass.
 *
 * libass is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libass is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with libass; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <stdlib.h>
#include <inttypes.h>
#include <ft2build.h>
#include FT_GLYPH_H

#include "ass_utils.h"

int mystrtoi(char **p, int *res)
{
    // NOTE: base argument is ignored, but not used in libass anyway
    double temp_res;
    char *start = *p;
    temp_res = strtod(*p, p);
    *res = (int) (temp_res + (temp_res > 0 ? 0.5 : -0.5));
    if (*p != start)
        return 1;
    else
        return 0;
}

int mystrtoll(char **p, long long *res)
{
    double temp_res;
    char *start = *p;
    temp_res = strtod(*p, p);
    *res = (int) (temp_res + (temp_res > 0 ? 0.5 : -0.5));
    if (*p != start)
        return 1;
    else
        return 0;
}

int mystrtou32(char **p, int base, uint32_t *res)
{
    char *start = *p;
    *res = strtoll(*p, p, base);
    if (*p != start)
        return 1;
    else
        return 0;
}

int mystrtod(char **p, double *res)
{
    char *start = *p;
    *res = strtod(*p, p);
    if (*p != start)
        return 1;
    else
        return 0;
}

int strtocolor(char **q, uint32_t *res)
{
    uint32_t color = 0;
    int result;
    char *p = *q;

    if (*p == '&')
        ++p;
    else
        ass_msg(MSGL_DBG2, "suspicious color format: \"%s\"\n", p);

    if (*p == 'H' || *p == 'h') {
        ++p;
        result = mystrtou32(&p, 16, &color);
    } else {
        result = mystrtou32(&p, 0, &color);
    }

    {
        unsigned char *tmp = (unsigned char *) (&color);
        unsigned char b;
        b = tmp[0];
        tmp[0] = tmp[3];
        tmp[3] = b;
        b = tmp[1];
        tmp[1] = tmp[2];
        tmp[2] = b;
    }
    if (*p == '&')
        ++p;
    *q = p;

    *res = color;
    return result;
}

// Return a boolean value for a string
char parse_bool(char *str)
{
    while (*str == ' ' || *str == '\t')
        str++;
    if (!strncasecmp(str, "yes", 3))
        return 1;
    else if (strtol(str, NULL, 10) > 0)
        return 1;
    return 0;
}

void ass_msg(int lvl, char *fmt, ...)
{
    va_list va;
    if (lvl > MSGL_INFO)
        return;
    va_start(va, fmt);
    vprintf(fmt, va);
    va_end(va);
}

unsigned ass_utf8_get_char(char **str)
{
    uint8_t *strp = (uint8_t *) * str;
    unsigned c = *strp++;
    unsigned mask = 0x80;
    int len = -1;
    while (c & mask) {
        mask >>= 1;
        len++;
    }
    if (len <= 0 || len > 4)
        goto no_utf8;
    c &= mask - 1;
    while ((*strp & 0xc0) == 0x80) {
        if (len-- <= 0)
            goto no_utf8;
        c = (c << 6) | (*strp++ & 0x3f);
    }
    if (len)
        goto no_utf8;
    *str = (char *) strp;
    return c;

  no_utf8:
    strp = (uint8_t *) * str;
    c = *strp++;
    *str = (char *) strp;
    return c;
}

// gaussian blur
void ass_gauss_blur(unsigned char *buffer,
          unsigned short *tmp2,
          int width, int height, int stride, int *m2, int r, int mwidth)
{

    int x, y;

    unsigned char *s = buffer;
    unsigned short *t = tmp2 + 1;
    for (y = 0; y < height; y++) {
        memset(t - 1, 0, (width + 1) * sizeof(short));

        for (x = 0; x < r; x++) {
            const int src = s[x];
            if (src) {
                register unsigned short *dstp = t + x - r;
                int mx;
                unsigned *m3 = (unsigned *) (m2 + src * mwidth);
                for (mx = r - x; mx < mwidth; mx++) {
                    dstp[mx] += m3[mx];
                }
            }
        }

        for (; x < width - r; x++) {
            const int src = s[x];
            if (src) {
                register unsigned short *dstp = t + x - r;
                int mx;
                unsigned *m3 = (unsigned *) (m2 + src * mwidth);
                for (mx = 0; mx < mwidth; mx++) {
                    dstp[mx] += m3[mx];
                }
            }
        }

        for (; x < width; x++) {
            const int src = s[x];
            if (src) {
                register unsigned short *dstp = t + x - r;
                int mx;
                const int x2 = r + width - x;
                unsigned *m3 = (unsigned *) (m2 + src * mwidth);
                for (mx = 0; mx < x2; mx++) {
                    dstp[mx] += m3[mx];
                }
            }
        }

        s += stride;
        t += width + 1;
    }

    t = tmp2;
    for (x = 0; x < width; x++) {
        for (y = 0; y < r; y++) {
            unsigned short *srcp = t + y * (width + 1) + 1;
            int src = *srcp;
            if (src) {
                register unsigned short *dstp = srcp - 1 + width + 1;
                const int src2 = (src + 128) >> 8;
                unsigned *m3 = (unsigned *) (m2 + src2 * mwidth);

                int mx;
                *srcp = 128;
                for (mx = r - 1; mx < mwidth; mx++) {
                    *dstp += m3[mx];
                    dstp += width + 1;
                }
            }
        }
        for (; y < height - r; y++) {
            unsigned short *srcp = t + y * (width + 1) + 1;
            int src = *srcp;
            if (src) {
                register unsigned short *dstp = srcp - 1 - r * (width + 1);
                const int src2 = (src + 128) >> 8;
                unsigned *m3 = (unsigned *) (m2 + src2 * mwidth);

                int mx;
                *srcp = 128;
                for (mx = 0; mx < mwidth; mx++) {
                    *dstp += m3[mx];
                    dstp += width + 1;
                }
            }
        }
        for (; y < height; y++) {
            unsigned short *srcp = t + y * (width + 1) + 1;
            int src = *srcp;
            if (src) {
                const int y2 = r + height - y;
                register unsigned short *dstp = srcp - 1 - r * (width + 1);
                const int src2 = (src + 128) >> 8;
                unsigned *m3 = (unsigned *) (m2 + src2 * mwidth);

                int mx;
                *srcp = 128;
                for (mx = 0; mx < y2; mx++) {
                    *dstp += m3[mx];
                    dstp += width + 1;
                }
            }
        }
        t++;
    }

    t = tmp2;
    s = buffer;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            s[x] = t[x] >> 8;
        }
        s += stride;
        t += width + 1;
    }
}

#ifdef CONFIG_ENCA
void *ass_guess_buffer_cp(unsigned char *buffer, int buflen,
                      char *preferred_language, char *fallback)
{
    const char **languages;
    size_t langcnt;
    EncaAnalyser analyser;
    EncaEncoding encoding;
    char *detected_sub_cp = NULL;
    int i;

    languages = enca_get_languages(&langcnt);
    ass_msg(MSGL_V, "ENCA supported languages: ");
    for (i = 0; i < langcnt; i++) {
        ass_msg(MSGL_V, "%s ", languages[i]);
    }
    ass_msg(MSGL_V, "\n");

    for (i = 0; i < langcnt; i++) {
        const char *tmp;

        if (strcasecmp(languages[i], preferred_language) != 0)
            continue;
        analyser = enca_analyser_alloc(languages[i]);
        encoding = enca_analyse_const(analyser, buffer, buflen);
        tmp = enca_charset_name(encoding.charset, ENCA_NAME_STYLE_ICONV);
        if (tmp && encoding.charset != ENCA_CS_UNKNOWN) {
            detected_sub_cp = strdup(tmp);
            ass_msg(MSGL_INFO, "ENCA detected charset: %s\n", tmp);
        }
        enca_analyser_free(analyser);
    }

    free(languages);

    if (!detected_sub_cp) {
        detected_sub_cp = strdup(fallback);
        ass_msg(MSGL_INFO,
               "ENCA detection failed: fallback to %s\n", fallback);
    }

    return detected_sub_cp;
}
#endif
