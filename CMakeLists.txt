cmake_minimum_required(VERSION 3.10)

project(textualcosmic)

add_executable(textualcosmic CosmicDeck.cpp CosmicServer.cpp DestinyDeck.cpp GameEvent.cpp GameState.cpp PlayerColors.cpp PlayerInfo.cpp PlayerRole.cpp TextualCosmic.cpp TurnPhase.cpp)

add_subdirectory(Aliens)

target_include_directories(textualcosmic PUBLIC include)
target_link_libraries(textualcosmic PUBLIC Aliens)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Always build as debug for now?
target_compile_options(textualcosmic PUBLIC -g -O3 -std=c++14 -Wall -pedantic)

install(TARGETS textualcosmic DESTINATION bin)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "01")
include(CPack)
