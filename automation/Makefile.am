automationdir = $(aegisubdatadir)/automation

data_general= \
	automation3.txt \
	demos/raytracer-test1.ass \
	include/Aegisub.pm \
	include/Aegisub/Progress.pm \
	include/Aegisub/Script.pm \
	include/Auto4Utils.pm \
	include/readme.txt \
	v4-docs/basic-function-interface.txt \
	v4-docs/configuration-dialogs.txt \
	v4-docs/file-streams.txt \
	v4-docs/misc.txt \
	v4-docs/overview.txt \
	v4-docs/progress-reporting.txt \
	v4-docs/subtitle-data.txt \
	v4-docs/template-scripting-ideas.ass


data_auto4_lua = \
	autoload/cleantags-autoload.lua \
	autoload/kara-templater.lua \
	autoload/karaoke-auto-leadin.lua \
	autoload/macro-1-edgeblur.lua \
	autoload/macro-2-mkfullwitdh.lua \
	demos/future-windy-blur.lua \
	demos/raytracer.lua \
	include/cleantags.lua \
	include/karaskel-adv.lua \
	include/karaskel-auto4.lua \
	include/karaskel-base.lua \
	include/karaskel.lua \
	include/unicode.lua \
	include/utils-auto4.lua \
	include/utils.lua

data_auto3_lua = \
	auto3/readme.txt \
	auto3/line-per-syllable.auto3 \
	auto3/multi-template.auto3 \
	auto3/simple-k-replacer.auto3 \
	demos/auto3/readme.txt \
	demos/auto3/1-minimal.lua \
	demos/auto3/10-furigana.ass \
	demos/auto3/10-furigana.lua \
	demos/auto3/2-dump.lua \
	demos/auto3/3-include.lua \
	demos/auto3/4-text_extents.lua \
	demos/auto3/5-configuration.lua \
	demos/auto3/6-simple-effect.lua \
	demos/auto3/7-advanced-effect.lua \
	demos/auto3/8-skeleton.lua \
	demos/auto3/9-advanced-skeleton.lua \
	include/karaskel-adv.auto3 \
	include/karaskel-base.auto3 \
	include/karaskel.auto3 \
	include/utils.auto3

data_auto4_perl = \
	autoload/macro-1p-edgeblur.pl \
	demos/perl-console.pl \
	include/Aegisub/PerlConsole.pm \
	v4-docs/perl-api.txt


data_auto4_ruby = \
	demos/k-replacer.rb \
	include/karaoke.rb \
	include/utils.rb

nobase_automation_DATA =

if HAVE_AUTOMATION
nobase_automation_DATA += $(data_general)
endif

if HAVE_AUTO4_LUA
nobase_automation_DATA += $(data_auto4_lua)
endif

if HAVE_AUTO3_LUA
nobase_automation_DATA += $(data_auto3_lua)
endif

if HAVE_AUTO4_PERL
nobase_automation_DATA += $(data_auto4_perl)
endif

if HAVE_AUTO4_RUBY
nobase_automation_DATA += $(data_auto4_ruby)
endif


EXTRA_DIST = \
	$(data_general) \
	$(data_auto4_lua) \
	$(data_auto3_lua) \
	$(data_auto4_perl) \
	$(data_auto4_ruby)
