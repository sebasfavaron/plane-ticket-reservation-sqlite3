default:
	gcc server.c -pthread -l sqlite3
	gcc requestHandler.c -pthread -l sqlite3
	gcc client.c -pthread -l sqlite3