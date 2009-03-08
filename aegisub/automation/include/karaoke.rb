module Aegisub


	# parsing karaoke line
	# should work more or less like the lua version
	# input: dialogue line with karaoke tags
	# output: number of syllables in karaoke
	def parse_karaoke(line)
		return 0 if line[:class] != :dialogue
		return line[:karaoke].size if line[:karaoke].class == Array
		karaoke = []
		time = 0
		line[:text].scan(/(?:{.*?\\(K|k[fto]?)(\d+).*?}([^{]*))|({.*?})([^{]*)/) do |k|
			if $1	# karaoke tag
				ktag = $1
				kdur = $2.to_i
				syl = Hash.new
				syl[:start_time] = time
				if ktag == 'kt'
					time = kdur*10
					syl[:duration] = 0
				else
					time += kdur*10
					syl[:duration] = kdur
				end
				syl[:end_time] = time
				syl[:tag] = ktag
				syl[:text] = $&
				syl[:text_stripped] = $3
				karaoke << syl
			else	# no karaoke - append to the last syllable
				tag = $4
				text = $5
				if not karaoke.empty?
					karaoke.last[:text] << tag << text
					karaoke.last[:text_stripped] << text if text and tag !~ /\\p\d/ # no drawings 
				end
			end
		end
		line[:karaoke] = karaoke
		return karaoke.size
	end
	
	# replaces matched pattern in the line with an evaluated template
	# input: line, template (string), strip (bool), pattern (regexp or string)
	# output: line with karaoke effect
	def k_replace(line, template, strip, pattern = /\\(:?K|k[fo]?\d+)/)  # default pattern = any karaoke tag		
		return if parse_karaoke(line) == 0

		res = ""
		t = template.gsub(/\$(start|end|dur|mid|text|i|kind)/, '_\1')
		_i = 0
		line[:karaoke].each do |s|
			_start = s[:start_time]
			_end = s[:end_time]
			_dur = s[:duration]
			_mid = _start + _dur*5
			_text = s[:text_stripped]
			_kind = s[:tag]
			ev = t.gsub(/(_(:?start|end|dur|mid|text|i|kind))/) { |m| eval($1).to_s } # evalute variables
			ev.gsub!(/\%([^%]+)\%/) { |m| eval($1).to_s } # evaluate expressions
			res << (strip ? "{" << ev << "}" << s[:text_stripped] : s[:text].gsub!(pattern, ev) )
			_i += 1
		end
		line[:text] = res
	end

end
