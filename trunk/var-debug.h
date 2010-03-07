#ifndef VAR_DEBUG_H
#define VAR_DEBUG_H

//----------------------------------------------------------------------------
class TVariavel;

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Debug */
class TVarDebug /// Variáveis Debug
{
public:
    static bool Func(TVariavel * v, const char * nome);
        ///< Função da variável
    static int  getValor(const char * defvar1);
        ///< Ler valor numérico da variável
    static void setValor(const char * defvar1, int valor);
        ///< Alterar valor numérico da variável
};

//----------------------------------------------------------------------------

#endif
