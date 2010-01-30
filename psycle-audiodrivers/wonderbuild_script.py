#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2009-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

if __name__ == '__main__':
	import sys, os
	dir = os.path.dirname(__file__)
	sys.argv.append('--src-dir=' + dir)
	try: from wonderbuild.main import main
	except ImportError:
		dir = os.path.abspath(os.path.join(dir, os.pardir, 'build-systems', 'wonderbuild'))
		if dir not in sys.path: sys.path.append(dir)
		try: from wonderbuild.main import main
		except ImportError:
			print >> sys.stderr, 'could not find wonderbuild'
			sys.exit(1)
		else: main()
	else: main()

from wonderbuild.script import ScriptTask, ScriptLoaderTask

class Wonderbuild(ScriptTask):
	@property
	def common(self): return self._common

	@property
	def mod_dep_phases(self): return self._mod_dep_phases

	def __call__(self, sched_ctx):
		project = self.project
		top_src_dir = self.src_dir.parent
		src_dir = self.src_dir / 'src'

		helpers = ScriptLoaderTask.shared(project, top_src_dir / 'psycle-helpers')
		for x in sched_ctx.parallel_wait(helpers): yield x
		helpers = helpers.script_task
		self._common = common = helpers.common
		helpers = helpers.mod_dep_phases
		pch = common.pch
		cfg = common.cfg.clone()

		from wonderbuild.cxx_tool_chain import PkgConfigCheckTask, ModTask
		from wonderbuild.std_checks.winmm import WinMMCheckTask
		from wonderbuild.std_checks.dsound import DSoundCheckTask
		from wonderbuild.install import InstallTask

		check_cfg = cfg.clone()
		gstreamer = PkgConfigCheckTask.shared(check_cfg, ['gstreamer-0.10'])
		jack = PkgConfigCheckTask.shared(check_cfg, ['jack >= 0.101.1'])
		alsa = PkgConfigCheckTask.shared(check_cfg, ['alsa >= 1.0'])
		esound = PkgConfigCheckTask.shared(check_cfg, ['esound'])
		dsound = DSoundCheckTask.shared(check_cfg)
		winmm = WinMMCheckTask.shared(check_cfg)

		for x in sched_ctx.parallel_wait(
			gstreamer, jack, alsa, esound, dsound, winmm
		): yield x

		class UniformMod(ModTask):
			def __init__(self, name, path, deps=None, kind=ModTask.Kinds.LOADABLE):
				ModTask.__init__(self, name, kind, cfg, (name, 'default'))
				self.path = path
				if deps is not None: self.public_deps += deps

			def __call__(self, sched_ctx):
				if self.kind == ModTask.Kinds.PROG: self.private_deps = [pch.prog_task]
				else: self.private_deps = [pch.lib_task]
				self.public_deps += [helpers]
				req = self.all_deps
				for x in sched_ctx.parallel_wait(*req): yield x
				self.result = min(bool(r) for r in req)
				self.cxx_phase = self.__class__.InstallHeaders(self)
				for x in ModTask.__call__(self, sched_ctx): yield x
		
			def do_mod_phase(self):
				self.cfg.include_paths.appendleft(src_dir)
				self.cfg.defines['UNIVERSALIS__META__MODULE__NAME'] = '"' + self.name +'"'
				self.cfg.defines['UNIVERSALIS__META__MODULE__VERSION'] = 0
				if self.path.exists:
					for s in self.path.find_iter(in_pats = ('*.cpp',), prune_pats = ('todo',)): self.sources.append(s)
				else: self.sources.append(self.path.parent / (self.path.name + '.cpp'))

			def apply_cxx_to(self, cfg):
				if not self.cxx_phase.dest_dir in cfg.include_paths: cfg.include_paths.append(self.cxx_phase.dest_dir)
				ModTask.apply_cxx_to(self, cfg)

			class InstallHeaders(InstallTask):
				def __init__(self, outer):
					InstallTask.__init__(self, outer.project, outer.name + '-headers')
					self.outer = outer
				
				@property
				def trim_prefix(self): return src_dir

				@property
				def dest_dir(self): return self.fhs.include

				@property
				def sources(self):
					try: return self._sources
					except AttributeError:
						self._sources = []
						if self.outer.path.exists:
							for s in self.outer.path.find_iter(in_pats = ('*.hpp', '*.h'),
								ex_pats = ('*.private.hpp',), prune_pats = ('todo',)): self._sources.append(s)
						for h in ('.hpp', '.h'):
							f = self.outer.path.parent / (self.outer.path.name + h)
							if f.exists:
								self._sources.append(f)
								break
						return self._sources
		
		class AudioDriverMod(UniformMod):
			def __init__(self, name, path, deps=None, define_name=None):
				UniformMod.__init__(self,
					'psycle-audiodriver-' + name,
					src_dir / 'psycle' / 'audiodrivers' / path,
					deps
				)
				self.define_name = define_name is not None and define_name or name.replace('-', '_').upper()
			
			def apply_defines_to(self, cfg):
				cfg.defines['PSYCLE__' + self.define_name + '_AVAILABLE'] = None
		
			def do_mod_phase(self):
				self.apply_defines_to(self.cfg)
				UniformMod.do_mod_phase(self)
				
			def apply_cxx_to(self, cfg): self.apply_defines_to(cfg)

		if gstreamer: gst_driver = AudioDriverMod('gstreamer', 'gstreamerout', deps=(gstreamer,))
		if alsa: alsa_driver = AudioDriverMod('alsa', 'alsaout', deps=(alsa,))
		if esound: esound_driver = AudioDriverMod('esound', 'esoundout', deps=(esound,))
		if dsound: dsound_driver = AudioDriverMod('microsoft-direct-sound', 'microsoftdirectsoundout', deps=(dsound,))
		file_driver = AudioDriverMod('wave-file', 'wavefileout')
		if False: # these drivers need testing
			if jack: jack_driver = AudioDriverMod('jack', 'jackout', deps=(jack,))
			if winmm: winmm_driver = AudioDriverMod('microsoft-mme', 'microsoftmmewaveout', deps=(winmm,))
			# netaudio
			# asio

		class AudioDriversMod(ModTask):
			def __init__(self):
				name = 'psycle-audiodrivers'
				ModTask.__init__(self, name, ModTask.Kinds.LIB, cfg, (name, 'default'))

			def __call__(self, sched_ctx):
				self.private_deps = [pch.lib_task]
				self.public_deps = [helpers]
				req = self.public_deps + self.private_deps
				opt = [alsa, jack, esound, dsound, winmm, gstreamer] # netaudio, asio
				for x in sched_ctx.parallel_wait(*(req + opt)): yield x
				self.result = min(bool(r) for r in req)
				self.public_deps += [o for o in opt if o]
				self.cxx_phase = AudioDriversMod.InstallHeaders(self.project, self.name + '-headers')
				for x in ModTask.__call__(self, sched_ctx): yield x

			def _apply_defines(self, cfg):
				d = cfg.defines
				if gstreamer: d['PSYCLE__GSTREAMER_AVAILABLE'] = None
				if alsa:      d['PSYCLE__ALSA_AVAILABLE'] = None
				if esound:    d['PSYCLE__ESOUND_AVAILABLE'] = None
				if dsound:    d['PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE'] = None
				if False: # these drivers need testing
					if jack:      d['PSYCLE__JACK_AVAILABLE'] = None
					if netaudio:  d['PSYCLE__NET_AUDIO_AVAILABLE'] = None
					if asio:      d['PSYCLE__STEINBERG_ASIO_AVAILABLE'] = None
					if winmm:     d['PSYCLE__MICROSOFT_MME_AVAILABLE'] = None
			
			def do_mod_phase(self):
				self.cfg.include_paths.appendleft(src_dir)
				self._apply_defines(self.cfg)
				self.cfg.defines['UNIVERSALIS__META__MODULE__NAME'] = '"' + self.name +'"'
				self.cfg.defines['UNIVERSALIS__META__MODULE__VERSION'] = 0
				s = self.sources
				dir = src_dir / 'psycle' / 'audiodrivers'
				s.append(dir / 'audiodriver.cpp')
				s.append(dir / 'wavefileout.cpp')
				if gstreamer: s.append(dir / 'gstreamerout.cpp')
				if alsa:      s.append(dir / 'alsaout.cpp')
				if esound:    s.append(dir / 'esoundout.cpp')
				if dsound:    s.append(dir / 'microsoftdirectsoundout.cpp')
				if False: # these drivers need testing
					if jack:      s.append(dir / 'jackout.cpp')
					if netaudio:  s.append(dir / 'netaudioout.cpp')
					if asio:      s.append(dir / 'asiointerface.cpp')
					if winmm:     s.append(dir / 'microsoftmmewaveout.cpp')

			def apply_cxx_to(self, cfg):
				if not self.cxx_phase.dest_dir in cfg.include_paths: cfg.include_paths.append(self.cxx_phase.dest_dir)
				self._apply_defines(cfg)
				ModTask.apply_cxx_to(self, cfg)

			class InstallHeaders(InstallTask):
				@property
				def trim_prefix(self): return src_dir

				@property
				def dest_dir(self): return self.fhs.include

				@property
				def sources(self):
					try: return self._sources
					except AttributeError:
						self._sources = s = []
						dir = src_dir / 'psycle' / 'audiodrivers'
						s.append(dir / 'audiodriver.h')
						if False: # done now in individual driver tasks
							s.append(dir / 'wavefileout.h')
							if gstreamer: s.append(dir / 'gstreamerout.h')
							if alsa:      s.append(dir / 'alsaout.h')
							if esound:    s.append(dir / 'esoundout.h')
							if dsound:    s.append(dir / 'microsoftdirectsoundout.h')
							if False: # these drivers need testing
								if jack:      s.append(dir / 'jackout.h')
								if netaudio:  s.append(dir / 'netaudioout.h')
								if asio:      s.append(dir / 'asioout.h')
								if winmm:     s.append(dir / 'microsoftmmewaveout.h')
						return s

		self._mod_dep_phases = mod_dep_phases = AudioDriversMod()
