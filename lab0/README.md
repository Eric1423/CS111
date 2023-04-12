## UID: 306064800

(IMPORTANT: Only replace the above numbers with your true UID, do not modify spacing and newlines, otherwise your tarfile might not be created correctly)

# A Kernel Seedling

Count and return the current number of running processes

## Building

Explain how to build your kernel module
Write in c language;
I have three functions:

int proc_count(struct seq_file *m, void *v)
int **init proc_count_init(void)
void **exit proc_count_exit(void)

when the init being called, it will then call proc_count, find the file name
then call exit function when done

## Running

Explain how to run your kernel module and what to expect
make
sudo insmod proc_count.ko
cat /proc/count
it will first make, generate make files, and then load the module
the last command will return the total number of running processes

## Cleaning Up

Explain how to remove your kernel module and clean up the code

in order to remove current kernal module which has been loaded, call
sudo insmod \*your_kernel_name.ko
in the end of the program: call proc_remove(entry) in \_\_exit proc_count_exit(void)

## Testing

Report which kernel release version you tested your module on
(hint: use `uname`, check for options with `man uname`).
It should match release numbers as seen on <https://www.kernel.org/>.
5.14.8
uname -r
