FLAGS = -pthread -m64

compile:
	gcc main.c utilities.c signal_handler.c job.c csapp.c $(FLAGS) -o ../install/bin/lsh
	gcc foo.c -o ../install/bin/foo
	gcc loop.c -o ../install/bin/loop
	
clean:
	rm -rf ../install/bin/main

clean-all:
	rm -rf ../install/bin/*
