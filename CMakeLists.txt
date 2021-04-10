cmake_minimum_required(VERSION 3.10)

project(digitalcosmic)

add_executable(digitalcosmic CosmicDeck.cpp CosmicServer.cpp DestinyDeck.cpp DigitalCosmic.cpp GameEvent.cpp GameState.cpp PlayerColors.cpp PlayerInfo.cpp PlayerRole.cpp TurnPhase.cpp)

add_subdirectory(Aliens)

target_include_directories(digitalcosmic PUBLIC include)
target_link_libraries(digitalcosmic PUBLIC Aliens)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Always build as debug for now?
target_compile_options(digitalcosmic PUBLIC -g -O3 -std=c++14 -Wall -pedantic)

IF(MINGW)
	#Statically link minGW dependencies so users can install the project and run the binary without minGW
	set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ ${CMAKE_CXX_STANDARD_LIBRARIES}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
ENDIF()

IF(WIN32)
	target_link_libraries(digitalcosmic PUBLIC ws2_32) #Winsock
ENDIF()

install(TARGETS digitalcosmic DESTINATION bin)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "02")
include(CPack)
