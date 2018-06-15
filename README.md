# MiniShell

Lafayette Shell lsh - Huy Nguyen, Thanh Vu

To build the project, simply run

	make

which will generate three executable files in HuyThanh/install/bin/
These files are:

* lsh - the main shell program. To run lsh from src/, enter

	../install/bin/lsh

* loop - a program that runs an infinite loop. To run loop from src/, enter

	../install/bin/loop

* foo - a program sleeps for a number of seconds and then prints out a message "ending now". foo takes one argument from the command line which is interpreted as the sleep time (in seconds). If no arguments are provided, the default sleep time is 5 seconds. To run foo from src/, enter

	../intall/bin/foo [time]

loop and foo are two example object files that can be invoked within the lsh shell. Other available commands are documented in the man page, which can be shown by running from src/,

	man ../install/man/lsh
