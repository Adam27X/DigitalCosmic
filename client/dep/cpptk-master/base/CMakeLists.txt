add_library(cpptkbase cpptkbase.cc)
target_include_directories(cpptkbase PUBLIC .)

if(WIN32)
	target_include_directories(cpptkbase PUBLIC "C:/Program Files/Tcl86/include")
	target_include_directories(cpptkbase PUBLIC "C:/Program Files/boost_1_74_0/boost_1_74_0")
	target_compile_options(cpptkbase PUBLIC -std=c++17)
else()
	target_include_directories(cpptkbase PUBLIC /usr/include/tcl8.6) #TODO: Package this directly
	target_include_directories(cpptkbase PUBLIC /usr/include/) #For tk, which should also be packaged directly
	target_compile_options(cpptkbase PUBLIC -std=c++17 -Wno-parentheses -Wall -Wno-long-long -pedantic)
endif()

if(WIN32)
	find_library(LIBTCL tcl86t HINTS "C:/Program Files/Tcl86/lib")
	find_library(LIBTK tk86t HINTS "C:/Program Files/Tcl86/lib")
	target_link_libraries(cpptkbase PUBLIC ${LIBTCL})
	target_link_libraries(cpptkbase PUBLIC ${LIBTK})
else()
	target_link_libraries(cpptkbase PUBLIC tcl8.6)
	target_link_libraries(cpptkbase PUBLIC tk8.6)
endif()

install(TARGETS cpptkbase DESTINATION lib)
