cmake_minimum_required(VERSION 3.10)

project(textualcosmic)

add_executable(textualcosmic CosmicDeck.cpp CosmicServer.cpp DestinyDeck.cpp GameEvent.cpp GameState.cpp PlayerColors.cpp PlayerInfo.cpp PlayerRole.cpp TextualCosmic.cpp TurnPhase.cpp)

add_subdirectory(Aliens)
add_subdirectory(client)

target_link_libraries(textualcosmic PUBLIC Aliens)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Always build as debug for now?
target_compile_options(textualcosmic PUBLIC -g -O3 -std=c++17 -Wall -pedantic)

install(TARGETS textualcosmic DESTINATION bin)
