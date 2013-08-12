#! /usr/bin/env python
# This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
# copyright 2006-2013 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

from wonderbuild import UserReadableException
from wonderbuild.cxx_tool_chain import BuildCheckTask

class ValidCfgCheckTask(BuildCheckTask):
	'a check to simply test whether the user-provided compiler and linker flags are correct'

	@staticmethod
	def shared_uid(*args, **kw): return 'valid-user-provided-build-cfg-flags'

	source_text = '' # the base class already adds a main() function

	def __call__(self, sched_ctx):
		for x in BuildCheckTask.__call__(self, sched_ctx): yield x
		if not self: raise UserReadableException, str(self) + ': invalid user-provided build cfg flags'
