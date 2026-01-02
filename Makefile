SRC = main.cpp
TARGET = ${SRC:.cpp=}
LIB_SRCS = sched.cpp task.cpp tts.cpp
FLAGS = -std=c++20

${TARGET}: ${SRC} ${LIB_SRCS}
	g++ -o $@ $^ ${FLAGS} -g