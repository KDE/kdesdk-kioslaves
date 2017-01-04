cmake_minimum_required(VERSION 2.6)
project(kioslave)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

macro_optional_find_package(Perl 5.10)

macro_log_feature(PERL_FOUND "Perl" "Perl Programming Language" "http://www.perl.org/" FALSE "5.10" "Required in order to use the perldoc KIOSlave.")

if(PERL_FOUND)
  macro_optional_add_subdirectory(perldoc)
endif(PERL_FOUND)
