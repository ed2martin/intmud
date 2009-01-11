#ifndef SOCKET_H
#define SOCKET_H

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/types.h>
#endif
#include "var-socket.h"

#define SOCKET_REC 1024
#define SOCKET_ENV 2048

//----------------------------------------------------------------------------
/** Representa uma conex�o, um "socket" */
class TSocket : public TObjSocket /// Socket
{
public:
    TSocket(int socknum);       ///< Construtor
    ~TSocket();                 ///< Destrutor
    void Fechar();              ///< Socket fechou

    static const char * Conectar(int * socknum,
                            const char * ender, int porta,
                            struct sockaddr_in * conSock2);

    static void Fd_Set(fd_set * set_entrada, fd_set * set_saida);
    static void ProcEventos(fd_set * set_entrada, fd_set * set_saida);
    static void SairPend();  ///< Envia dados pendentes (programa vai encerrar)
    bool EnvMens(const char * mensagem);///< Envia mensagem conforme protocolo
private:
    void Processa(const char * buffer, int tamanho);
    bool EnvMens(const char * mensagem, int tamanho); ///< Envia mensagem pura
    void EnvPend();             ///< Envia dados pendentes
    void FecharSock();          ///< Fecha socket
    int  sock;                  ///< Socket; menor que 0 se estiver fechado
    char proto;                 ///< Protocolo
    char cores;                 ///< 0=sem 1=ao receber, 2=ao enviar, 3=com

// Para enviar mensagens
    char bufEnv[SOCKET_ENV];    ///< Cont�m a mensagem que ser� enviada
    unsigned int pontEnv;       ///< N�mero de bytes pendentes em bufEnv
    static bool boolenvpend;    ///< Verdadeiro se tem algum dado pendente

// Para receber mensagens
    char bufRec[SOCKET_REC];    ///< Cont�m a mensagem recebida
                            /**< Cada caracter ocupa dois bytes:
                             * - 1 byte = caracter ASCII
                             * - 1 byte = cor, conforme TObjSocket::CorAtual
                             */
    unsigned int pontRec;       ///< N�mero do byte que est� lendo
    char dadoRecebido;          ///< Para controle da mensagem recebida
                            /**<
                             * - 0 = comportamento padr�o
                             * - 0x0D, 0x0A = para detectar nova linha
                             * - 2 = para detectar seq��ncias de ESC
                             * - 21=recebeu Ctrl-C
                             * - 22=recebeu Ctrl-C X
                             * - 23=recebeu Ctrl-C XX
                             * - 24=recebeu Ctrl-C XX,
                             * - 25=recebeu Ctrl-C XX,X
                             * - 26=recebeu Ctrl-C XX,XX; esperando Ctrl+C
                             */
    char bufTelnet[8];          ///< Para interpretar o protocolo Telnet
    char bufESC[20];            ///< Para interpretar seq��ncias de ESC
    unsigned short pontESC;     ///< Ponteiro em bufESC
    unsigned short pontTelnet;  ///< Ponteiro em bufTelnet

    unsigned char CorAtual;     ///< Para controle da cor
                                /**
                                 * - Bits 0-3 = fundo
                                 * - Bits 4-6 = frente
                                 * - Bit  7 = negrito, 0=desativado
                                 */
    unsigned char CorAnterior;  ///< Com que cor iniciou a linha
    unsigned char CorIRC1;      ///< Para obter cores do IRC
    unsigned char CorIRC2;      ///< Para obter cores do IRC

    static TSocket * sockAtual; ///< Para saber quando apagou socket

// Lista ligada
    static TSocket * sInicio; ///< Primeiro objeto da lista ligada
    TSocket * sAntes;  ///< Objeto anterior da lista ligada
    TSocket * sDepois; ///< Pr�ximo objeto da lista ligada
};

//----------------------------------------------------------------------------

#endif
