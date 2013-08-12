#ifndef VAR_DEBUG_H
#define VAR_DEBUG_H

//----------------------------------------------------------------------------
class TVariavel;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Debug */
class TVarDebug /// Variáveis Debug
{
public:
    void Criar();           ///< Cria objeto
            /**< Após criado, acertar defvar, indice e chamar EndObjeto() */
    void Apagar();          ///< Apaga objeto
    void Mover(TVarDebug * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    static void FuncEvento(const char * evento, const char * texto);
        ///< Executa uma função
        /**< @param evento Nome do evento (ex. "msg")
         *   @param texto Texto do primeiro argumento, 0=nenhum texto */

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

    const char * defvar;    ///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

    static TVarDebug * ObjAtual; ///< Objeto atual, usado em FuncEvento()
    static TVarDebug * Inicio;   ///< Primeiro objeto
    TVarDebug * Antes;    ///< Objeto anterior
    TVarDebug * Depois;   ///< Próximo objeto
};

//----------------------------------------------------------------------------

#endif
