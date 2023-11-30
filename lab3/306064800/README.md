# Hash Hash Hash
Implement a hash table that is safe to use concurrently with two locking strategies and compare them with the base implementation.
## Building
```shell
make
```
## Running
```shell
`./hash-table-tester -t [number of threads] -s [number of entries]`
`./hash-table-tester -t 8 -s 50000`
Generation: 72,126 usec
Hash table base: 1,045,068 usec
  - 0 missing
Hash table v1: 1,263,738 usec
  - 0 missing
Hash table v2: 362,863 usec
  - 0 missing
```

## First Implementation
In the initial setup, a static variable pthread_mutex_t foo_mutex is established and set up at the start. Subsequently, within the hash_table_v1_add_entry function, a singular lock covers the whole segment, meaning it's activated at the function's commencement and released at its conclusion. This lock is then dismantled in the hash_table_v1_destroy function.

This approach is effective since the mutex is engaged prior to conducting operations like adding, retrieving, or updating entries, and it's disengaged once all tasks are completed. During this process, additional threads are temporarily halted, preventing them from accessing sensitive data.

### Performance
```shell
 `./hash-table-tester -t 4 -s 100000`
Output 1:
```
Generation: 72,034 usec
Hash table base: 1,024,322 usec
  - 0 missing
Hash table v1: 1,907,585 usec
  - 0 missing
Hash table v2: 444,674 usec
  - 0 missing```
Version 1 is a little slower/faster than the base version. As TODO

## Second Implementation
In the `hash_table_v2_add_entry` function, I TODO
In the second version, pthread_mutex_t foo_mutex is individually declared and initialized for each entry during the hash table's creation. In the hash_table_v2_add_entry function, a lock is applied to each specific entry. This mutex is locked when accessing a list entry and unlocked immediately after updating the value or adding the entry. Consequently, other threads are able to operate concurrently, provided they are working on different entries. The destruction of these mutexes occurs in the hash_table_v2_destroy function.

This method is effective as it secures the most crucial part of each entry. While a thread is engaged in updating or adding an entry, it prevents other threads from accessing that same entry. However, these other threads can still proceed with operations on other entries. This not only prevents errors but also enhances overall efficiency.
### Performance
```shell
./hash-table-tester -t 4 -s 100000

Output:
```
Generation: 72,850 usec
Hash table base: 1,206,084 usec
  - 0 missing
Hash table v1: 2,449,981 usec
  - 0 missing
Hash table v2: 347,911 usec
  - 0 missing
```
The enhanced performance in your second implementation, achieving a speedup of approximately 3.47 times compared to the basic hash table implementation with four threads, aligns well with the number of cores (four) on your machine. This notable improvement can be attributed to the strategic approach adopted in the locking mechanism.

In contrast to the first implementation, where the entire process of adding entries was locked, causing all other threads to wait, the second approach adopts a more granular locking strategy. It focuses on locking only the most critical sections for each entry, such as during the addition or updating of values. This refined method significantly reduces the total wait time. It achieves this by preventing multiple threads from accessing the same entry simultaneously, thereby reducing contention. Moreover, it permits other threads to either wait for a shorter duration or continue working on different entries, thus optimizing the overall efficiency and resource utilization in a multi-threaded environment.

```

TODO more results, speedup measurement, and analysis on v2

## Cleaning up
```shell
make clean
```