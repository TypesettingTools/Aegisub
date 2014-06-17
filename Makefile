include header.mk

ifeq (yes, $(BUILD_DARWIN))
osx-bundle:
	$(BIN_SHELL) tools/osx-bundle.sh "$(AEGISUB_COMMAND)" "$(BIN_WX_CONFIG)" DICT_DIR=$(DICT_DIR)

osx-dmg: osx-bundle
	codesign -s 'Mac Developer' --deep Aegisub.app || true
	$(BIN_SHELL) tools/osx-dmg.sh "$(BUILD_VERSION_STRING)"
endif

include Makefile.target
