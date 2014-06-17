include $(dir $(lastword $(MAKEFILE_LIST)))../../header.mk

universalchardet_OBJ := $(subst .cpp,.o,$(wildcard $(d)*.cpp))
universalchardet_CPPFLAGS := -D_X86_

LIB += universalchardet

include $(TOP)Makefile.target
