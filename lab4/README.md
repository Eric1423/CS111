# Hey! I'm Filing Here

In this lab, I successfully implemented the following 
Create a 1 MiB ext2 file system with 2 directories, 1 regular file, and 1 symbolic link.


## Building

‘make'

## Running

use `./ext2-create` to run the executable to create cs111-base.img
and use `mkdir mnt` to Create a directory to mount the filesystem to
`sudo mount -o loop cs111-base.img mnt` to mount the filesystem
use `ls -ain` To check the result, go to the mnt directory  by `cd mnt`

make # compile the executable
./ ext2 - create # run the executable to create cs111 -base.img
dumpe2fs cs111 -base.img # dumps the filesystem information to help debug
fsck.ext2 cs111 -base.img # this will check that your filesystem is correct
mkdir mnt # create a directory to mnt your filesystem to
sudo mount -o loop cs111 -base.img mnt # mount your filesystem , loop lets you use a file
sudo umount mnt # unmount the filesystem when you 're done
rmdir mnt # delete the directory used for mounting when you 're done


## Cleaning up

make clean
