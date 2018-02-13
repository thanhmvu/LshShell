compile: main.c job.o csapp.o
	gcc -O2 -lpthread -o ../install/bin/main main.c ../install/bin/job.o ../install/bin/csapp.o

job.o: job.c csapp.o
	gcc -O2 -c job.c -o ../install/bin/job.o ../install/bin/csapp.o

csapp.o: csapp.c
	gcc -O2 -c csapp.c -o ../install/bin/csapp.o

run:
	../install/bin/main
	
clean:
	rm -rf ../install/bin/*.o