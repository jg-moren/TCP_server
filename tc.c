//Exemplo de chamada: ./cli NomeUsu�rio IPUsu�rio PortaUsu�rio IPServidor
#include"sockets.h"
#include <bits/pthreadtypes.h>
#include <stdio.h>

#include <signal.h>
#include <pthread.h>
#include <string.h>

char NOME_USUARIO[TAM_MENSAGEM];
char IP_USUARIO[TAM_MENSAGEM];
char cPORTA_USUARIO[TAM_MENSAGEM];
int PORTA_USUARIO;
char IP_SERVIDOR[TAM_MENSAGEM];

pthread_mutex_t m_r;

#define MAX_HISTORICO 5

char historico[MAX_HISTORICO][TAM_MENSAGEM];
int  hist_count = 0;   /* quantas posições preenchidas (0..MAX_HISTORICO) */
int  hist_next  = 0;   /* próxima posição a escrever (circular) */
pthread_mutex_t m_hist;

void adicionar_historico(const char *msg)
{
    pthread_mutex_lock(&m_hist);

    strncpy(historico[hist_next], msg, TAM_MENSAGEM - 1);
    historico[hist_next][TAM_MENSAGEM - 1] = '\0';

    hist_next = (hist_next + 1) % MAX_HISTORICO;
    if(hist_count < MAX_HISTORICO)
        hist_count++;

    pthread_mutex_unlock(&m_hist);
}

void mostrar_historico()
{

    pthread_mutex_lock(&m_hist);

    int inicio = (hist_next - hist_count + MAX_HISTORICO) % MAX_HISTORICO;

    printf("\n--- Ultimas %d mensagens ---\n", hist_count);
    for(int i = 0; i < hist_count; i++)
    {
        int idice = (inicio + i) % MAX_HISTORICO;
        printf("%s\n", historico[idice]);
    }
    printf("---------------------------\n");
    fflush(stdout);

    pthread_mutex_unlock(&m_hist);
}

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
            char copia[TAM_MENSAGEM];

            pthread_mutex_lock(&m_r);
            printf("\nTCP cliente: (%s)\n",mensagem);fflush(stdout);
            strncpy(copia, mensagem, TAM_MENSAGEM - 1);
            copia[TAM_MENSAGEM - 1] = '\0';
            pthread_mutex_unlock(&m_r);

            /* Fora do lock de m_r para não aninhar mutexes */
            adicionar_historico(copia);
        }
    }
}

int registro()
{
    char mensagem[TAM_MENSAGEM];
    snprintf(mensagem, sizeof(mensagem), "R000%s|%s|%s|",
             NOME_USUARIO, IP_USUARIO, cPORTA_USUARIO);
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
    pthread_mutex_init(&m_hist,NULL);

    // chamada das pthreads
    if(0 || pthread_create(&t_receber,NULL,receber,NULL)) {
        printf("\nERRO: criando thread.\n");
        return;
    }

    char texto[TAM_MENSAGEM];


    while (scanf("%254s", texto) == 1)
    {
        char mensagem[TAM_MENSAGEM];
        snprintf(mensagem, sizeof(mensagem), "D000%s|%s|", NOME_USUARIO, texto);
        int tam = (int)strlen(mensagem) - 4;
        mensagem[1] = (char)('0' + (tam / 100) % 10);
        mensagem[2] = (char)('0' + (tam / 10) % 10);
        mensagem[3] = (char)('0' + (tam / 1) % 10);

        socket_enviar_mensagem(mensagem, IP_SERVIDOR, PORTA_SERVIDOR_TCP);

        /* Guarda a mensagem enviada no histórico (forma legível) */
        char entrada[TAM_MENSAGEM];
        snprintf(entrada, sizeof(entrada), "Voce: %s", texto);
        adicionar_historico(entrada);

        mostrar_historico();
    }

    pthread_kill(t_receber, 0);
    pthread_mutex_destroy(&m_r);
    pthread_mutex_destroy(&m_hist);
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
