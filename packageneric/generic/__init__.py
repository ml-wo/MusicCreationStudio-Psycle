#! /usr/bin/env python

# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2006 johan boule <bohan@jabber.org>
# copyright 2006 psycledelics http://psycle.pastnotecut.org

import sys, os.path, fnmatch

if False:
	SetOption('implicit_cache', True)
	SourceSignatures('MD5') ('timestamp')
	TargetSignatures('build') ('content')

#@staticmethod
def _pkg_config(context, packageneric, name, what):
	command = 'pkg-config --%s \'%s\'' % (what, name)
	#packageneric.trace('checking for ' + command + ' ... ')
	context.Message('checking for ' + command + ' ... ')
	result, output = context.TryAction(command)
	context.Result(result)
	return result, output

#@staticmethod
def _try_run(context, packageneric, description, text, language):
	#packageneric.trace('checking for ' + description + ' ... ')
	context.Message('checking for ' + description + ' ... ')
	result, output = context.TryRun(text, language)
	context.Result(result)
	return result, output
	
#@staticmethod
def _dump_environment(environment, all = False):
	if all:
		if False:
			print environment.Dump()
		else:
			dictionary = environment.Dictionary()
			keys = dictionary.keys()
			keys.sort()
			for key in keys:
				print '%s = %s' % (key, dictionary[key])
	else:
		def show(key):
			try:
				environment[key]
				if len(env[key]):
					print key, '->', environment[key], '->', environment.subst('$' + key)
				else:
					print key, '<- empty'
			except:
				pass
		show('CPPDEFINES')
		show('CPPPATH')
		show('CXXFLAGS')
		show('LIBPATH')
		show('LIBS')
		show('LINKFLAGS')

class Version:
	def __init__(self, major = 0, minor = 0, patch = 0):
		self._major = major
		self._minor = minor
		self._patch = patch

	def major(self):
		return self._major
		
	def minor(self):
		return self._minor
		
	def patch(self):
		return self._patch
		
	def __str__(self):
		return str(self._major) + '.' + str(self._minor) + '.' + str(self._patch)
	
	def __cmp__(self, other):
		result = cmp(self.major(), other.major())
		if result:
			return result
		result = cmp(self.minor(), other.minoer())
		if result:
			return result
		return cmp(self.patch(), other.patch())
	
class Packageneric:
	
	def	 environment(self):
		return self._environment
	
	def configure(self):
		return self._configure
	
	def finish_configure(self):
		self._environment = self.configure().Finish()

	def options(self):
		return self._options

	def	version(self):
		return self._version
		
	def command_line_arguments(self):
		return self._command_line_arguments
		
	def build_directory(self):
		return self._build_directory
	
	def __init__(self, command_line_arguments, Help, Configure):
		self._version = Version(0, 0)
		self.information('version ' + str(self.version()))
		
		self._command_line_arguments = command_line_arguments
		
		import SCons.Options
		self._options = SCons.Options.Options('packageneric.options', self.command_line_arguments())
		self.options().Add(SCons.Options.PathOption('packageneric__build_directory', 'directory where to build into', os.path.join('++packageneric', 'build', 'scons'), SCons.Options.PathOption.PathIsDirCreate))
		self.options().Add(SCons.Options.PathOption('packageneric__install_prefix', 'directory to install under', os.path.join('usr', 'local'), SCons.Options.PathOption.PathIsDirCreate))
		self.options().Add('packageneric__release', 'set to 1 to build for release', 0)
		
		import SCons.Environment
		self._environment = SCons.Environment.Environment(
			options = self.options(),
			CPPDEFINES = {'PACKAGENERIC__RELEASE' : '$packageneric__release'}
		)
		self.environment().EnsurePythonVersion(2, 3)
		self.environment().EnsureSConsVersion(0, 96)
		
		self.options().Save('packageneric.options', self.environment())
		
		self.environment().Help(self.options().GenerateHelpText(self.environment()))
		self._build_directory = os.path.join(self.environment()['packageneric__build_directory'], 'targets')
		self.environment().BuildDir(self.build_directory(), '.', duplicate = False)
		self.environment().CacheDir(os.path.join(self.environment()['packageneric__build_directory'], 'cache'))

		self._installation_prefix  = '$prefix'
		self._installation_etc     = '/etc'
		self._installation_bin     = '$prefix/bin'
		self._installation_lib     = '$prefix/lib'
		self._installation_include = '$prefix/include'
		self._installation_data    = '$prefix/share'
		self._installation_var     = '$prefix/var'
		if False:
			self.environment().Export('env installation_prefix')
		
		#from SCons import SConf.SCons
		#import SCons.Conftest
		#self._configure = self.environment().Configure(
		self._Configure = Configure
		self._configure = self._Configure(
			self.environment(),
			{
				'packageneric__pkg_config' : lambda context, packageneric, name, what: _pkg_config(context, packageneric, name, what),
				'packageneric__try_run' : lambda context, packageneric, description, text, language: _try_run(context, packageneric, description, text, language)
			}
		)
	
	class Person:
		def __init__(self, name, email = None):
			self._name = name
			self._email = email
			
		def name(self):
			return self._name
		
		def email(self):
			return self._email

	class Find:
		'''a forward iterator that traverses a directory tree'''
		
		def __init__(self, strip_path, path, pattern = '*'):
			self.strip_path = strip_path
			self.stack = [path]
			self.pattern = pattern
			self.files = []
			self.index = 0
			
		def __getitem__(self, index):
			while True:
				try:
					file = self.files[self.index]
					self.index = self.index + 1
				except IndexError:
					# pop next directory from stack
					self.directory = self.stack.pop()
					self.files = os.listdir(os.path.join(self.strip_path, self.directory))
					self.index = 0
				else:
					# got a filename
					path = os.path.join(self.directory, file)
					if os.path.isdir(os.path.join(self.strip_path, path)):
						self.stack.append(path)
					if fnmatch.fnmatchcase(file, self.pattern):
						return path
	
	def print_all_nodes(dirnode, level = 0):
		'''prints all the scons nodes that are children of this node, recursively.'''
		if type(dirnode)==type(''):
			dirnode=Dir(dirnode)
		dt = type(Dir('.'))
		for f in dirnode.all_children():
			if type(f) == dt:
				print "%s%s: .............." % (level * ' ', str(f))
				print_dir(f, level+2)
			print "%s%s" % (level * ' ', str(f))

	def Glob(includes = ['*'], excludes = None, dir = '.'):
		'''similar to glob.glob, except globs SCons nodes, and thus sees generated files and files from build directories.
		Basically, it sees anything SCons knows about.
		A key subtlety is that since this function operates on generated nodes as well as source nodes on the filesystem,
		it needs to be called after builders that generate files you want to include.
		
		It will return both Dir entries and File entries
		'''
		
		def fn_filter(node):
			fn = os.path.basename(str(node))
			match = False
			for include in includes:
				if fnmatch.fnmatchcase(fn, include):
					match = True
					break
			if match and not excludes is None:
				for exclude in excludes:
					if fnmatch.fnmatchcase(fn, exclude):
						match = False
						break
			return match

		def filter_nodes(where):
			children = filter(fn_filter, where.all_children(scan = False))
			nodes = []
			for f in children:
				nodes.append(gen_node(f))
			return nodes

		def gen_node(n):
			'''Checks first to see if the node is a file or a dir, then creates the appropriate node.
			(code seems redundant, if the node is a node, then shouldn't it just be left as is?)
			'''
			if type(n) in (type(''), type(u'')):
				path = n
			else:
				path = n.abspath
			if os.path.isdir(path):
				return Dir(n)
			else:
				return File(n)
		
		here = Dir(dir)
		nodes = filter_nodes(here)
		node_srcs = [n.srcnode() for n in nodes]
		src = here.srcnode()
		if src is not here:
			for s in filter_nodes(src):
				if s not in node_srcs:
					# Probably need to check if this node is a directory
					nodes.append(gen_node(os.path.join(dir, os.path.basename(str(s)))))
		return nodes

	class SourcePackage:
		def __init__(self, name = None, version = None, description = '', long_description = '', path = '.'):
			self._name = name
			if version is None:
				self._version = []
			else:
				self._version = version
			self._description= description
			self._long_description = long_description
			self._path = path
		
		def name(self):
			return self._name
			
		def version(self):
			return self._version
			
		def description(self):
			return self._description
			
		def long_description(self):
			return self._long_description
			
		def path(self):
			return self._path
		
	class File:
		def __init__(self):
			self.strip_path = None
			self.path = None
			self.install_path = None
		
	class SourceFile:
		def __init__(self, path):
			self.path = path
			self.defines = []
			self.include_path = []
		
	class ObjectFile:
		def __init__(self, source):
			self.source = source
			self.defines = source.defines
			self.include_path = source.include_path
			self.compiler_flags = None
		
	class CompilerFlags:
		class Define:
			def __init(name, value = ''):
				self.name = name
				self.value = value
		def __init__(self):
			self.defines = []
			self.include_path = []
			self.optimizations = []
			self.debugging_info = []
		
	class LinkerFlags:
		def __init_(self):
			self.library_path = []
			self.libraries = []
			self.optimizations = []
		
	def pkg_config(self, name, what):
		return self.configure().packageneric__pkg_config(self, name, what)

	def check_header(self, external_package, header, language = 'C++', optional = False):
		external_package._failed |= not self.configure().CheckHeader(header = header, language = language)
		if external_package.failed():
			if not optional:
				self.abort('could not find required header: ' + header)
			else:
				self.warning('could not find optional header: ' + header)
		return not external_package.failed()
	
	def check_library(self, external_package, library, language = 'C++', optional = False):
		external_package._failed |= not self.configure().CheckLib(library = library, language = language, autoadd = True)
		if external_package.failed():
			if not optional:
				self.abort('could not find required library: ' + library)
			else:
				self.warning('could not find optional library: ' + library)
		return not external_package.failed()

	def check_header_and_library(self, external_package, header, library, language = 'C++', optional = False):
		if False:
			external_package._failed |= not self.configure().CheckLibWithHeader(libs = library, header = header, language = language)
			if external_package.failed():
				if not optional:
					self.abort('could not find required header: ' + header)
				else:
					self.warning('could not find optional header: ' + header)
			return not external_package.failed()
		else:
			return \
				   self.check_header(external_package, header, language, optional) and \
				   self.check_library(external_package, library, language, optional)

	def try_run(self, description, text, language = '.cpp'):
		return self.configure().packageneric__try_run(self, description, text, language)
		
	def tty_font(self, font = '0', text = None):
		result = '\033[' + font + 'm'
		if text:
				result += text + self.tty_font()
		return result
	
	def message(self, message, font = None):
		prefix = __name__ + '(' + self.__class__.__name__ + '): '
		string = prefix
		for x in message:
			string += x
			if x == '\n':
					string += prefix
		if font:
			string = self.tty_font(font, string)
		print string

	def trace(self, message):
		self.message('trace: ' + message, '2;33')
		
	def information(self, message):
		self.message('information: ' + message, '34')
	
	def success(self, message):
		self.message('information: ' + message, '32')
		
	def warning(self, message):
		self.message('warning: ' + message, '35')
	
	def error(self, message):
		self.message('error: ' + message, '1;31')

	def abort(self, message):
		self.error(message + '\nbailing out.')
		sys.exit(1)
	
	class ExternalPackage:
		def __init__(
			self,
			packageneric,
			debian,
			debian_version_compare,
			pkg_config = None,
			pkg_config_version_compare = None
		):
			self._packageneric = packageneric
			self._pkg_config = pkg_config
			self._pkg_config_version_compare = pkg_config_version_compare
			self._debian = debian
			self._debian_version_compare = debian_version_compare
			self._failed = False
			
			# crap
			self._parsed = False
		
		def packageneric(self):
			return self._packageneric
		
		def debian(self):
			return self._debian
			
		def debian_version_compare(self):
			return self._debian_version_compare
			
		def pkg_config(self):
			return self._pkg_config

		def pkg_config_version_compare(self):
			return self._pkg_config_version_compare
		
		def failed(self):
			return self._failed
			
		# crap
		def env(self):
			if not self._parsed:
				self._parsed = True
				env = self.packageneric().environment().Copy()
				if not self.pkg_config() is None:
					if self.packageneric().pkg_config(self.pkg_config(), 'exists'):
						self.packageneric().pkg_config(self.pkg_config(), 'modversion')
						string = self.pkg_config()
						if not self.pkg_config_version_compare() is None:
							string += ' ' + self.pkg_config_version_compare()
						result = self.packageneric().pkg_config(string, 'exists')
			return env

		def __str__(self):
			string = ''
			if not self.pkg_config() is None:
				string += self.pkg_config()
				if not self.pkg_config_version_compare() is None:
					string += ' ' + self.pkg_config_version_compare()
				elif not self.debian_version_compare() is None:
					string += ' ' + self.debian_version_compare()
			else:
				string += self.debian()
				if not self.debian_version_compare() is None:
					string += ' ' + str(self.debian_version_compare())
			return string
		
		def show(self):
			self.packageneric().information('external package: ' + str(self))
			_dump_environment(self.env())
			
		def scons(self):
			env = self.packageneric.environment().Copy()
	
	class Module:
		class Types:
			files = 'files'
			shared_lib = 'shared_lib'
			static_lib = 'static_lib'
			bin = 'bin'
			python = 'python'
			
		def __init__(self, packageneric, name = None, version = None, description = '', public_requires = None):
			self._packageneric = packageneric
			self._name = name
			if version is None:
				self._version = []
			else:
				self._version = version
			self._description = description
			if public_requires is None:
				self._public_requires = []
			else:
				self._public_requires = public_requires
			self._sources = []
			self._headers = []
			self._include_path = []
			self._defines = {}
			
			# crap
			self._parsed = False
		
		def packageneric(self):
			return self._packageneric
			
		def name(self):
			return self._name
		
		def version(self):
			return self._version
		
		def description(self):
			return self._description
			
		def sources(self):
			return self._sources
		def add_source(self, source):
			self.sources().append(os.path.join(self.packageneric().build_directory(), source))
		def add_sources(self, sources):
			for x in sources: self.add_source(x)
			
		def headers(self):
			return self._headers
		def add_header(self, header):
			self.headers().append(header)
		def add_headers(self, headers):
			for x in headers: self.add_header(x)
			
		def include_path(self):
			return self._include_path
		def add_include_path(self, path):
			self._include_path.append(path)
		
		def defines(self):
			return self._defines
		def add_define(self, name, value):
			self._defines.append({name: value})
			
		def public_requires(self):
			return self._public_requires
		def add_public_requires(self, requires):
			self.public_requires().append(requires)
			
		# crap
		def environment(self):
			if not self._parsed:
				self._parsed = True
				public_requires = ''
				for x in self.public_requires():
					public_requires += ' ' + str(x)
				debian = ''
				debian_version_compare = ''
				self._environment = self.packageneric().environment().Copy()
			return self._environment
		
		def show(self):
			self.packageneric().information('module: ' + self.name() + ' ' + str(self.version()))
			public_requires = []
			for x in self.public_requires():
				public_requires.append(str(x))
			self.packageneric().information('module: requires: ' + str(public_requires))
			_dump_environment(self.environment())
			if False:
				self.packageneric().trace(str(self.sources()))
				self.packageneric().trace('========')
				self.packageneric().trace(str(self.headers()))
			
		def scons(self):
			env = self.packageneric().environment().Copy()
			env.Append(
				CPPPATH = self.include_path(),
				CPPDEFINES = self.defines()
			)
			env.Append(
				CPPDEFINES = {
					'PACKAGENERIC': '\\"/dev/null\\"',
					'PACKAGENERIC__PACKAGE__NAME': '\\"test\\"',
					'PACKAGENERIC__PACKAGE__VERSION': '\\"0\\"',
					'PACKAGENERIC__PACKAGE__VERSION__MAJOR': '0',
					'PACKAGENERIC__PACKAGE__VERSION__MINOR': '0',
					'PACKAGENERIC__PACKAGE__VERSION__PATCH': '0',
					'PACKAGENERIC__MODULE__NAME': '\\"test\\"',
					'PACKAGENERIC__MODULE__VERSION': '\\"0\\"',
					'PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_LIB': '\\"../lib\\"',
					'PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_SHARE': '\\"../share\\"',
					'PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_VAR': '\\"../var\\"',
					'PACKAGENERIC__CONFIGURATION__INSTALL_PATH__BIN_TO_ETC': '\\"../../etc\\"',
					'PACKAGENERIC__CONFIGURATION__COMPILER__HOST': '\\"test\\"'
				}
			)
			pkg_config = ''
			for x in self.public_requires():
				if not x.pkg_config() is None:
					pkg_config += ' ' + x.pkg_config()
					if not x.pkg_config_version_compare() is None:
						pkg_config += ' ' + x.pkg_config_version_compare()
			if not pkg_config == '':
				env.ParseConfig('pkg-config --cflags --libs \'' + pkg_config + '\'')
			_dump_environment(env)
			return env.SharedLibrary(os.path.join(self.packageneric().build_directory(), self.name()), self.sources())

	def module(self, name = None, version = None, description = '', public_requires = None):
		return Packageneric.Module(self, name, version, description, public_requires)
		
	class PkgConfigPackage:
		def __init__(self, name = None, version = None, description = '', modules = None):
			self._name = name
			if version is None:
				self._version = []
			else:
				self._version = version
			self._description = description
			if modules is None:
				self._modules = []
			else:
				self._modules = modules
				
		def name(self):
			return self._name
			
		def version(self):
			return self._version
		
		def description(self):
			return self._description
			
		def modules(self):
			return self._modules

	class DebianPackage:
		def __init__(
			self,
			source_package = None,
			name = None,
			section = None,
			architecture = 'any',
			description = '',
			long_description = ''
		):
			self._source_package = source_package
			self._name = name
			if section is None:
				self._section = self.source_package().section()
			else:
				self._section = section
			self._architecture = architecture
			self._provides = []
			self._depends = []
			self._recommends = []
			self._suggests = []
			self._description = description
			self._long_description = long_description
			self._files = []
			
		def source_package(self):
			return self._source_package
			
		def name(self):
			return self._name
			
		def section(self):
			return self._section
			
		def architecture(self):
			return self._architecture
		
		def provides(self):
			return self._provides
		
		def depends(self):
			return self._depends
		
		def recommends(self):
			return self._recommends
			
		def suggests(self):
			return self._suggests
		
		def description(self):
			return self._description
			
		def long_description(self):
			return self._long_description

		def	build_depends(self):
			class Depend:
				def __init__(self, name, version_compare = None):
					self._name = name
					self._version_compare = version_compare
					
				def name(self):
					return self._name
					
				def version_compare(self):
					return self._version_compare
			
			result = []
			for x in self.depends():
				result.append(Depend(x.name(), x.debian_version_compare()))
			return result
		
	class Debian:
		def __init__(
			self,
			source_package = None,
			section = 'libs',
			priority = 'optional',
			maintainer = '',
			uploaders = None,
			description = None,
			long_description = None,
			binary_packages = None,
			build_depends = None
		):
			self._source_package = source_package
			self._section = section
			self._priority = priority
			self._maintainer = maintainer
			if uploaders is None:
				self._uploaders = []
			else:
				self._uploaders = uploaders
			if description is None and not source_package is None:
				self._description = source_package.description
			else:
				self._description = description
			if long_description is None and not source_package is None:
				self._long_description = source_package.long_description
			else:
				self._description = description
			if binary_packages is None:
				self._binary_packages = []
			else:
				self._binary_packages = binary_packages
			if build_depends is None:
				self._build_depends = []
			else:
				self._build_depends = build_depends

		def source_package(self):
			return self._source_package
			
		def section(self):
			return self._section
			
		def priority(self):
			return self._priority
			
		def maintainer(self):
			return self._maintainer
			
		def uploaders(self):
			return self._uploaders
			
		def description(self):
			return self._description
			
		def long_description(self):
			return self._long_description
			
		def binary_packages(self):
			return self._binary_packages
			
		def	build_depends(self):
			result = self._build_depends
			for x in self.binary_packages():
				for xx in x.build_depends():
					if not xx in self._build_depends:
						result.append(xx)
			return result
		
		def control(self):
			string = ''
			string += 'Source: ' + self.source_package().name() + '\n'
			string += 'Section: ' + self.section() + '\n'
			string += 'Priority: ' + self.priority() + '\n'
			string += 'Build-Depends: scons'
			for x in self.build_depends():
				string += x.name() + ' (' + x.version_compare() + '), '
			string += '\n'
			if not self.maintainer() is None:
				string += 'Maintainer: ' + self.maintainer().name() + ' <' + self.maintainer().email() + '>\n'
			if len(self.uploaders()):
				string += 'Uploaders: '
				for x in self.uploaders():
					string += x.name() + ' <' + x.email() + '>, '
				string += '\n'
			string += 'Standards-Version: 3.6.2\n'
			for x in self.binary_packages():
				string += '\n'
				string += 'Package: ' + x.name() + '\n'
				if len(x.provides()):
					string += 'Provides: '
					for xx in x.provides():
						string += xx + ', '
					string += '\n'
				if len(x.recommends()):
					string += 'Recommends: '
					for xx in x.recommends():
						string += xx.name() + ' (' + xx.version_compare(), '), '
					string += '\n'
				if len(x.suggests()):
					string += 'Suggests: '
					for xx in x.suggests():
						string += xx.name() + ' (' + xx.version_compare() + '), '
					string += '\n'
				string += 'Depends: ${shlibs:Depends}, ${misc:Depends}'
				for xx in x.depends():
					if isinstance(x, Packageneric.ExternalPackage):
						string += xx.name() + ' (' + xx.version_compare() + '), '
				string += '\n'
				string += 'Section: '
				if x.section() is None:
					string += self.section()
				else:
					string += x.section()
				string += '\n'
				string += 'Architecture: ' + x.architecture() + '\n'
				string += 'Description: ' + x.description() + '\n '
				description = self.long_description() + '\n\n' + x.long_description()
				was_new_line = False
				for xx in description:
					if xx == '\n':
						if was_new_line:
							string += '.'
						was_new_line = True
						string += '\n '
					else:
						was_new_line = False
						string += xx
				string += '\n'
			return string
			
	class DistributionArchive:
		def __init__(self):
			self._remote_path = None
			self._source_packages = []
			self._binary_packages = []
			
		def remote_path(self):
			return self._remote_path
			
		def source_packages(self):
			return self._source_packages
			
		def binary_packages(self):
			return self._binary_packages
