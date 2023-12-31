#ifndef VAR_LISTAOBJ_H
#define VAR_LISTAOBJ_H

class TListaItem;
class TListaX;
class TClasse;
class TObjeto;
class TVariavel;
class TVarInfo;

//----------------------------------------------------------------------------
class TListaObj /// Variáveis ListaObj
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Apagar();          ///< Apaga objeto
    void Mover(TListaObj * destino, TObjeto * o);
        ///< Move TListaObj para outro lugar
    TListaX * AddInicio(TObjeto * obj);
        ///< Adiciona objeto no início da lista
    TListaX * AddFim(TObjeto * obj);
        ///< Adiciona objeto no fim da lista
    TListaX * AddLista(TVariavel * v, TListaX * litem, int tipo);
        ///< Adiciona um ou mais objetos na lista
        /**< @param v De v até Instr::VarAtual = objetos/listas a adicionar
         *   @param litem Item, no caso de adicionar antes ou depois de objeto
         *   @param tipo Aonde adicionar
         *           - 0,4 = no início da lista
         *           - 1,5 = no fim da lista
         *           - 2,6 = antes de litem
         *           - 3,7 = depois de litem
         *           - 0 a 3 = não verifica se objeto já está na lista
         *           - 4 a 7 = adiciona apenas se objeto não estiver na lista
         *   @return Endereço do primeiro objeto adicionado, ou 0 se não houver
         */
    int  getValor();        ///< Ler valor numérico da variável
    TListaX * Inicio;       ///< Primeiro item
    TListaX * Fim;          ///< Último item
    TObjeto * Objeto;       ///< Objeto em que a lista foi definida
    unsigned int Total;     ///< Quantidade de itens da lista

private:
    static bool FuncIni(TVariavel * v);
    static bool FuncFim(TVariavel * v);
    static bool FuncObjLista(TVariavel * v);
    static bool FuncObjIni(TVariavel * v);
    static bool FuncObjFim(TVariavel * v);
    static bool FuncAddIni(TVariavel * v);
    static bool FuncAddFim(TVariavel * v);
    static bool FuncAddIni1(TVariavel * v);
    static bool FuncAddFim1(TVariavel * v);
    static bool FuncRemove(TVariavel * v);
    static bool FuncRand(TVariavel * v);
    static bool FuncInverter(TVariavel * v);
    static bool FuncLimpar(TVariavel * v);
    static bool FuncApagar(TVariavel * v);
    static bool FuncPossui(TVariavel * v);
    static bool FuncTotal(TVariavel * v);

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
};

//----------------------------------------------------------------------------
class TListaItem /// Variável ListaItem, usada para acessar ListaObj
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Apagar();          ///< Apaga objeto
    void Mover(TListaItem * destino, TObjeto * o);
        ///< Move TListaItem para outro lugar
    int  getValor();        ///< Ler valor numérico da variável
    void MudarRef(TListaX * lista); ///< Muda referência de ListaX
    TListaItem * Antes;     ///< Objeto anterior
    TListaItem * Depois;    ///< Próximo objeto
    TListaX * ListaX;       ///< Qual objeto está apontando
    TObjeto * Objeto;       ///< Objeto em que a listaitem foi definida
    const char * defvar;    ///< Definição da variável; usado em var-sav.cpp
    unsigned int indice;    ///< Índice da variável

private:
    static bool FuncTotal(TVariavel * v);
    static bool FuncObj(TVariavel * v);
    static bool FuncObjLista(TVariavel * v);
    static bool FuncAntes(TVariavel * v);
    static bool FuncDepois(TVariavel * v);
    static bool FuncObjAntes(TVariavel * v);
    static bool FuncObjDepois(TVariavel * v);
    static bool FuncRemove(TVariavel * v);
    static bool FuncRemoveAntes(TVariavel * v);
    static bool FuncRemoveDepois(TVariavel * v);
    static bool FuncAddAntes(TVariavel * v);
    static bool FuncAddDepois(TVariavel * v);
    static bool FuncAddAntes1(TVariavel * v);
    static bool FuncAddDepois1(TVariavel * v);

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static void FMoverDef(TVariavel * v);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
    static TObjeto * FGetObj(TVariavel * v);
    static void FOperadorAtrib(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorCompara(TVariavel * v1, TVariavel * v2);
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
    TListaX * ListaDepois;  ///< Próximo objeto da lista
    TObjeto * Objeto;       ///< Objeto que está na lista
    TListaX * ObjAntes;     ///< Objeto anterior da lista
    TListaX * ObjDepois;    ///< Próximo objeto da lista
    TListaItem * ListaItem; ///< Primeira listaitem apontando para esse objeto
    static TListaX * EndMover; ///< Mover() acerta EndMover
};

//----------------------------------------------------------------------------
#define TOTAL_LISTAX 1023
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
