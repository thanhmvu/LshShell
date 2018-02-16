FLAGS = -pthread -m64

compile:
	gcc main.c signal_handler.c job.c csapp.c $(FLAGS) -o ../install/bin/main
	gcc 10sec_loop.c csapp.c -o ../install/bin/loop
	
clean:
	rm -rf ../install/bin/main

clean-all:
	rm -rf ../install/bin/*