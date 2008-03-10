#!/usr/bin/python

##################
# Important: This script is to be run from trunk/moon/test in WINDOWS
# This script grabs all the rendered pngs and tifs created by Silverlight 	
#    and renames them as masters for Moonlight testing
###############


import sys,os
import getopt

def usage():
	print "\nUsage: saveMasters.py [--missing, --regen]\n"

def main():
	
	# saveMasters.py --missing, --regen
	
	regen = False
	missing = False
	
	try:
		shortopts = 'hmr'
		longopts = ['help','missing','regen']
		opts, args = getopt.getopt(sys.argv[1:],shortopts, longopts)
	except getopt.GetoptError, err:
		print str(err)
		sys.exit(1)
	
		
	for o, a in opts:
		if o in ('-h','--help'):
			usage()
			return
		if o in ('-m','--missing'):
			missing = True
		if o in ('-r','--regen'):
			regen = True
			
	if not (missing or regen):
		usage()
		sys.exit(1)
		
	files = os.listdir(os.path.join(os.getcwd(),'xaml'))
	
	new_masters_count = 0
	new_masters = []

	for curfile in files:
		newfile = ''
		testname = curfile[:-9]
		if curfile.endswith('.xaml.png'):
			newfile = curfile[:-9] + 'Master.png'

		if curfile.endswith('.xaml.tif'):
			newfile = curfile[:-9] + 'Master.tif'

		if newfile != '':
			curpath = os.path.join('xaml',curfile)
			newpath = os.path.join('harness','masters',newfile)

			if regen:
				if os.path.exists(newpath): 
					print 'Deleting master for %s' % testname
					os.remove(newpath)
				new_masters.append(testname)
				#print 'Moving %s to %s' % (curpath, newpath)
				os.rename(curpath, newpath)
					
			else: # missing only - not regen
				if os.path.exists(newpath):
					pass
					#print 'Master exists at %s' % newpath
				else:		
					new_masters.append(testname)
					#new_masters_count += 1
					#print 'Moving %s to %s' % (curpath, newpath)
					os.rename(curpath, newpath)
	
	print "\n%s new masters found" % len(new_masters)
	for testname in new_masters:
		print "\t%s" % testname
	

if __name__ == '__main__':
	main()