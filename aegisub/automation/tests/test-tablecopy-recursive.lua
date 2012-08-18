script_name = "Test table.copy_deep"
script_description = "Tests the Auto4 Lua utils.lua table.copy_deep function"
script_author = "Niels Martin Hansen"

include "utils.lua"

function test_tablecopy_deep()
	local function test_table(tab, desc)
		local l = aegisub.log
		l("--- %15s -------------\n", desc)
		l("tab.a = %d\n", tab.a)
		l("type(tab.b) = %s\n", type(tab.b))
		l("tab.b.a = %s\n", tab.b.a)
		l("tab.c==tab.b ? %d\n", (tab.c==tab.b) and 1 or 0)
		l("type(tab.b.b) = %s\n", type(tab.b.b))
		l("type(tab.d) = %s\n", type(tab.d))
		l("tab.d.a == tab.d ? %d\n", (tab.d.a==tab.d) and 1 or 0)
		l("tab.e == tab ? %d\n", (tab.e==tab) and 1 or 0)
		l("\n")
	end
	
	local orgtab = {}
	orgtab.a = 1
	orgtab.b = {}
	orgtab.b.a = "hi"
	orgtab.c = orgtab.b
	orgtab.c.b = test_table
	orgtab.d = {}
	orgtab.d.a = orgtab.d
	orgtab.e = orgtab
	test_table(orgtab, "Original table")
	
	local cpytab = table.copy_deep(orgtab)
	test_table(cpytab, "Copied table")
end

aegisub.register_macro("TEST table.copy_deep", "", test_tablecopy_deep)
