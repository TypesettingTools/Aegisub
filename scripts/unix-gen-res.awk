# $Id$

! /CURSOR|^#|^$|^\// {
    image[$1] = $1
}

END {
  print("#define static") > RESRC_CPP
  for (v in image) {
    printf("#include \"../bitmaps/%s_xpm.xpm\"\n", image[v]) >> RESRC_CPP
  }

  print("#ifndef _RES_H") > RESRC_H
  print("#define _RES_H") >> RESRC_H
  for (v in image) {
    printf("extern char *%s_xpm[];\n", image[v]) >> RESRC_H
  }
  print("#endif /* _RES_H */") >> RESRC_H

}
