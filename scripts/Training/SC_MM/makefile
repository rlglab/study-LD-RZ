all: mm.cpp
	g++ -O3 -Wall -g -pthread -o mm mm.cpp -lboost_system -lboost_thread

debug:
	g++ -g -pthread -o mm mm.cpp

clean:
	rm -rf mm
	rm -rf *.dat
	rm -rf *.db
	rm -rf *.txt

