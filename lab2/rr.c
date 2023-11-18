#include <fcntl.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <unistd.h>

/* A process table entry.  */
struct process
{
  long pid;
  long arrival_time;
  long burst_time;

  TAILQ_ENTRY(process)
  pointers;

  /* Additional fields here */
  long next_arrival_time;
  long time_processed;
  long initial_execution_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

static long
next_int(char const **data, char const *data_end)
{
  long current = 0;
  bool int_start = false;
  char const *d;

  for (d = *data; d < data_end; d++)
  {
    char c = *d;
    if ('0' <= c && c <= '9')
    {
      int_start = true;
      if (ckd_mul(&current, current, 10) || ckd_add(&current, current, c - '0'))
      {
        fprintf(stderr, "integer overflow\n");
        exit(1);
      }
    }
    else if (int_start)
      break;
  }

  if (!int_start)
  {
    fprintf(stderr, "missing integer\n");
    exit(1);
  }

  *data = d;
  return current;
}

static long
next_int_from_c_str(char const *data)
{
  return next_int(&data, strchr(data, 0));
}
struct process_set
{
  long nprocesses;
  struct process *process;
};

static struct process_set
init_processes(char const *filename)
{
  int fd = open(filename, O_RDONLY);
  if (fd < 0)
  {
    perror("open");
    exit(1);
  }

  struct stat st;
  if (fstat(fd, &st) < 0)
  {
    perror("stat");
    exit(1);
  }

  size_t size;
  if (ckd_add(&size, st.st_size, 0))
  {
    fprintf(stderr, "%s: file size out of range\n", filename);
    exit(1);
  }

  char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    perror("mmap");
    exit(1);
  }

  char const *data_end = data_start + size;
  char const *data = data_start;

  long nprocesses = next_int(&data, data_end);
  if (nprocesses <= 0)
  {
    fprintf(stderr, "no processes\n");
    exit(1);
  }

  struct process *process = calloc(sizeof *process, nprocesses);
  if (!process)
  {
    perror("calloc");
    exit(1);
  }

  for (long i = 0; i < nprocesses; i++)
  {
    process[i].pid = next_int(&data, data_end);
    process[i].arrival_time = next_int(&data, data_end);
    process[i].burst_time = next_int(&data, data_end);
    process[i].next_arrival_time = process[i].arrival_time;
    process[i].time_processed = 0;
    process[i].initial_execution_time = -1;

    if (process[i].burst_time == 0)
    {
      fprintf(stderr, "process %ld has zero burst time\n",
              process[i].pid);
      exit(1);
    }
  }

  if (munmap(data_start, size) < 0)
  {
    perror("munmap");
    exit(1);
  }
  if (close(fd) < 0)
  {
    perror("close");
    exit(1);
  }
  return (struct process_set){nprocesses, process};
}

void add_process_to_list(struct process_list *plist, struct process *new_proc, bool prefer_end_on_tie)
{

  // If the list is empty, insert the process at the end and exit the function.
  if (TAILQ_EMPTY(plist))
  {
    TAILQ_INSERT_TAIL(plist, new_proc, pointers);
    return;
  }

  struct process *current;
  // Traverse the list to find the right insertion point for the new process.
  TAILQ_FOREACH(current, plist, pointers)
  {
    // Check the rearrival time to decide whether to insert before the current process.
    if ((prefer_end_on_tie && new_proc->next_arrival_time < current->next_arrival_time) ||
        (!prefer_end_on_tie && new_proc->next_arrival_time <= current->next_arrival_time))
    {
      // Insert the new process before the current one based on the condition and exit.
      TAILQ_INSERT_BEFORE(current, new_proc, pointers);
      return;
    }
  }

  // If no suitable position was found, insert the process at the end of the list.
  TAILQ_INSERT_TAIL(plist, new_proc, pointers);
}

void initialize_linked_list(struct process_list *plist, struct process_set *pset)
{
  // Iterate over each process in the set and insert it into the linked list.
  for (int index = 0; index < pset->nprocesses; ++index)
  {
    // Insert each process into the list.
    // The third parameter 'true' indicates the insertion mode, which you can adjust as per your function's logic.
    add_process_to_list(plist, &(pset->process[index]), true);
  }
}

int compare(const void *a, const void *b)
{
  return (*(long *)a - *(long *)b);
}

long calculate_time_slice(long fixed_quantum, struct process_list *pl, long passed_time)
{
  // Use the fixed quantum if provided.
  if (fixed_quantum > 0)
  {
    return fixed_quantum;
  }

  // Determine the number of processes that have arrived.
  struct process *p;
  long arrived_count = 0;
  TAILQ_FOREACH(p, pl, pointers)
  {
    if (p->arrival_time <= passed_time)
    {
      arrived_count++;
    }
  }

  // Allocate memory to store the consumed times of arrived processes.
  long *consumed_times = (long *)malloc(arrived_count * sizeof(long));
  int index = 0;
  TAILQ_FOREACH(p, pl, pointers)
  {
    if (p->arrival_time <= passed_time)
    {
      consumed_times[index++] = p->time_processed;
    }
  }

  // Sort the consumed times to find the median.
  qsort(consumed_times, index, sizeof(long), compare);

  long median_time_slice;

  if (arrived_count == 0)
  {
    median_time_slice = 1;
  }
  else if (arrived_count % 2 == 0)
  {
    long middle_sum = consumed_times[arrived_count / 2 - 1] + consumed_times[arrived_count / 2];
    median_time_slice = (middle_sum + 1) / 2;
    if (middle_sum % 2 == 0)
    {
      median_time_slice = middle_sum / 2;
    }
  }
  else
  {
    median_time_slice = consumed_times[arrived_count / 2];
  }

  // Ensure the median time slice is at least 1.
  median_time_slice = median_time_slice > 0 ? median_time_slice : 1;

  // Free the allocated memory.
  free(consumed_times);

  return median_time_slice;
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "%s: usage: %s file quantum\n", argv[0], argv[0]);
    return 1;
  }

  struct process_set ps = init_processes(argv[1]);
  long quantum_length = (strcmp(argv[2], "median") == 0 ? -1
                                                        : next_int_from_c_str(argv[2]));
  if (quantum_length == 0)
  {
    fprintf(stderr, "%s: zero quantum length\n", argv[0]);
    return 1;
  }

  struct process_list list;
  TAILQ_INIT(&list);

  long total_wait_time = 0;
  long total_response_time = 0;

  /* Your code here */
  long elapsed_time = 0;
  initialize_linked_list(&list, &ps); // Construct a linked list from the process set

  struct process *curr_proc; // Pointer to the current process
  long curr_quantum;         // Time slice for current round of execution
  long prev_pid = -1;        // Previous process ID for context switching

  while (!TAILQ_EMPTY(&list))
  {
    curr_quantum = calculate_time_slice(quantum_length, &list, elapsed_time);
    curr_proc = TAILQ_FIRST(&list); // Get the first process from the list

    // Advance the elapsed time if the next process hasn't arrived yet
    if (curr_proc->next_arrival_time > elapsed_time)
    {
      elapsed_time = curr_proc->next_arrival_time;
    }
    else if (prev_pid != curr_proc->pid && prev_pid != -1)
    {
      elapsed_time += 1; // Context switching time
    }
    prev_pid = curr_proc->pid; // Update the previous process ID

    // Set the first executed time if this is the first time the process is run
    if (curr_proc->initial_execution_time == -1)
    {
      curr_proc->initial_execution_time = elapsed_time;
    }

    // Check if the process can finish in the current quantum
    if (curr_proc->burst_time - curr_proc->time_processed <= curr_quantum)
    {
      elapsed_time += (curr_proc->burst_time - curr_proc->time_processed);
      total_wait_time += (elapsed_time - curr_proc->arrival_time - curr_proc->burst_time);
      total_response_time += (curr_proc->initial_execution_time - curr_proc->arrival_time);
      TAILQ_REMOVE(&list, curr_proc, pointers); // Remove process from list as it's completed
    }
    else
    {
      // Process can't finish; it consumes its quantum and will be reinserted
      elapsed_time += curr_quantum;

      struct process *new_proc = malloc(sizeof(struct process)); // Create a new process for reinsertion
      *new_proc = *curr_proc;                                    // Copy the current process
      new_proc->next_arrival_time = elapsed_time;
      new_proc->time_processed += curr_quantum; // Update consumed time

      TAILQ_REMOVE(&list, curr_proc, pointers);    // Remove current process
      add_process_to_list(&list, new_proc, false); // Reinsert the process with updated times
    }
  }

  /* End of "Your code here" */

  printf("Average wait time: %.2f\n",
         total_wait_time / (double)ps.nprocesses);
  printf("Average response time: %.2f\n",
         total_response_time / (double)ps.nprocesses);

  if (fflush(stdout) < 0 || ferror(stdout))
  {
    perror("stdout");
    return 1;
  }

  free(ps.process);
  return 0;
}
