#ifndef VAR_LISTAOBJ_H
#define VAR_LISTAOBJ_H

//----------------------------------------------------------------------------
class TListaItem;
class TListaX;
class TObjeto;
class TVariavel;

class TListaObj /// Vari�veis ListaObj
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TListaObj * destino); ///< Move TListaObj para outro lugar
    void EndObjeto(TObjeto * obj);
    TListaX * AddInicio(TObjeto * obj); ///< Adiciona objeto no in�cio da lista
    TListaX * AddFim(TObjeto * obj); ///< Adiciona objeto no fim da lista
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();        ///< Ler valor num�rico da vari�vel
    TListaX * Inicio;       ///< Primeiro item
    TListaX * Fim;          ///< �ltimo item
    TObjeto * Objeto;       ///< Objeto em que a lista foi definida
};

//----------------------------------------------------------------------------
class TListaItem /// Vari�vel ListaItem, usada para acessar ListaObj
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TListaItem * destino); ///< Move TListaItem para outro lugar
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();        ///< Ler valor num�rico da vari�vel
    void MudarRef(TListaX * lista); ///< Muda refer�ncia de ListaX
    TListaItem * Antes;     ///< Objeto anterior
    TListaItem * Depois;    ///< Pr�ximo objeto
    TListaX * ListaX;       ///< Qual objeto est� apontando
};

//----------------------------------------------------------------------------
class TListaX /// Um item de ListaObj
{
public:
    static TListaX * Criar(); ///< Cria um objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TListaX * destino); ///< Move TListaX para outro lugar
    TListaObj * Lista;      ///< A qual lista pertence
    TListaX * ListaAntes;   ///< Objeto anterior da lista
    TListaX * ListaDepois;  ///< Pr�ximo objeto da lista
    TObjeto * Objeto;       ///< Objeto que est� na lista
    TListaX * ObjAntes;     ///< Objeto anterior da lista
    TListaX * ObjDepois;    ///< Pr�ximo objeto da lista
    TListaItem * ListaItem; ///< Primeira listaitem apontando para esse objeto
};

//----------------------------------------------------------------------------
#define TOTAL_LISTAX 1024
class TGrupoX /// Um grupo de objetos TListaX
{
public:
    static void ProcEventos();  ///< Apaga objetos TGrupoX se necess�rio
    static TListaX * Criar();   ///< Aloca TListaX na mem�ria
    static void Apagar(TListaX *); ///< Libera TListaX da mem�ria
    static void Debug();        ///< Para testar listaobj e listaitem
private:
    static TGrupoX * Disp;      ///< Lista de TGrupoX dispon�vels (com Total=0)
    static TGrupoX * Usado;     ///< Lista de TGrupoX usados (com Total!=0)
    static unsigned long Tempo; ///< Quando o �ltimo GrupoX mudou
    unsigned int Total;         ///< N�mero de objetos TListaX usados
    TGrupoX * Depois;           ///< Pr�xima vari�vel TGrupoX
    TListaX Lista[TOTAL_LISTAX]; ///< Objetos TListaX
};

//----------------------------------------------------------------------------

#endif
