lagrange: lagrange.cc
	g++ -Wall -Wextra -O2 -o lagrange lagrange.cc -lemon -I ../lemon/include -L ../lemon/lib -lboost_program_options

debug: lagrange.cc
	g++ -Wall -Wextra -g -o lagrange lagrange.cc -lemon -I ../lemon/include -L ../lemon/lib -lboost_program_options
