BIN := test

CXXFLAGS := -g #-DNDEBUG -O2
LIB :=

SRC := $(wildcard *.cc)
DEP := $(SRC:.cc=.d)
OBJ := $(SRC:.cc=.o)

CXXFLAGS += -MD

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

clean:
	$(RM) *.d *.o $(BIN)

cleanall: clean
	$(RM) tags


ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), cleanall)
sinclude $(DEP)
endif
endif

.PHONY: all clean cleanall
