# HW2 Synchronization and Concurrency

Solves programming problems centered around maximizing the runtime efficiency of the program by maximizing its multi-threaded concurrency and fairness.

## Problems

### Problem 1 WPI's Summer Spectacular

The program simulates performers performing on a stage. The management of the threads is done with condition variables.

The program will run forever until it receives a SIGINT or a SIGKILL.
Use the "kill" command or ctrl+c to trigger the program's signal handler and
terminate the program.

The code for this problem can be found in "summer.c".

The program creates threads for:
- 15 Flamenco Dancers
- 8 Torch Jugglers
- 2 Soloists

The program maximizes the amount of concurrency within the following rules:
- The stage has slots for 4 performers
- There can only be one type of performer on stage at any one time
- Soloist threads consume all 4 stage slots when performing
- Threads cannot busy wait. They must be made dormant until the next time they are to perform.

Thread fairness is enforced by:
1. One type of performer can only fill the stage one time before picking a new type of performer to perform.
2. Weighting the random pick of the new performer type based on how many there are in each group.
  - There are 15 dancers so they can fill the stage ~4 times. Therefore, the dancer performer type will perform roughly twice as many times as jugglers and soloists.
3. Each type of performer type only has to sleep for 3 performances before they are guaranteed to be picked to perform.

More details can be found in "HW2.pdf".
A more detailed explanation of how the program avoids the starvation of the threads is in "problem1_explanation.txt".

#### Output Snipit

![image](https://user-images.githubusercontent.com/32044950/120704898-27103480-c485-11eb-9e3e-08d941414ccc.png)

### Problem 2 Shipping Shape-Up

The program simulates the creation and processing of a queue of packages. 20 packages are created in a queue with each package containing 1 to 4 processing requirements.
Those are: 
- Weighing
- Barcoding
- X-raying
- Jostling

4 teams of threads are created with 10 threads belonging to each team. The teams are red, blue, green, and yellow.

The goal is to use synchronization primitives to maximize concurrency and avoid deadlock in the program.

A diagram of the simulated environment is shown below
![image](https://user-images.githubusercontent.com/32044950/120705799-43f93780-c486-11eb-8b9d-f0638febcad7.png)

The rules of the simulated environment are:
- Once placed on a station a package cannot be picked back up until all processing requirements are met
  - Packages can only be moved between processing stations using the conveyor belts shown in the diagram
- Only one conveyor belt can be used at any one time
- Fairness in processing packages is enforced between each of the teams of threads

The program avoids deadlock by first constructing an adjacency matrix for a
directed graph that represents future movements of packages that have already
been put onto a processing station. As long as there are no detected cycles in the graph
deadlock in the simulation is avoided.

More details of the assignment can be found in "HW2.pdf".
More in-depth explanation of the solution can be found in "problem2_explanation.txt".

#### Output Snipit

![image](https://user-images.githubusercontent.com/32044950/120707995-fe8a3980-c488-11eb-8d29-938cfc5f61f5.png)

## Usage

### Compilation
- `make`: Default is `make all` to compile both problems 1 and 2.
- `make all`: Compile both problem 1 and 2.
- `make summer`: Compile problem 1.
- `make fedoops`: Compile problem 2.

### Running
#### Problem 1 WPI's Summer Spectacular:
Do `./summer` in a terminal.
Make sure the "seed.txt" file is in the same directory as the executable.

The program will run forever until it receives a SIGINT or a SIGKILL.
Use the "kill" command or ctrl+c to trigger the program's signal handler and
terminate the program.

#### Problem 2 Shipping Shape-Up:
Do `./fedoops` in a terminal.
Make sure the "seed.txt" file is in the same directory as the executable.

The program will terminate after processing 20 packages.
