conf_pkg = configuration_data()

if host_machine.system() == 'windows'
    installer_setup = find_program(meson.project_source_root() / 'tools/win-installer-setup.ps1')
    run_target('win-installer',
               command: [installer_setup, meson.project_build_root(), meson.project_source_root()])
    portable_setup = find_program(meson.project_source_root() / 'packages/win_installer/portable/create-portable.ps1')
    run_target('win-portable', command: [portable_setup, meson.project_build_root(), meson.project_source_root()])
elif host_machine.system() == 'darwin'
    fontconfig_conf = run_command('pkg-config', '--variable=confdir', 'fontconfig').stdout().strip()
    bundle_app_sh = find_program(meson.project_source_root() / 'tools/osx-bundle.sh')
    run_target('osx-bundle',
               command: [bundle_app_sh, meson.project_source_root(), meson.project_build_root(), 'wx-config', fontconfig_conf, '',
                         get_option('build_osx_bundle') ? 'TRUE' : 'FALSE'])
    build_dmg_sh = find_program(meson.project_source_root() / 'tools/osx-dmg.sh')
    run_target('osx-build-dmg',
               command: [build_dmg_sh, meson.project_source_root(), meson.project_build_root(), meson.project_version()])
elif host_machine.system() == 'linux'

        dependency_control_setup = find_program(meson.project_source_root() / 'tools/dependency-control.sh')
        run_target('linux-dependency-control',
                command: [dependency_control_setup, meson.project_build_root(), meson.project_source_root()])
        installer_setup = find_program(meson.project_source_root() / 'tools/ubuntu-build-deb.sh')
        run_target('ubuntu-deb',
                command: [installer_setup, meson.project_build_root(), meson.project_source_root()])
        assdraw_compile_and_setup = find_program(meson.project_source_root() / 'tools/ubuntu-build-assdraw.sh')
        run_target('ubuntu.assdraw-deb',
                command: [assdraw_compile_and_setup, meson.project_build_root(), meson.project_source_root()])
        

## TODO : use meson.project_build_root()

 #    'ubuntu-install-local': 'sudo dpkg -i build/$(ls build/  | grep aegisub_.*deb)'





    conf_pkg.set('AEGISUB_COMMAND', 'aegisub')

    desktop_template = configure_file(input: 'desktop/aegisub.desktop.template.in',
                                      output: 'aegisub.desktop.template',
                                      configuration: conf_pkg)

    i18n = import('i18n')
    i18n.merge_file(input: desktop_template,
                    output: 'aegisub.desktop',
                    type: 'desktop',
                    po_dir: '../po',
                    install_dir: meson.project_build_root() / 'packages' / 'applications')

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
