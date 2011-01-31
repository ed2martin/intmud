#ifndef VAR_DEBUG_H
#define VAR_DEBUG_H

//----------------------------------------------------------------------------
class TVariavel;

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo Debug */
class TVarDebug /// Vari�veis Debug
{
public:
    static bool Func(TVariavel * v, const char * nome);
        ///< Fun��o da vari�vel
    static int  getTipo(int numfunc);
        ///< Retorna o tipo de vari�vel
    static int  getInt(int numfunc);
        ///< Ler valor num�rico da vari�vel como int
    static double getDouble(int numfunc);
        ///< Ler valor num�rico da vari�vel como double
    static void setValor(int numfunc, int valor);
        ///< Alterar valor num�rico da vari�vel
    static void Exec();
        ///< Para executar passo-a-passo
};

//----------------------------------------------------------------------------

#endif
