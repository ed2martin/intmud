#ifndef VAR_OUTROS_H
#define VAR_OUTROS_H

#define TESPERA_MAX 100  // Tempo m�ximo que pode esperar (10 = 1 segundo)

class TVariavel;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo REF */
class TVarRef /// Vari�veis REF
{
public:
    void MudarPont(TObjeto * obj); ///< Muda a vari�vel TVarRef::Pont
    void Mover(TVarRef * destino); ///< Move TVarRef para outro lugar
    TObjeto * Pont;     ///< Objeto atual
    TVarRef * Antes;
    TVarRef * Depois;
};

//----------------------------------------------------------------------------
/** Trata das vari�veis intinc e intdec */
class TVarIncDec /// Vari�veis intinc e intdec
{
public:
    int getInc(int numfunc); ///< Obt�m o valor de intinc
    int getDec(int numfunc); ///< Obt�m o valor de intdec
    void setInc(int numfunc, int valor); ///< Muda o valor de intinc
    void setDec(int numfunc, int valor); ///< Muda o valor de intdec
    bool FuncInc(TVariavel * v, const char * nome);
        ///< Fun��es de intinc
    bool FuncDec(TVariavel * v, const char * nome);
        ///< Fun��es de intdec
    bool FuncVetorInc(TVariavel * v, const char * nome);
        ///< Fun��es de vetores de intinc
    bool FuncVetorDec(TVariavel * v, const char * nome);
        ///< Fun��es de vetores de intdec
private:
    int valor; ///< Tempo, usando tanto em intinc quanto em intdec
};

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo inttempo */
class TVarIntTempo /// Vari�veis intinc e intdec
{
public:
    static void PreparaIni();
    static int TempoEspera();
    static void ProcEventos(int TempoDecorrido);
    int  getValor(int numfunc); ///< Ler valor da vari�vel
    void setValor(int numfunc, int valor); ///< Mudar valor da vari�vel
    void Mover(TVarIntTempo * destino); ///< Move para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Func(TVariavel * v, const char * nome); ///< Fun��es de inttempo

    const char * defvar;///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor
    bool parado;            ///< Se o contador est� parado

private:
    static void DebugVet(bool mostrar); ///< Checa se listas ligadas est�o OK
    unsigned short IndiceMenos;   ///< �ndice em VetMenos
    unsigned short IndiceMais;    ///< �ndice em VetMais
    union {
        TVarIntTempo * Antes;     ///< Objeto anterior na lista ligada
        int ValorParado;          ///< Valor quando contador est� parado
    };
    TVarIntTempo * Depois;        ///< Pr�ximo objeto na lista ligada

    static unsigned int TempoMenos;  ///< Contagem de tempo com VetMenos
    static unsigned int TempoMais;   ///< Contagem de tempo com VetMais
    static TVarIntTempo ** VetMenos; ///< Primeira lista ligada
    static TVarIntTempo ** VetMais;  ///< Segunda lista ligada
};

//----------------------------------------------------------------------------
/** Executa eventos */
class TVarIntExec /// Vari�veis intinc e intdec
{
public:
    static void ProcEventos(); ///< Processa eventos pendentes
    int  getValor(); ///< Ler valor da vari�vel
    void setValor(int valor); ///< Mudar valor da vari�vel
    void Mover(TVarIntExec * destino); ///< Move para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);

    const char * defvar;///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

private:
    TVarIntExec * Antes;        ///< Objeto anterior na lista ligada
    TVarIntExec * Depois;        ///< Pr�ximo objeto na lista ligada
    static TVarIntExec * Inicio; ///< Primeiro item da lista ligada
    static TVarIntExec * Fim; ///< �ltimo item da lista ligada
};

//----------------------------------------------------------------------------
/// Processa fun��es de vetores de txt1 a txt512
bool FuncVetorTxt(TVariavel * v, const char * nome);

/// Processa fun��es de vetores de vari�veis int1
bool FuncVetorInt1(TVariavel * v, const char * nome);

/// L� valor num�rico de vetor de int1
int GetVetorInt1(TVariavel * v);

/// Altera valor num�rico de vetor de int1
void SetVetorInt1(TVariavel * v, int valor);

//----------------------------------------------------------------------------

#endif
