#ifndef VAR_SOCKET_H
#define VAR_SOCKET_H

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/types.h>
 #include <netinet/in.h>
#endif

//----------------------------------------------------------------------------
class TVariavel;
class TVarSocket;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Representa uma conex�o de uma vari�vel Socket */
class TObjSocket /// Conex�o de Socket
{
public:
    TObjSocket();               ///< Construtor
    virtual ~TObjSocket();      ///< Destrutor
    bool Enviar(const char * mensagem);///< Envia mensagem conforme protocolo

protected:
    virtual bool EnvMens(const char * mensagem)=0; ///< Envia mensagem
                    /**< A mensagem segue o formato:
                     *  - Byte =0 -> fim da mensagem
                     *  - Byte =1 -> pr�ximos dois bytes = cor
                     *     - Byte 0 Bits 0-3 = cor de fundo
                     *     - Byte 0 Bits 4-6 = cor das letras
                     *     - Byte 0 Bit 7 =1 se negrito (cor das letras mais forte)
                     *     - Byte 1 Bit 0 =1 se sublinhado
                     *     .
                     *  - Byte =2 -> echo off
                     *  - Byte =3 -> echo on
                     *  - Byte =4 -> go ahead
                     *  - Byte =5 -> beep
                     *  - Byte ='\n' -> passar para pr�xima linha
                     */
    virtual void Fechar(void)=0; ///< Fecha socket, pode apagar socket
    virtual int  Variavel(char num, int valor)=0;
                    ///< L� ou altera uma vari�vel
                    /**< @param num n�mero da vari�vel
                     *         - 1 = proto
                     *         - 2 = cores
                     *         - 3 = aflooder
                     *         - 4 = eco
                     *  @param valor Novo valor, se for >= 0
                     *  @return valor atual da vari�vel
                     */
    virtual void Endereco(int num, char * mens, int total);
                    ///< Anota uma informa��o da conex�o
                    /**< @param num O que informar
                     *    - 0 = endere�o da conex�o local
                     *    - 1 = endere�o da conex�o remota
                     *    - 2 = assinatura MD5
                     *    - 3 = assinatura SHA1
                     *   @param mens Aonde colocar o texto
                     *   @param total Tamanho do buffer em mens */
    void RetiraVarSocket(); ///< Retira objeto da lista ligada de TVarSocket
    void FuncFechou(const char * txt);
                    ///< Executa fun��o _fechou
                    /**< @param txt Texto que cont�m o motivo
                     *   @note Pode apagar o pr�prio objeto */
    bool FuncEvento(const char * evento, const char * texto, int valor=-1);
                    ///< Executa uma fun��o
                    /**< @param evento Nome do evento (ex. "msg")
                     *   @param texto Texto do primeiro argumento, 0=nenhum texto
                     *   @param valor Segundo argumento, <0 = nenhum valor
                     *   @return true se n�o apagou o objeto, false se apagou
                     */

// Vari�veis usadas para enviar mensagens
    unsigned short CorEnvia;     ///< Cor atual, ao enviar
    unsigned short ColunaEnvia;  ///< Quantos caracteres j� enviou

private:
// Para saber quando objetos foram apagados
    static TObjSocket * sockObj; ///< Usado para saber se objeto foi apagado
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
    char nome[256];      ///< Nome da m�quina
    char ip[256];        ///< Endere�o IP
private:
    TVarSocket * Socket; ///< Para onde gerar os eventos
    static TDNSSocket * Inicio; ///< Lista ligada de TDNSSocket
    TDNSSocket * Antes;  ///< Objeto anterior
    TDNSSocket * Depois; ///< Pr�ximo objeto
#ifdef __WIN32__
    HANDLE hthread;      ///< Para saber quando a Thread terminou
#else
    int recdescr;        ///< Para receber informa��es do outro processo
#endif
    friend class TVarSocket;
};

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo Socket */
class TVarSocket /// Vari�veis Socket
{
public:
    void Apagar();          ///< Apaga objeto
    void MudarSock(TObjSocket * socket); ///< Muda a vari�vel TVarSocket::Socket
    void Mover(TVarSocket * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    void Igual(TVarSocket * v);     ///< Operador de atribui��o igual
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    bool FuncAbrir(TVariavel * v, int valor);
    bool FuncFechar(TVariavel * v, int valor);
    bool FuncEndereco(TVariavel * v, int valor);
    bool FuncVariaveis(TVariavel * v, int valor);
    bool FuncEventoIP(TVariavel * v, int valor);
    bool FuncNomeIP(TVariavel * v, int valor);
    bool FuncIPNome(TVariavel * v, int valor);
    bool FuncIPValido(TVariavel * v, int valor);
    bool FuncIniSSL(TVariavel * v, int valor);
    static int getTipo(int numfunc);
                            ///< Retorna o tipo de vari�vel
    int  getValor(int numfunc);
                            ///< Ler valor num�rico da vari�vel
    void setValor(int numfunc, int valor);
                            ///< Mudar o valor num�rico da vari�vel

    const char * defvar;    ///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

    TObjSocket * Socket;    ///< Conex�o atual
    TVarSocket * Antes;     ///< Objeto anterior da mesma conex�o
    TVarSocket * Depois;    ///< Pr�ximo objeto da mesma conex�o
};

//----------------------------------------------------------------------------

#endif
