#ifndef VAR_SOCKET_H
#define VAR_SOCKET_H

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/types.h>
 #include <netinet/in.h>
#endif

class TVariavel;
class TVarInfo;
enum TVarTipo : unsigned char;
class TVarSocket;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Representa uma conexão de uma variável Socket */
class TObjSocket /// Conexão de Socket
{
public:
    TObjSocket();               ///< Construtor
    virtual ~TObjSocket();      ///< Destrutor
    bool Enviar(const char * mensagem, int codigo);
        ///< Envia mensagem conforme protocolo
        /**< @param mensagem Endereço dos bytes a enviar
         *   @param codigo Tipo de mensagem; usado somente no Papovox
         *   @return true se conseguiu enviar, false se não conseguiu */

protected:
    virtual bool EnvMens(const char * mensagem, int codigo)=0;
        ///< Envia mensagem
        /**< A mensagem segue o formato:
         *  - Byte =0 -> fim da mensagem
         *  - Byte =1 -> próximos dois bytes = cor
         *     - Byte 0 Bits 0-3 = cor de fundo
         *     - Byte 0 Bits 4-6 = cor das letras
         *     - Byte 0 Bit 7 =1 se negrito (cor das letras mais forte)
         *     - Byte 1 Bit 0 =1 se sublinhado
         *     .
         *  - Byte =2 -> echo off
         *  - Byte =3 -> echo on
         *  - Byte =4 -> go ahead
         *  - Byte =5 -> beep
         *  - Byte ='\n' -> passar para próxima linha
         */
    virtual void Fechar(void)=0; ///< Fecha socket, pode apagar socket
    virtual int  Variavel(char num, int valor)=0;
                    ///< Lê ou altera uma variável
                    /**< @param num número da variável
                     *         - 1 = proto
                     *         - 2 = cores
                     *         - 3 = aflooder
                     *         - 4 = eco
                     *  @param valor Novo valor, se for >= 0
                     *  @return valor atual da variável
                     */
    virtual void Endereco(int num, char * mens, int total);
                    ///< Anota uma informação da conexão
                    /**< @param num O que informar
                     *    - 0 = endereço da conexão local
                     *    - 1 = endereço da conexão remota
                     *    - 2 = assinatura MD5
                     *    - 3 = assinatura SHA1
                     *   @param mens Aonde colocar o texto
                     *   @param total Tamanho do buffer em mens */
    void RetiraVarSocket(); ///< Retira objeto da lista ligada de TVarSocket
    void FuncFechou(const char * txt);
                    ///< Executa função _fechou
                    /**< @param txt Texto que contém o motivo
                     *   @note Pode apagar o próprio objeto */
    void FuncEvento(const char * evento, const char * texto, int v1=-1, int v2=-1);
                    ///< Executa uma função
                    /**< @param evento Nome do evento (ex. "msg")
                     *   @param texto Texto do primeiro argumento, 0=nenhum texto
                     *   @param v1 Segundo argumento, se menor que 0 não tem
                     *   @param v2 Terceiro argumento, se menor que 0 não tem
                     *   @note Pode apagar o próprio objeto */

// Variáveis usadas para enviar mensagens
    unsigned short CorEnvia;     ///< Cor atual, ao enviar
    unsigned short ColunaEnvia;  ///< Quantos caracteres já enviou

private:
// Para saber quando objetos foram apagados
    static TVarSocket * varObj;  ///< Usado para saber se objeto foi apagado

// Lista ligadas
    TVarSocket * Inicio; ///< Lista ligada de TVarSocket
    friend class TVarSocket;
};

//----------------------------------------------------------------------------
class TDNSSocket /// Resolve DNS em segundo plano
{
public:
    TDNSSocket(TVarSocket * var, const char * ender);
            ///< Construtor; faz *nomeini=0 se ocorreu algum erro
    ~TDNSSocket();       ///< Destrutor
    static void Fd_Set(fd_set * set_entrada);
    static void ProcEventos(fd_set * set_entrada);
                         ///< Processa eventos
    char nomeini[256];   ///< Nome a ser pesquisado
    char nome[256];      ///< Nome da máquina
    char ip[256];        ///< Endereço IP
private:
    TVarSocket * Socket; ///< Para onde gerar os eventos
    static TDNSSocket * Inicio; ///< Lista ligada de TDNSSocket
    TDNSSocket * Antes;  ///< Objeto anterior
    TDNSSocket * Depois; ///< Próximo objeto
#ifdef __WIN32__
    HANDLE hthread;      ///< Para saber quando a Thread terminou
#else
    int recdescr;        ///< Para receber informações do outro processo
#endif
    friend class TVarSocket;
};

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Socket */
class TVarSocket /// Variáveis Socket
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Apagar();          ///< Apaga objeto
    void MudarSock(TObjSocket * socket); ///< Muda a variável TVarSocket::Socket
    void Mover(TVarSocket * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    bool FuncMsg(TVariavel * v);
    bool FuncAbrir(TVariavel * v);
    bool FuncAbrirSsl(TVariavel * v);
    bool FuncFechar(TVariavel * v);
    bool FuncIpLocal(TVariavel * v);
    bool FuncIp(TVariavel * v);
    bool FuncMd5(TVariavel * v);
    bool FuncSha1(TVariavel * v);
    bool FuncAFlooder(TVariavel * v);
    bool FuncCores(TVariavel * v);
    bool FuncOpcTelnet(TVariavel * v);
    bool FuncPosX(TVariavel * v);
    bool FuncProto(TVariavel * v);
    bool FuncEventoIP(TVariavel * v);
    bool FuncNomeIP(TVariavel * v);
    bool FuncIPNome(TVariavel * v);
    bool FuncIPValido(TVariavel * v);
    bool FuncIniSSL(TVariavel * v);

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static TVarTipo FTipo(TVariavel * v);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static void FMoverDef(TVariavel * v);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
    static void FOperadorAtrib(TVariavel * v);

    int  getValor(int numfunc);
                            ///< Ler valor numérico da variável
    void setValor(int numfunc, int valor);
                            ///< Mudar o valor numérico da variável

    const char * defvar;    ///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

    TObjSocket * Socket;    ///< Conexão atual
    TVarSocket * Antes;     ///< Objeto anterior da mesma conexão
    TVarSocket * Depois;    ///< Próximo objeto da mesma conexão
};

//----------------------------------------------------------------------------

#endif
