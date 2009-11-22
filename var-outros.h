#ifndef VAR_OUTROS_H
#define VAR_OUTROS_H

#define TESPERA_MAX 10  // Tempo m�ximo que pode esperar (10 = 1 segundo)

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
    int getInc(); ///< Obt�m o valor de intinc
    int getDec(); ///< Obt�m o valor de intdec
    void setInc(int valor); ///< Muda o valor de intinc
    void setDec(int valor); ///< Muda o valor de intdec
private:
    unsigned int valor; ///< Tempo, usando tanto em intinc quanto em intdec
};

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo inttempo */
class TVarIntTempo /// Vari�veis intinc e intdec
{
public:
    static void PreparaIni();
    static int TempoEspera();
    static void ProcEventos(int TempoDecorrido);
    int  getValor(); ///< Ler valor da vari�vel
    void setValor(int valor); ///< Mudar valor da vari�vel
    void Mover(TVarIntTempo * destino); ///< Move para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);

    const char * defvar;///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

private:
    static void DebugVet(bool mostrar); ///< Checa se listas ligadas est�o OK
    unsigned char IndiceMenos;   ///< �ndice em VetMenos
    unsigned char IndiceMais;    ///< �ndice em VetMais
    TVarIntTempo * Antes;        ///< Objeto anterior na lista ligada
    TVarIntTempo * Depois;       ///< Pr�ximo objeto na lista ligada

    static unsigned int TempoMenos;  ///< Contagem de tempo com VetMenos
    static unsigned int TempoMais;   ///< Contagem de tempo com VetMais
    static TVarIntTempo ** VetMenos; ///< Primeira lista ligada
    static TVarIntTempo ** VetMais;  ///< Segunda lista ligada
};

//----------------------------------------------------------------------------
/// Processa fun��es de vetores de txt1 a txt512
bool FuncVetorTxt(TVariavel * v, const char * nome);

//----------------------------------------------------------------------------

#endif