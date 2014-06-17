include $(dir $(lastword $(MAKEFILE_LIST)))../../header.mk

LUAJIT_DIR := $(abspath $(d)src)

$(LIBS_LUA): $(LUAJIT_DIR)/host/minilua
	$(MAKE) -C $(LUAJIT_DIR) amalg

$(LUAJIT_DIR)/host/minilua:
	$(MAKE) -C $(LUAJIT_DIR) host/minilua

clean-luajit:
	$(MAKE) -C $(LUAJIT_DIR) clean

clean: clean-luajit
distclean: clean-luajit
