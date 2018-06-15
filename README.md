# MiniShell

Shell name: Lafayette Shell `lsh`   
Authors: Huy Nguyen, Thanh Vu

## How to run
To build the project, simply run

	make

which will generate three executable files in HuyThanh/install/bin/
These files are:

* lsh - the main shell program. To run lsh from src/, enter   

	`../install/bin/lsh`

* loop - a program that runs an infinite loop. To run loop from src/, enter   

	`../install/bin/loop`

* foo - a program sleeps for a number of seconds and then prints out a message "ending now". foo takes one argument from the command line which is interpreted as the sleep time (in seconds). If no arguments are provided, the default sleep time is 5 seconds. To run foo from src/, enter   

	`../intall/bin/foo [time]`

loop and foo are two example object files that can be invoked within the lsh shell. Other available commands are documented in the man page, which can be shown by running from src/,   

	`man ../install/man/lsh`

## Sample routine
```
fire@os5:~/LshShell/src$
fire@os5:~/LshShell/src$ make
gcc main.c utilities.c signal_handler.c job.c csapp.c -pthread -m64 -o ../install/bin/lsh

gcc foo.c -o ../install/bin/foo
gcc loop.c -o ../install/bin/loop 
fire@os5:~/LshShell/src$ ../install/bin/lsh
lsh> 
lsh> cowsay hello you
| hello you |
 \ 
  \ ^__^
    (oo)\_______
    (__)\       )\/\
        ||----w |
        ||     ||
lsh> 
lsh> lshprompt=huy/thanh/cow/shell
huy/thanh/cow/shell> authors=huy-nguyen_thanh-vu
huy/thanh/cow/shell> echo $authors
huy-nguyen_thanh-vu
huy/thanh/cow/shell> authors=
huy/thanh/cow/shell> echo $authors

huy/thanh/cow/shell> ls | more | sort | grep .c
csapp.c
docs
foo.c
job.c
loop.c
main.c
signal_handler.c
utilities.c
huy/thanh/cow/shell> ../install/bin/foo 7 &  
[0] 17469 Running 	 ../install/bin/foo 7 & 

huy/thanh/cow/shell> ../install/bin/foo 7 &  
[1] 17470 Running 	 ../install/bin/foo 7 &  

huy/thanh/cow/shell> 
huy/thanh/cow/shell> 
huy/thanh/cow/shell> ending now 

huy/thanh/cow/shell> 
huy/thanh/cow/shell> ending now 

huy/thanh/cow/shell> stats -l
No stats enabled
huy/thanh/cow/shell> stats -uvi
huy/thanh/cow/shell> stats -l
u: user mode is enabled
v: voluntary context switches is enabled
i: involuntary context switches is enabled
huy/thanh/cow/shell> 
huy/thanh/cow/shell> 
huy/thanh/cow/shell> ls
csapp.c  csapp.h  docs	foo.c  job.c  job.h  loop.c  main.c  Makefile  man  README  signal_handler.c  signal_handler.h	utilities.c  utilities.h

[STATS] user mode: 0.000000 seconds
[STATS] voluntary context switches: 19
[STATS] involuntary context switches: 67
huy/thanh/cow/shell> 
huy/thanh/cow/shell> stats -c
huy/thanh/cow/shell> stats -l
No stats enabled
huy/thanh/cow/shell> q
fire@os5:~/LshShell/src$ 
```
