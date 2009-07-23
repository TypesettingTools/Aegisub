from buildbot.steps import transfer
from buildbot.steps.shell import WithProperties
from buildbot.process.buildstep import BuildStep

class agi_upload(transfer.FileUpload):
	def __init__(self, slavesrc, masterdest, workdir="build", maxsize=None,
		blocksize=16*1024, mode=None, **buildstep_kwargs):

		transfer.FileUpload.__init__(self, slavesrc, masterdest, workdir, maxsize, blocksize, mode, **buildstep_kwargs)
              
	def start(self):
		properties = self.build.getProperties()
		masterdest = properties.render(self.masterdest)
		self.addURL("dist-http", "http://ftp.aegisub.org/pub/" + masterdest)
		self.addURL("dist-ftp", "ftp://ftp.aegisub.org/pub/" + masterdest)

		return transfer.FileUpload.start(self)

	def finish(self, result):
		transfer.finished(self, result)
