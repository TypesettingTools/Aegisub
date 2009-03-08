#ifndef _HTYPES_HXX_
#define _HTYPES_HXX_

#define MAXDELEN    8192

#define ROTATE_LEN   5

#define ROTATE(v,q) \
   (v) = ((v) << (q)) | (((v) >> (32 - q)) & ((1 << (q))-1));

// hentry options
#define H_OPT        (1 << 0)
#define H_OPT_ALIASM (1 << 1)
#define H_OPT_PHON   (1 << 2)

#define HENTRY_WORD(h) &(h->word)
#define HENTRY_DATA(h) (h->var ? ((h->var & H_OPT_ALIASM) ? \
    *((char **) (&(h->word) + h->blen + 1)) : &(h->word) + h->blen + 1) : NULL)
#define HENTRY_FIND(h,p) (HENTRY_DATA(h) ? strstr(HENTRY_DATA(h), p) : NULL)

// approx. number  of user defined words
#define USERWORD 1000

struct hentry
{
  unsigned char blen; // word length in bytes
  unsigned char clen; // word length in characters (different for UTF-8 enc.)
  short    alen;      // length of affix flag vector
  unsigned short * astr;  // affix flag vector
  struct   hentry * next; // next word with same hash code
  struct   hentry * next_homonym; // next homonym word (with same hash code)
  char     var;       // variable fields (only for special pronounciation yet)
  char     word;      // variable-length word (8-bit or UTF-8 encoding)
};

#endif
