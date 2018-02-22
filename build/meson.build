version_sh = find_program('./version.sh')
version_inc = include_directories('.')
version_h = custom_target('git_version.h',
                          command : [version_sh, meson.source_root()],
                          build_always : true,  # has internal check whether target file will be refreshed
                          output : ['git_version.h', 'git_version.xml'])
