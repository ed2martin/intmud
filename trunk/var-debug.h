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
    static int  getValor(const char * defvar1);
        ///< Ler valor num�rico da vari�vel
    static void setValor(const char * defvar1, int valor);
        ///< Alterar valor num�rico da vari�vel
};

//----------------------------------------------------------------------------

#endif
