control: main.o NL_UGVLink.o arduino-serial-lib.o inet_comm.o
	gcc $(CFLAGS) -Wall -o  control -lpthread main.o NL_UGVLink.o arduino-serial-lib.o inet_comm.o

main.o:  main.c NL_UGVLink.h arduino-serial-lib.h inet_comm.h
	gcc $(CFLAGS) -Wall -c main.c

arduino-serial-lib.o: arduino-serial-lib.c arduino-serial-lib.h
	gcc $(CFLAGS) -Wall -c arduino-serial-lib.c

NL_UGVLink.o: NL_UGVLink.c NL_UGVLink.h
	gcc $(CFLAGS) -Wall -c NL_UGVLink.c

inet_comm.o: inet_comm.c inet_comm.h
	gcc $(CFLAGS) -Wall -c inet_comm.c


clean:
	rm *.o control




