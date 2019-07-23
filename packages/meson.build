c_pkg = configuration_data()
c_pkg.set('AEGISUB_COMMAND', 'aegisub')

if host_machine.system() == 'windows'
elif host_machine.system() == 'darwin'
else
    desktop_template = configure_file(input: 'desktop/aegisub.desktop.template.in',
                                      output: 'aegisub.desktop.template',
                                      configuration: c_pkg)

    # TODO: switch this to gettext from intltool
    #       requires modifying the autotools chain and a recent-ish gettext
    #       https://wiki.gnome.org/MigratingFromIntltoolToGettext
    #
    # i18n = import('i18n')
    # i18n.merge_file(input: desktop_template,
    #                 output: 'aegisub.desktop',
    #                 type: 'desktop',
    #                 po_dir: '../po',
    #                 install: true,
    #                 install_dir: join_paths(get_option('datadir'), 'applications'))
    intltool_merge = find_program('intltool-merge')
    desktop = custom_target('aegisub.desktop',
                            input: desktop_template,
                            output: 'aegisub.desktop',
                            command: [intltool_merge, '--desktop-style', '--quiet', '../po', '@INPUT@', '@OUTPUT@'],
                            install: true,
                            install_dir: get_option('datadir') / 'applications')
endif

aegisub_logos = ['16x16.png', '22x22.png', '24x24.png', '32x32.png', '48x48.png', '64x64.png', 'scalable.svg']

foreach s: aegisub_logos
    dir = s.split('.')[0]
    ext = s.split('.')[1]
    install_data('desktop' / dir / 'aegisub.' + ext, 
                 install_dir: get_option('datadir') / 'icons' / 'hicolor' / dir / 'apps')
endforeach
