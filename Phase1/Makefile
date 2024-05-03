CC = gcc
CFLAGS = `pkg-config --cflags --libs gtk+-3.0`
LIBS = `pkg-config --cflags --libs gtk+-3.0`
PROG_GUI = GUI
PROG_PROCESS_GENERATOR = process_generator

${PROG_GUI}: ${PROG_GUI}.c
	${CC} ${CFLAGS} -o ${PROG_GUI}.out ${PROG_GUI}.c ${LIBS}

${PROG_PROCESS_GENERATOR}: ${PROG_PROCESS_GENERATOR}.c
	${CC} ${CFLAGS} -o ${PROG_PROCESS_GENERATOR}.out ${PROG_PROCESS_GENERATOR}.c ${LIBS}

build:
	gcc clk.c -o clk.out
	gcc -o scheduler.out scheduler.c -lm
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out
	${CC} ${CFLAGS} -o ${PROG_PROCESS_GENERATOR}.out ${PROG_PROCESS_GENERATOR}.c ${LIBS}
	${CC} ${CFLAGS} -o ${PROG_GUI}.out ${PROG_GUI}.c ${LIBS}

clean:
	rm -f *.out TestCases/processes.txt

testcase:
	./test_generator.out

all: clean build ${PROG_GUI} ${PROG_PROCESS_GENERATOR}

run:
	./${PROG_GUI}.out &
	
