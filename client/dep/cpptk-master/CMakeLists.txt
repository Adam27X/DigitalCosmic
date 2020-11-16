add_library(cpptk cpptk.cc)
target_include_directories(cpptk PUBLIC .)

if(WIN32)
	target_include_directories(cpptk PUBLIC "C:/Program Files/Tcl86/include")
	target_include_directories(cpptk PUBLIC "C:/Program Files/boost_1_74_0/boost_1_74_0")
	target_compile_options(cpptk PUBLIC -std=c++17)
else()
	target_include_directories(cpptk PUBLIC /usr/include/tcl8.6) #TODO: Package this directly
	target_include_directories(cpptk PUBLIC /usr/include/) #For tk, which should also be packaged directly
	target_compile_options(cpptk PUBLIC -std=c++17 -Wno-parentheses -Wall -Wno-long-long -pedantic)
endif()

add_subdirectory(base)
target_link_libraries(cpptk cpptkbase)

install(TARGETS cpptk DESTINATION lib)
