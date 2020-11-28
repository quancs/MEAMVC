mavc: main.o meavc.o
	g++ main.o meavc.o -O3 -o meavc
main.o: main.cpp
	g++ -c main.cpp
meavc.o: meavc.cpp meavc.h
	g++ -c meavc.cpp meavc.h

clean:
	rm -f *.o *.h.gch
