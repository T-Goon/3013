All code for CS 3013 project 1.

To Compile:
  Put
  "Makefile",
  "prolific.c",
  "generation.c",
  "explorer.c"
  "slug.c", and
  "slugrace.c"
  into the same directory.

  To compile all files run
  "make" or "make all"
  in a terminal.

  To compile prolific.c run
  "make prolific"

  To compile generation.c run
  "make generation"

  To compile explorer.c run
  "make explorer"

  To compile slug.c run
  "make slug"

  To compile slugrace.c run
  "make slugrace"

Usage:
  prolific-
    "./prolific"
  generation-
    "./generation"
  explorer-
    "./explorer"
  slug-
    "./slug [1|2|3|4]"
  slugrace-
    "./slugrace"

Data structures and Algorithms:
  prolific-
    No data structure was used to keep track of background jobs.
    The algorithm linearly created and waited for child processes to finish.
  generation-
    No data structure was used to keep track of background jobs.
    The algorithm created and waited for each child process to finish.
  explorer-
    No data structure was used to keep track of background jobs.
    The algorithm created and waited for each child process to finish.
  slug-
    There were no background jobs to keep track of.
  slugrace-
    An array was used to keep track of the background jobs' PIDs.
    The algorithm checked for any of the child processes terminating every .25
    seconds and removes the terminated processes from the array.

Testing:
  All of the program where manually and the outputs were compared against the
  example outputs provided in the project assignment PDF.
