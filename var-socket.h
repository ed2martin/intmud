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
/** Representa uma conexão de uma variável Socket */
class TObjSocket /// Conexão de Socket
{
public:
    TObjSocket();               ///< Construtor
    virtual ~TObjSocket();      ///< Destrutor
    bool Enviar(const char * mensagem);///< Envia mensagem conforme protocolo

protected:
    virtual bool EnvMens(const char * mensagem)=0; ///< Envia mensagem
                    /**< A mensagem segue o formato:
                     *  - Byte =0 -> fim da mensagem
                     *  - Byte =1 -> próximo byte = cor
                     *     - Bits 0-3 = cor de fundo
                     *     - Bits 4-6 = cor das letras
                     *     - Bit 7 =1 se negrito (cor das letras mais forte)
                     *     .
                     *  - Byte ='\n' -> passar para próxima linha
                     */
    virtual void Fechar(void)=0; ///< Fecha socket, pode apagar socket
    virtual int  Variavel(char num, int valor)=0;
                    ///< Lê ou altera uma variável
                    /**< @param num número da variável
                     *         - '0' = proto
                     *         - '1' = cores
                     *         - '2' = aflooder
                     *         - '3' = eco
                     *  @param valor Novo valor, se for >= 0
                     *  @return valor atual da variável
                     */
    virtual const char * Endereco(bool remoto);
                    ///< Retorna o endereço local ou remoto da conexão

    void RetiraVarSocket(); ///< Retira objeto da lista ligada de TVarSocket
    void FuncFechou(const char * txt);
                    ///< Executa função _fechou
                    /**< @param txt Texto que contém o motivo
                     *   @note Pode apagar o próprio objeto */
    bool FuncEvento(const char * evento, const char * texto, int valor=-1);
                    ///< Executa uma função
                    /**< @param evento Nome do evento (ex. "msg")
                     *   @param texto Texto do primeiro argumento, 0=nenhum texto
                     *   @param valor Segundo argumento, <0 = nenhum valor
                     *   @return true se não apagou o objeto, false se apagou
                     */

// Variáveis usadas para enviar mensagens
    unsigned char CorEnvia;     ///< Cor atual, ao enviar
    short CorInic;      ///< Cor do início da linha, ou -1 se não mudou
    unsigned short ColunaEnvia;  ///< Quantos caracteres já enviou
    unsigned short ColunaMin;   ///< Menor quantidade de caracteres/linha
    unsigned short ColunaMax;   ///< Maior quantidade de caracteres/linha

private:
// Para saber quando objetos foram apagados
    static TObjSocket * sockObj; ///< Usado para saber se objeto foi apagado
    static TVarSocket * varObj;  ///< Usado para saber se objeto foi apagado

// Lista ligadas
    TVarSocket * Inicio; ///< Lista ligada de TVarSocket
    friend class TVarSocket;
};

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Socket */
class TVarSocket /// Variáveis Socket
{
public:
    void MudarSock(TObjSocket * socket); ///< Muda a variável TVarSocket::Socket
    void Mover(TVarSocket * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    void Igual(TVarSocket * v);     ///< Operador de atribuição igual
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor(const char * defvar1);
                            ///< Ler valor numérico da variável
    void setValor(const char * defvar1, int valor);
                            ///< Mudar o valor numérico da variável

    const char * defvar;    ///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

    TObjSocket * Socket;   ///< Conexão atual
    TVarSocket * Antes; ///< Objeto anterior da mesma conexão
    TVarSocket * Depois;///< Próximo objeto da mesma conexão
};

//----------------------------------------------------------------------------

#endif
