
static __inline__ int
fromhex (int a) 
{
    if (a >= '0' && a <= '9') 
      return a - '0';
    else if (a >= 'a' && a <= 'f') 
      return a - 'a' + 10;
    else if (a >= 'A' && a <= 'F') 
      return a - 'A' + 10;
    else    
      error ("Reply contains invalid hex digit %d", a);
}

static __inline__ int
hex2bin (const char *hex, char *bin, int count)
{
    int i;  

    for (i = 0; i < count; i++)
    {
        if (hex[0] == 0 || hex[1] == 0)
        {
          /* Hex string is short, or of uneven length.
             Return the count that has been converted so far. */
            return i;
        }
        *bin++ = fromhex (hex[0]) * 16 + fromhex (hex[1]);
        hex += 2;
    }
    return i;
}

#define THIS_TARGET() \
	(current_q6_target ? 					\
		(strcmp(current_q6_target, TARGET_NAME) == 0)	\
		:						\
		0)


extern char *q6targetargsInfo[256];

#ifdef WIN32
#define strdupa strdup
#define sleep(n) Sleep(n*1000)
#endif
