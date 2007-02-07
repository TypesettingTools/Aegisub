include Aegisub

class Object
    def deep_clone
        Marshal.load(Marshal.dump(self))
    end
end



module Aegisub

	class ScriptCfg
		def initialize	# constructor
			@opt = []
			@x = 0
			@y = 0
			@labels = true
			@width = 1 # TODO
			@height = 1
		end
		
	private
		def control(type, name, opt = {})
			@opt << {:class => type, :name => name, :x => @x, :y => @y, 
				:width => 1, :height => 1}.merge!(opt)
		end

		# some meta-programming :]		
		def self.create_functions(*arr)
			arr.each do |a|
				class_eval(%Q[
					def #{a.to_s}(name, text, opt = {})
						if @labels; label text, opt; @x += 1; end
						control "#{a.to_s}", name, opt
						@y += 1
						@x = 0
					end
				])
			end
		end

	public
		create_functions *[:edit, :intedit, :floatedit, :textbox, 
			:dropdown, :checkbox, :color, :coloralpha, :alpha ]

		def no_labels; @labels = false; end
	
		def label(text, opt = {})
			control :label, text, opt.merge({:label => text})
		end
		
		def header(text, opt = {})
			label text, opt.merge!({:width => 2})
			@y += 1
		end

		def to_ary	# conversion to array
			@opt
		end

	end

	# inserts line with options into the file
	def write_options(subs, name, opt, sep = "~~")
		i = 0
		while subs[i][:class] != :info do
		       	i += 1
		end
		while subs[i][:class] == :info do
			i += 1
		end
		l = {:class => :info, :key => name, :value => opt.to_a.flatten!.join(sep), :section => "[Script Info]"}
		subs = subs.insert(i, l)
	end

	# returns hash with options loaded from the subs
	def read_options(subs, name, sep = "~~")
		i = 0
		i += 1 while subs[i][:class] != :info
		i += 1 while subs[i][:class] == :info && subs[i][:key] != name
		return {} if subs[i][:class] != :info
		a = subs[i][:value].split(sep)
		h = {}
		(a.size/2).times do |j|			
			h[a[2*j].to_sym] = a[2*j+1]
		end
		return h
	end
	
end

