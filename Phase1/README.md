# OS Scheduler Project

This project aims to implement an OS scheduler with various scheduling algorithms as part of the Operating Systems course at Cairo University, Faculty of Engineering, Computer Engineering Department. The scheduler will be developed in the C programming language and run on the Linux platform.

## Objectives

The primary objectives of this project are as follows:

- Evaluate and compare different scheduling algorithms.
- Gain practical experience in utilizing Inter-Process Communication (IPC) techniques.
- Optimize the algorithms and data structures for efficient memory and time usage.

## System Description

The system assumes a computer with a single CPU and infinite memory. The scheduler, along with its complementary components, will be developed to track and manage processes using the specified scheduling algorithms.

## Project Structure

The project will be divided into the following components:

1. **Process Generator**: Simulates the creation and initialization of processes, reads input files, prompts the user for the chosen scheduling algorithm and its parameters (if any), and creates the scheduler and clock processes.

2. **Clock**: Emulates an integer time clock, which will be utilized for time-based operations and scheduling decisions.

3. **Scheduler**: Serves as the core component responsible for managing processes and their states. It implements three scheduling algorithms: Non-preemptive Highest Priority First (HPF), Shortest Remaining Time Next (SRTN), and Round Robin (RR).

4. **Process**: Simulates CPU-bound processes that will interact with the scheduler. Each process will notify the scheduler upon termination.

5. **Input/Output**: Handles input and output operations, including reading process information from input files and generating output files containing scheduling logs and performance metrics.

6. **Install Dependencies**: Use pip to install the required Python packages listed in `requirements.txt`:

    ```bash
    pip install -r requirements.txt
    ```

## Contributors

- [abdo mohamed](https://github.com/abdomohamed96)
- [Ali Afifi Hussain](https://github.com/Ali-Afifi-Hussain)
- [Youssef Bahy](https://github.com/EngYoussefBahy)
- [Omar Hashem](https://github.com/omarhashem80)
