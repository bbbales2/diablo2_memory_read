.DEFAULT_GOAL := a.out

a.out: main.cpp
	clang++ -std=c++11 -m32 main.cpp
	sudo chown root:root a.out
	sudo chmod +s a.out
