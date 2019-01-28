if host_machine.system() == 'windows'
	version_sh = find_program('.\\version.ps1')
else
	version_sh = find_program('./version.sh')
endif
version_inc = include_directories('.')
version_h = custom_target('git_version.h',
                          command : [version_sh, meson.source_root()],
                          build_by_default: true,
                          build_always_stale: true, # has internal check whether target file will be refreshed
                          output: ['git_version.h', 'git_version.xml'])
