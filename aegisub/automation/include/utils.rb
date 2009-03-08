#include Aegisub

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

	# inserts lines with options into [Script Info] section
	def write_options(subs, opt, sep = "~~")
		subs.collect! do |l|
			if l[:class] == :info
				info = true
				value = opt.delete(l[:key])
				l[:value] = value.instance_of?(Hash) ? value.to_a.flatten!.join(sep) : value.to_s if value
				l
			elsif info
				r = [l]
				opt.each do |key, val|
					r << {:class => :info, :key => key, 
					:value => value.instance_of?(Hash) ? value.to_a.flatten!.join(sep) : value.to_s,
					:section => "[Script Info]"}
				end
				info = false
				r
			else
				l			
			end
		end	
	end

	# returns a hash with options from [Script Info] section	
	def read_options(subs, name, sep = "~~")		
		opt = {}
		subs.each { |l| opt[l[:key].to_sym] = l[:value] if l[:class] == :info }
		n_sym = name.to_sym
		if opt[n_sym]	# parsing of script specific options
			a = opt[n_sym].split(sep)
			h = {}
			(a.size/2).times { |j|	h[a[2*j].to_sym] = a[2*j+1] }
			opt[n_sym] = h
		end
		return opt
	end

	def rgb_to_ssa(*c)
		res = "&H"
		c.reverse_each {|v| res << "%02X" % v}
		res << "&"
		return res
	end

	def ssa_to_rgb(col)
		res = []
		col.scan(/[0-9a-fA-F]{2}/) { res.unshift $1.to_i(16) }
		res
	end
end

