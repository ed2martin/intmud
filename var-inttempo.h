#ifndef VAR_INTTEMPO_H
#define VAR_INTTEMPO_H

class TVariavel;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo inttempo */
class TVarIntTempo /// Variáveis inttempo
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
    bool FuncVetor(TVariavel * v, const char * nome); ///< Funções de vetores

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

#endif
