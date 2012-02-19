#!c:\python26
import fnmatch, os, string

files = os.listdir(os.curdir)

for filename in fnmatch.filter(files, "*.p6t"):
	#fn = string.split(filename, '.')
	print("rtfview " + filename + " -PIC")
	os.system("rtfview " + filename + " -PIC")
