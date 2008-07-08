#!/usr/bin/python
import xml.dom.minidom
import getopt
import sys,os

##########################
# Important: This script is to be run from trunk/moon/test
# This file converts a Moonlight DRTList.xml to the MS DRTList.txt
##########################

class drtTest:
	def __init__(self,id):
		self.id = id
		self.feature = 'unknown'
		self.owner = 'moonlight'
		
	def _tif_or_png(self):
		filename = self.master10.split('/')[-1] # mytest.xaml
		
		filename = filename.split('.')[0] # mytest
		
		if self.feature == "Animation":
			filename += ".tif"
		else:
			filename += ".png"
		self.master10 = "../harness/masters/%s" % filename
			
		
	def toBadXML(self):
		self._tif_or_png()
		x = '\t<Test id="%s">\n' % (self.id)
		x += '\t\t<inputFile="%s"/>\n' % self.input
		x += '\t\t<masterFile10="%s"/>\n' % self.master10
		if self.master11 != None and self.master11 != '':
			x += '\t\t<masterFile11="%s"/>\n' % self.master11
		x += '\t\t<owner="%s"/>\n' % self.owner
		x += '\t\t<featureName="%s"/>\n' % self.feature
		x += '\t</Test>'
		return x

def usage():
	print "\nUsage: convert-drt.py [--missing] [drtlist]\n"
	print " --missing 	Creates a drtlist.txt of only test with missing master files"

def main():

	# saveMasters.py --missing, --regen

	regen = False
	missing = False

	try:
		shortopts = 'hm'
		longopts = ['help','missing',]
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

	filename = "drtlist.xml"
	if len(args) > 0:
		filename = args[0]


			
	filename = os.path.join("xaml",filename)

	print "drtlist = %s" % filename

	doc = xml.dom.minidom.parse("xaml/drtlist.xml")
	drtnode = doc.childNodes[0]

	print "<DRTList>"

	for testnode in drtnode.childNodes:
		if ((testnode.localName is not None) and (testnode.localName != "")):
			#print'\t<Test id="%s">' % (testnode.getAttribute("id"))
			
			#if os.path.exists():

			test = drtTest(testnode.getAttribute("id"))
			test.input = testnode.getAttribute("inputFile")
			test.master10 = testnode.getAttribute('masterFile10')
			test.master11 = testnode.getAttribute('masterFile11')

			if testnode.getAttribute('inputFile').find('animation') != -1:
				test.feature = "Animation"
			elif testnode.getAttribute('featureName') != '':
				test.feature = testnode.getAttribute('featureName')

			#print '\t\t<featureName="%s"/>' % feature

			# Added these attributes if non-existent
			if testnode.getAttribute('owner') != '':
				test.owner = testnode.getAttribute('owner')


			print test.toBadXML()
			#print "\t</Test>"	

	print "</DRTList>"	


if __name__ == '__main__':
	main()
