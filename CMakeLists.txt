# cmake copiato e modificato da
# https://github.com/SasLuca/raylib-cmake-template
cmake_minimum_required(VERSION 4.0)
project(lsystems)
set(CMAKE_BUILD_TYPE DEBUG)

# usiamo gli standard C 99 e C++ 17 (standard scelti un po' a caso)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

# configura raylib
# opzioni per il cmake di raylib, non compilare niente apparte la libreria
# (la repo include pure degli esempi di solito che vengono compilati pure loro)
set(BUILD_EXAMPLES FALSE CACHE BOOL "" FORCE)
set(BUILD_GAMES    FALSE CACHE BOOL "" FORCE)
# set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)

# raylib l'ho scaricato direttamente in thirdparty, facevo prima che
# usare un submodules di git o fetchcontent di cmake
set(RAYLIB_DIR "${CMAKE_CURRENT_LIST_DIR}/thirdparty/raylib-6.0")
add_subdirectory(${RAYLIB_DIR})

# crea l'eseguibile lsystems
# sources = tutti i file che finiscono in .cpp presenti in in ./src
# sarebbe a dire, tutti i file c++ da compilare per il main (salvo librerie)
file(GLOB
  SOURCES                # nome della variabile dove mettere i file
  CONFIGURE_DEPENDS      # ricrea il file make quando vengono aggiunti file
  LIST_DIRECTORIES FALSE # non mettere cartelle alla lista di file
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
# caretelle dove cercare i file che includiamo
set(INCLUDED "${CMAKE_CURRENT_LIST_DIR}/include")

# crea target eseguibile con quanto descritto sopra
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
  PRIVATE ${SOURCES})
target_include_directories(${PROJECT_NAME}
  PRIVATE ${INCLUDED} "${RAYLIB_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
