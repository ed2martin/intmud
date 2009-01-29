#ifndef VAR_SERV_H
#define VAR_SERV_H

#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <sys/types.h>
#endif

//----------------------------------------------------------------------------
class TVariavel;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo Serv */
class TVarServ /// Vari�veis Serv
{
public:
    void Criar();           ///< Cria objeto
                            /** @note Acertar tamb�m defvar, classe e objeto */
    void Apagar();          ///< Apaga objeto
    void Fechar();          ///< Fecha porta
    void Mover(TVarServ * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Abrir(const char * ender, unsigned short porta);
    static void Fd_Set(fd_set * set_entrada);
    static void ProcEventos(fd_set * set_entrada);
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
    int  sock;                  ///< Socket; menor que 0 se estiver fechado
    static TVarServ * varObj;   ///< Usado para saber se objeto foi apagado
    static TVarServ * Inicio;   ///< Primeiro objeto (com sock>=0)
    TVarServ * Antes;           ///< Objeto anterior (se sock>=0)
    TVarServ * Depois;          ///< Pr�ximo objeto (se sock>=0)
};

//----------------------------------------------------------------------------

#endif
