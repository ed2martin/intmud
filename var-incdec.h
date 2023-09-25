#ifndef VAR_INCDEC_H
#define VAR_INCDEC_H

class TVariavel;
class TVarInfo;

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
    static bool FuncVetorInc(TVariavel * v, const char * nome);
        ///< Funções de vetores de intinc
    static bool FuncVetorDec(TVariavel * v, const char * nome);
        ///< Funções de vetores de intdec

    static const TVarInfo * InicializaInc();
        ///< Retorna informações de intinc
    static const TVarInfo * InicializaDec();
        ///< Retorna informações de intdec
private:
    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);

    int valor; ///< Tempo, usando tanto em intinc quanto em intdec
};

//----------------------------------------------------------------------------

#endif
