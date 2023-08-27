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
class TVarServObj /// Conexões pendentes via SSL
{
public:
    TVarServObj(TVarServ * serv, int s);  ///< Construtor
    ~TVarServObj();     ///< Destrutor

    int sock;           ///< Socket
    SSL * sockssl;      ///< Objeto da conexão segura
    char acaossl:1;     ///< O que fazer na conexão SSL
            /**<  - 0 esperando dados para recv()
             *    - 1 esperando dados para send() */
    unsigned char tempo;///< Em quanto tempo deve conectar, em décimos de segundo

    TVarServ * Serv;    ///< Objeto serv que está recebendo a conexão
    TVarServObj * Antes; ///< Objeto anterior
    TVarServObj * Depois; ///< Próximo objeto
};

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Serv */
class TVarServ /// Variáveis Serv
{
public:
    void Criar();           ///< Cria objeto
                            /**< @note Acertar também defvar, classe e objeto */
    void Apagar();          ///< Apaga objeto
    void Fechar();          ///< Fecha porta
    void Mover(TVarServ * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Abrir(const char * ender, unsigned short porta);
    static int Fd_Set(fd_set * set_entrada, fd_set * set_saida);
    static void ProcEventos(fd_set * set_entrada, int tempo);
    void ExecEvento(int localSocket, SSL * sslSocket); ///< Gera evento
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();    ///< Ler valor numérico da variável

    const char * defvar;///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

private:
    bool FuncFechar(TVariavel * v);    ///< Processa função Fechar
    bool FuncAbrir(TVariavel * v);     ///< Processa função Abrir
    bool FuncAbrirSSL(TVariavel * v);  ///< Processa função AbrirSSL
    bool FuncIniSSL(TVariavel * v);    ///< Processa função IniSSL

    bool modossl;               ///< Se deve usar conexão segura (SSL)
    int  sock;                  ///< Socket; menor que 0 se estiver fechado
    static TVarServ * varObj;   ///< Usado para saber se objeto foi apagado
    static TVarServ * Inicio;   ///< Primeiro objeto (com sock>=0)
    TVarServ * Antes;           ///< Objeto anterior (se sock>=0)
    TVarServ * Depois;          ///< Próximo objeto (se sock>=0)
    TVarServObj * ServInicio;   ///< Primeiro objeto TVarServObj
    TVarServObj * ServFim;      ///< Último objeto TVarServObj

    friend class TVarServObj;
};

//----------------------------------------------------------------------------

#endif
