cmake_minimum_required(VERSION 3.28.1)

set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(nrpause
  VERSION   "1.0.1"
  LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(mini
  GIT_REPOSITORY        https://github.com/metayeti/mINI.git
  GIT_TAG               0.9.18)

set(ER_WITH_HOOKS OFF)
set(ER_NIGHTREIGN ON)
FetchContent_Declare(elden-x
  GIT_REPOSITORY        https://github.com/ThomasJClark/elden-x.git
  GIT_TAG               a223f4e55232cbb63c52aab6e2fc34200cb1169f)
  
add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)

FetchContent_MakeAvailable(mini elden-x)
  
target_compile_definitions(elden-x PUBLIC -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF)

add_library(${PROJECT_NAME} SHARED nrpause.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

add_definitions(-DPROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}>
  ${CMAKE_SOURCE_DIR}/LICENSE.txt
  ${CMAKE_SOURCE_DIR}/nrpause.ini
  ${CMAKE_SOURCE_DIR}/nrpause.me3
  COMMAND_EXPAND_LISTS)

target_link_libraries(${PROJECT_NAME} PRIVATE mINI elden-x)
