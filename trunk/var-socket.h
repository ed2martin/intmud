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
                    /** A mensagem segue o formato:
                     *  - Byte =0 -> fim da mensagem
                     *  - Byte =1 -> pr�ximo byte = cor
                     *     - Bits 0-3 = cor de fundo
                     *     - Bits 4-6 = cor das letras
                     *     - Bit 7 =1 se negrito (cor das letras mais forte)
                     *     .
                     *  - Byte ='\n' -> passar para pr�xima linha
                     */
    virtual void Fechar(void)=0; ///< Fecha socket, pode apagar socket

    void FuncFechou();  ///< Executa fun��o _fechou
                        /** @note Pode apagar o pr�prio objeto */
    bool FuncMsg(const char * mens);  ///< Executa fun��o _msg
                        /** @return true se n�o apagou o objeto */

// Vari�veis usadas para enviar mensagens
    short CorInic;      ///< Cor do in�cio da linha, ou -1 se n�o mudou
    unsigned char CorEnvia;     ///< Cor atual, ao enviar
    unsigned char ColunaEnvia;  ///< Quantos caracteres j� enviou
    unsigned char ColunaMin;    ///< Menor quantidade de caracteres/linha
    unsigned char ColunaMax;    ///< Maior quantidade de caracteres/linha

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
    int  getValor();    ///< Ler valor num�rico da vari�vel

    const char * defvar;///< Como foi definida a vari�vel
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
