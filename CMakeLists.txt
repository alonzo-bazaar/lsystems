# cmake inizialmente fottuto e riadattato da:
# https://github.com/SasLuca/raylib-cmake-template
cmake_minimum_required(VERSION 4.0)
project(lsystems)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# usiamo gli standard C99 per C (raylib è in C quindi da specificare)
# e C++ 17 per il nostro progetto
# standard scelti un po' a caso
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

### configura raylib
#
# opzioni che verrano viste dal cmake di raylib
# non compilare niente apparte la libreria
# (la repo include parecchi esempi e il cmake di default compila pure quelli)
set(BUILD_EXAMPLES FALSE CACHE BOOL "" FORCE)
set(BUILD_GAMES    FALSE CACHE BOOL "" FORCE)
# set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
# raylib l'ho scaricato direttamente in thirdparty, per fare prima
set(RAYLIB_DIR "${CMAKE_CURRENT_LIST_DIR}/thirdparty/raylib-6.0")
add_subdirectory(${RAYLIB_DIR})


### crea l'eseguibile lsystems
#
# sources = tutti i file che finiscono in .cpp presenti in in ./src
# sarebbe a dire, tutti i (nostri) file c++ da compilare per il main
# tranne main.cpp
# 
# nella stringa alla fine della chiamata a file
# "...src/[!main]*.cpp" vuol dire matcha tutti i file .cpp in src/
# che non iniziano con main, è un po' un azzigogolo
# lo fo per adesso, poi aggiungo una libreria lsystem a parte
# per la parte di test visto che non voglio linkare
# i test con raylib quindi mi serve che la variable LSYSTEMS_SOURCES
# includa tutto *tranne* i file in src che dipendono da raylib
# che per adesso è solo il main
# lascia fa' è un macello solo per farlo girare mo', poi lo sistemo

file(GLOB
  LSYSTEMS_SOURCES       # nome della variabile dove mettere i file
  CONFIGURE_DEPENDS      # ricrea il file make quando vengono aggiunti file
  LIST_DIRECTORIES FALSE # non mettere cartelle alla lista di file
  "${CMAKE_CURRENT_LIST_DIR}/src/[!main]*.cpp"
)
set(LSYSTEMS_MAIN "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp")
set(LSYSTEMS_INCLUDES "${CMAKE_CURRENT_LIST_DIR}/include")
set(RAYLIB_INCLUDES "${RAYLIB_DIR}/src")

# crea target eseguibile con quanto descritto sopra
add_executable("${PROJECT_NAME}_main")
target_sources(
  "${PROJECT_NAME}_main"
  PUBLIC ${LSYSTEMS_MAIN} ${LSYSTEMS_SOURCES}
)
target_include_directories(
  "${PROJECT_NAME}_main"
  PUBLIC ${LSYSTEMS_INCLUDES} ${RAYLIB_INCLUDES}
)
target_link_libraries(
  "${PROJECT_NAME}_main"
  PUBLIC raylib
)

### crea l'eseguibile lsystems_test
# usiamo googletest, anch'esso scaricato direttamente in thirdparty
# codice fottuto e riadattato da:
# https://google.github.io/googletest/quickstart-cmake.html

# roba per windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

set(GTEST_DIR "${CMAKE_CURRENT_LIST_DIR}/thirdparty/googletest-1.17.0")
add_subdirectory(${GTEST_DIR})
file(GLOB
  TEST_SOURCES
  CONFIGURE_DEPENDS
  LIST_DIRECTORIES FALSE
  "${CMAKE_CURRENT_LIST_DIR}/test/*.cpp"
)

enable_testing()
add_executable("${PROJECT_NAME}_test")
target_sources(
  "${PROJECT_NAME}_test"
  PUBLIC ${LSYSTEMS_SOURCES}
  PUBLIC ${TEST_SOURCES}
)
target_include_directories(
  "${PROJECT_NAME}_test"
  PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include"
)
target_link_libraries(
  "${PROJECT_NAME}_test"
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests("${PROJECT_NAME}_test")
