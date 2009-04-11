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
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();        ///< Ler valor num�rico da vari�vel
    TListaX * Inicio;       ///< Primeiro item
    TListaX * Fim;          ///< �ltimo item
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
    TListaObj * Lista;      ///< A qual lista pertence
    TListaX * ListaAntes;   ///< Objeto anterior da lista
    TListaX * ListaDepois;  ///< Pr�ximo objeto da lista
    TObjeto * Objeto;       ///< Objeto que est� na lista
    TListaX * ObjAntes;     ///< Objeto anterior da lista
    TListaX * ObjDepois;    ///< Pr�ximo objeto da lista
    TListaItem * ListaItem; ///< Primeira listaitem apontando para esse objeto
};

#endif
