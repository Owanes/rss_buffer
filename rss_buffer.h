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

/*
 * Following implements simple string buffer interface.
 * Buffer should be created with initial size. Buffer
 * size 'inflates' when there is no room for new data.
 * 'Inflates' means simple algorithm of calculating
 * new buffer size by doubling current buffer size until
 * it fits new data. Reallocation happens once.
 *
 * Best suited for mid sized string collections.
 * For smaller strings use char[].
 * It probably can handle bigger strings as well, but
 * for 4gb string sudden growth to 8gb can screw you up.
 *
 * With initial size choosen correctly, this buffer
 * should be really efficient. It probably should be
 * power of 2.
 *
 */

#ifndef RSS_BUFFER_H_INCLUDED
#define RSS_BUFFER_H_INCLUDED

#include <stddef.h>
#include <stdarg.h>

/* */
typedef struct rss_buf rss_buffer;

/* Creates buffer for string
   Returns NULL on failure  */
rss_buffer *rssbuf_crt(const size_t);

/* Frees buffer */
void rssbuf_free(rss_buffer **);

/* Clears and resets buffer
   NOTE: it zeroes buffer and
         keeps current size */
void rssbuf_clr(rss_buffer *);

/* Appends string to buffer, inflating it if necessary.
   Returns 0 on fail */
int rssbuf_append_str(rss_buffer *, const char *);

/* Appends string to buffer via delegation to lclbuf_vsprintf,
   inflating if necessary. Returns 0 on fail */
int rssbuf_sprintf(rss_buffer *, const char *, ...)
  __attribute__ ((format(printf, 2, 3)));

/* Appends string to buffer via vsnprintf or vsprintf,
   inflating if necessary. Returns 0 on fail */
int rssbuf_vsprintf(rss_buffer *, const char *, va_list)
  __attribute__ ((format(printf, 2, 0)));

/* Returns actual buffer size */
size_t rssbuf_get_act_size(rss_buffer *);

/* Returns pointer to stored string */
const char *rssbuf_get_data(rss_buffer *);

#endif /* RSS_BUFFER_H_INCLUDED */
