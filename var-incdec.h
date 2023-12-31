#ifndef VAR_INCDEC_H
#define VAR_INCDEC_H

class TVariavel;
class TVarInfo;

//----------------------------------------------------------------------------
/** Trata das variáveis intinc e intdec */
class TVarIncDec /// Variáveis intinc e intdec
{
public:
    static const TVarInfo * InicializaInc();
        ///< Retorna informações de intinc
    static const TVarInfo * InicializaDec();
        ///< Retorna informações de intdec

private:
    int getInc(int numfunc); ///< Obtém o valor de intinc
    int getDec(int numfunc); ///< Obtém o valor de intdec
    void setInc(int numfunc, int valor); ///< Muda o valor de intinc
    void setDec(int numfunc, int valor); ///< Muda o valor de intdec

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedimInc(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FRedimDec(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBoolInc(TVariavel * v);
    static bool FGetBoolDec(TVariavel * v);
    static int FGetIntInc(TVariavel * v);
    static int FGetIntDec(TVariavel * v);
    static double FGetDoubleInc(TVariavel * v);
    static double FGetDoubleDec(TVariavel * v);
    static const char * FGetTxtInc(TVariavel * v);
    static const char * FGetTxtDec(TVariavel * v);
    static void FOperadorAtribInc(TVariavel * v1, TVariavel * v2);
    static void FOperadorAtribDec(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2Inc(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2Dec(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorComparaInc(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorComparaDec(TVariavel * v1, TVariavel * v2);
    static bool FuncAbs(TVariavel * v); ///< Processa função Abs
    static bool FuncPosInc(TVariavel * v); ///< Processa função Pos de intinc
    static bool FuncPosDec(TVariavel * v); ///< Processa função Pos de intdec
    static bool FuncNegInc(TVariavel * v); ///< Processa função Neg de intinc
    static bool FuncNegDec(TVariavel * v); ///< Processa função Neg de intdec
    static bool FuncVetorInc(TVariavel * v, const char * nome);
        ///< Funções de vetores de intinc
    static bool FuncVetorDec(TVariavel * v, const char * nome);
        ///< Funções de vetores de intdec

    int valor; ///< Tempo, usando tanto em intinc quanto em intdec
};

//----------------------------------------------------------------------------

#endif
