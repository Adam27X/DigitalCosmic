add_executable(client TextualCosmicClient.cpp ../CosmicDeck.cpp ../GameEvent.cpp) #TODO: Make a shared lib for files needed by both the client and server
target_include_directories(client PUBLIC ..)
target_compile_options(client PUBLIC -g -O3 -std=c++17 -Wall -pedantic)
install(TARGETS client DESTINATION bin)
