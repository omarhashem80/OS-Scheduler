PROG = GUI
CC = gcc
CFLAGS = `pkg-config --cflags --libs gtk+-3.0`
LIBS = `pkg-config --cflags --libs gtk+-3.0`
${PROG}: ${PROG}.c
		${CC} ${CFLAGS} -o ${PROG}.out ${PROG}.c ${LIBS}

build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc -o scheduler.out scheduler.c -lm
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  TestCases/processes.txt

testcase:
	./outs/test_generator.out
all: clean build

run:
	./GUI.out