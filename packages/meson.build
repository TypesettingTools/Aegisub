conf_pkg = configuration_data()

if host_machine.system() == 'windows'
elif host_machine.system() == 'darwin'
    # temporary hack until version.sh generates this properly
    run_command('cp', meson.source_root() / 'tools' / 'osx-bundle.sed', meson.build_root())
    fontconfig_conf = run_command('pkg-config', '--variable=confdir', 'fontconfig').stdout().strip()
    run_target('osx-bundle',
               command: ['../tools/osx-bundle.sh', meson.source_root(), meson.build_root(), 'wx-config', fontconfig_conf, '',
                         get_option('build_osx_bundle') ? 'TRUE' : 'FALSE'])
else
    conf_pkg.set('AEGISUB_COMMAND', 'aegisub')

    desktop_template = configure_file(input: 'desktop/aegisub.desktop.template.in',
                                      output: 'aegisub.desktop.template',
                                      configuration: conf_pkg)

    i18n = import('i18n')
    i18n.merge_file(input: desktop_template,
                    output: 'aegisub.desktop',
                    type: 'desktop',
                    po_dir: '../po',
                    install: true,
                    install_dir: datadir / 'applications')

    aegisub_logos = ['16x16.png', '22x22.png', '24x24.png', '32x32.png', '48x48.png', '64x64.png', 'scalable.svg']

    foreach s: aegisub_logos
        dir = s.split('.')[0]
        ext = s.split('.')[1]
        install_data('desktop' / dir / 'aegisub.' + ext,
                     install_dir: datadir / 'icons' / 'hicolor' / dir / 'apps')
    endforeach
endif
