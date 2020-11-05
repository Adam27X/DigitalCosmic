add_executable(client TextualCosmicClient.cpp)
target_compile_options(client PUBLIC -g -O3 -std=c++17 -Wall -pedantic)
install(TARGETS client DESTINATION bin)
