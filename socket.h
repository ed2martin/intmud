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
#include "ssl.h"

#define SOCKET_REC 512
#define SOCKET_ENV 2048

//----------------------------------------------------------------------------
class TSocketRec /// Um caracter recebido de socket
{
public:
    unsigned short carac; ///< caracter ASCII
    unsigned short cor;   ///< cor, conforme TObjSocket::CorAtual
};

//----------------------------------------------------------------------------
/** Representa uma conex�o, um "socket" */
class TSocket : public TObjSocket /// Socket
{
public:
    TSocket(int socknum, SSL * sslnum); ///< Construtor
    ~TSocket();                 ///< Destrutor
    void Fechar();              ///< Socket fechou

    static int NomeParaIps(const char * nome, char * ip, int tamanho);
            ///< Obt�m os IPs a partir do nome
            /**< @param nome Endere�o
             *   @param ip Aonde colocar os IPs, separados por espa�o
             *   @param tamanho Tamanho de ip em bytes
             *   @return 0 se sucesso ou c�digo de erro, -1 se nenhum endere�o */
    static int IpParaNome(const char * ip, char * nome, int tamanho);
            ///< Obt�m o nome a partir do IP
            /**< @param ip Aonde colocar os IPs, separados por espa�o
             *   @param nome Endere�o
             *   @param tamanho Tamanho de nome em bytes
             *   @return 0 se sucesso ou c�digo de erro, -1 se nenhum endere�o */
    static void IpParaString(struct sockaddr * sa, size_t salen,
            char * s, size_t maxlen);
            ///< Converte endere�o IP em sockaddr para texto
            /**  @param sa Estrutura sockaddr
             *   @param salen Tamanho da estrutura em sa
             *   @param s Aonde colocar o texto
             *   @param maxlen Tamanho de s em bytes
             *   @note s ser� um texto vazio se n�o for poss�vel obter o endere�o */
    static bool IpValido(const char * host);
            ///< Retorna true se for um endere�o IP v�lido

    static void SockConfig(int socknum);
    static TSocket * Conectar(const char * ender, int porta, bool ssl);
            ///< Cria objeto TSocket a partir de endere�o e porta
            /**< @param ender  Endere�o a conectar
             *   @param porta  Porta
             *   @param ssl    Se deve usar conex�o segura (SSL)
             *   @return Objeto TSocket ou 0 se ocorreu erro
                    (provavelmente endere�o inv�lido) */
    static const char * TxtErro(int erro);
            ///< Fornece a descri��o de um erro a partir do n�mero
            /**< @param erro N�mero do erro
             *   @return Texto contendo o erro
             *   @note Pr�ximas chamadas a essa fun��o podem alterar
             *         a string original */
    static void Fd_Set(fd_set * set_entrada, fd_set * set_saida, fd_set * set_err);
    static void ProcEventos(fd_set * set_entrada,
                                fd_set * set_saida, fd_set * set_err);
            ///< Processa eventos
    static void SairPend();  ///< Envia dados pendentes (programa vai encerrar)
    void CriaSSL(const char * ender = nullptr);
            ///< Cria conex�o SSL (vari�vel sockssl) a partir do socket
            /**< @param ender Endere�o do host ou nullptr se n�o for usado */
    bool Conectado();   ///< Retorna verdadeiro se estiver conectado
    bool EnvMens(const char * mensagem, int codigo);
            ///< Envia mensagem conforme protocolo
            /**< @param mensagem Endere�o dos bytes a enviar
             *   @param codigo Tipo de mensagem; usado somente no Papovox
             *   @return true se conseguiu enviar, false se n�o conseguiu */
    int  Variavel(char num, int valor);
    void Endereco(int num, char * mens, int total);
            ///< Retorna uma informa��o da conex�o, vide TObjSocket:Endereco
private:
    int  Processa(const char * buffer, int tamanho);
            ///< Processa dados recebidos em TSocket::ProcEventos
            /**< @param buffer Endere�o do buffer
             *   @param tamanho Tamanho do buffer
             *   @return Quantos bytes ainda falta processar
             *   @note Pode apagar o pr�prio objeto */
    void EventoMens(bool completo);
            ///< Gera evento de acordo com a mensagem recebida
            /**< @param completo Se recebeu a mensagem completa
             *                  (arg1 do evento _msg)
             *   @note Pode apagar o pr�prio objeto */
    bool EnvMensBytes(const char * mensagem, int tamanho);
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
    SSL * sockssl;              ///< Se !=0, � o objeto da conex�o segura
    char proto; ///< Protocolo (quando sock>=0), vide TSocketProto
    char cores;                 ///< 0=sem 1=ao receber, 2=ao enviar, 3=com
    unsigned char receberssl:1; ///< Se deve verificar dados recebidos antes de enviar
    unsigned char eventoenv:1;  ///< Se deve gerar evento _env
                                /**< @note Caracteres de controle de Telnet
                                  *  n�o devem gerar eventos _env */
    unsigned char usaropctelnet:1; ///< Vari�vel socket.opctelnet
    unsigned char sockenvrec:1; ///< 1=socket fechou ao enviar, 0=ao receber
    struct sockaddr_storage conSock; ///< Usado principalmente quando proto=0

// Para enviar mensagens
    char bufEnv[SOCKET_ENV];    ///< Cont�m a mensagem que ser� enviada
    unsigned int pontEnv;       ///< N�mero de bytes pendentes em bufEnv
    unsigned int pontEnvSsl;    ///< Quantos bytes tentou enviar ou 0 se n�o tentou
    static bool boolenvpend;    ///< Verdadeiro se tem algum dado pendente

// Para receber mensagens
    TSocketRec bufRec[SOCKET_REC];
            ///< Cont�m a mensagem recebida
    unsigned int pontRec;
            ///< N�mero do byte que est� lendo
    char dadoRecebido;
            ///< Para controle da mensagem recebida
            /**< - 0 = comportamento padr�o
             *   - 0x0D, 0x0A = para detectar nova linha
             *   - 2 = para detectar sequ�ncias de ESC
             *   - 21=recebeu Ctrl-C
             *   - 22=recebeu Ctrl-C X
             *   - 23=recebeu Ctrl-C XX
             *   - 24=recebeu Ctrl-C XX,
             *   - 25=recebeu Ctrl-C XX,X
             *   - 26=recebeu Ctrl-C XX,XX; esperando Ctrl+C */
    unsigned char bufTelnet[8]; ///< Para interpretar o protocolo Telnet
    unsigned char bufESC[20];   ///< Para interpretar sequ�ncias de ESC
    unsigned short pontESC;     ///< Ponteiro em bufESC
    unsigned short pontTelnet;  ///< Ponteiro em bufTelnet
    unsigned char telnetopc[10];///< Para responder algumas op��es do telnet
    int telnetecho;             ///< Para lidar com envio de caracteres ECHO

    unsigned short CorAtual;
            ///< Para controle da cor, vide TConsole::CorAtual
    unsigned short CorAnterior;  ///< Com que cor iniciou a linha
    unsigned char CorIRC1;      ///< Para obter cores do IRC
    unsigned char CorIRC2;      ///< Para obter cores do IRC

    unsigned long AFlooder;     ///< Contador do anti-flooder, 0=desativado

    static TSocket * SockAtual; ///< Para saber quando apagou socket

// Lista ligada
    static TSocket * sInicio; ///< Primeiro objeto da lista ligada
    TSocket * sAntes;  ///< Objeto anterior da lista ligada
    TSocket * sDepois; ///< Pr�ximo objeto da lista ligada
};

//----------------------------------------------------------------------------

#endif
