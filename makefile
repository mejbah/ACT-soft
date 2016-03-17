include /home/mejbah/pintool/pin-2.10-45467-gcc.3.4.6-ia32_intel64-linux/source/tools/makefile.gnu.config 
LINKER =${CXX}
#CXXFLAGS = -I$(PIN_HOME)/InstLib -fomit-frame-pointer -Wall -Werror -Wno-unknown-pragmas $(DBG) $(OPT) -MMD
CXXFLAGS = -I$(PIN_HOME)/InstLib -fomit-frame-pointer -Wall -Wno-unknown-pragmas $(DBG) $(OPT) -MMD
CXXFLAGS += `pkg-config --cflags opencv`
OPENCV_LIBS = `pkg-config --libs opencv`
LINK_LIBSTDC=
LINK_PTHREAD=-lpthread
OPT =${COPT} -DPRINT_LOCK -g -DPRINT_FREE -std=c++0x 
# This flag ensures lock, unlock and free statements to be printed in the
# profile files

PKG_CONFIG_PATH := /home/mejbah/opencv-libs/lib/pkgconfig:${PKG_CONFIG_PATH}
export PKG_CONFIG_PATH

MENTALIST_OBJS = mentalist.o read_symbols.o lm.o mlp.o 

ALL_BINARIES = mentalist.so 
#ALL_BINARIES = filter nova_modules.so
all: $(ALL_BINARIES)

mentalist.o : mentalist.cpp read_symbols.h 
	${CXX} ${OPT} $(CXXFLAGS) ${PIN_CXXFLAGS} ${OUTOPT}$@ $< 
lm.o : LM.cpp LM.h MLP.h
	${CXX} ${OPT} $(CXXFLAGS) ${PIN_CXXFLAGS} ${OUTOPT}$@ $< 
mlp.o : MLP.cpp MLP.h
	${CXX} ${OPT} $(CXXFLAGS) ${PIN_CXXFLAGS} ${OUTOPT}$@ $<	
read_symbols.o : read_symbols.cpp read_symbols.h
	${CXX} ${OPT} $(CXXFLAGS) ${PIN_CXXFLAGS} ${OUTOPT}$@ $< 

mentalist.so : $(MENTALIST_OBJS)
	${LINKER} ${PIN_LDFLAGS} ${OPENCV_LIBS} $(LINK_DEBUG) ${LINK_OUT}$@ ${MENTALIST_OBJS} ${PIN_LPATHS} ${PIN_LIBS} $(DBG) ${LINK_LIBSTDC}



## cleaning
clean:
	rm -rf *.d *.so *.o ${ALL_BINARIES}
