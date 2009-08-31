#ifndef VAR_LISTAOBJ_H
#define VAR_LISTAOBJ_H

//----------------------------------------------------------------------------
class TListaItem;
class TListaX;
class TObjeto;
class TVariavel;

class TListaObj /// Variáveis ListaObj
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TListaObj * destino); ///< Move TListaObj para outro lugar
    void EndObjeto(TObjeto * obj);
    TListaX * AddInicio(TObjeto * obj); ///< Adiciona objeto no início da lista
    TListaX * AddFim(TObjeto * obj); ///< Adiciona objeto no fim da lista
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();        ///< Ler valor numérico da variável
    TListaX * Inicio;       ///< Primeiro item
    TListaX * Fim;          ///< Último item
    TObjeto * Objeto;       ///< Objeto em que a lista foi definida
};

//----------------------------------------------------------------------------
class TListaItem /// Variável ListaItem, usada para acessar ListaObj
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TListaItem * destino); ///< Move TListaItem para outro lugar
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();        ///< Ler valor numérico da variável
    void MudarRef(TListaX * lista); ///< Muda referência de ListaX
    TListaItem * Antes;     ///< Objeto anterior
    TListaItem * Depois;    ///< Próximo objeto
    TListaX * ListaX;       ///< Qual objeto está apontando
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
    TListaX * ListaDepois;  ///< Próximo objeto da lista
    TObjeto * Objeto;       ///< Objeto que está na lista
    TListaX * ObjAntes;     ///< Objeto anterior da lista
    TListaX * ObjDepois;    ///< Próximo objeto da lista
    TListaItem * ListaItem; ///< Primeira listaitem apontando para esse objeto
};

//----------------------------------------------------------------------------
#define TOTAL_LISTAX 1024
class TGrupoX /// Um grupo de objetos TListaX
{
public:
    static void ProcEventos();  ///< Apaga objetos TGrupoX se necessário
    static TListaX * Criar();   ///< Aloca TListaX na memória
    static void Apagar(TListaX *); ///< Libera TListaX da memória
    static void Debug();        ///< Para testar listaobj e listaitem
private:
    static TGrupoX * Disp;      ///< Lista de TGrupoX disponívels (com Total=0)
    static TGrupoX * Usado;     ///< Lista de TGrupoX usados (com Total!=0)
    static unsigned long Tempo; ///< Quando o último GrupoX mudou
    unsigned int Total;         ///< Número de objetos TListaX usados
    TGrupoX * Depois;           ///< Próxima variável TGrupoX
    TListaX Lista[TOTAL_LISTAX]; ///< Objetos TListaX
};

//----------------------------------------------------------------------------

#endif
