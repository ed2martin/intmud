#ifndef VAR_DEBUG_H
#define VAR_DEBUG_H

//----------------------------------------------------------------------------
class TVariavel;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo Debug */
class TVarDebug /// Vari�veis Debug
{
public:
    void Criar();           ///< Cria objeto
            /**< Ap�s criado, acertar defvar, indice e chamar EndObjeto() */
    void Apagar();          ///< Apaga objeto
    void Mover(TVarDebug * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    static void FuncEvento(const char * evento, const char * texto);
        ///< Executa uma fun��o
        /**< @param evento Nome do evento (ex. "msg")
         *   @param texto Texto do primeiro argumento, 0=nenhum texto */

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

    const char * defvar;    ///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

    static TVarDebug * ObjAtual; ///< Objeto atual, usado em FuncEvento()
    static TVarDebug * Inicio;   ///< Primeiro objeto
    TVarDebug * Antes;    ///< Objeto anterior
    TVarDebug * Depois;   ///< Pr�ximo objeto
};

//----------------------------------------------------------------------------

#endif
