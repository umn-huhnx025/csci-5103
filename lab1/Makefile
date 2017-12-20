CXX = g++
CXXFLAGS := -Wall -g -std=c++11 -I.
LIB := -lrt
SRC := uthread.cpp tcb.cpp
BINDIR = ./bin
PROG = test_yield_main test_async_read_main test_create_main test_scheduler_main
LIST = $(addprefix $(BINDIR)/, $(PROG))

all: uthread $(LIST)

uthread: $(SRC) main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIB)

$(BINDIR)/%: $(SRC) tests/%.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIB)

$(BINDIR):
	@mkdir -p bin

clean:
	-rm -rf uthread bin
