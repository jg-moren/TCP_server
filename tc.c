//Exemplo de chamada: ./cli NomeUsu�rio IPUsu�rio PortaUsu�rio IPServidor
#include"sockets.h"
#include <bits/pthreadtypes.h>
#include <stdio.h>

#include <signal.h>
#include <pthread.h>
#include <string.h>

const char NOME_USUARIO[TAM_MENSAGEM]; 
const char IP_USUARIO[TAM_MENSAGEM];          
const char cPORTA_USUARIO[TAM_MENSAGEM];
const int PORTA_USUARIO;
const char IP_SERVIDOR[TAM_MENSAGEM];

pthread_mutex_t m_r;

void *receber(void *param)
{
    int sock = criar_socket(PORTA_USUARIO);
    char mensagem[TAM_MENSAGEM];
    
    memset((void *) mensagem,(int) NULL, sizeof(mensagem));
    
    for(;;)
    {
        int status = socket_receber_mensagem(mensagem, sock);
        if(status == 200)
        {
            pthread_mutex_lock(&m_r);
            printf("\nTCP cliente: (%s)\n",mensagem);fflush(stdout);
            pthread_mutex_unlock(&m_r);
        }
    }
}

int registro()
{
    char mensagem[TAM_MENSAGEM];
    memset((void *) mensagem,(int) NULL, sizeof(mensagem));
    strcat(mensagem, "R000");
    strcat(mensagem, NOME_USUARIO);
    strcat(mensagem, "|");
    strcat(mensagem, IP_USUARIO);
    strcat(mensagem, "|");
    strcat(mensagem, cPORTA_USUARIO);
    strcat(mensagem, "|");
    int tam = strlen(mensagem) - 4;
    //mensagem[1] = '0' + (tam / 100) % 10; 
    //mensagem[2] = '0' + (tam / 10) % 10; 
    //mensagem[3] = '0' + (tam / 1) % 10; 

    printf("%s\n", mensagem);
    int status = socket_enviar_mensagem(mensagem, IP_SERVIDOR, PORTA_SERVIDOR_TCP);

    return status;
}


void menu()
{
   int status = registro(); 

    if(status != 200)
    {
        printf("Nao foi possivel conctar com o servidor\n");
        return ;
    }

    pthread_t t_receber;

    pthread_mutex_init(&m_r,NULL);

    // chamada das pthreads
    if(0 || pthread_create(&t_receber,NULL,receber,NULL)) {
        printf("\nERRO: criando thread.\n");
        return;
    }

    char texto[TAM_MENSAGEM];


    for(;;)
    {

        scanf("%s", texto);

        char mensagem[TAM_MENSAGEM];
        memset((void *) mensagem,(int) NULL, TAM_MENSAGEM);
        strcat(mensagem, "D000");
        strcat(mensagem, NOME_USUARIO);
        strcat(mensagem, "|");
        strcat(mensagem, texto);
        strcat(mensagem, "|");
        int tam = strlen(mensagem) - 4;
        mensagem[1] = (char)('0' + (tam / 100) % 10); 
        mensagem[2] = (char)('0' + (tam / 10) % 10); 
        mensagem[3] = (char)('0' + (tam / 1) % 10); 



        socket_enviar_mensagem(mensagem, IP_SERVIDOR, PORTA_SERVIDOR_TCP);

    }

    pthread_kill(t_receber, 0);
    pthread_mutex_destroy(&m_r);
}

int main(int argc, char *argv[])
{
#ifdef WIN
    WORD wPackedValues;
    WSADATA  SocketInfo;
    int      nLastError,
	         nVersionMinor = 1,
	         nVersionMajor = 1;
    wPackedValues = (WORD)(((WORD)nVersionMinor)<< 8)|(WORD)nVersionMajor;
    nLastError = WSAStartup(wPackedValues, &SocketInfo);
#endif

    if (argc != 5)    /* Testa se o n�mero de par�metros est� correto */
    {
        printf("Uso: %s NOME_USUARIO IP_USUARIO PORTA_USUARIO IP_SERVIDOR\n", argv[0]);
        return(1);
    }
    memset((void *) NOME_USUARIO ,(int) NULL,TAM_MENSAGEM);
    strcpy(NOME_USUARIO ,argv[1]); 
    memset((void *) IP_USUARIO ,(int) NULL,TAM_MENSAGEM);
    strcpy(IP_USUARIO ,argv[2]);
    memset((void *) cPORTA_USUARIO ,(int) NULL,TAM_MENSAGEM);
    strcpy(cPORTA_USUARIO ,argv[3]);
    PORTA_USUARIO = atoi(cPORTA_USUARIO);
    memset((void *) IP_SERVIDOR ,(int) NULL,TAM_MENSAGEM);
    strcpy(IP_SERVIDOR ,argv[4]);

    menu();

    return(0);
}
