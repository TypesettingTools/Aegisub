# $Id$

! /CURSOR|^#|^$|^\// {
    image[$1] = $1
}

END {
  print("#define static") > RES_CPP
  for (v in image) {
    printf("#include \"../bitmaps/%s_xpm.xpm\"\n", image[v]) >> RES_CPP
  }

  print("#ifndef _RES_H") > RES_H
  print("define _RES_H") >> RES_H
  for (v in image) {
    printf("extern char *%s_xpm[];\n", image[v]) >> RES_H
  }
  print("#endif /* _RES_H */") >> RES_H

}
