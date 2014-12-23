gst: main.o NL_UGVLink.o js.o 
	gcc $(CFLAGS) -Wall -o  gst -lpthread main.o NL_UGVLink.o js.o 

main.o:  main.c NL_UGVLink.h js.h 
	gcc $(CFLAGS) -Wall -c main.c

NL_UGVLink.o: NL_UGVLink.c NL_UGVLink.h
	gcc $(CFLAGS) -Wall -c NL_UGVLink.c

js.o: js.c js.h
	gcc $(CFLAGS) -Wall -c js.c
clean:
	rm *.o control




