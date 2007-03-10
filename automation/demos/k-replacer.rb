load 'karaoke.rb'
load 'utils.rb'

include Aegisub

$script_name = "simple k-replacer"
$script_description = "k-replacer test"
$script_author = "Pomyk"
$script_version = "1"

register_macro("Simple k-replacer", "k-replacer macro", :k_replace_macro, nil)
register_filter("Simple k-replacer", "k-replacer filter", 100, :k_replace_filter, :k_replace_cfg)


def k_replace_macro(subs, sel, act)
		
	cfg = k_replace_cfg(subs, nil)
	ok, opt = display_dialog(cfg, nil)
	return if not ok	# cancelled

	write_options(subs, {$script_name => opt})
	subs.each do |l|
		k_replace(l, opt[:templ], opt[:strip]) if l[:class] == :dialogue && 	# replace if its dialogue
			(opt[:style] =="" || l[:style] == opt[:style])			# and has the right style
	end
	return subs
end

def k_replace_filter(subs, opt)
	subs.each do |l|
		k_replace(l, opt[:templ], opt[:strip]) if l[:class] == :dialogue && 	# replace if its dialogue
			opt[:style] =="" || l[:style] == opt[:style]			# and has the right style
	end
	return subs
end

def k_replace_cfg(subs, opt)
	styles = []
	subs.each { |l|		# read style names
		styles << l[:name] if l[:class] == :style
		break if l[:class] == :dialogue
	}
	header_text = <<-head
Expressions are enclosed in % pairs.
Variables:
  $start = Start-time of syllable (ms)
  $end = End-time of syllable (ms)
  $mid = Time midways through the syllable (ms)
  $dur = Duration of syllable (cs)
Calculation example:
  \\t($start,%$start+$dur*2%,\\fscx110)
  \\t(%$start+$dur*2%,$end,\\fscx90)
head
	opt ||= {}
	cfg = ScriptCfg.new		# helper class for building dialogs
	cfg.header header_text, :x => 1, :width => 1
	cfg.edit :templ, "template", :text => opt[:templ]
	cfg.dropdown :style, "Style", :items => styles, :value => opt[:style]
	cfg.checkbox :strip, "", :label => "Strip tags?", :value => (opt[:strip] == "true" ? true : false)
	cfg.to_ary # convert to array
end

