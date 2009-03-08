
import shutil
import os

destpath = "../locale"
os.mkdir(destpath)

for lang in file("LINGUAS").readlines():
	lang = lang.strip()
	langpath = destpath + "/" + lang
	os.mkdir(langpath)
	shutil.copyfile(lang + ".po", langpath + "/aegisub.po")
	if os.access("wxstd-" + lang + ".mo", os.R_OK):
		shutil.copyfile("wxstd-" + lang + ".mo", langpath + "/wxstd.mo")
	if os.access(lang + ".gmo", os.R_OK):
		shutil.copyfile(lang + ".gmo", langpath + "/aegisub.mo")
	else:
		shutil.copyfile(lang + ".mo", langpath + "/aegisub.mo")

