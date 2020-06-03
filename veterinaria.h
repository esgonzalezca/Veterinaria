#ifndef VETERINARIA_H_INCLUDED
#define VETERINARIA_H_INCLUDED
#define TamanoVeterinaria 10000000L

struct dogType{
    long id;
    char nombre[32];
    char tipo[32];
    long edad;
    char raza[16];
    long estatura;
    float peso;
    char sexo[2];
    short initialized;
    long next;
    long prev;
};


void veterinaria();
void escribir(struct dogType *a);
long HashFunction(char* nombre);







#endif // VETERINARIA_H_INCLUDED
