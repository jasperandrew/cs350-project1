# Jasper Andrew, Jacob Zwickler
make: lab4.o 
	g++ -o lab4 lab4.o

lab4.o: lab4.cpp
	g++ -std=c++11 -c -o lab4.o lab4.cpp

t: make
	./lab4 $(ARGS)

clean:
	rm *.o *~ lab4 lab4.exe