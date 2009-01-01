#ifndef VAR_SOCKET_H
#define VAR_SOCKET_H

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/types.h>
#endif

#define SOCKET_REC 1024
#define SOCKET_ENV 2048

//----------------------------------------------------------------------------
class TVariavel;
class TVarSocket;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Representa uma conex�o, um "socket" */
class TSocket /// Socket
{
public:
    TSocket();      ///< Construtor
    ~TSocket();     ///< Destrutor
    bool Aberto(void);          ///< Retorna verdadeiro se dispositivo aberto

    static void Fd_Set(fd_set * set_entrada, fd_set * set_saida);
    static void ProcEventos(fd_set * set_entrada, fd_set * set_saida);
    bool EnvMens(const char * mensagem);///< Envia mensagem conforme protocolo
private:
    void Processa(const char * buffer, int tamanho);
    bool EnvMens(const char * mensagem, int tamanho); ///< Envia mensagem pura
    void EnvPend();             ///< Envia dados pendentes
    void Fechar(void);          ///< Fecha socket
    int  sock;                  ///< Socket; menor que 0 se estiver fechado

// Para enviar mensagens
    char bufEnv[SOCKET_ENV];    ///< Cont�m a mensagem que ser� enviada
    unsigned int pontEnv;       ///< N�mero de bytes pendentes em bufEnv
    static bool boolenvpend;    ///< Verdadeiro se tem algum dado pendente

// Para receber mensagens
    char bufRec[SOCKET_REC];    ///< Cont�m a mensagem recebida
                            /**< Cada caracter ocupa dois bytes:
                             * - 1 byte = caracter ASCII
                             * - 1 byte = cor, conforme TSocket::CorAtual
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
    unsigned char CorIRC1;      ///< Para obter cores do IRC
    unsigned char CorIRC2;      ///< Para obter cores do IRC

// Para saber quando objetos foram apagados
    static TSocket * sockObj;   ///< Usado para saber se objeto foi apagado
    static TVarSocket * varObj; ///< Usado para saber se objeto foi apagado

// Listas ligadas
    TVarSocket * Inicio; ///< Lista ligada de TVarSocket
    static TSocket * sInicio; ///< Primeiro objeto da lista ligada
    TSocket * sAntes;  ///< Objeto anterior da lista ligada
    TSocket * sDepois; ///< Pr�ximo objeto da lista ligada
    friend class TVarSocket;
};

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo Socket */
class TVarSocket /// Vari�veis Socket
{
public:
    void MudarSock(TSocket * socket); ///< Muda a vari�vel TVarSocket::Socket
    void Mover(TVarSocket * destino); ///< Move TVarSock para outro lugar
    void Igual(TVarSocket * v);     ///< Operador de atribui��o igual
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int getValor();     ///< Ler valor num�rico da vari�vel

    const char * defvar;///< Como foi definida a vari�vel
    TClasse * classe;   ///< Em que classe est� definido
    TObjeto * objeto;   ///< Em que objeto est� definido

    TSocket * Socket;   ///< Conex�o atual
    TVarSocket * Antes; ///< Objeto anterior da mesma conex�o
    TVarSocket * Depois;///< Pr�ximo objeto da mesma conex�o
};

//----------------------------------------------------------------------------

#endif
