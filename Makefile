TEXTUALCOSMIC := textualcosmic

program_CXX_SRCS := $(wildcard *.cpp)
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_INCLUDE_DIRS := .
CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -g -O3 -std=c++17 -Wall -pedantic
LDFLAGS := #-L/usr/lib #Boost

.PHONY: all clean distclean

all: $(TEXTUALCOSMIC)

debug: CXXFLAGS = -g -O3 -std=c++17 -Wall -pedantic -DDEBUG $(EXTRA_FLAGS)
debug: $(TEXTUALCOSMIC)

$(TEXTUALCOSMIC): $(program_CXX_OBJS) 
		$(CXX) $(CPPFLAGS) $(LDFLAGS) $(program_CXX_OBJS) -o $(TEXTUALCOSMIC)

clean:
		@- $(RM) $(TEXTUALCOSMIC) $(program_CXX_OBJS) *~ 

distclean: clean
