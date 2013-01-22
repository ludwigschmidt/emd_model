emd_flow: main.cc emd_flow.o emd_flow.h
	g++ -Wall -Wextra -O2 -o emd_flow main.cc emd_flow.o -lboost_program_options

emd_flow.o: emd_flow.cc emd_flow.h
	g++ -Wall -Wextra -O2 -c -o emd_flow.o emd_flow.cc -lemon -I ../lemon/include -L ../lemon/lib
