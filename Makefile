app: main.o logger.o app_msg.o utils.o
	gcc -o app main.o logger.o app_msg.o utils.o -lpthread -lrt
main.o: main.c logger.h app_msg.h utils.h
	gcc -Wall -c main.c
logger.o: logger.c logger.h
	gcc -Wall -c logger.c
app_msg.o: app_msg.c app_msg.h logger.h
	gcc -Wall -c app_msg.c
utils.o: utils.c utils.h logger.h
	gcc -Wall -c utils.c
clean:
	rm app *.o

