#  ___  ___ _____         _ ___
# |   \| __|_   _|__  ___| | _ ) _____ __
# | |) | _|  | |/ _ \/ _ \ | _ \/ _ \ \ /
# |___/|_|   |_|\___/\___/_|___/\___/_\_\
#
#  DFToolBox - Copyright (C) 2020 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.15)
project(dftoolbox VERSION 0.1.0)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Installs TARGET into libexec and a wrapper script into bin that
# passes the shared data directory (share/dftoolbox) via --datadir
function(install_with_wrapper TARGET)
  install(TARGETS ${TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

  file(WRITE ${CMAKE_BINARY_DIR}/${TARGET}.sh
    "#!/bin/sh\n"
    "exec \"${CMAKE_INSTALL_FULL_LIBEXECDIR}/${TARGET}\""
    " --datadir \"${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}\" \"$@\"\n")

  install(PROGRAMS
    ${CMAKE_BINARY_DIR}/${TARGET}.sh
    RENAME ${TARGET}
    DESTINATION ${CMAKE_INSTALL_BINDIR})
endfunction()

option(WARNINGS "Switch on extra warnings" OFF)
option(WERROR "Turn warnings into errors" OFF)

find_package(PkgConfig REQUIRED)
pkg_search_module(SDL REQUIRED sdl IMPORTED_TARGET)
pkg_search_module(SDL_MIXER REQUIRED SDL_mixer IMPORTED_TARGET)
find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

add_library(libdftoolbox STATIC
  "src/util.cpp"
  "src/system.cpp"
  "src/tljpak.cpp"
  "src/location.cpp"
  "src/savegame.cpp"
  "src/data_manager.cpp"
  "src/shark3d.cpp"
  "src/shark3d_nodes.cpp"
  "src/config.cpp")
target_compile_definitions(libdftoolbox PUBLIC "USE_SDL")

add_library(tinygettext STATIC src/tinygettext.cpp)

add_executable(dftool-dialog "src/dialog.cpp")
target_link_libraries(dftool-dialog libdftoolbox tinygettext)

add_executable(dftool-extract "src/extract.cpp")
target_link_libraries(dftool-extract libdftoolbox)

add_executable(dftool-bundle "src/bundle.cpp")
target_link_libraries(dftool-bundle libdftoolbox)

add_executable(dftool-dialog-fix "src/dialog-fix.cpp")
target_link_libraries(dftool-dialog-fix libdftoolbox)

add_executable(dftool-savegame2tga "src/savegame2tga.cpp")
target_link_libraries(dftool-savegame2tga libdftoolbox)

add_executable(dftool-insert "src/insert.cpp")
target_link_libraries(dftool-insert libdftoolbox)

add_executable(dftool-ddstool "src/ddstool.cpp")
target_link_libraries(dftool-ddstool libdftoolbox)

add_executable(dftool-shark3d "src/shark3d_main.cpp")
target_link_libraries(dftool-shark3d libdftoolbox)

add_executable(dftool-stfu4 "src/stfu4.cpp")
target_link_libraries(dftool-stfu4 libdftoolbox)

add_executable(dftool-xarc "src/xarc.cpp")
target_link_libraries(dftool-xarc libdftoolbox)

add_executable(dftool-xmg "src/xmg.cpp")
target_link_libraries(dftool-xmg libdftoolbox)

add_executable(dftool-xrc "src/xrc.cpp")
target_link_libraries(dftool-xrc libdftoolbox)

add_executable(dftool-cir "src/cir.cpp")
target_link_libraries(dftool-cir libdftoolbox)

add_executable(dfmodtool
  "src/dfmodactivator.cpp"
  "src/dfmodactivator.hpp"
  "src/pak_insert_thread.cpp"
  "src/icons.cpp"
  "src/progress_dialog.cpp"
  "src/progress_dialog.hpp"
  "src/progress_logger.cpp"
  # + (cross and ['dfmodtool-res.o'] or []),
  )
target_link_libraries(dfmodtool PRIVATE
  libdftoolbox
  Qt6::Widgets)

install(TARGETS
  dftool-dialog
  dftool-extract
  dftool-bundle
  dftool-dialog-fix
  dftool-savegame2tga
  dftool-insert
  dftool-ddstool
  dftool-shark3d
  dftool-stfu4
  dftool-xarc
  dftool-xmg
  dftool-xrc
  dftool-cir
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(dftoolbox
  "src/dftoolbox.cpp"
  "src/dftoolbox.hpp"
  "src/shark_view.cpp"
  "src/shark_view.hpp"
  "src/image_view.cpp"
  "src/image_view.hpp"
  "src/dialog_view.cpp"
  "src/dialog_view.hpp"
  "src/searchbar.cpp"
  "src/searchbar.hpp"
  "src/export_dialog.cpp"
  "src/export_dialog.hpp"
  "src/dreamfall_file_entry.cpp"
  "src/directory_view.cpp"
  "src/directory_view.hpp"
  "src/sound_view.cpp"
  "src/sound_view.hpp"
  "src/ui_util.cpp"
  "src/ui_util.hpp"
  "src/icons.cpp"
  "src/progress_dialog.cpp"
  "src/progress_dialog.hpp"
  "src/progress_logger.cpp"
  # (cross and ['dftoolbox-res.o'] or []),
  )
target_link_libraries(dftoolbox PRIVATE
  libdftoolbox
  Qt6::Widgets
  PkgConfig::SDL
  PkgConfig::SDL_MIXER)
install_with_wrapper(dftoolbox)
install_with_wrapper(dfmodtool)

# Shared runtime data of dftoolbox and dfmodtool: icons (PNG and the
# GIFs of the mods), the mods dfmodtool lists and the known filenames
install(DIRECTORY icons DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
  PATTERN "*.xcf" EXCLUDE)
install(DIRECTORY mods DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

install(FILES df-directory.txt  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

# EOF #
