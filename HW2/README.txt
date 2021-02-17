
Compilation:
  "make":
    Default is "make all" to compile both problem 1 and 2.
  "make all":
    Compile both problem 1 and 2.
  "make summer":
    Compile problem 1.

Usage:
  problem 1:
    Do "./summer" in a terminal.
    Make sure the "seed.txt" file is in the same directory as the executable.

    The program will run forever until it receives a SIGINT or a SIGKILL.
    Use the "kill" command or ctrl+c to trigger the program's signal handler and
    terminate the program.
