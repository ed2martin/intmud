#ifndef VAR_PROG_H
#define VAR_PROG_H

//----------------------------------------------------------------------------
#include "classe.h"
class TVariavel;
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
    bool FuncExiste(TVariavel * v); ///< Processa fun��o existe
    bool FuncVarComum(TVariavel * v); ///< Processa fun��o varcomum
    bool FuncVarLocal(TVariavel * v); ///< Processa fun��o varlocal
    bool FuncVarSav(TVariavel * v); ///< Processa fun��o varsav
    bool FuncVarNum(TVariavel * v); ///< Processa fun��o varnum
    bool FuncVarTexto(TVariavel * v); ///< Processa fun��o vartexto
    bool FuncVarTipo(TVariavel * v); ///< Processa fun��o vartipo
    bool FuncVarVetor(TVariavel * v); ///< Processa fun��o varvetor

    bool FuncIniClasse(TVariavel * v); ///< Processa fun��o iniclasse
    bool FuncIniFunc(TVariavel * v); ///< Processa fun��o inifunc
    bool FuncIniFunc2(TVariavel * v); ///< Processa fun��o inifunc2
    bool FuncIniHerda(TVariavel * v); ///< Processa fun��o iniherda
    bool FuncIniHerda2(TVariavel * v); ///< Processa fun��o iniherda2
    bool FuncIniHerdaInv(TVariavel * v); ///< Processa fun��o iniherdainv
    bool FuncIniClasseLin(TVariavel * v); ///< Processa fun��o iniclasselin

    bool FuncLin(TVariavel * v); ///< Processa fun��o lin
    bool FuncNivel(TVariavel * v); ///< Processa fun��o nivel
    bool FuncDepois(TVariavel * v); ///< Processa fun��o depois
    bool FuncTexto(TVariavel * v); ///< Processa fun��o texto

    void MudaConsulta(int valor); ///< Muda o valor da vari�vel consulta
    unsigned char consulta; ///< O que consultar, 0=n�o est� consultando nada
    TClasse * Classe;   ///< A classe que est� sendo consultada
    union {
        TClasse ** PontAtual;  ///< Valor atual da busca
        TClasse * ClasseAtual; ///< Valor atual da busca como classe
        const char * TextoAtual;  ///< Valor atual da busca como texto
        int       ValorAtual;  ///< Valor atual da busca como int
    };
    union {
        TClasse ** PontFim;  ///< Valor final da busca
        TClasse * ClasseFim; ///< Valor final da busca como classe
        const char * TextoFim;  ///< Valor final da busca como texto
        int       ValorFim;  ///< Valor final da busca como int
        TClasse * ClasseHerda[HERDA_TAM]; ///< Usado em IniHerda2
    };

    static TVarProg * Inicio;   ///< Primeiro objeto (com consulta!=0)
    TVarProg * Antes;           ///< Objeto anterior (se consulta!=0)
    TVarProg * Depois;          ///< Pr�ximo objeto (se consulta!=0)
};

//----------------------------------------------------------------------------

#endif
