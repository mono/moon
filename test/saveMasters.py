#!/usr/bin/python

##################
# Important: This script is to be run from trunk/moon/test in WINDOWS
# This script grabs all the rendered pngs and tifs created by Silverlight 	
#    and renames them as masters for Moonlight testing
###############


import sys,os

files = os.listdir(os.path.join(os.getcwd(),'xaml'))

for curfile in files:
	newfile = ''
	if curfile.endswith('.xaml.png'):
		newfile = curfile[:-9] + 'Master.png'
		
	if curfile.endswith('.xaml.tif'):
		newfile = curfile[:-9] + 'Master.tif'
	
	if newfile != '':
		curpath = os.path.join('xaml',curfile)
		newpath = os.path.join('harness','masters',newfile)

		if os.path.exists(newpath): # Delete existing master
			#print 'Deleting master %s' % newpath
			print 'Master exists at %s' % newpath
			#os.remove(newpath)
		else:
			print 'Moving %s to %s' % (curpath, newpath)
			os.rename(curpath, newpath)
	
