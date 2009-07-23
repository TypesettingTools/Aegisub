automationdir = $(aegisubdatadir)/automation
share_docdir = $(aegisubdocdir)

# share/
data_general= \
	include/Aegisub.pm \
	include/Aegisub/Progress.pm \
	include/Aegisub/Script.pm \
	include/Auto4Utils.pm

data_auto4_lua = \
	autoload/cleantags-autoload.lua \
	autoload/kara-templater.lua \
	autoload/karaoke-auto-leadin.lua \
	autoload/macro-1-edgeblur.lua \
	autoload/macro-2-mkfullwitdh.lua \
	include/cleantags.lua \
	include/karaskel-auto4.lua \
	include/karaskel.lua \
	include/unicode.lua \
	include/utils-auto4.lua \
	include/utils.lua


# share/doc

data_general_doc = \
	demos/raytracer-test1.ass \
	v4-docs/basic-function-interface.txt \
	v4-docs/configuration-dialogs.txt \
	v4-docs/file-streams.txt \
	v4-docs/misc.txt \
	v4-docs/overview.txt \
	v4-docs/progress-reporting.txt \
	v4-docs/subtitle-data.txt \
	v4-docs/template-scripting-ideas.ass

data_auto4_lua_doc = \
	demos/future-windy-blur.lua \
	demos/raytracer.lua


nobase_share_doc_DATA =
nobase_automation_DATA =

if HAVE_AUTOMATION
nobase_automation_DATA += $(data_general)
nobase_share_doc_DATA += $(data_general_doc)
endif

if HAVE_AUTO4_LUA
nobase_automation_DATA += $(data_auto4_lua)
nobase_share_doc_DATA += $(data_auto4_lua_doc)
endif


EXTRA_DIST = \
	$(data_general) \
	$(data_auto4_lua) \
	$(data_general_doc) \
	$(data_auto4_lua_doc)
