#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "string.h"
#include "ctype.h"
#include "curses.h"
#include <signal.h>
#include <termios.h>
#include "veterinaria.h"

//Libreria propia

long numOfDogs;

#define PORT 3535

void SetUpClient(struct sockaddr_in *client, int *clientfd, int *r, char *direccion)
{

	*clientfd = socket(AF_INET, SOCK_STREAM, 0);
	if (*clientfd < 0)
	{
		perror("\n-->Error en socket():");
		exit(-1);
	}
	struct sockaddr_in address;
    address.sin_family = AF_INET;

	client->sin_family = AF_INET;
	client->sin_port = htons(PORT);

	inet_aton(direccion, &(client->sin_addr));

	*r = connect(*clientfd, (struct sockaddr *)client, (socklen_t)sizeof(struct sockaddr));
	


	
	
	  refresh();
	if (*r < 0)
	{
		perror("\n-->Error en connect(): ");
		exit(-1);
	}
	return;
}
//Funcionalidades
void makeRegister(int myClientfd)
{ //Opcion 1

	struct dogType *perro = malloc(sizeof(struct dogType));

	char nombre[32];
	printw("Ingrese el nombre del perro: \n ");
	refresh();
	scanw("%32s", nombre);

	char *s = &(nombre[0]); //Formateamos el nombre a Primer caracter mayuscula y el resto minuscula
	while (*s)
	{
		*s = tolower((unsigned char)*s);
		s++;
	}
	s = &(nombre[0]);
	*s = toupper((unsigned char)*s);

	strcpy(perro->nombre, nombre);

	char tipo[32];
	printw("Ingrese el tipo del perro: \n ");
	refresh();
	scanw("%32s", tipo);
	strcpy(perro->tipo, tipo);

	int edad;
	printw("Ingrese la edad del perro: \n ");
	refresh();
	scanw("%d", &edad);
	perro->edad = edad;

	char raza[16];
	printw("Ingrese la raza del perro: \n");
	refresh();
	scanw("%16s", raza);
	strcpy(perro->raza, raza);

	long estatura;
	printw("Ingrese la estatura del perro: \n");
	refresh();
	scanw("%ld", &estatura);
	perro->estatura = estatura;

	float peso;
	printw("Ingrese el peso del perro: \n");
	refresh();
	scanw("%f", &peso);
	perro->peso = peso;

	char sexo[2];
	printw("Ingrese 'M' si es macho o 'F' si es hembra: \n");
	refresh();
	scanw("%2s", sexo);
	strcpy(perro->sexo, sexo);
	//Nos comunicamos con el servidor
	int opcion1 = 1;
	int r;
	r = send(myClientfd, &opcion1, sizeof(int), 0);
	if (r < 0)
	{
		perror("\n-->Error en send(): ");
		exit(-1);
	}
	//sleep(0.1);
	r = send(myClientfd, perro, sizeof(struct dogType), 0);
	if (r < 0)
	{
		perror("\n-->Error en send(): ");
		exit(-1);
	}
	struct dogType *perroVerificar = malloc(sizeof(struct dogType));
	r = recv(myClientfd, perroVerificar, sizeof(struct dogType), 0);
	printw("%s %s %s", "El perro ingresado es", perroVerificar->nombre, "\n");
	printw("%s %d %s", "Su id es:", perroVerificar->id, "\n");
	printw("%s %i %s", "Su estado es:", perroVerificar->initialized, "\n");
	refresh();

	//Guardamos la informacion del nuevo perro en numOfDogs y liberamos memoria

	free(perro);
	free(perroVerificar);
}

void showRegister(int myClientfd)
{

	int opcion2 = 2;
	int r;
	r = send(myClientfd, &opcion2, sizeof(int), 0);
	if (r < 0)
	{
		perror("\n-->Error en send(): ");
		exit(-1);
	}

	//Recibimos la cantidad de perros para mostrarlo en pantalla
	r = recv(myClientfd, &numOfDogs, sizeof(long), 0);
	if (r < 0)
	{
		perror("\n-->recv error: ");
		exit(-1);
	}
	//Se muestra la cantidad de perros y se solicita el id a ver
	printw("%s %ld %s", "Cantidad de perros registrados:\n", numOfDogs, "\n");
	printw("%s", "Ingrese No. ID a ver:");

	//Se manda el id ingresado por el cliente al servidor
	long id;
	scanw("%ld", &id);
	refresh();
	r = send(myClientfd, &id, sizeof(long), 0);
	if (r < 0)
	{
		perror("\n-->send error: ");
		exit(-1);
	}
	//recibimos la estructura enviada por el servidor
	struct dogType *dog = malloc(sizeof(struct dogType));
	r = recv(myClientfd, dog, sizeof(struct dogType), 0);
	if (r < 0)
	{
		perror("\n-->recv error: ");
		exit(-1);
	}

	//Se valida si el struct es un perro valido
	if (dog->initialized == 0)
	{
		printw("%s", "El id no es valido\n");
		refresh();
		printw("Presione cualquier tecla para continuar..\n");
		getch();
		clear();
		return;
	}

	//Se muestra la informacion del perro ingresado
	printw("%s %s %s", "\nEl perro ingresado es", dog->nombre, "\n");
	printw("%s %ld %s", "ID ", dog->id, "\n");
	printw("%s %s %s", "Tipo ", dog->tipo, "\n");
	printw("%s %d %s", "Edad ", dog->edad, "\n");
	printw("%s %d %s", "Estatura ", dog->estatura, " cm \n");
	printw("%s %f %s", "Peso ", dog->peso, " Kg \n");
	printw("%s %s %s", "Raza ", dog->raza, "\n");
	printw("%s %s %s", "Sexo ", dog->sexo, "\n\n");
	printw("%s %i %s", "Su estado es:", dog->initialized, "\n");
	refresh();

	//Se pregunta si se quiere ver la historia medica

	int verificador = 0;
	do
	{
		printw("%s", "Desea ver la historia medica del perro?[S/N]:");
		refresh();
		char respuesta[1];
		short numero = 0;
		scanw("%s", respuesta);
		//Se comparada la entrada del usuario para verificar que sea valida.
		if (strcmp(respuesta, "S") == 0 || strcmp(respuesta, "N") == 0)
		{

			if (strcmp(respuesta, "S") == 0)
			{
				numero = 1;
				r = send(myClientfd, &numero, sizeof(short), 0);
				if (r < 0)
				{
					perror("\n-->send error: ");
					exit(-1);
				}

				refresh();
				long fsize;
				r = recv(myClientfd, &fsize, sizeof(long), 0);
				if (r < 0)
				{
					perror("\n-->recv error: ");
					exit(-1);
				}

				char *datos = malloc(fsize + 1);
				if (fsize != 0)
				{
					r = recv(myClientfd, datos, fsize, 0);
					if (r < 0)
					{
						perror("\n-->recv error: ");
						exit(-1);
					}
				}

				char dirFile_0[] = "temp/";
				char dirFile_1[] = "touch temp/";
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

				FILE *fp2 = fopen(dirFile2, "w"); //Abro la historia
				if (fsize != 0)
					fwrite(datos, fsize, 1, fp2);
				fclose(fp2);

				//Para abrir o crear usamos nano, asi que queremos construir una cadena dirFile...
				//de la manera "nano HM/id.txt para almacenar las Historias Medicas"
				char dirFile_3[] = "nano temp/";

				char *dirFile3 = "";
				//nos aseguramos de tener memoria para la construccion del datos dirFile
				if ((dirFile3 = malloc(strlen(dirFile_3) + strlen(file) + strlen(dirFile_2) + 1)) != NULL)
				{
					dirFile3[0] = '\0'; // ensures the memory is an empty datos
					strcat(dirFile3, dirFile_3);
					strcat(dirFile3, file);
					strcat(dirFile3, dirFile_2);
				}
				//Se abre o crea el archivo en la carpeta HM con el nombre del id del perro
				printw("\n%s\n", dirFile3);
				system(dirFile3);

				printw("Presione cualquier tecla para continuar..\n");
				getch();
				clear();

				FILE *fp3 = fopen(dirFile2, "r"); //Abro la historia
				fseek(fp3, 0, SEEK_END);
				long fsize2 = ftell(fp3);
				fseek(fp3, 0, SEEK_SET); /* same as rewind(f); */
				free(datos);
				char *datos2 = malloc(fsize2 + 1);
				if (fsize2 != 0)
				{

					fread(datos2, 1, fsize2, fp3);

					datos2[fsize2] = 0;
				}
				fclose(fp3);
				r = send(myClientfd, &fsize2, sizeof(long), 0);
				if (r < 0)
				{
					perror("\n-->send error: ");
					exit(-1);
				}
				if (fsize2 != 0)
				{

					r = send(myClientfd, datos2, fsize2, 0);
					printw("\n%s\n", datos2);
					refresh();
					if (r < 0)
					{
						perror("\n-->send error: ");
						exit(-1);
					}
				}
				char rem[] = "rm ";
				char *dirFile4;
				if ((dirFile4 = malloc(strlen(rem) + strlen(dirFile2) + 1)) != NULL)
				{
					dirFile4[0] = '\0'; // ensures the memory is an empty datos
					strcat(dirFile4, rem);
					strcat(dirFile4, dirFile2);
				}
				system(dirFile4);

				free(datos2);
			}
			else
			{
				numero = 0;
				r = send(myClientfd, &numero, sizeof(short), 0);
				if (r < 0)
				{
					perror("\n-->send error: ");
					exit(-1);
				}
			}

			system("clear");

			verificador = -1;
		}
	} while (verificador == 0);

	free(dog);
}

void deleteRegister(int myClientfd)
{

	int opcion3 = 3;
	int r;
	r = send(myClientfd, &opcion3, sizeof(int), 0);
	if (r < 0)
	{
		perror("\n-->Error en send(): ");
		exit(-1);
	}

	//Recibimos la cantidad de perros para mostrarlo en pantalla

	r = recv(myClientfd, &numOfDogs, sizeof(long), 0);
	if (r < 0)
	{
		perror("\n-->recv error: ");
		exit(-1);
	}
	//Se muestra la cantidad de perros y se solicita el id a ver
	printw("%s %d %s", "Cantidad de perros registrados:\n", numOfDogs, "\n");
	printw("%s", "Ingrese No. ID a borrar:");

	//Se manda el id ingresado por el cliente al servidor
	long id;
	scanw("%ld", &id);
	refresh();
	r = send(myClientfd, &id, sizeof(long), 0);
	if (r < 0)
	{
		perror("\n-->send error: ");
		exit(-1);
	}
	//recibimos la estructura enviada por el servidor
	short estado;
	r = recv(myClientfd, &estado, sizeof(short), 0);
	if (r < 0)
	{
		perror("\n-->recv error: ");
		exit(-1);
	}

	//Se valida si el struct es un perro valido
	if (estado == 0)
	{
		printw("%s", "El id no es valido\n");
		refresh();
		return;
	}
	else
	{
		printw("%s %ld %s", "El perro con id: \n", id, "\nfue borrado :)\n");
		refresh();
		return;
	}
}

void seekRegister(WINDOW *w, int myClientfd)
{
	long id;
	system("clear");
	//Preguntamos el nombre a buscar
	printw("%s", "Por favor ingrese el nombre del perro a buscar:");
	refresh();
	char nombre[32];
	scanw("%s", &nombre);

	//Formateamos el nombre a Primer caracter mayuscula y el resto minuscula
	char *s = &(nombre[0]);
	while (*s)
	{
		*s = tolower((unsigned char)*s);
		s++;
	}
	s = &(nombre[0]);
	*s = toupper((unsigned char)*s);

	//Nos comunicamos con el servidor
	int opcion4 = 4;
	int r;
	r = send(myClientfd, &opcion4, sizeof(int), 0);
	if (r < 0)
	{
		perror("\n-->Error en send(): ");
		exit(-1);
	}
	//sleep(0.1);
	r = send(myClientfd, nombre, sizeof(nombre), 0);
	if (r < 0)
	{
		perror("\n-->Error en send(): ");
		exit(-1);
	}

	endwin();
	system("clear");

	//se reciben avisos de que hay un perro más
	//si se recibe un 1, se espera el id del perro y se muestra en pantalla
	//si se recibe un 0, se sale del bucle
	short aviso = 1;
	do
	{
		r = recv(myClientfd, &aviso, sizeof(short), 0);
		if (aviso == 1)
		{
			r = recv(myClientfd, &id, sizeof(long), 0);
			printf("%s %s %ld\n", nombre, " ID: ", id);
		}
	} while (aviso == 1);

	printf("%s", "Presione cualquier tecla para continuar..");

	struct termios info;
	tcgetattr(0, &info);		  /* get current terminal attirbutes; 0 is the file descriptor for stdin */
	info.c_lflag &= ~ICANON;	  /* disable canonical mode */
	info.c_cc[VMIN] = 1;		  /* wait until at least one keystroke available */
	info.c_cc[VTIME] = 0;		  /* no timeout */
	tcsetattr(0, TCSANOW, &info); /* set immediately */

	getchar();
	system("clear");

	return;
}

void MENU(int myClientFd)
{ //Funcion menu ciclica que sera ejecutada en main()

	WINDOW *w = initscr();
	//Inicio la capacidad de utilizar funciones de curses.h

	int opc = 0; //Declaracion de variable para elegir opcion
	int aux = 0; //Variable para salir del ciclo

	do
	{

		clear(); //limpio pantalla

		//Opciones, se usa printw en vez de printf para manejo de pantalla

		printw("MENU\n\n");
		printw("1.Ingresar registro.\n");
		printw("2.Ver registro.\n");
		printw("3.Borrar registro.\n");
		printw("4.Buscar registro.\n");
		printw("5.SALIR.\n");
		refresh();		   //Permite mostrar todos los printw
		scanw("%d", &opc); //pido la opcion deseada por el usuario
		clear();
		//Segun la opcion se va por el caso correspondiente
		switch (opc)
		{
		case 1:
		{
			makeRegister(myClientFd);
			printw("Presione cualquier tecla para continuar..\n");
			getch();
			clear();
			break;
		}
		case 2:
		{
			showRegister(myClientFd);
			// printw("Presione cualquier tecla para continuar..\n");
			// getch();
			// clear();
			break;
		}
		case 3:
		{
			deleteRegister(myClientFd);
			printw("Presione cualquier tecla para continuar..\n");
			getch();
			clear();
			break;
		}
		case 4:
		{
			seekRegister(w, myClientFd);

			break;
		}
		case 5:
		{
			aux = -1;
		}
		};

	} while (aux != -1);

	endwin(); //Termino el manejo de pantalla

	system("clear");
}

int main(int argc, char *argv[])
{

	int clientfd, r;
	struct sockaddr_in client;
	//struct hostent *he;

	SetUpClient(&client, &clientfd, &r, argv[1]);
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	int res = getpeername(clientfd, (struct sockaddr *)&addr, &addr_size);
	char *clientip = (char *)malloc(20 * sizeof(char));
	strcpy(clientip, inet_ntoa(addr.sin_addr));
	

	MENU(clientfd);

	close(clientfd);
}
