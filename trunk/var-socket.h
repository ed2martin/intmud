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
                     *  - Byte =1 -> pr�ximo byte = cor
                     *     - Bits 0-3 = cor de fundo
                     *     - Bits 4-6 = cor das letras
                     *     - Bit 7 =1 se negrito (cor das letras mais forte)
                     *     .
                     *  - Byte ='\n' -> passar para pr�xima linha
                     */
    virtual void Fechar(void)=0; ///< Fecha socket, pode apagar socket
    virtual int  Variavel(char num, int valor)=0;
                    ///< L� ou altera uma vari�vel
                    /**< @param num n�mero da vari�vel
                     *         - '0' = proto
                     *         - '1' = cores
                     *         - '2' = aflooder
                     *         - '3' = eco
                     *  @param valor Novo valor, se for >= 0
                     *  @return valor atual da vari�vel
                     */
    virtual const char * Endereco(bool remoto);
                    ///< Retorna o endere�o local ou remoto da conex�o

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
    unsigned char CorEnvia;     ///< Cor atual, ao enviar
    short CorInic;      ///< Cor do in�cio da linha, ou -1 se n�o mudou
    unsigned short ColunaEnvia;  ///< Quantos caracteres j� enviou
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
/** Trata das vari�veis do tipo Socket */
class TVarSocket /// Vari�veis Socket
{
public:
    void MudarSock(TObjSocket * socket); ///< Muda a vari�vel TVarSocket::Socket
    void Mover(TVarSocket * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    void Igual(TVarSocket * v);     ///< Operador de atribui��o igual
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor(const char * defvar1);
                            ///< Ler valor num�rico da vari�vel
    void setValor(const char * defvar1, int valor);
                            ///< Mudar o valor num�rico da vari�vel

    const char * defvar;    ///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

    TObjSocket * Socket;   ///< Conex�o atual
    TVarSocket * Antes; ///< Objeto anterior da mesma conex�o
    TVarSocket * Depois;///< Pr�ximo objeto da mesma conex�o
};

//----------------------------------------------------------------------------

#endif
