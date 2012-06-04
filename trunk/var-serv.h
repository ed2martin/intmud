#ifndef VAR_SERV_H
#define VAR_SERV_H

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/types.h>
#endif
#include "ssl.h"

//----------------------------------------------------------------------------
class TVariavel;
class TClasse;
class TObjeto;
class TVarServ;

//----------------------------------------------------------------------------
class TVarServObj /// Conex�es pendentes via SSL
{
public:
    TVarServObj(TVarServ * serv, int s);  ///< Construtor
    ~TVarServObj();     ///< Destrutor

    int sock;           ///< Socket
    SSL * sockssl;      ///< Objeto da conex�o segura
    char acaossl:1;     ///< O que fazer na conex�o SSL
            /**<  - 0 esperando dados para recv()
             *    - 1 esperando dados para send() */
    unsigned char tempo;///< Em quanto tempo deve conectar, em d�cimos de segundo

    TVarServ * Serv;    ///< Objeto serv que est� recebendo a conex�o
    TVarServObj * Antes; ///< Objeto anterior
    TVarServObj * Depois; ///< Pr�ximo objeto
};

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo Serv */
class TVarServ /// Vari�veis Serv
{
public:
    void Criar();           ///< Cria objeto
                            /**< @note Acertar tamb�m defvar, classe e objeto */
    void Apagar();          ///< Apaga objeto
    void Fechar();          ///< Fecha porta
    void Mover(TVarServ * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Abrir(const char * ender, unsigned short porta);
    static int Fd_Set(fd_set * set_entrada, fd_set * set_saida);
    static void ProcEventos(fd_set * set_entrada, int tempo);
    void ExecEvento(int localSocket, SSL * sslSocket); ///< Gera evento
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();    ///< Ler valor num�rico da vari�vel

    const char * defvar;///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

private:
    bool modossl;           ///< Se deve usar conex�o segura (SSL)
    int  sock;                  ///< Socket; menor que 0 se estiver fechado
    static TVarServ * varObj;   ///< Usado para saber se objeto foi apagado
    static TVarServ * Inicio;   ///< Primeiro objeto (com sock>=0)
    TVarServ * Antes;           ///< Objeto anterior (se sock>=0)
    TVarServ * Depois;          ///< Pr�ximo objeto (se sock>=0)
    TVarServObj * ServInicio;   ///< Primeiro objeto TVarServObj
    TVarServObj * ServFim;      ///< �ltimo objeto TVarServObj

    friend class TVarServObj;
};

//----------------------------------------------------------------------------

#endif
