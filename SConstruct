#-*- Python -*-

import os
import pickle
import re
from build_config import *
from build_support import *

env = Environment()

missing_pkg_config = not WhereIs('pkg-config')

##################################################
# create options                                 #
##################################################
options_cache_filename = 'options.cache'
old_options_cache = ParseOptionsCacheFile(options_cache_filename)
options = Options(options_cache_filename)
options.Add(BoolOption('debug', 'Generate debug code', 0))
options.Add(BoolOption('multithreaded', 'Generate multithreaded code', 1))
options.Add(BoolOption('dynamic', 'Generate a shared (dynamic-link) library', 1))
if WhereIs('ccache'):
    options.Add(BoolOption('use_ccache', 'Use ccache to build GG', 0))
if WhereIs('distcc'):
    options.Add(BoolOption('use_distcc', 'Use distcc to build GG', 0))
if str(Platform()) == 'win32':
    options.Add('prefix', 'Location to install GG', 'C:\\')
else:
    options.Add('prefix', 'Location to install GG', '/usr/local')
options.Add('scons_cache_dir', 'Directory to use for SCons object file caching (specifying any directory will enable caching)')
options.Add('incdir', 'Location to install headers', os.path.normpath(os.path.join('$prefix', 'include')))
options.Add('bindir', 'Location to install executables', os.path.normpath(os.path.join('$prefix', 'bin')))
options.Add('libdir', 'Location to install libraries', os.path.normpath(os.path.join('$prefix', 'lib')))
if not missing_pkg_config:
    options.Add('pkgconfigdir', 'Location to install pkg-config .pc files', '/usr/lib/pkgconfig')
options.Add(BoolOption('disable_allegro_hack',
                       'Disable the hack for Allegro-linked DevILs (ignored if DevIL is not linked against Allegro)',
                       0))
options.Add(BoolOption('disable_sdl', 'Disables GG SDL support (the GiGiSDL library)', 0))
options.Add('with_boost', 'Root directory of boost installation')
options.Add('with_boost_include', 'Specify exact include dir for boost headers')
options.Add('with_boost_libdir', 'Specify exact library dir for boost library')
options.Add('boost_lib_suffix', 'Specify the suffix placed on user-compiled Boost libraries (e.g. "-vc71-mt-gd-1_31")')
options.Add('boost_signals_namespace',
            'Specify alternate namespace used for boost::signals (only needed if you changed it using the BOOST_SIGNALS_NAMESPACE define when you built boost)')
options.Add('with_sdl', 'Root directory of SDL installation')
options.Add('with_sdl_include', 'Specify exact include dir for SDL headers')
options.Add('with_sdl_libdir', 'Specify exact library dir for SDL library')
options.Add('with_ft', 'Root directory of FreeType2 installation')
options.Add('with_ft_include', 'Specify exact include dir for FreeType2 headers')
options.Add('with_ft_libdir', 'Specify exact library dir for FreeType2 library')
options.Add('with_devil', 'Root directory of DevIL installation')
options.Add('with_devil_include', 'Specify exact include dir for DevIL headers')
options.Add('with_devil_libdir', 'Specify exact library dir for DevIL library')


##################################################
# build vars                                     #
##################################################
# important vars that need to be set for later use when generating and building certain targets
# This is supposed to be detected during configuration, but sometimes isn't, and is harmless, so we're including it all
# the time now on POSIX systems.
env['need__vsnprintf_c'] = str(Platform()) == 'posix'
env['devil_with_allegro'] = False


# fill Environment using saved and command-line provided options, save options for next time, and fill environment
# with save configuration values.
import sys
preconfigured = False
force_configure = False
command_line_args = sys.argv[1:]
if 'configure' in command_line_args:
    force_configure = True
elif ('-h' in command_line_args) or ('--help' in command_line_args):
    preconfigured = True # this is just to ensure config gets skipped when help is requested
ms_linker = 'msvs' in env['TOOLS'] or 'msvc' in env['TOOLS']

env_cache_keys = [
    'CCFLAGS',
    'CPPDEFINES',
    'CPPFLAGS',
    'CPPPATH',
    'CXXFLAGS',
    'LIBPATH',
    'LIBS',
    'LINKFLAGS',
    'need__vsnprintf_c',
    'devil_with_allegro'
    ]
if not force_configure:
    try:
        f = open('config.cache', 'r')
        up = pickle.Unpickler(f)
        pickled_values = up.load()
        for key, value in pickled_values.items():
            env[key] = value
        preconfigured = True
        if ('-h' not in command_line_args) and ('--help' not in command_line_args):
            print 'Using previous successful configuration; if you want to re-run the configuration step, run "scons configure".'
    except Exception:
        pass

options.Update(env)

if env.has_key('use_distcc') and env['use_distcc']:
    env['CC'] = 'distcc %s' % env['CC']
    env['CXX'] = 'distcc %s' % env['CXX']
    for i in ['HOME',
              'DISTCC_HOSTS',
              'DISTCC_VERBOSE',
              'DISTCC_LOG',
              'DISTCC_FALLBACK',
              'DISTCC_MMAP',
              'DISTCC_SAVE_TEMPS',
              'DISTCC_TCP_CORK',
              'DISTCC_SSH']:
        if os.environ.has_key(i) and not env.has_key(i):
            env['ENV'][i] = os.environ[i]

if env.has_key('use_ccache') and env['use_ccache']:
    env['CC'] = 'ccache %s' % env['CC']
    env['CXX'] = 'ccache %s' % env['CXX']
    for i in ['HOME',
              'CCACHE_DIR',
              'CCACHE_TEMPDIR',
              'CCACHE_LOGFILE',
              'CCACHE_PATH',
              'CCACHE_CC',
              'CCACHE_PREFIX',
              'CCACHE_DISABLE',
              'CCACHE_READONLY',
              'CCACHE_CPP2',
              'CCACHE_NOSTATS',
              'CCACHE_NLEVELS',
              'CCACHE_HARDLINK',
              'CCACHE_RECACHE',
              'CCACHE_UMASK',
              'CCACHE_HASHDIR',
              'CCACHE_UNIFY',
              'CCACHE_EXTENSION']:
        if os.environ.has_key(i) and not env.has_key(i):
            env['ENV'][i] = os.environ[i]

Help(GenerateHelpText(options, env))
options.Save('options.cache', env)

# detect changes in the "reconfigurable" options (those that should trigger the configuration step when changed)
new_options_cache = ParseOptionsCacheFile('options.cache')
reconfigurable_options = [
    'disable_allegro_hack',
    'disable_sdl',
    'with_boost',
    'with_boost_include',
    'with_boost_libdir',
    'boost_lib_suffix',
    'boost_signals_namespace',
    'with_sdl',
    'with_sdl_include',
    'with_sdl_libdir',
    'with_ft',
    'with_ft_include',
    'with_ft_libdir',
    'with_devil',
    'with_devil_include',
    'with_devil_libdir'
    ]
if preconfigured:
    for i in old_options_cache.keys():
        if i in reconfigurable_options and (not new_options_cache.has_key(i) or old_options_cache[i] != new_options_cache[i]):
            preconfigured = False
            break
if preconfigured:
    for i in new_options_cache.keys():
        if i in reconfigurable_options and (not old_options_cache.has_key(i) or old_options_cache[i] != new_options_cache[i]):
            preconfigured = False
            break

if not env['multithreaded']:
    if not env['disable_sdl']:
        print 'Warning: since multithreaded code is disabled, the GiGiSDL build is disabled as well.'
        env['disable_sdl'] = 1
if str(Platform()) == 'win32':
    if not env['multithreaded']:
        if env['dynamic']:
            print 'Warning: since the Win32 platform does not support multithreaded DLLs, multithreaded static libraries will be produced instead.'
            env['dynamic'] = 0


##################################################
# check configuration                            #
##################################################
if not env.GetOption('clean'):
    if not preconfigured:
        conf = env.Configure(custom_tests = {'CheckVersionHeader' : CheckVersionHeader,
                                             'CheckPkgConfig' : CheckPkgConfig,
                                             'CheckPkg' : CheckPkg,
                                             'CheckBoost' : CheckBoost,
                                             'CheckBoostLib' : CheckBoostLib,
                                             'CheckSDL' : CheckSDL,
                                             'CheckLibLTDL' : CheckLibLTDL,
                                             'CheckConfigSuccess' : CheckConfigSuccess})

        
        if str(Platform()) == 'posix':
            print 'Configuring for POSIX system...'
        elif str(Platform()) == 'win32':
            print 'Configuring for WIN32 system...'
        else:
            print 'Configuring unknown system (assuming the system is POSIX-like) ...'

        if OptionValue('boost_signals_namespace', env):
            signals_namespace = OptionValue('boost_signals_namespace', env)
            env.Append(CPPDEFINES = [
                ('BOOST_SIGNALS_NAMESPACE', signals_namespace),
                ('signals', signals_namespace)
                ])

        boost_libs = [
            ('boost_signals', 'boost/signals.hpp', 'boost::signals::connection();'),
            ('boost_filesystem', 'boost/filesystem/operations.hpp', 'boost::filesystem::initial_path();')
            ]
        if not conf.CheckBoost(boost_version_string, boost_libs, conf, not ms_linker):
            Exit(1)

        # pthreads
        if str(Platform()) == 'posix':
            if env['multithreaded']:
                if conf.CheckCHeader('pthread.h') and conf.CheckLib('pthread', 'pthread_create', autoadd = 0):
                    env.Append(CCFLAGS = ['-pthread'])
                    env.Append(LINKFLAGS = ['-pthread'])
                else:
                    Exit(1)

        # GL and GLU
        if str(Platform()) == 'win32':
            env.Append(LIBS = [
                'opengl32.lib',
                'glu32.lib'
                ])
        else:
            if not conf.CheckCHeader('GL/gl.h') or \
                   not conf.CheckCHeader('GL/glu.h') or \
                   not conf.CheckLib('GL', 'glBegin') or \
                   not conf.CheckLib('GLU', 'gluLookAt'):
                Exit(1)

        # SDL
        sdl_config = WhereIs('sdl-config')
        if not env['disable_sdl']:
            if not conf.CheckSDL(options, conf, sdl_config, not ms_linker):
                Exit(1)

        # pkg-config
        pkg_config = conf.CheckPkgConfig('0.15.0')

        # FreeType2
        AppendPackagePaths('ft', env)
        found_it_with_pkg_config = False
        if pkg_config:
            if conf.CheckPkg('freetype2', ft_pkgconfig_version):
                env.ParseConfig('pkg-config --cflags --libs freetype2')
                found_it_with_pkg_config = True
        if not found_it_with_pkg_config:
            version_regex = re.compile(r'FREETYPE_MAJOR\s*(\d+).*FREETYPE_MINOR\s*(\d+).*FREETYPE_PATCH\s*(\d+)', re.DOTALL)
            if not conf.CheckVersionHeader('freetype2', 'freetype/freetype.h', version_regex, ft_version, True):
                Exit(1)
        if not conf.CheckCHeader('ft2build.h'):
            Exit(1)
        if str(Platform()) != 'win32':
            if not conf.CheckLib('freetype', 'FT_Init_FreeType'):
                Exit(1)
        else:
            env.Append(LIBS = [ft_win32_lib_name])

        # DevIL (aka IL)
        AppendPackagePaths('devil', env)
        version_regex = re.compile(r'IL_VERSION\s*(\d+)')
        if not conf.CheckVersionHeader('DevIL', 'IL/il.h', version_regex, devil_version, True, 'Checking DevIL version >= %s... ' % devil_version_string):
            Exit(1)
        if not conf.CheckCHeader('IL/il.h') or \
               not conf.CheckCHeader('IL/ilu.h') or \
               not conf.CheckCHeader('IL/ilut.h'):
            print "Note: since SDL support is disabled, the SDL headers are not be in the compiler's search path during tests.  If DevIL was built with SDL support, this may cause the seach for ilut.h to fail."
            Exit(1)
        if str(Platform()) != 'win32' and (not conf.CheckLib('IL', 'ilInit') or \
                                           not conf.CheckLib('ILU', 'iluInit') or \
                                           not conf.CheckLib('ILUT', 'ilutInit')):
            print 'Trying IL again with local _vnsprintf.c...'
            if conf.CheckLib('IL', 'ilInit', header = '#include "../src/_vsnprintf.c"') and \
                   conf.CheckLib('ILU', 'iluInit', header = '#include "../src/_vsnprintf.c"') and \
                   conf.CheckLib('ILUT', 'ilutInit', header = '#include "../src/_vsnprintf.c"'):
                env['need__vsnprintf_c'] = True
                print 'DevIL is broken.  It depends on a function _vnsprintf that does not exist on your system.  GG will provide a vnsprintf wrapper called _vnsprintf.'
            else:
                Exit(1)
        from sys import stdout
        stdout.write('Checking for DevIL OpenGL support... ')
        devil_gl_check_app = """
#include <IL/ilut.h>
#ifndef ILUT_USE_OPENGL
#error "DevIL not built with OpenGL support"
#endif
int main() {
    return 0;
}
"""
        if not conf.TryCompile(devil_gl_check_app, '.c'):
            print 'no'
        else:
            print 'yes'
        stdout.write('Checking for DevIL Allegro support... ')
        devil_gl_check_app = """
#include <IL/ilut.h>
#ifndef ILUT_USE_ALLEGRO
#error "DevIL not built with Allegro support"
#endif
int main() {
    return 0;
}
"""
        devil_with_allegro = conf.TryCompile(devil_gl_check_app, '.c');
        print devil_with_allegro and 'yes' or "no (That's good!)"
        if devil_with_allegro:
            if env['disable_allegro_hack']:
                print """Your DevIL-library is linked with Allegro, and this requires placing an "END_OF_MAIN"-macro after the program's main routine. You have chosen to omit the hack that would eliminate this requirement! This means that _every_ program that uses this library has to #include <allegro.h> and place END_OF_MAIN() after the main function."""
            else:
                print """Your DevIL-library is linked with Allegro, and this would require placing an "END_OF_MAIN"-macro after the program's main routine. GG contains a hack to remove this requirement, but it's a dirty hack and might not work. Note that your Allegro programs will crash if you use GiGi and forget the "END_OF_MAIN"-macro! If you want to be able to use GiGi in Allegro-programs, run configure with the '--disable-allegro-hack' option."""

        # ltdl
        if str(Platform()) != 'win32':
            if not conf.CheckLibLTDL():
                print 'Check libltdl/config.log to see what went wrong.'
                Exit(1)

        # create GG/Config.h
        gg_config_h_values = {
            'gg_devil_with_allegro' : env['devil_with_allegro'] and '#define GG_DEVIL_WITH_ALLEGRO 1' or '/* #undef GG_DEVIL_WITH_ALLEGRO */',
            'gg_no_allegro_hack' : env['disable_allegro_hack'] and '#define GG_NO_ALLEGRO_HACK 1' or '/* #undef GG_NO_ALLEGRO_HACK */'
            }
        sys.stdout.write('Creating GG/Config.h from GG/Config.h.in... ')
        try:
            in_f = open(os.path.normpath('GG/Config.h.in'), 'r')
            out_f = open(os.path.normpath('GG/Config.h'), 'w')
            out_f.write(in_f.read() % gg_config_h_values)
        except Exception:
            print 'failed'
            print 'Unable to create GG/Config.h from GG/Config.h.in'
            Exit(1)
        print 'ok'

        # finish config and save results for later
        conf.CheckConfigSuccess(True)
        conf.Finish();
        f = open('config.cache', 'w')
        p = pickle.Pickler(f)
        cache_dict = {}
        for i in env_cache_keys:
            cache_dict[i] = env.has_key(i) and env.Dictionary(i) or []
        p.dump(cache_dict)

	# for win32, create libltdl/config.h
        if str(Platform()) == 'win32':
            f = open(os.path.normpath('libltdl/config.h'), 'w')
            f.write("""/* WARNING: Generated by GG's SConstruct file.  All local changes will be lost! */
#define error_t int
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_CTYPE_H 1
#define HAVE_MEMORY_H 1
#define HAVE_ERRNO_H 1
#define __WIN32__
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define LTDL_OBJDIR ".libs"
#define LTDL_DLOPEN_DEPLIBS 1
#define LTDL_SHLIBPATH_VAR "PATH"
#define LTDL_SHLIB_EXT ".dll"
""")
            f.close()

        # copy ltdl.h and config.h into the header tree before compiling
        Execute(Copy(os.path.normpath('GG/ltdl.h'), os.path.normpath('libltdl/ltdl.h')))
        Execute(Copy(os.path.normpath('GG/ltdl_config.h'), os.path.normpath('libltdl/config.h')))

        if 'configure' in command_line_args:
            Exit(0)

##################################################
# define targets                                 #
##################################################
env.Append(CPPPATH = [
    '#',
    '#/libltdl'
    ])

if str(Platform()) == 'win32':
    if env['multithreaded']:
        if env['dynamic']:
            if env['debug']:
                code_generation_flag = '/MDd'
            else:
                code_generation_flag = '/MD'
        else:
            if env['debug']:
                code_generation_flag = '/MTd'
            else:
                code_generation_flag = '/MT'
    else:
        if env['debug']:
            code_generation_flag = '/MLd'
        else:
            code_generation_flag = '/ML'
    flags = [
        #env['debug'] and '/Od' or '/Ox',
        code_generation_flag,
        '/EHsc',
        '/W3',
        '/Zc:forScope',
        '/GR',
        '/Gd',
        '/Zi',
        '/wd4099', '/wd4251', '/wd4800', '/wd4267', '/wd4275', '/wd4244', '/wd4101', '/wd4258', '/wd4351', '/wd4996'
        ]
    env.Append(CCFLAGS = flags)
    env.Append(CPPDEFINES = [
        (env['debug'] and '_DEBUG' or 'NDEBUG'),
        'WIN32',
        '_WINDOWS'
        ])
    if env['dynamic']:
        env.Append(CPPDEFINES = [
        'GIGI_EXPORTS',
        '_USRDLL',
        '_WINDLL'
        ])
    env.Append(LINKFLAGS = [
        '/NODEFAULTLIB:LIBCMT',
        '/DEBUG'
        ])
    env.Append(LIBS = [
        'kernel32',
        'user32',
        'gdi32',
        'winspool',
        'comdlg32',
        'advapi32',
        'shell32',
        'ole32',
        'oleaut32',
        'uuid',
        'odbc32',
        'odbccp32',
        ])
else:
    if env['debug']:
        env.Append(CCFLAGS = ['-Wall', '-g', '-O0'])
    else:
        env.Append(CCFLAGS = ['-Wall', '-O2'])

Export('env')

# define libGiGi objects
env['libltdl_defines'] = [
    'HAVE_CONFIG_H'
    ]
if env.has_key('CPPDEFINES'):
    env['libltdl_defines'] += env['CPPDEFINES']
gigi_objects, gigi_sources = SConscript(os.path.normpath('src/SConscript'))
result_objects, result_sources = SConscript(os.path.normpath('libltdl/SConscript'))
gigi_objects += result_objects
gigi_sources += result_sources

# define libGiGiSDL objects
if not env['disable_sdl']:
    sdl_env = env.Copy()
    if str(Platform()) == 'win32':
        sdl_env.Append(LIBS = ['SDL', 'GiGi'])
    gigi_sdl_objects, gigi_sdl_sources = SConscript(os.path.normpath('src/SDL/SConscript'), exports = 'sdl_env')

if env['dynamic']:
    lib_gigi = env.SharedLibrary('GiGi', gigi_objects)
    if not env['disable_sdl']:
        lib_gigi_sdl = sdl_env.SharedLibrary('GiGiSDL', gigi_sdl_objects)
else:
    lib_gigi = env.StaticLibrary('GiGi', gigi_objects)
    if not env['disable_sdl']:
        lib_gigi_sdl = sdl_env.StaticLibrary('GiGiSDL', gigi_sdl_objects)

Depends(lib_gigi_sdl, lib_gigi)

if not missing_pkg_config and str(Platform()) != 'win32':
    gigi_pc = env.Command('GiGi.pc', 'GiGi.pc.in', CreateGiGiPCFile)
    if not env['disable_sdl']:
        gigi_sdl_pc = env.Command('GiGiSDL.pc', 'GiGiSDL.pc.in', CreateGiGiSDLPCFile)

header_dir = os.path.normpath(os.path.join(env.subst(env['incdir']), 'GG'))
lib_dir = os.path.normpath(env.subst(env['libdir']))

# install target
gigi_libname = str(lib_gigi[0])
gigi_sdl_libname = str(lib_gigi_sdl[0])
installed_gigi_libname = gigi_libname
installed_gigi_sdl_libname = gigi_sdl_libname
if str(Platform()) == 'posix' and env['dynamic']:
    gigi_version_suffix = '.' + gigi_version
    installed_gigi_libname += gigi_version_suffix
    installed_gigi_sdl_libname += gigi_version_suffix

for root, dirs, files in os.walk('GG'):
    if '.svn' in dirs:
        dirs.remove('.svn')
    if 'CVS' in dirs:
        dirs.remove('CVS')
    for f in [f for f in files if f.find('.scons') != 0]:
        Alias('install', Install(os.path.normpath(os.path.join(env.subst(env['incdir']), root)),
                                 os.path.normpath(os.path.join(root, f))))

if str(Platform()) == 'win32':
    Alias('install', Install(lib_dir, lib_gigi))
else:
    Alias('install', InstallAs(lib_dir + '/' + installed_gigi_libname, lib_gigi))
    if env['dynamic']:
        Alias('install',
              env.Command(lib_dir + '/' + gigi_libname,
                          lib_dir + '/' + installed_gigi_libname,
                          'ln -s ' + lib_dir + '/' + installed_gigi_libname + ' ' + lib_dir + '/' + gigi_libname))
if not missing_pkg_config and str(Platform()) != 'win32':
    Alias('install', Install(env.subst(env['pkgconfigdir']), gigi_pc))
if not env['disable_sdl']:
    if str(Platform()) == 'win32':
        Alias('install', Install(lib_dir, lib_gigi_sdl))
    else:
        Alias('install', InstallAs(lib_dir + '/' + installed_gigi_sdl_libname, lib_gigi_sdl))
        if env['dynamic']:
            Alias('install',
                  env.Command(lib_dir + '/' + gigi_sdl_libname,
                              lib_dir + '/' + installed_gigi_sdl_libname,
                              'ln -s ' + lib_dir + '/' + installed_gigi_sdl_libname + ' ' + lib_dir + '/' + gigi_sdl_libname))
    if not missing_pkg_config and str(Platform()) != 'win32':
        Alias('install', Install(env.subst(env['pkgconfigdir']), gigi_sdl_pc))

deletions = [
    Delete(header_dir),
    Delete(os.path.normpath(os.path.join(lib_dir, str(lib_gigi[0]))))
    ]
if str(Platform()) == 'posix' and env['dynamic']:
    deletions.append(Delete(os.path.normpath(os.path.join(lib_dir, installed_gigi_libname))))
if not missing_pkg_config and str(Platform()) != 'win32':
    deletions.append(Delete(os.path.normpath(os.path.join(env.subst(env['pkgconfigdir']), str(gigi_pc[0])))))
if not env['disable_sdl']:
    deletions.append(Delete(os.path.normpath(os.path.join(lib_dir, str(lib_gigi_sdl[0])))))
    if str(Platform()) == 'posix' and env['dynamic']:
        deletions.append(Delete(os.path.normpath(os.path.join(lib_dir, installed_gigi_sdl_libname))))
    if not missing_pkg_config and str(Platform()) != 'win32':
        deletions.append(Delete(os.path.normpath(os.path.join(env.subst(env['pkgconfigdir']), str(gigi_sdl_pc[0])))))
uninstall = env.Command('uninstall', '', deletions)
env.AlwaysBuild(uninstall)
env.Precious(uninstall)

# MSVC project target
##if str(Platform()) == 'win32':
##    variant_name = ''
##    if env['multithreaded']:
##        if env['dynamic']:
##            if env['debug']:
##                variant_name = 'Multi-threaded Debug DLL'
##            else:
##                variant_name = 'Multi-threaded Release DLL'
##        else:
##            if env['debug']:
##                variant_name = 'Multi-threaded Debug'
##            else:
##                variant_name = 'Multi-threaded Release'
##    else:
##        if env['debug']:
##            variant_name = 'Single-threaded Debug'
##        else:
##            variant_name = 'Single-threaded Release'
##    gigi_project = MSVSProject(target = 'msvc/GiGi' + env['MSVSPROJECTSUFFIX'],
##                               srcs = gigi_sources,
##                               incs = '',
##                               localincs = '',
##                               resources = '',
##                               misc = '',
##                               buildtarget = lib_gigi,
##                               variant = variant_name)
##    Alias('msvc_project', gigi_project)
##    if not env['disable_sdl']:
##        gigi_sdl_project = MSVSProject(target = 'msvc/GiGiSDL' + env['MSVSPROJECTSUFFIX'],
##                                       srcs = gigi_sources,
##                                       incs = '',
##                                       localincs = '',
##                                       resources = '',
##                                       misc = '',
##                                       buildtarget = lib_gigi,
##                                       variant = variant_name)
##        Alias('msvc_project', gigi_sdl_project)

# default targets
if env['disable_sdl']:
    Default(lib_gigi)
else:
    Default(lib_gigi, lib_gigi_sdl)

if OptionValue('scons_cache_dir', env):
    CacheDir(OptionValue('scons_cache_dir', env))

