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
project(dftoolbox VERSION 0.0.0)

include(GNUInstallDirs)

function(install_with_wrapper TARGET)
  install(TARGETS ${TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

  file(WRITE ${CMAKE_BINARY_DIR}/${TARGET}.sh
    "#!/bin/sh\n"
    "exec \"${CMAKE_INSTALL_FULL_LIBEXECDIR}/${TARGET}\""
    " --datadir \"${CMAKE_INSTALL_FULL_DATADIR}/${TARGET}\" \"$@\"\n")

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
pkg_search_module(FOX REQUIRED fox IMPORTED_TARGET)

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

add_executable(dialog "src/dialog.cpp")
target_link_libraries(dialog libdftoolbox tinygettext)

add_executable(extract "src/extract.cpp")
target_link_libraries(extract libdftoolbox)

add_executable(bundle "src/bundle.cpp")
target_link_libraries(bundle libdftoolbox)

add_executable(dialog-fix "src/dialog-fix.cpp")
target_link_libraries(dialog-fix libdftoolbox)

add_executable(savegame2tga "src/savegame2tga.cpp")
target_link_libraries(savegame2tga libdftoolbox)

add_executable(insert "src/insert.cpp")
target_link_libraries(insert libdftoolbox)

add_executable(ddstool "src/ddstool.cpp")
target_link_libraries(ddstool libdftoolbox)

add_executable(shark3d "src/shark3d_main.cpp")
target_link_libraries(shark3d libdftoolbox)

add_executable(stfu4 "src/stfu4.cpp")
target_link_libraries(stfu4 libdftoolbox)

add_executable(xarc "src/xarc.cpp")
target_link_libraries(xarc libdftoolbox)

add_executable(xmg "src/xmg.cpp")
target_link_libraries(xmg libdftoolbox)

add_executable(xrc "src/xrc.cpp")
target_link_libraries(xrc libdftoolbox)

add_executable(cir "src/cir.cpp")
target_link_libraries(cir libdftoolbox)

add_executable(dfmodtool
  "src/dfmodactivator.cpp"
  "src/pak_insert_thread.cpp"
  "src/icons.cpp"
  "src/progress_dialog.cpp"
  "src/progress_logger.cpp"
  # + (cross and ['dfmodtool-res.o'] or []),
  )
target_link_libraries(dfmodtool PRIVATE
  libdftoolbox
  PkgConfig::FOX)

add_executable(dftoolbox
  "src/dftoolbox.cpp"
  "src/shark_view.cpp"
  "src/image_view.cpp"
  "src/dialog_view.cpp"
  "src/searchbar.cpp"
  "src/export_dialog.cpp"
  "src/dreamfall_file_entry.cpp"
  "src/directory_view.cpp"
  "src/sound_view.cpp"
  "src/icons.cpp"
  "src/progress_dialog.cpp"
  "src/progress_logger.cpp"
  # (cross and ['dftoolbox-res.o'] or []),
  )
target_link_libraries(dftoolbox PRIVATE
  libdftoolbox
  PkgConfig::FOX
  PkgConfig::SDL
  PkgConfig::SDL_MIXER)
install_with_wrapper(dftoolbox)

# EOF #
