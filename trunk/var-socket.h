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
/** Representa uma conexão, um "socket" */
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
    char bufEnv[SOCKET_ENV];    ///< Contém a mensagem que será enviada
    unsigned int pontEnv;       ///< Número de bytes pendentes em bufEnv
    static bool boolenvpend;    ///< Verdadeiro se tem algum dado pendente

// Para receber mensagens
    char bufRec[SOCKET_REC];    ///< Contém a mensagem recebida
                            /**< Cada caracter ocupa dois bytes:
                             * - 1 byte = caracter ASCII
                             * - 1 byte = cor, conforme TSocket::CorAtual
                             */
    unsigned int pontRec;       ///< Número do byte que está lendo
    char dadoRecebido;          ///< Para controle da mensagem recebida
                            /**<
                             * - 0 = comportamento padrão
                             * - 0x0D, 0x0A = para detectar nova linha
                             * - 2 = para detectar seqüências de ESC
                             * - 21=recebeu Ctrl-C
                             * - 22=recebeu Ctrl-C X
                             * - 23=recebeu Ctrl-C XX
                             * - 24=recebeu Ctrl-C XX,
                             * - 25=recebeu Ctrl-C XX,X
                             * - 26=recebeu Ctrl-C XX,XX; esperando Ctrl+C
                             */
    char bufTelnet[8];          ///< Para interpretar o protocolo Telnet
    char bufESC[20];            ///< Para interpretar seqüências de ESC
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
    TSocket * sDepois; ///< Próximo objeto da lista ligada
    friend class TVarSocket;
};

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Socket */
class TVarSocket /// Variáveis Socket
{
public:
    void MudarSock(TSocket * socket); ///< Muda a variável TVarSocket::Socket
    void Mover(TVarSocket * destino); ///< Move TVarSock para outro lugar
    void Igual(TVarSocket * v);     ///< Operador de atribuição igual
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int getValor();     ///< Ler valor numérico da variável

    const char * defvar;///< Como foi definida a variável
    TClasse * classe;   ///< Em que classe está definido
    TObjeto * objeto;   ///< Em que objeto está definido

    TSocket * Socket;   ///< Conexão atual
    TVarSocket * Antes; ///< Objeto anterior da mesma conexão
    TVarSocket * Depois;///< Próximo objeto da mesma conexão
};

//----------------------------------------------------------------------------

#endif
