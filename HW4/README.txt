CS 3013 Project 4 - Scheduling Policies

Workloads:
  workload_1.in:
    Round robin scheduling will perform the same as FIFO if all jobs are below the
    specified time slice. FIFO makes all jobs that run have the same wait time
    and response time. Therefore, the workload_1 has all jobs of length 2 which is
    less than the time slice of 3.
  workload_2:
    Front-loading the large jobs will make FIFO perform much worse than SJF in terms
    of turnaround time. Therefore, the workload_2 has one large job followed by a
    some jobs of length 1. The length of the first large job and the number of length
    1 jobs was found by trial and error until the turnaround time for FIFO was about
    10 times that of SJF's.
  workload_3:
    Round robin scheduling will perform the same as FIFO if all jobs are below the
    specified time slice. SJF will perform the same as FIFO is all the jobs are of the
    same length. With all the scheduling policies performing as FIFO all of the average
    metrics will be the same.
  workload_4:
    The average wait time of RR can be kept low by having jobs that have a length
    of less than the time slice in the beginning and putting one large job at the end.
    The length of the last job does not affect the average wait time and can be changed
    to increase the average turnaround time to greater than 100 time units.
  workload_5:
    Given that the first job has length 3 and that there are only 3 jobs total,
    to get an average response time of 5 the response times of the 3 jobs must sum
    to 15. This can be done by only changing the length of the second job. To get
    an average turnaround time of 13 the turnaround times must sum to 39. After the
    length of the second job is found the length of the third job can be adjusted
    to meet that requirement.

Scheduling Policies - Implementation:
  FIFO:
    FIFO did not need any extra data structures to implement. It just worked its
    way down the linked list of jobs and completes them as they are encountered.
  SJF:
    SJF did not need any extra data structures to implement. It just walks the
    linked list of jobs to find the shortest one and then does that job. This
    repeats until there are no jobs left.
  RR:
    In my RR implementation if a job cannot be completed within one time slice
    then the field for length is updated to record progress and then that job
    is moved to the end of the jobs linked list. The ordering of the jobs
    linked list determines the ordering of the round robin behavior.

  Scheduling Policies - Analysis:
    FIFO:
      A linked list containing all of the metrics for the jobs was added to as
      FIFO ran. Using an integer to keep track of the current time in the
      execution all of the metrics for FIFO could be calculated as each job was
      completed.
    SJF:
      The analysis for SJF is the same as for FIFO and the metrics data was added
      after each job ran to completion.
    RR:
      The analysis for round robin also used the same metrics linked list at in
      FIFO and SJF. However, I also needed to add a field in the job struct to
      keep track of the time that each job was stopped in the case of the job
      having a length that is greater than the time slice. This was used to
      calculate the correct wait time.

      The response time is set when a job is added to the metrics list for the
      first time.

      The turnaround time is set when a job is completed.

      The wait time is first set as the response time and then added to later
      if it has not been completed yet.
