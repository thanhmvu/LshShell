FLAGS = -O2 -lpthread

compile:
	gcc main.c signal_handler.c job.c csapp.c $(FLAGS) -o ../install/bin/main
	gcc infinite_loop.c csapp.c -o ../install/bin/infinite_loop

run:
	../install/bin/main

clean:
	rm -rf ../install/bin/main

clean-all:
	rm -rf ../install/bin/*