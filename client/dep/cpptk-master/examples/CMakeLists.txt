cmake_minimum_required(VERSION 3.10)

if(WIN32)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .dll)
ELSE()
	SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
ENDIF()

project(example1)

get_filename_component(PARENT_DIR ../ ABSOLUTE)
add_subdirectory(${PARENT_DIR} build_cpptk)

add_executable(example1 example1.cc)

if(WIN32)
	target_include_directories(example1 PUBLIC "C:/Program Files/Tcl86/include")
	target_compile_options(example1 PUBLIC -std=c++17)
else()
	target_include_directories(example1 PUBLIC /usr/include/tcl8.6) #TODO: Package this directly
	target_include_directories(example1 PUBLIC /usr/include/) #For tk, which should also be packaged directly
	target_compile_options(example1 PUBLIC -std=c++17 -Wno-parentheses -Wall -Wno-long-long -pedantic)
endif()

target_link_libraries(example1 cpptk)

install(TARGETS example1 DESTINATION bin)

project(example2)

add_executable(example2 example2.cc)

if(WIN32)
	target_include_directories(example2 PUBLIC C:/Program Files/Tcl86)
	target_compile_options(example2 PUBLIC -std=c++17)
else()
	target_include_directories(example2 PUBLIC /usr/include/tcl8.6) #TODO: Package this directly
	target_include_directories(example2 PUBLIC /usr/include/) #For tk, which should also be packaged directly
	target_compile_options(example2 PUBLIC -std=c++17 -Wno-parentheses -Wall -Wno-long-long -pedantic)
endif()

target_link_libraries(example2 cpptk)

install(TARGETS example2 DESTINATION bin)

project(cosmic)

add_executable(cosmic cosmic.cpp)

if(WIN32)
	target_include_directories(cosmic PUBLIC C:/Program Files/Tcl86)
	target_compile_options(cosmic PUBLIC -std=c++17)
else()
	target_include_directories(cosmic PUBLIC /usr/include/tcl8.6) #TODO: Package this directly
	target_include_directories(cosmic PUBLIC /usr/include/) #For tk, which should also be packaged directly
	target_compile_options(cosmic PUBLIC -std=c++17 -Wno-parentheses -Wall -Wno-long-long -pedantic)
endif()

target_link_libraries(cosmic cpptk)

install(TARGETS cosmic DESTINATION bin)
