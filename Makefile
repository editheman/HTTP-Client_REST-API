CC=gcc
CFLAGS=-Wall

# client: client.o parson.o buffer.o requests.o
# 	$(CC) $^ -o $@

# client.o: client.c
# 	$(CC) $(CFLAGS) $< -c

# parson.o: parson.c
# 	$(CC) $(CFLAGS) $< -c

# buffer.o: buffer.c
# 	$(CC) $(CFLAGS) $< -c

# requests.o: requests.c
# 	$(CC) $(CFLAGS) $< -c

client: client.c requests.c buffer.c parson.c helpers.c
	$(CC) -o client client.c requests.c buffer.c helpers.c parson.c -Wall

run: client
	./client

clean:
	rm -f *.o client