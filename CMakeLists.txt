cmake_minimum_required(VERSION 3.16)
project(app CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(UI "tui" CACHE STRING "Фронтенд: tui или ray")
set_property(CACHE UI PROPERTY STRINGS tui ray)

add_executable(app_${UI} main.cpp ${UI}_front.cpp)

if(MSVC)
  target_compile_options(app_${UI} PRIVATE /W4)
else()
  target_compile_options(app_${UI} PRIVATE -Wall -Wextra)
endif()

if(UI STREQUAL "tui")
  if(WIN32)
    find_package(unofficial-pdcurses CONFIG REQUIRED)
    target_link_libraries(app_tui PRIVATE unofficial::pdcurses::pdcurses)
  elseif(APPLE)
    target_link_libraries(app_tui PRIVATE ncurses)
  else()
    set(CURSES_NEED_NCURSES TRUE)
    set(CURSES_NEED_WIDE TRUE)
    find_package(Curses REQUIRED)
    target_include_directories(app_tui PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(app_tui PRIVATE ${CURSES_LIBRARIES})
  endif()
elseif(UI STREQUAL "ray")
  find_package(raylib QUIET)
  if(NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      raylib
      GIT_REPOSITORY https://github.com/raysan5/raylib.git
      GIT_TAG 5.5
      GIT_SHALLOW TRUE
    )
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(raylib)
  endif()
  target_link_libraries(app_ray PRIVATE raylib)
  if(APPLE)
    target_link_libraries(app_ray PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
  endif()

else()
  message(FATAL_ERROR "Неизвестный UI '${UI}'. Допустимо: tui, ray")
endif()
