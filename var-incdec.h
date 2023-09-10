#ifndef VAR_INCDEC_H
#define VAR_INCDEC_H

class TVariavel;

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

#endif
