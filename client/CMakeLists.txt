#Add in cpptk GUI library
add_subdirectory(dep/cpptk-master)

add_executable(client TextualCosmicClient.cpp ../CosmicDeck.cpp ../GameEvent.cpp) #TODO: Make a shared lib for files needed by both the client and server
target_include_directories(client PUBLIC ..)
target_compile_options(client PUBLIC -g -O3 -std=c++14 -Wall -pedantic)

IF(WIN32)
	target_link_libraries(client ws2_32) #Winsock
	target_include_directories(client PUBLIC C:/Program Files/Tcl86)
ELSE()
	target_include_directories(client PUBLIC /usr/include/tcl8.6)
	target_include_directories(client PUBLIC /usr/include) #For Tk...is this necessary?
ENDIF()

target_link_libraries(client cpptk)

install(TARGETS client DESTINATION bin)
