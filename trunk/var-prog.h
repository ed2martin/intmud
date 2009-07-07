#ifndef VAR_PROG_H
#define VAR_PROG_H

//----------------------------------------------------------------------------
class TVariavel;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo Prog */
class TVarProg /// Vari�veis Prog
{
public:
    void Criar();           ///< Cria objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TVarProg * destino); ///< Move objeto para outro lugar
    static void ProcEventos(); ///< Processa altera��es pendentes no programa
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();    ///< Ler valor num�rico da vari�vel

private:
    bool FuncExiste(TVariavel * v);
    bool FuncVarComum(TVariavel * v);
    bool FuncVarLocal(TVariavel * v);
    bool FuncVarSav(TVariavel * v);
    bool FuncVarTexto(TVariavel * v);
    bool FuncVarTipo(TVariavel * v);
    bool FuncVarVetor(TVariavel * v);

    bool FuncIniClasse(TVariavel * v);
    bool FuncIniFunc(TVariavel * v);
    bool FuncIniFunc2(TVariavel * v);

    bool FuncLin(TVariavel * v);
    bool FuncDepois(TVariavel * v);
    bool FuncTexto(TVariavel * v);

    void MudaConsulta(int valor); ///< Muda o valor da vari�vel consulta
    unsigned char consulta; ///< O que consultar, 0=n�o est� consultando nada
    TClasse * Classe;   ///< A classe que est� sendo consultada
    union {
        TClasse * ClasseAtual; ///< Valor atual da busca como classe
        char *    TextoAtual;  ///< Valor atual da busca como texto
        int       ValorAtual;  ///< Valor atual da busca como int
    };
    union {
        TClasse * ClasseFim; ///< Valor final da busca como classe
        char *    TextoFim;  ///< Valor final da busca como texto
        int       ValorFim;  ///< Valor final da busca como int
    };

    static TVarProg * Inicio;   ///< Primeiro objeto (com consulta!=0)
    TVarProg * Antes;           ///< Objeto anterior (se consulta!=0)
    TVarProg * Depois;          ///< Pr�ximo objeto (se consulta!=0)
};

//----------------------------------------------------------------------------

#endif
