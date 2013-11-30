CPP=g++
OPT=-Wall
COMPONLY=-c

all: compile
	$(CPP) xess.o -o xess $(OPT)

compile:
	$(CPP) xess.cpp -o xess.o $(OPT) $(COMPONLY)

clean:
	rm *\.o

install:
	cp xess $(INSTALL_PATH)/xess/xess