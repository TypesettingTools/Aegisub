/BITMAP/ {
  gsub(/\"bitmaps\//, "", $3)
  gsub(/\"/, "", $3)
  image[$1] = $3
}

END {
  printf(" \
all: bmp2xpm \n \
.PHONY: all bmp2xpm \n \
CONVERT ?= %s \n \
", BIN_CONVERT)

# Yes, the 'sed' hack is annoying, if anyone wants to know why it's nessicary
# I direct them to coders/xpm.c:734 in ImageMagick 6.5.1
  for (v in image) {
    printf("%s_xpm.xpm: %s\n", v, image[v])
    printf("	$(CONVERT) -transparent \"#c0c0c0\" %s xpm:- | sed \"2 s/^static char \\*xpm__\\[\\] =/const char \\*"v"_xpm\\[\\] =/\" > %s_xpm.xpm\n\n", image[v], v)
  }

  printf("bmp2xpm:")
  for (v in image)
    printf(" %s_xpm.xpm", v)
}
