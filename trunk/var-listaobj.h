#ifndef VAR_LISTAOBJ_H
#define VAR_LISTAOBJ_H

//----------------------------------------------------------------------------
class TListaItem;
class TListaX;
class TClasse;
class TObjeto;
class TVariavel;

class TListaObj /// Vari�veis ListaObj
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TListaObj * destino); ///< Move TListaObj para outro lugar
    TListaX * AddInicio(TObjeto * obj);
        ///< Adiciona objeto no in�cio da lista
    TListaX * AddFim(TObjeto * obj);
        ///< Adiciona objeto no fim da lista
    TListaX * AddLista(TVariavel * v, TListaX * litem, int tipo);
        ///< Adiciona um ou mais objetos na lista
        /**< @param v De v at� Instr::VarAtual = objetos/listas a adicionar
         *   @param litem Item, no caso de adicionar antes ou depois de objeto
         *   @param tipo Aonde adicionar
         *           - 0,4 = no in�cio da lista
         *           - 1,5 = no fim da lista
         *           - 2,6 = antes de litem
         *           - 3,7 = depois de litem
         *           - 0 a 3 = n�o verifica se objeto j� est� na lista
         *           - 4 a 7 = adiciona apenas se objeto n�o estiver na lista
         *   @return Endere�o do primeiro objeto adicionado, ou 0 se n�o houver
         */
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();        ///< Ler valor num�rico da vari�vel
    TListaX * Inicio;       ///< Primeiro item
    TListaX * Fim;          ///< �ltimo item
    TObjeto * Objeto;       ///< Objeto em que a lista foi definida
    unsigned int Total;     ///< Quantidade de itens da lista

    bool FuncIni(TVariavel * v);
    bool FuncFim(TVariavel * v);
    bool FuncObjLista(TVariavel * v);
    bool FuncObjIni(TVariavel * v);
    bool FuncObjFim(TVariavel * v);
    bool FuncAddIni(TVariavel * v);
    bool FuncAddFim(TVariavel * v);
    bool FuncAddIni1(TVariavel * v);
    bool FuncAddFim1(TVariavel * v);
    bool FuncRemove(TVariavel * v);
    bool FuncRand(TVariavel * v);
    bool FuncLimpar(TVariavel * v);
    bool FuncApagar(TVariavel * v);
    bool FuncPossui(TVariavel * v);
    bool FuncTotal(TVariavel * v);
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
    TObjeto * Objeto;       ///< Objeto em que a listaitem foi definida
    const char * defvar;    ///< Defini��o da vari�vel; usado em var-sav.cpp
    unsigned int indice;    ///< �ndice da vari�vel

    bool FuncTotal(TVariavel * v);
    bool FuncObj(TVariavel * v);
    bool FuncObjLista(TVariavel * v);
    bool FuncAntes(TVariavel * v);
    bool FuncDepois(TVariavel * v);
    bool FuncObjAntes(TVariavel * v);
    bool FuncObjDepois(TVariavel * v);
    bool FuncRemove(TVariavel * v);
    bool FuncRemoveAntes(TVariavel * v);
    bool FuncRemoveDepois(TVariavel * v);
    bool FuncAddAntes(TVariavel * v);
    bool FuncAddDepois(TVariavel * v);
    bool FuncAddAntes1(TVariavel * v);
    bool FuncAddDepois1(TVariavel * v);
};

//----------------------------------------------------------------------------
class TListaX /// Um item de ListaObj
{
public:
    static TListaX * Criar(); ///< Cria um objeto
    void Apagar();          ///< Apaga objeto
                            /**< Importante: ao apagar um objeto TListaX,
                                 qualquer outro objeto TListaX
                                 pode ser movido */
    void Mover(TListaX * destino); ///< Move TListaX para outro lugar
    TListaObj * Lista;      ///< A qual lista pertence
    TListaX * ListaAntes;   ///< Objeto anterior da lista
    TListaX * ListaDepois;  ///< Pr�ximo objeto da lista
    TObjeto * Objeto;       ///< Objeto que est� na lista
    TListaX * ObjAntes;     ///< Objeto anterior da lista
    TListaX * ObjDepois;    ///< Pr�ximo objeto da lista
    TListaItem * ListaItem; ///< Primeira listaitem apontando para esse objeto
    static TListaX * EndMover; ///< Mover() acerta EndMover
};

//----------------------------------------------------------------------------
#define TOTAL_LISTAX 1023
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
