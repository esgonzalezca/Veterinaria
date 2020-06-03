#include "stdio.h" //Librerias estandar para flujo de datos y control de pantalla
#include "stdlib.h"
#include <time.h>
#include "string.h"
#include "ctype.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include "veterinaria.h" //Libreria propia
#include <pthread.h>

#define PORT 3535
#define BACKLOG 2

long numOfDogs = 0;
int Serverfd, Clientfd;
int NumClientes;
int numberHilo;
//Funciones iniciar servidor
struct  dataClient
{
	char *ip;
	int clientFd;
	/* data */
};


void sighandler(int signum)
{
	close(Clientfd);
	close(Serverfd);
	exit(1);
}

void WriteLog(char *ip,short opc,char *datos){

	FILE * fp = fopen("serverDogs.log","a");
	time_t timer;
    char *fecha = malloc(35);
	char * log;
    struct tm* tm_info;
	int longitud;
    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(fecha, 34, "FECHA: %Y-%m-%d T %H:%M:%S ", tm_info);

	char cliente[] = "CLIENTE: ";
	
	switch(opc){
		case 1:
			longitud = strlen(fecha) + strlen(cliente) + strlen(ip) + strlen(" inserta ") + strlen(datos) + 1;
			log = malloc(longitud);
			log[0] = '\0';
			strcat(log,fecha);
			strcat(log,cliente);
			strcat(log,ip);
			strcat(log," inserta ");
			strcat(log,datos);
			break;
		case 2:
			longitud = strlen(fecha) + strlen(cliente) + strlen(ip) + strlen(" lee ") + strlen(datos) + 1;
			log = malloc(longitud);
			log[0] = '\0';
			strcat(log,fecha);
			strcat(log,cliente);
			strcat(log,ip);
			strcat(log," lee ");
			strcat(log,datos);
			break;
		case 3:
			longitud = strlen(fecha) + strlen(cliente) + strlen(ip) + strlen(" borra ") + strlen(datos) + 1;
			log = malloc(longitud);
			log[0] = '\0';
			strcat(log,fecha);
			strcat(log,cliente);
			strcat(log,ip);
			strcat(log," borra ");
			strcat(log,datos);
			break;
		case 4:
			longitud = strlen(fecha) + strlen(cliente) + strlen(ip) + strlen(" busca ") + strlen(datos) + 1;
			log = malloc(longitud);
			log[0] = '\0';
			strcat(log,fecha);
			strcat(log,cliente);
			strcat(log,ip);
			strcat(log," busca ");
			strcat(log,datos);
			break;
		default:
			break;
	}
	strcat(log,"\n");
	printf("%s\n",log);
	fwrite(log,longitud,1,fp);
	fclose(fp);
	free(fecha);
	free(log);
	return;
}

void SetUpServer(struct sockaddr_in *server, int *Serverfd, int *opt)
{
	int r;
	*Serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*Serverfd < 0)
	{
		perror("\n-->Error en socket():");
		exit(-1);
	}

	server->sin_family = AF_INET;
	server->sin_port = htons(PORT);
	server->sin_addr.s_addr = INADDR_ANY;
	bzero(server->sin_zero, 8);

	setsockopt(*Serverfd, SOL_SOCKET, SO_REUSEADDR, (const char *)opt, sizeof(int));

	r = bind(*Serverfd, (struct sockaddr *)server, sizeof(struct sockaddr));
	if (r < 0)
	{
		perror("\n-->Error en bind(): ");
		exit(-1);
	}

	r = listen(*Serverfd, BACKLOG);
	if (r < 0)
	{
		perror("\n-->Error en Listen(): ");
		exit(-1);
	}
	return;
}
//Funciones helper manejo datos basicos veterinaria
long sumarPerro()
{ //Funcion para modificar infoVet y mantener numOfDogs entre instancias

	FILE *fp;
	fp = fopen("infoVet.dat", "rb+");
	rewind(fp);
	long numberOfDogs;
	fread(&numberOfDogs, sizeof(numberOfDogs), 1, fp);
	numberOfDogs++;
	fclose(fp);
	fp = fopen("infoVet.dat", "wb+");
	rewind(fp);
	fwrite(&numberOfDogs, sizeof(numberOfDogs), 1, fp);
	fclose(fp);
	return numberOfDogs;
}
long borrarPerro()
{ //Funcion para modificar infoVet y mantener numOfDogs entre instancias
	FILE *fp;
	fp = fopen("infoVet.dat", "rb+");
	rewind(fp);
	long numberOfDogs;
	fread(&numberOfDogs, sizeof(numberOfDogs), 1, fp);
	numberOfDogs--;
	fclose(fp);
	fp = fopen("infoVet.dat", "wb+");
	rewind(fp);
	fwrite(&numberOfDogs, sizeof(numberOfDogs), 1, fp);
	fclose(fp);
	return numberOfDogs;
}
long getNumOfDogs()
{ //Funcion para obtener numOfDogs de infoVet.dat

	FILE *fp;
	fp = fopen("infoVet.dat", "rb+");
	rewind(fp);
	long numberOfDogs;
	fread(&numberOfDogs, sizeof(numberOfDogs), 1, fp);
	fclose(fp);
	return numberOfDogs;
}
//Funcionalidades
struct dogType makeRegister(struct dogType *perro)
{

	//Empezamos la busqueda del lugar en registro, si ya esta ocupada buscamos la siguiente y etc..

	FILE *fp = fopen("dataDogs.dat", "rb+");

	long hashNumber = HashFunction(perro->nombre);
	struct dogType *perroCopia = malloc(sizeof(struct dogType));

	fseek(fp, (sizeof(struct dogType) * hashNumber), SEEK_SET);
	fread(perroCopia, sizeof(struct dogType), 1, fp);

	perro->initialized = 1;

	//Caso 1: Si la posicion del hash esta vacia, guardamos el perro en esa posicion
	if (perroCopia->initialized == 0)
	{
		rewind(fp);
		fseek(fp, (sizeof(struct dogType) * hashNumber), SEEK_SET);
		perro->next = -1;
		perro->prev = -1;
		perro->id = hashNumber;
		fwrite(perro, sizeof(struct dogType), 1, fp);
	}
	else
	{
		//Caso 2: la posicion del hash esta ocupada por un perro
		// ... entonces de igual forma se sigue la lista enlazada para saber donde unirlo
		while (perroCopia->next != -1)
		{
			long siguiente = perroCopia->next;
			rewind(fp);
			fseek(fp, (sizeof(struct dogType) * siguiente), SEEK_SET);
			fread(perroCopia, sizeof(struct dogType), 1, fp);
		};
		perro->prev = perroCopia->id;
		perro->next = -1;
		//buscamos un lugar libre
		while (!feof(fp))
		{
			fread(perroCopia, sizeof(struct dogType), 1, fp);
			if (perroCopia->initialized == 0)
			{
				rewind(fp);
				fseek(fp, sizeof(struct dogType) * perroCopia->id, SEEK_SET);
				break;
			}
		};
		//escribimos
		perro->id = perroCopia->id;
		fwrite(perro, sizeof(struct dogType), 1, fp);
		//por ultimo como la lista es doble enlazada tenemos que poner el next al anterior
		rewind(fp);
		fseek(fp, sizeof(struct dogType) * perro->prev, SEEK_SET);
		fread(perroCopia, sizeof(struct dogType), 1, fp);
		perroCopia->next = perro->id;
		rewind(fp);
		fseek(fp, sizeof(struct dogType) * perro->prev, SEEK_SET);
		fwrite(perroCopia, sizeof(struct dogType), 1, fp);
	}

	rewind(fp);
	fseek(fp, sizeof(struct dogType) * perro->id, SEEK_SET);
	fread(perro, sizeof(struct dogType), 1, fp);
	numOfDogs = sumarPerro();
	return *perro;
}

struct dogType showRegister(long id, int clientfd)
{ //Opcion 2

	struct dogType *dog = malloc(sizeof(struct dogType));
	int r;
	//Se busca en el archivo el struct correspondiente al id
	FILE *fp = fopen("dataDogs.dat", "rb+");
	rewind(fp);
	fseek(fp, (sizeof(struct dogType) * id), SEEK_SET);
	fread(dog, sizeof(struct dogType), 1, fp);
	fclose(fp);
	r = send(clientfd, dog, sizeof(struct dogType), 0);
	if (r < 0)
	{
		perror("\n-->send error: ");
		exit(-1);
	}
	if (dog->initialized == 0)
	{
		printf("%s", "El id no es valido\n");
		return *dog;
	}

	//Esperamos si el cliente quiere ver la historia medica
	short respuesta;
	r = recv(clientfd, &respuesta, sizeof(short), 0);
	//printf("%d\n", respuesta);
	if (r < 0)
	{
		perror("\n-->recv error: ");
		exit(-1);
	}
	
	if (respuesta == 1)
	{
		
		//Para abrir o crear usamos nano, asi que queremos construir una cadena dirFile...
		//de la manera "nano HM/id.txt para almacenar las Historias Medicas"
		char dirFile_0[] = "HM/";
		char dirFile_1[] = "touch HM/";
		char dirFile_2[] = ".txt";
		char file[21];
		sprintf(file, "%ld", id);
		char *dirFile;
		//nos aseguramos de tener memoria para la construccion del datos dirFile
		if ((dirFile = malloc(strlen(dirFile_1) + strlen(file) + strlen(dirFile_2) + 1)) != NULL)
		{
			dirFile[0] = '\0'; // ensures the memory is an empty datos
			strcat(dirFile, dirFile_1);
			strcat(dirFile, file);
			strcat(dirFile, dirFile_2);
		}
		system(dirFile);
		char *dirFile2;
		if ((dirFile2 = malloc(strlen(dirFile_0) + strlen(file) + strlen(dirFile_2) + 1)) != NULL)
		{
			dirFile2[0] = '\0'; // ensures the memory is an empty datos
			strcat(dirFile2, dirFile_0);
			strcat(dirFile2, file);
			strcat(dirFile2, dirFile_2);
		}

		FILE *fp2 = fopen(dirFile2, "r+"); //Abro la historia

		fseek(fp2, 0, SEEK_END);
		long fsize = ftell(fp2);
		fseek(fp2, 0, SEEK_SET); /* same as rewind(f); */

		char *datos = malloc(fsize + 1);
		fread(datos, 1, fsize, fp2);
		fclose(fp2);

		datos[fsize] = 0;
		r = send(clientfd, &fsize, sizeof(long), 0);
		if (r < 0)
		{
			perror("\n-->send error: ");
			exit(-1);
		}
		if (fsize != 0)
		{
			r = send(clientfd, datos, fsize, 0);
			if (r < 0)
			{
				perror("\n-->send error: ");
				exit(-1);
			}
		}

		r = recv(clientfd, &fsize, sizeof(long), 0);
		if (r < 0)
		{
			perror("\n-->recv error: ");
			exit(-1);
		}
		char *datos2 = malloc(fsize + 1);
		if (fsize != 0)
		{
			r = recv(clientfd, datos2, fsize, 0);
			if (r < 0)
			{
				perror("\n-->recv error: ");
				exit(-1);
			}
			//printf("\n%s\n", datos2);
		}

		FILE *fp3 = fopen(dirFile2, "w"); //Abro la historia
		fwrite(datos2, fsize, 1, fp3);
		fclose(fp3);
		free(datos);
		free(datos2);
	}

	return *dog;
}

short deleteRegister(long id)
{ //Opcion 3

	struct dogType *dog = malloc(sizeof(struct dogType));

	//Se busca y lee en el archivo el struct correspondiente al id
	FILE *fp = fopen("dataDogs.dat", "rb+");
	rewind(fp);
	fseek(fp, (sizeof(struct dogType) * id), SEEK_SET);
	fread(dog, sizeof(struct dogType), 1, fp);

	//Se valida si el struct es un perro valido
	if (dog->initialized == 0)
	{
		short respuesta = dog->initialized;
		fclose(fp);
		free(dog);
		return respuesta;
	}

	struct dogType *dogAux = malloc(sizeof(struct dogType));

	if (dog->prev != -1 && dog->next != -1)
	{ //Caso 1: Borramos cualquier nodo en medio de la LL

		//Leemos el nodo anterior y cambiamos su referencia al next
		fseek(fp, (sizeof(struct dogType) * dog->prev), SEEK_SET);
		fread(dogAux, sizeof(struct dogType), 1, fp);
		dogAux->next = dog->next;
		//Reescribimos
		fseek(fp, (sizeof(struct dogType) * dog->prev), SEEK_SET);
		fwrite(dogAux, sizeof(struct dogType), 1, fp);

		//Leemos el nodo posterior y cambiamos su referencia al prev
		fseek(fp, (sizeof(struct dogType) * dog->next), SEEK_SET);
		fread(dogAux, sizeof(struct dogType), 1, fp);
		dogAux->prev = dog->prev;
		//Reescribimos
		fseek(fp, (sizeof(struct dogType) * dog->next), SEEK_SET);
		fwrite(dogAux, sizeof(struct dogType), 1, fp);

		//Escribimos la struct de forma para que se se elimine
		strcpy(dogAux->nombre, "eliminado");
		dogAux->id = dog->id;
		dogAux->initialized = 0;
		dogAux->prev = -1;
		dogAux->next = -1;
		//Eliminamos el nodo
		fseek(fp, (sizeof(struct dogType) * dog->id), SEEK_SET);
		fwrite(dogAux, sizeof(struct dogType), 1, fp);
	}
	else if (dog->prev == -1 && dog->next != -1)
	{ //Caso 2: Borramos head de la LL

		//Leemos el nodo posterior y cambiamos su referencia al prev por -1 y el id por el del head
		fseek(fp, (sizeof(struct dogType) * dog->next), SEEK_SET);
		fread(dogAux, sizeof(struct dogType), 1, fp);
		dogAux->prev = -1;
		//guardamos el id del nodo en idEliminated para poder borrarlo despues
		long idEliminated = dogAux->id;
		dogAux->id = dog->id;
		//Reescribimos el nodo posterior
		fseek(fp, (sizeof(struct dogType) * dog->id), SEEK_SET);
		fwrite(dogAux, sizeof(struct dogType), 1, fp);

		//Escribimos la struct de forma para que se se elimine
		dogAux->id = idEliminated;
		strcpy(dogAux->nombre, "eliminado");
		dogAux->initialized = 0;
		dogAux->prev = -1;
		dogAux->next = -1;
		//Eliminamos el nodo
		fseek(fp, (sizeof(struct dogType) * idEliminated), SEEK_SET);
		fwrite(dogAux, sizeof(struct dogType), 1, fp);

		//Leemos la nueva head
		fseek(fp, (sizeof(struct dogType) * dog->id), SEEK_SET);
		fread(dogAux, sizeof(struct dogType), 1, fp);

		//Si tiene una referencia a next cambiamos la referencia de este nodo a prev al id actual.
		if (dogAux->next != -1)
		{
			//Leemos el next del nuevo head
			fseek(fp, (sizeof(struct dogType) * dogAux->next), SEEK_SET);
			fread(dogAux, sizeof(struct dogType), 1, fp);
			//Cambiamos la referencia
			dogAux->prev = dog->id;
			//Reescribimos
			fseek(fp, (sizeof(struct dogType) * dogAux->id), SEEK_SET);
			fwrite(dogAux, sizeof(struct dogType), 1, fp);
		}
	}
	else if (dog->next == -1 && dog->prev != -1)
	{ //Caso 3: Borramos Tail de la LL

		//Leemos el nodo previo al tail
		fseek(fp, (sizeof(struct dogType) * dog->prev), SEEK_SET);
		fread(dogAux, sizeof(struct dogType), 1, fp);
		//Le asignamos su referencia a next por -1
		dogAux->next = -1;
		//Reescribimos
		fseek(fp, (sizeof(struct dogType) * dog->prev), SEEK_SET);
		fwrite(dogAux, sizeof(struct dogType), 1, fp);

		//Escribimos la struct de forma para que se se elimine
		dogAux->id = dog->id;
		strcpy(dogAux->nombre, "eliminado");
		dogAux->initialized = 0;
		dogAux->prev = -1;
		dogAux->next = -1;
		//Eliminamos la tail
		fseek(fp, (sizeof(struct dogType) * dog->id), SEEK_SET);
		fwrite(dogAux, sizeof(struct dogType), 1, fp);
	}
	else
	{ //Borramos la unica instancia del nombre en el archivo

		//Buscamos la instancia en el archivo
		fseek(fp, (sizeof(struct dogType) * dog->id), SEEK_SET);
		fread(dogAux, sizeof(struct dogType), 1, fp);
		//Escribimos la struct de forma para que se se elimine
		dogAux->id = dog->id;
		strcpy(dogAux->nombre, "eliminado");
		dogAux->initialized = 0;
		dogAux->prev = -1;
		dogAux->next = -1;
		//Eliminamos el registro
		fseek(fp, (sizeof(struct dogType) * dog->id), SEEK_SET);
		fwrite(dogAux, sizeof(struct dogType), 1, fp);
	}

	short respuesta = dog->initialized;
	numOfDogs = borrarPerro();
	printf("%d", respuesta);
	fclose(fp);
	free(dog);
	free(dogAux);

	return respuesta;
}

void seekRegister(char nombre[32], int *clientfdIntern)
{
	int r;
	//Buscamos el hash del nombre para tener la LL donde esten las ocurrencias
	long identifier = HashFunction(nombre);

	struct dogType *dog = malloc(sizeof(struct dogType));

	//Leemos la estructura head en el archivo correspondiente al hash
	FILE *fp = fopen("dataDogs.dat", "rb+");
	rewind(fp);
	fseek(fp, (sizeof(struct dogType) * identifier), SEEK_SET);
	fread(dog, sizeof(struct dogType), 1, fp);

	//Imprimimos todos los nodos que tengan el mismo nombre en la LL
	int counter = 0;

	char name[32];
	strcpy(name, nombre);

	//cada vez que soliamos imprimir ahora se mandan 2 datos:
	//un mensaje diciendo que hay otro perro con ese nombre
	//el id del perro
	short aviso;
	do
	{

		if (strcmp(name, dog->nombre) == 0)
		{
			aviso = 1;
			r = send(*clientfdIntern, &aviso, sizeof(short), 0);
			if (r < 0)
			{
				perror("\n-->send error: ");
				exit(-1);
			}
			r = send(*clientfdIntern, &(dog->id), sizeof(long), 0);
			if (r < 0)
			{
				perror("\n-->send error: ");
				exit(-1);
			}
			//printf("%s %s %ld\n",dog->nombre," ID: ",dog->id);
			//refresh();
			counter += 1;
		}

		long siguiente = dog->next;

		if (siguiente != -1)
		{
			rewind(fp);
			fseek(fp, (sizeof(struct dogType) * siguiente), SEEK_SET);
			fread(dog, sizeof(struct dogType), 1, fp);

			if (strcmp(name, dog->nombre) == 0 && dog->next == -1)
			{
				aviso = 1;
				r = send(*clientfdIntern, &aviso, sizeof(short), 0);
				if (r < 0)
				{
					perror("\n-->send error: ");
					exit(-1);
				}
				r = send(*clientfdIntern, &(dog->id), sizeof(long), 0);
				if (r < 0)
				{
					perror("\n-->send error: ");
					exit(-1);
				}
				//printf("%s %s %ld\n",dog->nombre," ID: ",dog->id);
				//refresh();
				counter += 1;
			}
		}
	} while (dog->next != -1);

	//se manda el aviso de que ya no hay más perros
	aviso = 0;
	r = send(*clientfdIntern, &aviso, sizeof(short), 0);
	if (r < 0)
	{
		perror("\n-->send error: ");
		exit(-1);
	}

	free(dog);

	return;
}

int *atenderCliente(void *myDataClient) //Funcion switch encargada de desplegar informacion a los hilos
{

	struct dataClient *myCurrentData= malloc(sizeof(struct dataClient));
	myCurrentData = (struct dataClient*)myDataClient;
	//int *fdaux = (int *)Clientefd;
	int clientfdIntern, r;
	clientfdIntern = myCurrentData->clientFd;
	char * ClientsIP = malloc(17);
	strcpy(ClientsIP,myCurrentData->ip);
	long id;

	
	free(myCurrentData);


	int comando;
	struct dogType *myPerro = malloc(sizeof(struct dogType));
	char *LogDatos;
	NumClientes++;
	do
	{	
		LogDatos = malloc(30);
		r = recv(clientfdIntern, &comando, sizeof(int), 0);
		switch (comando)
		{
		case 1:

			r = recv(clientfdIntern, myPerro, sizeof(struct dogType), 0);
			if (r < 0)
			{
				perror("\n-->send error: ");
				exit(-1);
			}
			//printf(myPerro->nombre);
			struct dogType perroComprobar = makeRegister(myPerro);
			//printf(perroComprobar.nombre);
			r = send(clientfdIntern, &perroComprobar, sizeof(struct dogType), 0);
			if (r < 0)
			{
				perror("\n-->send error: ");
				exit(-1);
			}

			sprintf(LogDatos, "%ld", perroComprobar.id);
			WriteLog(ClientsIP,1,LogDatos);
			free(LogDatos);
			break;

		case 2:
			//Obtenemos numero de struct y enviamos a cliente
			numOfDogs = getNumOfDogs();

			r = send(clientfdIntern, &numOfDogs, sizeof(long), 0);
			if (r < 0)
			{
				perror("\n-->send error: ");
				exit(-1);
			}
			//Recibimos el id del struct que el cliente quiere
			r = recv(clientfdIntern, &id, sizeof(long), 0);
			if (r < 0)
			{
				perror("\n-->recv error: ");
				exit(-1);
			}
			//Enviamos la estructura
			*myPerro = showRegister(id, clientfdIntern);
			sprintf(LogDatos, "%ld", myPerro->id);
			WriteLog(ClientsIP,2,LogDatos);
			free(LogDatos);
			
			break;
		case 3:
			//Obtenemos numero de struct y enviamos a cliente
			numOfDogs = getNumOfDogs();
			r = send(clientfdIntern, &numOfDogs, sizeof(long), 0);
			if (r < 0)
			{
				perror("\n-->send error: ");
				exit(-1);
			}
			//Recibimos el id del struct que el cliente quiere
			r = recv(clientfdIntern, &id, sizeof(long), 0);
			if (r < 0)
			{
				perror("\n-->recv error: ");
				exit(-1);
			}
			
			//Enviamos un short con el estado del perro borrado
			short comunismo;
			comunismo = deleteRegister(id);
			r = send(clientfdIntern, &comunismo, sizeof(short), 0);
			if (r < 0)
			{
				perror("\n-->send error: ");
				exit(-1);
			}
			if(comunismo != 0){
				sprintf(LogDatos, "%ld", id);
				WriteLog(ClientsIP,3,LogDatos);
			}
			free(LogDatos);
			break;
		case 4:;
			char nombre[32];
			//recibimos el nombre del perro
			r = recv(clientfdIntern, nombre, sizeof(nombre), 0);
			if (r < 0)
			{
				perror("\n-->recv error: ");
				exit(-1);
			}
			seekRegister(nombre, &clientfdIntern);
			WriteLog(ClientsIP,4,nombre);
			free(LogDatos);
			break;
		case 5:
			free(LogDatos);
			break;

		default:
			break;
		}
	} while (comando != 5);

	//AtenderCliente();
	NumClientes--;
	free(myPerro);
}

int main()
{ //MAIN

	numOfDogs = getNumOfDogs();
	pid_t pid;
	int NumClientes = 0, r, opt = 1;
	numberHilo = 0;
	void sighandler(int);

	signal(SIGINT, sighandler);
	struct sockaddr_in server, client[32];
	socklen_t tamano[32];
	pthread_t hilo[300];
	SetUpServer(&server, &Serverfd, &opt);

	while (NumClientes < 32)
	{

		
		tamano[numberHilo]=sizeof(struct sockaddr_in);
		Clientfd = accept(Serverfd, (struct sockaddr *)&client[numberHilo],&tamano[numberHilo] );

		
		
		
		

		if (Clientfd < 0)
		{
			perror("\n-->Error en accept: ");
			exit(-1);
		}

		struct dataClient *myDataClient=malloc(sizeof(struct dataClient));
		myDataClient->clientFd=Clientfd;
		myDataClient->ip= malloc(17);
		myDataClient->ip=inet_ntoa(client[numberHilo].sin_addr);
		
		
		
		r = pthread_create(&hilo[numberHilo], NULL, (void *)atenderCliente, (void *)myDataClient);
		numberHilo++;
		if (r != 0)
		{
			perror("\n-->pthread_create error: ");
			exit(-1);
		}
	}
	
	return 0;
}
