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
    static int  getTipo(int numfunc);
        ///< Retorna o tipo de variável
    static int  getInt(int numfunc);
        ///< Ler valor numérico da variável como int
    static double getDouble(int numfunc);
        ///< Ler valor numérico da variável como double
    static void setValor(int numfunc, int valor);
        ///< Alterar valor numérico da variável
    static void Exec();
        ///< Para executar passo-a-passo
};

//----------------------------------------------------------------------------

#endif
