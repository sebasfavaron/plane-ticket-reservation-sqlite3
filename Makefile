default:
	gcc -pthread -l sqlite3 -o client client.c
	#gcc -pthread -l sqlite3 -o requestHandler requestHandler.c
	gcc -pthread -l sqlite3 -o server server.c
