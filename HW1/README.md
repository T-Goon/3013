# HW1 The Process Party

Solves 5 coding problems centering on the creation of different processes in C.

For detailed information on the 5 problems look in "HW1.pdf".

**NOTE:** The code in this assignment was developped on an Ubuntu system and may only work properly on a similar Linux system.

## Problems

### The Prolific Parent

A parent process creates between 10 and 15 children processes. Each child process prints information to the terminal to introduce itself, waits a few seconds, and then 
terminates.

The code for this problem is in "prolific.c".

#### Output Snipit

![image](https://user-images.githubusercontent.com/32044950/120563867-a0514e00-c3d7-11eb-9b84-2adc2452f42c.png)


### Lifespan Generation

A random number of between 7 and 10 generations is chosen. The main process generates a child process and counts for 1 generation. That child process then generates another
that counts for the second generation. This repeats until the chosen number between 7 and 10 is reached. Each of the parent processes will wait for their child process to 
terminate before terminating themselves.

The code for this problem is in "generation.c".

#### Output Snipit

![image](https://user-images.githubusercontent.com/32044950/120563913-b2cb8780-c3d7-11eb-9975-68ed00a5e673.png)

### The Explorer

The main process will randomly change its working directory 5 times and move between:
-  "/home"
-  "/proc
-  "/proc/sys"
-  "/usr"
-  "/usr/bin"
-  "/bin"

After each change in the working directory the main process will create a child process. The child process will run the `ls -alh` command and then terminate. The main process 
waits for the child process to terminate before moving on.

The code for this problem is in "explorer.c".

**NOTE:** This problem may only run on a Linux system.

#### Output Snipit

![image](https://user-images.githubusercontent.com/32044950/120563952-cb3ba200-c3d7-11eb-92ca-712cb6c9945b.png)

### The Slug

This program selects a time between 1 and 5 seconds, waits for that amount of time, runs either `last -d --fulltimes` or `id -u` commands and then terminates.

The code for this problem is in "slug.c".

#### Output Snipit

![image](https://user-images.githubusercontent.com/32044950/120563983-dabaeb00-c3d7-11eb-845e-f963bcc8a6f5.png)

### The Slug Race

The main process makes 4 child processes that will each run the slug executeable. Every .25 seconds the main process will print a note to the terminal indicating which of its
child processes are still running. When any of the children processes complete the main process will report the child processes results. The children processes need not 
complete in any specified order. The main process will terminate once all children have completed running.

The code for this problem is in "slugrace.c".

#### Output Snipit

![image](https://user-images.githubusercontent.com/32044950/120564014-f4f4c900-c3d7-11eb-9e25-f8c2c1ae0946.png)

## Usage
### To Compile:
Put
- "Makefile",
- "prolific.c",
- "generation.c",
- "explorer.c"
- "slug.c", and
- "slugrace.c"
into the same directory.

To compile all files run
`make` or `make all`
in a terminal.

To compile prolific.c run
`make prolific`

To compile generation.c run
`make generation`

To compile explorer.c run
`make explorer`

To compile slug.c run
`make slug`

To compile slugrace.c run
`make slugrace`

### To run:

#### prolific
`./prolific`

#### generation
`./generation`
#### explorer
`./explorer`

#### slug
Put a "seed.txt" file into the same directory as slug.
Make sure it contains only a single integer.
`./slug [1|2|3|4]`

### slugrace
Put a "seed_slug_1.txt", "seed_slug_2.txt", "seed_slug_3.txt", and 
"seed_slug_4.txt" file into the same directory as slugrace.
Make sure they all only contain a single integer.
`./slugrace`

## Data structures and Algorithms:

### prolific
No data structure was used to keep track of background jobs.
The algorithm linearly created and waited for child processes to finish.

### generation
No data structure was used to keep track of background jobs.
The algorithm created and waited for each child process to finish.

### explorer
No data structure was used to keep track of background jobs.
The algorithm created and waited for each child process to finish.

### slug
There were no background jobs to keep track of.

### slugrace
An array was used to keep track of the background jobs' PIDs.
The algorithm checked for any of the child processes terminating every .25
seconds and removes the terminated processes from the array.

## Testing
All of the program where manually and the outputs were compared against the
example outputs provided in the project assignment PDF.
