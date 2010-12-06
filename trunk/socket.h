#ifndef SOCKET_H
#define SOCKET_H

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/types.h>
 #include <netinet/in.h>
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

    static void SockConfig(int socknum);
    static TSocket * Conectar(const char * ender, int porta);
            ///< Cria objeto TSocket a partir de endere�o e porta
            /**< @param ender  Endere�o a conectar
             *   @param porta  Porta
             *   @return Objeto TSocket ou 0 se ocorreu erro
                    (provavelmente endere�o inv�lido) */
    static const char * TxtErro(int erro);
            ///< Fornece a descri��o de um erro a partir do n�mero
            /**< @param erro N�mero do erro
             *   @return Texto contendo o erro
             *   @note Pr�ximas chamadas a essa fun��o podem alterar
             *         a string original */
    static int  Fd_Set(fd_set * set_entrada, fd_set * set_saida, fd_set * set_err);
    static void ProcEventos(int tempoespera, fd_set * set_entrada,
                                fd_set * set_saida, fd_set * set_err);
    static void SairPend();  ///< Envia dados pendentes (programa vai encerrar)
    bool EnvMens(const char * mensagem);
            ///< Envia mensagem conforme protocolo
            /**< @param mensagem Endere�o dos bytes a enviar
             *   @return true se conseguiu enviar, false se n�o conseguiu */
    int  Variavel(char num, int valor);
    const char * Endereco(bool remoto);
            ///< Retorna o endere�o local ou remoto da conex�o
private:
    void Processa(const char * buffer, int tamanho, bool completo);
            ///< Processa dados recebidos em TSocket::ProcEventos
            /**< @param buffer Endere�o do buffer
             *   @param tamanho Tamanho do buffer
             *   @param completo Se recebeu a mensagem completa
             *                  (arg1 do evento _msg) */
    bool EnvMens(const char * mensagem, int tamanho);
            ///< Envia mensagem pura
            /**< @param mensagem Endere�o dos bytes a enviar
             *   @param tamanho Tamanho da mensagem
             *   @return true se conseguiu enviar, false se n�o conseguiu */
    void EnvPend();             ///< Envia dados pendentes
    void FecharSock(int erro, bool env);
            ///< Fecha socket
            /**< @param erro C�digo de erro que fechou o socket
             *   @param env  Se fechou enviando (true) ou recebendo (false) */
    int  sock;                  ///< Socket; menor que 0 se estiver fechado
    int  sockerro;              ///< C�digo de erro que fechou o socket
    char proto;
            ///< Protocolo (quando sock>=0)
            /**< - 0 = conectando
             *   - 1 = Telnet, s� mensagens inteiras
             *   - 2 = Telnet, recebe mensagens incompletas (sem o \\n)
             *   - 3 = IRC
             *   - 4 = Papovox */
    char cores;                 ///< 0=sem 1=ao receber, 2=ao enviar, 3=com
    unsigned char esperatelnet; ///< Para receber mensagens sem "\n"
    char eventoenv:1;           ///< Se deve gerar evento _env
                                /**< @note Caracteres de controle de Telnet
                                  *  n�o devem gerar eventos _env */
    char ecotelnet:1;           ///< Vari�vel socket.eco
    char sockenvrec:1;          ///< 1=socket fechou ao enviar, 0=ao receber
    struct sockaddr_in conSock; ///< Usado principalmente quando proto=0

// Para enviar mensagens
    char bufEnv[SOCKET_ENV];    ///< Cont�m a mensagem que ser� enviada
    unsigned int pontEnv;       ///< N�mero de bytes pendentes em bufEnv
    static bool boolenvpend;    ///< Verdadeiro se tem algum dado pendente

// Para receber mensagens
    char bufRec[SOCKET_REC];
            ///< Cont�m a mensagem recebida
            /**< Cada caracter ocupa dois bytes:
             * - 1 byte = caracter ASCII
             * - 1 byte = cor, conforme TObjSocket::CorAtual */
    unsigned int pontRec;
            ///< N�mero do byte que est� lendo
    char dadoRecebido;
            ///< Para controle da mensagem recebida
            /**< - 0 = comportamento padr�o
             *   - 0x0D, 0x0A = para detectar nova linha
             *   - 2 = para detectar seq��ncias de ESC
             *   - 21=recebeu Ctrl-C
             *   - 22=recebeu Ctrl-C X
             *   - 23=recebeu Ctrl-C XX
             *   - 24=recebeu Ctrl-C XX,
             *   - 25=recebeu Ctrl-C XX,X
             *   - 26=recebeu Ctrl-C XX,XX; esperando Ctrl+C */
    char bufTelnet[8];          ///< Para interpretar o protocolo Telnet
    char bufESC[20];            ///< Para interpretar seq��ncias de ESC
    unsigned short pontESC;     ///< Ponteiro em bufESC
    unsigned short pontTelnet;  ///< Ponteiro em bufTelnet

    unsigned char CorAtual;
            ///< Para controle da cor
            /**< - Bits 0-3 = fundo
             *   - Bits 4-6 = frente
             *   - Bit  7 = negrito, 0=desativado */
    unsigned char CorAnterior;  ///< Com que cor iniciou a linha
    unsigned char CorIRC1;      ///< Para obter cores do IRC
    unsigned char CorIRC2;      ///< Para obter cores do IRC

    unsigned long AFlooder;     ///< Contador do anti-flooder, 0=desativado

    static TSocket * sockAtual; ///< Para saber quando apagou socket

// Lista ligada
    static TSocket * sInicio; ///< Primeiro objeto da lista ligada
    TSocket * sAntes;  ///< Objeto anterior da lista ligada
    TSocket * sDepois; ///< Pr�ximo objeto da lista ligada
};

//----------------------------------------------------------------------------

#endif
