all:
	gcc veterinaria.c p2-dogServer.c -lm -pthread -o ProgramaServidor
	gcc p2-dogClient.c -lm -lncurses -o ProgramaCliente
	mv ProgramaServidor Servidor
	mv ProgramaCliente Cliente


compile:
