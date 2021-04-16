/* Copyright (c) 2021 Ovanes O. Ohanesian

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.   */

#include "rss_buffer.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef ENABLE_RSSBUF_SANITY_CHECK
#define RSSBUF_SANITY_CHECK(buf) assert(buf->beg <= buf->cur \
                            && buf->cur <= buf->end \
                            && "Local buffer went insane!")
#else
#define RSSBUF_SANITY_CHECK(buf)
#endif

/* Buffer operates on 3 pointers:
   beginning, current position and end of buffer */
struct rss_buf
{
    char *beg;
    char *cur;
    char *end;
};

/* Helper malloc */
static rss_buffer *malloc_buf()
{
    rss_buffer *buf = malloc(sizeof(rss_buffer));
    buf->beg = NULL;
    buf->cur = NULL;
    buf->end = NULL;
    return buf;
}

/* Returns total reserved buffer size */
static size_t rssbuf_get_rsrvd_size(rss_buffer *buf)
{
    RSSBUF_SANITY_CHECK(buf);

    return (size_t)(buf->end - buf->beg);
}

/* Returns reserved leftover buffer size */
static size_t rssbuf_get_leftover_size(rss_buffer *buf)
{
    RSSBUF_SANITY_CHECK(buf);

    return (size_t)(rssbuf_get_rsrvd_size(buf) - rssbuf_get_act_size(buf));
}

/* Reallocates buffer and repositions
   buffer pointers */
static int realloc_buf(rss_buffer *buf, const size_t sz)
{
    size_t cur_pos = rssbuf_get_act_size(buf);

    char *tmp = realloc(buf->beg, sz);
    if (!tmp)
        return 0;

    buf->beg = tmp;
    buf->end = buf->beg + sz;

    if (cur_pos <= sz) {
        buf->cur = buf->beg + cur_pos;
        memset(buf->cur, 0, rssbuf_get_leftover_size(buf));
    }
    else {
        buf->cur = buf->end;
    }
    return 1;
}

/* Ensures that buffer size is at least sz */
static int rssbuf_ensure_size(rss_buffer *buf, const size_t sz)
{
    if (rssbuf_get_rsrvd_size(buf) < sz)
        return realloc_buf(buf, sz);

    return 1;
}

/* 'inflation'  here means doubling
    buffer size until sz fits there */
static int inflate_to_include(rss_buffer *buf, size_t sz)
{
    size_t new_len = rssbuf_get_rsrvd_size(buf);

    while(new_len < rssbuf_get_act_size(buf) + sz)
        new_len *= 2;

    return rssbuf_ensure_size(buf, new_len);
}

rss_buffer *rssbuf_crt(const size_t sz)
{
    if (sz == 0)
        return NULL;

    rss_buffer *buf = malloc_buf();
    buf->beg = calloc(1, sz);
    if (!buf->beg) {
        rssbuf_free(&buf);
        return NULL;
    }

    buf->cur = buf->beg;
    buf->end = buf->beg + sz;

    return buf;
}

void rssbuf_free(rss_buffer **buf)
{
    if ((*buf)->beg)
        free((*buf)->beg);
    free(*buf);
    *buf = NULL;
}

size_t rssbuf_get_act_size(rss_buffer *buf)
{
    RSSBUF_SANITY_CHECK(buf);

    return (size_t)(buf->cur - buf->beg);
}

const char *rssbuf_get_data(rss_buffer *buf)
{
    return buf->beg;
}

int rssbuf_append_str(rss_buffer *buf, const char *str)
{
    size_t extra_sz = strlen(str);
    if (!inflate_to_include(buf, extra_sz))
        return 0;

    strncpy(buf->cur, str, extra_sz);
    buf->cur += extra_sz;
    return 1;
}

void rssbuf_clr(rss_buffer *buf)
{
    memset(buf->beg, 0, rssbuf_get_rsrvd_size(buf));
    buf->cur = buf->beg;
}

int rssbuf_sprintf(rss_buffer *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int res = rssbuf_vsprintf(buf, fmt, args);

    va_end(args);
    return res;
}

int rssbuf_vsprintf(rss_buffer *buf, const char *fmt, va_list args)
{
    va_list args_c;
    va_copy(args_c, args);

    size_t availible = rssbuf_get_leftover_size(buf);

    int required = vsnprintf(buf->cur, availible, fmt, args) + 1;
    va_end(args);

    if(required < 0)
        return 0;

    if((size_t)required > availible) {
        if (!inflate_to_include(buf, (size_t)required))
            return 0;

        int written = vsprintf(buf->cur, fmt, args_c);
        if(written < 0)
            return 0;
    }
    va_end(args_c);

    buf->cur += required - 1;

    return 1;
}
