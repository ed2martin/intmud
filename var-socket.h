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
                    /** A mensagem segue o formato:
                     *  - Byte =0 -> fim da mensagem
                     *  - Byte =1 -> próximo byte = cor
                     *     - Bits 0-3 = cor de fundo
                     *     - Bits 4-6 = cor das letras
                     *     - Bit 7 =1 se negrito (cor das letras mais forte)
                     *     .
                     *  - Byte ='\n' -> passar para próxima linha
                     */
    virtual void Fechar(void)=0; ///< Fecha socket, pode apagar socket

    void FuncFechou();  ///< Executa função _fechou
                        /** @note Pode apagar o próprio objeto */
    bool FuncMsg(const char * mens);  ///< Executa função _msg
                        /** @return true se não apagou o objeto */

// Variáveis usadas para enviar mensagens
    short CorInic;      ///< Cor do início da linha, ou -1 se não mudou
    unsigned char CorEnvia;     ///< Cor atual, ao enviar
    unsigned char ColunaEnvia;  ///< Quantos caracteres já enviou
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
/** Trata das variáveis do tipo Socket */
class TVarSocket /// Variáveis Socket
{
public:
    void MudarSock(TObjSocket * socket); ///< Muda a variável TVarSocket::Socket
    void Mover(TVarSocket * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    void Igual(TVarSocket * v);     ///< Operador de atribuição igual
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();    ///< Ler valor numérico da variável

    const char * defvar;///< Como foi definida a variável
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
