/*
 * MIT License
 *
 * Copyright (c) 2026 Klevis Imeri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DRAG_SHARED_H
#define DRAG_SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "macros.h"
#define FONT8x16_IMPLEMENTATION
#include "font8x16.h"
#include <wayland-util.h>


#define COLOR_BG   0xFF222222
#define COLOR_TEXT 0xFFFFFFFF
#define COLOR_BORDER 0xFF151515

static const int BORDER_WIDTH = 3;
static const int CHAR_W = 8;
static const int CHAR_H = 16;
static const int PADDING_X = 12;
static const int PADDING_Y = 8;

// RFC 3986
// Turns '/home/user/My File.txt' into 'file:///home/user/My%20File.txt\r\n'
char* CreateUriList(const char *path) {
    if (!path) return NULL;

    size_t len = strlen(path);
    void *output = malloc(len * 3 + 16); 
    if (!output) return NULL;

    char *p = (char*)output;
    p += sprintf(p, "file://");

    if (path[0] != '/') *p++ = '/';

    for (const char *s = path; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '-' || c == '.' || 
                c == '_' || c == '~' || c == '/') {
            *p++ = c;
        } else {
            p += sprintf(p, "%%%02X", c);
        }
    }

    *p++ = '\r';
    *p++ = '\n';
    *p = '\0';
    return (char*)output;
}

typedef struct {
    char *uri;
    char *name;
    struct wl_list link;
} FileInfo;

void FileInfoFree(FileInfo *info) {
    if (!info) return;
    if (info->uri) free(info->uri);
    if (info->name) free(info->name);
    free(info);
}

int CommandLineArguments(int argc, char **argv, struct wl_list *uris) {
    if (argc < 2) {
        printf("Usage: %s [file_paths]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        char *path = realpath(argv[i], NULL);
        if (!path) {
            LOG("Error resolving path %s", argv[i]);
            return 1;
        }

        char *uri = CreateUriList(path);
        if (!uri) {
            LOG("Error creating uri");
            return 1; 
        }

        FileInfo *result = (FileInfo*)calloc(1, sizeof(FileInfo));
        if (!result) {
            LOG("Memory allocation failed");
            return 1; 
        }

        result->uri = uri;
        uri = NULL; 

        char *name_ptr = strrchr(path, '/');
        if (name_ptr) name_ptr++;
        else name_ptr = path;

        result->name = strdup(name_ptr);

        if (!result->name) {
            LOG("String duplication failed");
            return 1;
        }

        LOG("Adding name=%s uri=%s\n", result->name, result->uri);
        wl_list_insert(uris, &result->link);
        free(path);
    }

    return 0;
}

static int GetFileListSize(struct wl_list *uris, int *w, int *h) {
    size_t max_len = 0;
    size_t lines = 0;
    FileInfo *file;
    wl_list_for_each(file, uris, link) {
        size_t len = strlen(file->name);
        if (len > max_len) max_len = len;
        lines++;
    }

    if (!lines || max_len > (INT_MAX - PADDING_X * 2) / CHAR_W ||
            lines > (INT_MAX - PADDING_Y * 2) / CHAR_H) return 0;

    *w = (int)max_len * CHAR_W + PADDING_X * 2;
    *h = (int)lines * CHAR_H + PADDING_Y * 2;
    return 1;
}

static void RenderFileListToBuffer(struct wl_list *uris, unsigned int *pixels, int w, int h) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int border = x < BORDER_WIDTH || x >= w - BORDER_WIDTH ||
                    y < BORDER_WIDTH || y >= h - BORDER_WIDTH;
            pixels[(size_t)y * w + x] = border ? COLOR_BORDER : COLOR_BG;
        }
    }

    int line = 0;
    FileInfo *file;
    wl_list_for_each(file, uris, link) {
        int len = strlen(file->name);
        for (int i = 0; i < len; i++) {
            unsigned char c = (unsigned char)file->name[i];
            if (c >= sizeof(font8x16) / sizeof(font8x16[0])) c = '?';
            for (int r = 0; r < CHAR_H; r++) {
                for (int col = 0; col < CHAR_W; col++) {
                    if (font8x16[c][r] & (0x80 >> col)) {
                        int x = PADDING_X + i * CHAR_W + col;
                        int y = PADDING_Y + line * CHAR_H + r;
                        pixels[(size_t)y * w + x] = COLOR_TEXT;
                    }
                }
            }
        }
        line++;
    }
}

#endif // DRAG_SHARED_H
