# rss_buffer - Really simple string buffer

  Following implements simple string buffer interface.
  Buffer should be created with initial size. Buffer
  size 'inflates' when there is no room for new data.
  'Inflates' means simple algorithm of calculating
  new buffer size by doubling current buffer size until
  it fits new data. Reallocation happens once.
 
  Best suited for mid sized string collections.
  For smaller strings use char[].
  It probably can handle bigger strings as well, but
  for 4gb string sudden growth to 8gb can screw you up.
 
  With initial size choosen correctly, this buffer
  should be really efficient. It probably should be
  power of 2.


It was created for personal use, but might also be helpful or come as an inspiration to you.

My stream of consciousness about this:
- You can only append strings, no mid insertions.
- You can't trim or remove anything.
- rssbuf_sprintf is really handy and i like it a lot
- *There are probably some bugs.*

**Example :**
``` C
#include <stdio.h>

#include "rss_buffer.h"

void *free_and_exit(rss_buffer *buf)
{
    if (buf)
        rssbuf_free(&buf);
    return NULL;
}

rss_buffer *construct_xml_from_data(const char *name, float weight,
                            int count, int data[], int data_count,
                            const char *address)
{
    rss_buffer *buf = rssbuf_crt(128);
    if (!buf)
        return NULL;
        
    if (!rssbuf_append_str(buf, "<Root>\n")) return free_and_exit(buf);

    if (!rssbuf_sprintf(buf, "<Name>%s</Name>\n", name)) return free_and_exit(buf);

    if (!rssbuf_sprintf(buf, "<Info>You have %i items, %.3f each</Info>\n",
                        count, weight)) return free_and_exit(buf);

    if (!rssbuf_append_str(buf, "<SomeData>\n")) return free_and_exit(buf);
    for (int i = 0; i < data_count; ++i)
    {
        if (!rssbuf_sprintf(buf, "\t<DataPiece>%i</DataPiece>\n", data[i])) return free_and_exit(buf);
    }
    if (!rssbuf_append_str(buf, "</SomeData>\n")) return free_and_exit(buf);

    if (!rssbuf_sprintf(buf, "<Pickup>%s</Pickup>\n", address)) return free_and_exit(buf);
    
    if (!rssbuf_append_str(buf, "</Root>\n")) return free_and_exit(buf);

    return buf;
}
   
int main()
{
    int data[] = {50, 47, 47, 94, 15};
    rss_buffer *buf = construct_xml_from_data("My Name", 1.568f, 3,
                                              data, 5, "Happy str. Wonderful town, YCA");

    if (!buf)
    {
        printf("Oops");
        return 1;
    }

    printf("Your xml:\n%s", rssbuf_get_data(buf));
    rssbuf_free(&buf);
    return 0;
}
```

## Output:
```
Your xml:
<Root>
<Name>My Name</Name>
<Info>You have 3 items, 1.568 each</Info>
<SomeData>
	<DataPiece>50</DataPiece>
	<DataPiece>47</DataPiece>
	<DataPiece>47</DataPiece>
	<DataPiece>94</DataPiece>
	<DataPiece>15</DataPiece>
</SomeData>
<Pickup>Happy str. Wonderful town, YCA</Pickup>
</Root>
```
