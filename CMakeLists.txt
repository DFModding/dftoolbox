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

if(NOT DEFINED PROJECT_VERSION_FULL)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PROJECT_VERSION_FULL LIMIT_COUNT 1)
endif()
# project() only accepts numeric versions
string(REGEX MATCH "^[0-9]+(\\.[0-9]+)*" _version_numeric "${PROJECT_VERSION_FULL}")
project(dftoolbox VERSION ${_version_numeric} LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Windows: no console window, an icon, version information and UTF-8
# as the code page of the file functions (INT-10, INT-11, INT-12)
function(add_windows_resources TARGET DISPLAY_NAME)
  if(WIN32)
    set(ICO_PATH "${CMAKE_SOURCE_DIR}/icons/${TARGET}.ico")
    set(MANIFEST_PATH "${CMAKE_SOURCE_DIR}/dist/windows/app.manifest")
    configure_file(dist/windows/app.rc.in ${TARGET}.rc @ONLY)
    target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.rc)
    set_target_properties(${TARGET} PROPERTIES WIN32_EXECUTABLE TRUE)
  endif()
endfunction()

option(WARNINGS "Switch on extra warnings" OFF)
option(WERROR "Turn warnings into errors" OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Multimedia)

if(WIN32)
  enable_language(RC)
endif()

add_library(libdftoolbox STATIC
  "src/util.cpp"
  "src/system.cpp"
  "src/tljpak.cpp"
  "src/location.cpp"
  "src/savegame.cpp"
  "src/data_manager.cpp"
  "src/shark3d.cpp"
  "src/shark3d_nodes.cpp"
  "src/config.cpp"
  "src/progress_logger.cpp"
  "src/dreamfall_file_entry.cpp"
  "src/model_animation.cpp"
  "src/model_bundle.cpp"
  "src/model_scene.cpp")
set_property(SOURCE "src/system.cpp" APPEND PROPERTY COMPILE_DEFINITIONS
  DFTOOLBOX_VERSION="${PROJECT_VERSION_FULL}"
  DFTOOLBOX_INSTALL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
set_property(SOURCE "src/startup_report.cpp" APPEND PROPERTY COMPILE_DEFINITIONS
  DFTOOLBOX_BUILD_TYPE="$<CONFIG>"
  DFTOOLBOX_TARGET="${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}")

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

add_executable(dftool-model "src/model_main.cpp")
target_link_libraries(dftool-model libdftoolbox)

add_executable(dfmodtool
  "src/dfmodactivator.cpp"
  "src/dfmodactivator.hpp"
  "src/pak_insert_thread.cpp"
  "src/icons.cpp"
  "src/progress_dialog.cpp"
  "src/progress_dialog.hpp"
  "src/startup_report.cpp"
  "src/startup_report.hpp"
  )
target_link_libraries(dfmodtool PRIVATE
  libdftoolbox
  Qt6::Widgets)
add_windows_resources(dfmodtool "DFModTool")

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
  dftool-model
  dfmodtool
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(dftoolbox
  "src/dftoolbox.cpp"
  "src/dftoolbox.hpp"
  "src/shark_view.cpp"
  "src/shark_view.hpp"
  "src/image_view.cpp"
  "src/image_view.hpp"
  "src/model_view.cpp"
  "src/model_view.hpp"
  "src/dialog_view.cpp"
  "src/dialog_view.hpp"
  "src/searchbar.cpp"
  "src/searchbar.hpp"
  "src/export_dialog.cpp"
  "src/export_dialog.hpp"
  "src/directory_view.cpp"
  "src/directory_view.hpp"
  "src/sound_view.cpp"
  "src/sound_view.hpp"
  "src/ui_util.cpp"
  "src/ui_util.hpp"
  "src/icons.cpp"
  "src/progress_dialog.cpp"
  "src/progress_dialog.hpp"
  "src/startup_report.cpp"
  "src/startup_report.hpp"
  )
target_link_libraries(dftoolbox PRIVATE
  libdftoolbox
  Qt6::Widgets
  Qt6::OpenGL
  Qt6::OpenGLWidgets
  Qt6::Multimedia)
add_windows_resources(dftoolbox "DFToolBox")
install(TARGETS dftoolbox RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Shared runtime data of dftoolbox and dfmodtool: icons (PNG and the
# GIFs of the mods), the mods dfmodtool lists and the known filenames
if(WIN32)
  set(DFTOOLBOX_INSTALL_DATA_DIR "data")
else()
  set(DFTOOLBOX_INSTALL_DATA_DIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
endif()

install(DIRECTORY icons DESTINATION ${DFTOOLBOX_INSTALL_DATA_DIR}
  PATTERN "*.xcf" EXCLUDE
  PATTERN "*.ico" EXCLUDE
  PATTERN "hicolor" EXCLUDE)
install(DIRECTORY mods DESTINATION ${DFTOOLBOX_INSTALL_DATA_DIR})

install(FILES df-directory.txt DESTINATION ${DFTOOLBOX_INSTALL_DATA_DIR})

if(NOT WIN32)
  install(FILES dist/dftoolbox.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install(FILES dist/dftoolbox.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
  install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
  install(FILES man/dftoolbox.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

if(BUILD_TESTING)
  add_executable(test-system "tests/test_system.cpp")
  target_include_directories(test-system PRIVATE src)
  target_link_libraries(test-system libdftoolbox)
  add_test(NAME system COMMAND test-system)

  add_test(NAME dftoolbox-version COMMAND dftoolbox --version)
  string(REPLACE "+" "\\+" _version_regex "${PROJECT_VERSION_FULL}")
  set_tests_properties(dftoolbox-version PROPERTIES
    PASS_REGULAR_EXPRESSION "^dftoolbox ${_version_regex}\n$")
  add_test(NAME dftoolbox-help COMMAND dftoolbox --help)
  add_test(NAME dftoolbox-unknown-option COMMAND dftoolbox --no-such-option)
  set_tests_properties(dftoolbox-unknown-option PROPERTIES WILL_FAIL TRUE)
endif()

# EOF #
