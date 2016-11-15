
LIBS=-lz
CC=g++
LDFLAGS=-I.


IDIR=includes
SDIR=src
ODIR=$(SDIR)/obj
CPPFLAGS=-std=c++11 -O3 -Wall -I$(IDIR)/

_DEPS = Cache.hpp Config.hpp Cpu.hpp debug.hpp defines.hpp Ins.hpp Mem_Hierarchy.hpp Mem_Interface.hpp Memory.hpp Mmu.hpp Request.hpp Response.hpp Simulate.hpp Trace.hpp Cpu_Prefetch_InsOnly.hpp Cache_L1i_Standalone.hpp Br_Predictor.hpp Driver.hpp 2level.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Trace.o Cpu.o Simulate.o Config.o debug.o Ins.o main.o Cache.o Mem_Interface.o Memory.o Mmu.o Request.o Response.o Mem_Hierarchy.o Cpu_Prefetch_InsOnly.o Cache_L1i_Standalone.o Br_Predictor.o 2level.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEP)
	$(CC) -c -o $@ $< $(CPPFLAGS)

mp_tc_sim: $(OBJ)
	$(CC) -o $@ $^ $(CPPFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

# Trace.o: Trace.cpp
# 	g++ $(CPPFLAGS) Trace.cpp

# Cpu.o: Cpu.cpp
# 	g++ $(CPPFLAGS) Cpu.cpp

# Simulate.o: Simulate.cpp
# 	g++ $(CPPFLAGS) Simulate.cpp

# Config.o: Config.cpp
# 	g++ $(CPPFLAGS) Config.cpp

# debug.o: debug.cpp
# 	g++ $(CPPFLAGS) debug.cpp

# Ins.o: Ins.cpp

# mp_tc_sim: main.o Ins.o Trace.o Cpu.o Simulate.o Config.o debug.o
# 	g++ $(LDFLAGS) -o mp_tc_sim main.o Ins.o Trace.o Cpu.o Simulate.o Config.o debug.o $(LDLIBS)

# clean:
# 	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~