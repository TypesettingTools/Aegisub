/BITMAP/ {
  gsub(/\"bitmaps\//, "", $3)
  gsub(/\"/, "", $3)
  image[$1] = $3
}

END {
  printf(" \
all: bmp2xpm wxicon_xpm.xpm \n \
.PHONY: all bmp2xpm \n \
CONVERT ?= convert \n \
")

  printf("wxicon_xpm.xpm: icon.ico\n	$(CONVERT) 'icon.ico[2]' wxicon_xpm.xpm\n");

  for (v in image) {
    printf("%s_xpm.xpm: %s\n", v, image[v])
    printf("	$(CONVERT) -transparent \"#c0c0c0\" %s %s_xpm.xpm\n\n", image[v], v)
  }

  printf("bmp2xpm:")
  for (v in image)
    printf(" %s_xpm.xpm", v)
}
