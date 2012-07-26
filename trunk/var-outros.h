#ifndef VAR_OUTROS_H
#define VAR_OUTROS_H

#define TESPERA_MAX 100  // Tempo máximo que pode esperar (10 = 1 segundo)

class TVariavel;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo REF */
class TVarRef /// Variáveis REF
{
public:
    void MudarPont(TObjeto * obj); ///< Muda a variável TVarRef::Pont
    void Mover(TVarRef * destino); ///< Move TVarRef para outro lugar
    TObjeto * Pont;     ///< Objeto atual
    TVarRef * Antes;
    TVarRef * Depois;
};

//----------------------------------------------------------------------------
/** Trata das variáveis intinc e intdec */
class TVarIncDec /// Variáveis intinc e intdec
{
public:
    int getInc(int numfunc); ///< Obtém o valor de intinc
    int getDec(int numfunc); ///< Obtém o valor de intdec
    void setInc(int numfunc, int valor); ///< Muda o valor de intinc
    void setDec(int numfunc, int valor); ///< Muda o valor de intdec
    bool FuncInc(TVariavel * v, const char * nome);
        ///< Funções de intinc
    bool FuncDec(TVariavel * v, const char * nome);
        ///< Funções de intdec
    bool FuncVetorInc(TVariavel * v, const char * nome);
        ///< Funções de vetores de intinc
    bool FuncVetorDec(TVariavel * v, const char * nome);
        ///< Funções de vetores de intdec
private:
    int valor; ///< Tempo, usando tanto em intinc quanto em intdec
};

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo inttempo */
class TVarIntTempo /// Variáveis intinc e intdec
{
public:
    static void PreparaIni();
    static int TempoEspera();
    static void ProcEventos(int TempoDecorrido);
    int  getValor(int numfunc); ///< Ler valor da variável
    void setValor(int numfunc, int valor); ///< Mudar valor da variável
    void Mover(TVarIntTempo * destino); ///< Move para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Func(TVariavel * v, const char * nome); ///< Funções de inttempo

    const char * defvar;///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor
    bool parado;            ///< Se o contador está parado

private:
    static void DebugVet(bool mostrar); ///< Checa se listas ligadas estão OK
    unsigned short IndiceMenos;   ///< Índice em VetMenos
    unsigned short IndiceMais;    ///< Índice em VetMais
    union {
        TVarIntTempo * Antes;     ///< Objeto anterior na lista ligada
        int ValorParado;          ///< Valor quando contador está parado
    };
    TVarIntTempo * Depois;        ///< Próximo objeto na lista ligada

    static unsigned int TempoMenos;  ///< Contagem de tempo com VetMenos
    static unsigned int TempoMais;   ///< Contagem de tempo com VetMais
    static TVarIntTempo ** VetMenos; ///< Primeira lista ligada
    static TVarIntTempo ** VetMais;  ///< Segunda lista ligada
};

//----------------------------------------------------------------------------
/** Executa eventos */
class TVarIntExec /// Variáveis intinc e intdec
{
public:
    static void ProcEventos(); ///< Processa eventos pendentes
    int  getValor(); ///< Ler valor da variável
    void setValor(int valor); ///< Mudar valor da variável
    void Mover(TVarIntExec * destino); ///< Move para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);

    const char * defvar;///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

private:
    TVarIntExec * Antes;        ///< Objeto anterior na lista ligada
    TVarIntExec * Depois;        ///< Próximo objeto na lista ligada
    static TVarIntExec * Inicio; ///< Primeiro item da lista ligada
    static TVarIntExec * Fim; ///< Último item da lista ligada
};

//----------------------------------------------------------------------------
/// Processa funções de vetores de txt1 a txt512
bool FuncVetorTxt(TVariavel * v, const char * nome);

/// Processa funções de vetores de variáveis int1
bool FuncVetorInt1(TVariavel * v, const char * nome);

/// Lê valor numérico de vetor de int1
int GetVetorInt1(TVariavel * v);

/// Altera valor numérico de vetor de int1
void SetVetorInt1(TVariavel * v, int valor);

//----------------------------------------------------------------------------

#endif
