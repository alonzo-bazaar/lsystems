# cmake inizialmente fottuto e riadattato da:
# https://github.com/SasLuca/raylib-cmake-template
cmake_minimum_required(VERSION 4.0)
project(lsystems)

#set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_BUILD_TYPE DEBUG)
# https://stackoverflow.com/questions/66147941
# How to enabled gdb -g3 debug level with CMake?
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -ggdb")
# (questo per clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# usiamo gli standard C99 per C (raylib è in C quindi da specificare)
# e C++ 23 per il nostro progetto
# metterei C++ 26 ma non so quanto sia diffuso al momento
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 23)

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
file(GLOB
  LSYSTEMS_SOURCES       # nome della variabile dove mettere i file
  CONFIGURE_DEPENDS      # ricrea il file make quando vengono aggiunti file
  LIST_DIRECTORIES FALSE # non mettere cartelle alla lista di file
  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)
set(LSYSTEMS_MAIN "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp")
# main di test per la parte di json
set(LSYSTEMS_JSON_MAIN "${CMAKE_CURRENT_LIST_DIR}/src/json_main.cpp")
# main di test per la parte di menu
set(LSYSTEMS_MENU_MAIN "${CMAKE_CURRENT_LIST_DIR}/src/menu_main.cpp")

# rimuovi i file di main da lsystem_sources che sarebbero più file di libreria
list(REMOVE_ITEM LSYSTEMS_SOURCES "${LSYSTEMS_MAIN}")
list(REMOVE_ITEM LSYSTEMS_SOURCES "${LSYSTEMS_JSON_MAIN}")
list(REMOVE_ITEM LSYSTEMS_SOURCES "${LSYSTEMS_MENU_MAIN}")

set(
  LSYSTEMS_INCLUDES
  "${CMAKE_CURRENT_LIST_DIR}/include"
  "${CMAKE_CURRENT_LIST_DIR}/thirdparty/nlohmann-json/single_include/nlohmann"
)
set(RAYLIB_INCLUDES "${RAYLIB_DIR}/src")

message("-> -> -> begin some debug information <- <- <-")
message("LSYSTEMS_MAIN=${LSYSTEMS_MAIN}")
message("LSYSTEMS_JSON_MAIN=${LSYSTEMS_JSON_MAIN}")
message("LSYSTEMS_MENU_MAIN=${LSYSTEMS_MENU_MAIN}")
message("")
message("LSYSTEMS_SOURCES=${LSYSTEMS_SOURCES}")
message("--> -> -> end some debug information <- <- <--")

# crea target eseguibile con quanto descritto sopra
add_library("${PROJECT_NAME}_lib" "${LSYSTEMS_SOURCES}")
target_include_directories(
  "${PROJECT_NAME}_lib"
  PUBLIC "${LSYSTEMS_INCLUDES}" "${RAYLIB_INCLUDES}"
)
target_link_libraries(
  "${PROJECT_NAME}_lib"
  PUBLIC raylib
)

set(COMPILE_MAIN_EXE ON)
set(COMPILE_JSON_EXE OFF)
set(COMPILE_MENU_EXE OFF)
set(COMPILE_TEST_EXE OFF)

if(COMPILE_MAIN_EXE)
  add_executable("${PROJECT_NAME}_main" "${LSYSTEMS_MAIN}")
  target_include_directories(
    "${PROJECT_NAME}_main"
    PUBLIC "${LSYSTEMS_INCLUDES}" "${RAYLIB_INCLUDES}"
  )

  target_link_libraries(
    "${PROJECT_NAME}_main"
    PUBLIC raylib
    PUBLIC "${PROJECT_NAME}_lib"
  )
endif()

if(COMPILE_JSON_EXE)
  add_executable("${PROJECT_NAME}_json")
  target_sources(
    "${PROJECT_NAME}_json"
    PUBLIC ${LSYSTEMS_JSON_MAIN} ${LSYSTEMS_SOURCES}
  )
  target_include_directories(
    "${PROJECT_NAME}_json"
    PUBLIC ${LSYSTEMS_INCLUDES} ${RAYLIB_INCLUDES}
  )
  target_link_libraries(
    "${PROJECT_NAME}_json"
    PUBLIC raylib
  )
endif()

if(COMPILE_MENU_EXE)
  add_executable("${PROJECT_NAME}_menu")
  target_sources(
    "${PROJECT_NAME}_menu"
    PUBLIC ${LSYSTEMS_MENU_MAIN} ${LSYSTEMS_SOURCES}
  )
  target_include_directories(
    "${PROJECT_NAME}_menu"
    PUBLIC ${LSYSTEMS_INCLUDES} ${RAYLIB_INCLUDES}
  )
  target_link_libraries(
    "${PROJECT_NAME}_menu"
    PUBLIC raylib
  )
endif()

if(COMPILE_TEST_EXE)
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
  add_executable("${PROJECT_NAME}_test" "${TEST_SOURCES}")
  target_include_directories(
    "${PROJECT_NAME}_test"
    PUBLIC ${LSYSTEMS_INCLUDES} ${RAYLIB_INCLUDES}
  )
  # link test target againt raylib since we're probably gonna use
  # raylib math stuff within tests as well
  target_link_libraries(
    "${PROJECT_NAME}_test"
    PUBLIC raylib
    PUBLIC "${PROJECT_NAME}_lib"
    PUBLIC GTest::gtest_main
  )

  include(GoogleTest)
  gtest_discover_tests("${PROJECT_NAME}_test")
endif()
