##  -*- python -*-
##  $Id: SConstruct,v 1.1 2006/05/26 00:18:38 ingo Exp ingo $
##  ___  ___ _____         _ ___
## |   \| __|_   _|__  ___| | _ ) _____ __
## | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
## |___/|_|   |_|\___/\___/_|___/\___/_\_\
##
##  DFToolBox - Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>
##
##  This program is free software; you can redistribute it and/or
##  modify it under the terms of the GNU General Public License
##  as published by the Free Software Foundation; either version 2
##  of the License, or (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
## 
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, write to the Free Software
##  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
##  02111-1307, USA.
##

env = Environment(CCFLAGS = ['-O2', '-Wall', '-g', '-DDEBUG'])

#fox_LIBS = ['FOX-1.6', 'Xext', 'X11', 'Xcursor', 'Xrandr', 'dl', 'pthread', 'rt',
#            'jpeg', 'png', 'tiff', 'z', 'bz2', 'm', 'GLU', 'GL']

fox_LIBS = ['FOX-1.6', 'Xext', 'X11', 'Xft', 'fontconfig', 'Xcursor', 'Xrandr', 'dl', 'pthread',
            'rt', 'jpeg', 'png', 'tiff', 'z', 'bz2', 'm', 'cups', 'nsl', 'GLU', 'GL']
fox_LIBPATH = [] # ['/home/ingo/projects/dftoolbox/libs/lib']
fox_CPPPATH = ['/usr/include/fox-1.6'] # ['/home/ingo/projects/dftoolbox/libs/include/fox-1.6']
fox_CXXFLAGS = []

sdl_LIBS     = ['SDL_mixer', 'SDL']
sdl_CPPATH   = []
env['CCFLAGS']  += ['-DUSE_SDL']

cross = False
if cross: # Make this True for cross compiling
    env['CXX']        = 'i586-mingw32msvc-g++'
    env['AS']         = 'i586-mingw32msvc-as'
    env['RANLIB']     = 'i586-mingw32msvc-ranlib'
    env['AR']         = 'i586-mingw32msvc-ar'
    env['PROGSUFFIX'] = '.exe'
    env['BUILDDIR']   = 'win32'
    env['CCFLAGS']    = ['-O2', '-Wall', '-s', '-DDEBUG'] # '-DUSE_SDL'
    fox_LIBS     = ['FOX-1.6', 'comctl32', 'wsock32', 'winspool', 'mpr', 'm', 'glu32', 'opengl32']
    fox_LIBPATH  = ['/home/ingo/projects/dftoolbox/libs-win32/lib']
    fox_CPPPATH  = ['/home/ingo/projects/dftoolbox/libs-win32/include/fox-1.6']
    fox_CXXFLAGS = ['-mwindows', '-e', '_mainCRTStartup']
    sdl_LIBS     = ['winmm'] # ['mingw32', 'SDLmain', 'SDL_mixer', 'SDL']
    sdl_CPPATH   = [] # ['/home/ingo/projects/dftoolbox/libs-win32/include/']

env.StaticLibrary('dftoolbox', ['src/util.cpp',
                                'src/system.cpp',
                                'src/tljpak.cpp',
                                'src/location.cpp',
                                'src/savegame.cpp',
                                'src/data_manager.cpp',
                                'src/shark3d.cpp',
                                'src/shark3d_nodes.cpp',
                                'src/config.cpp'
                                ])
env.StaticLibrary('tinygettext', ['src/tinygettext.cpp'])

env.Program('extract',
            ['src/extract.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('bundle',
            ['src/bundle.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('dialog',
            ['src/dialog.cpp'],
            LIBS=['dftoolbox', 'tinygettext'],
            LIBPATH='.')

env.Program('dialog-fix',
            ['src/dialog-fix.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('savegame2tga',
            ['src/savegame2tga.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('insert',
            ['src/insert.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('ddstool',
            ['src/ddstool.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('shark3d',
            ['src/shark3d_main.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('stfu4',
            ['src/stfu4.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('xarc',
            ['src/xarc.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('xmg',
            ['src/xmg.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('xrc',
            ['src/xrc.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

env.Program('cir',
            ['src/cir.cpp'],
            LIBS=['dftoolbox'],
            LIBPATH='.')

progress_dialog_obj = env.Object('src/progress_dialog.cpp',
                                 LINKFLAGS = fox_CXXFLAGS,
                                 CPPPATH   = fox_CPPPATH,
                                 LIBPATH   = ['.'] + fox_LIBPATH,
                                 LIBS      = ['dftoolbox'] + fox_LIBS)

icon_obj = env.Object('src/icons.cpp',
                      LINKFLAGS = fox_CXXFLAGS,
                      CPPPATH   = fox_CPPPATH,
                      LIBPATH   = ['.'] + fox_LIBPATH,
                      LIBS      = ['dftoolbox'] + fox_LIBS)

progress_logger = env.Object('src/progress_logger.cpp')

env.Program('dfmodtool',
            ['src/dfmodactivator.cpp',
             'src/pak_insert_thread.cpp',
             icon_obj,
             progress_logger,
             progress_dialog_obj] + (cross and ['dfmodtool-res.o'] or []),
            LINKFLAGS = fox_CXXFLAGS,
            CPPPATH   = fox_CPPPATH,
            LIBPATH   = ['.'] + fox_LIBPATH,
            LIBS      = ['dftoolbox'] + fox_LIBS)

env.Program('dftoolbox',
            ['src/dftoolbox.cpp',
             'src/shark_view.cpp',
             'src/image_view.cpp',
             'src/dialog_view.cpp',
             'src/searchbar.cpp',
             'src/export_dialog.cpp',
             'src/dreamfall_file_entry.cpp',
             'src/directory_view.cpp',
             'src/sound_view.cpp',
             icon_obj,
             progress_logger,
             progress_dialog_obj] + (cross and ['dftoolbox-res.o'] or []),
            LINKFLAGS = fox_CXXFLAGS,
            CPPPATH   = fox_CPPPATH  + sdl_CPPATH,
            LIBPATH   = ['.'] + fox_LIBPATH,
            LIBS      = ['dftoolbox'] + fox_LIBS + sdl_LIBS)

# EOF #
