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
    unsigned char CorEnvia;     ///< Cor atual, ao enviar
    short CorInic;      ///< Cor do in�cio da linha, ou -1 se n�o mudou
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
/** Trata das vari�veis do tipo Socket */
class TVarSocket /// Vari�veis Socket
{
public:
    void MudarSock(TObjSocket * socket); ///< Muda a vari�vel TVarSocket::Socket
    void Mover(TVarSocket * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    void Igual(TVarSocket * v);     ///< Operador de atribui��o igual
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
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

    TObjSocket * Socket;   ///< Conex�o atual
    TVarSocket * Antes; ///< Objeto anterior da mesma conex�o
    TVarSocket * Depois;///< Pr�ximo objeto da mesma conex�o
};

//----------------------------------------------------------------------------

#endif
