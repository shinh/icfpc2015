SRCS := $(wildcard *.cc)
OBJS := $(patsubst %.cc,%.o,$(SRCS))
CXXFLAGS := -O -g -MMD -W -Wall
EXES := play_icfp2015

all: $(EXES)

clean:
	rm -f $(OBJS) $(EXES)

$(OBJS): %.o: %.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@

play_icfp2015: $(OBJS)
	$(CXX) $^ -o $@

-include *.d
