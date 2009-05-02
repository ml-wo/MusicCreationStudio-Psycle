#! /usr/bin/env python

# This is a program which will poll a (remote) Hg repository, looking for
# new revisions. It then uses the 'buildbot sendchange' command to deliver
# information about the Change to a (remote) buildmaster. It can be run from
# a cron job on a periodic basis, or can be told (with the 'watch' option) to
# automatically repeat its check every 10 minutes.

# This script does not store any state information, so to avoid spurious
# changes you must use the 'watch' option and let it run forever.

# You will need to provide it with the location of the buildmaster's
# PBChangeSource port (in the form hostname:portnum), and the svnurl of the
# repository to watch.

# 2006.03.15 by John Pye
# 2006.03.29 by Niklaus Giger, added support to run under windows, added invocation option
# 2006.08.23 by Johan Boule, added support for multiple commits in the log
# 2007.07.07 this file is deprecated since buildbot 0.7.5 has a buildbot.changes.svnpoller.SVNPoller class
# 2008.07.28 by Johan Boule, adapted to mercurial

import commands, xml.dom.minidom, sys, time, os
if sys.platform == 'win32': import win32pipe

def check_changes(repo, master, verbose=False, old_revision = -1):
	template="'" + '<logentry revision="{rev}"><author>{author|user}</author>\\n<msg>\\n{desc}\\n</msg>\\n<paths>{files}<paths></logentry>\\n' + "'"
	if old_revision >= 0: revision_range = str(old_revision + 1) + ':tip'
	else: revision_range = 'tip'
	cmd = 'hg incoming --noninteractive --template ' + template + ' --rev ' + revision_range + ' ' + repo
	if verbose: print "Getting last revision of repository: " + repo
	if sys.platform == 'win32':
		f = win32pipe.popen(cmd)
		xml1 = ''.join(f.readlines())
		f.close()
	else: xml1 = commands.getoutput(cmd)
	if verbose: print "XML\n-----------\n" + xml1 + "\n\n"
	doc = xml.dom.minidom.parseString(xml1)
	els = doc.getElementsByTagName("logentry")
	if els:
		for el in els:
			revision = int(el.getAttribute("revision"))
			author = "".join([t.data for t in el.getElementsByTagName("author")[0].childNodes])
			comments = "".join([t.data for t in el.getElementsByTagName("msg")[0].childNodes])
			paths = []
			for p in el.getElementsByTagName("paths")[0]: paths.append("".join([t.data for t in p.childNodes]))
			if verbose: print "PATHS" ; print paths
			cmd = 'buildbot sendchange --master=' + master + ' --revision="' + str(revision) + '" --username="' + author + '" --comments="' + comments + '" ' + ' '.join(paths)
			if True or verbose: print time.strftime("%H.%M.%S ") + cmd
			if sys.platform == 'win32':
				f = win32pipe.popen(cmd)
				print time.strftime("%H.%M.%S ") + "Revision " + revision + ": " + ''.join(f.readlines())
				f.close()
			else: xml1 = commands.getoutput(cmd)  
	else:
		if True or verbose: print time.strftime("%H.%M.%S ") + "nothing has changed since revision " + str(old_revision)
	return revision

if __name__ == '__main__':
	if len(sys.argv) == 4 and sys.argv[3] == 'watch':
		old_revision = -1
		print "Watching for changes in repo "+  sys.argv[1] + " master " +  sys.argv[2] 
		while True:
			try: old_revision = check_changes(sys.argv[1], sys.argv[2], True, old_revision)
			except: pass
			time.sleep(5*60)
	elif len(sys.argv) == 3: check_changes(sys.argv[1], sys.argv[2], True )
	else: print os.path.basename(sys.argv[0]) + ":  http://host/path/to/repo master:port [watch]"