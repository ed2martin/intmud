#ifndef VAR_PROG_H
#define VAR_PROG_H

//----------------------------------------------------------------------------
#include "classe.h"
class TVariavel;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Prog */
class TVarProg /// Variáveis Prog
{
public:
    void Criar();           ///< Cria objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TVarProg * destino); ///< Move objeto para outro lugar
    static void ProcEventos(); ///< Processa alterações pendentes no programa
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();    ///< Ler valor numérico da variável

private:
    bool FuncExiste(TVariavel * v); ///< Processa função existe
    bool FuncVarComum(TVariavel * v); ///< Processa função varcomum
    bool FuncVarLocal(TVariavel * v); ///< Processa função varlocal
    bool FuncVarSav(TVariavel * v); ///< Processa função varsav
    bool FuncVarNum(TVariavel * v); ///< Processa função varnum
    bool FuncVarTexto(TVariavel * v); ///< Processa função vartexto
    bool FuncVarTipo(TVariavel * v); ///< Processa função vartipo
    bool FuncVarVetor(TVariavel * v); ///< Processa função varvetor

    bool FuncIniClasse(TVariavel * v); ///< Processa função iniclasse
    bool FuncIniFunc(TVariavel * v); ///< Processa função inifunc
    bool FuncIniFunc2(TVariavel * v); ///< Processa função inifunc2
    bool FuncIniHerda(TVariavel * v); ///< Processa função iniherda
    bool FuncIniHerda2(TVariavel * v); ///< Processa função iniherda2
    bool FuncIniHerdaInv(TVariavel * v); ///< Processa função iniherdainv
    bool FuncIniClasseLin(TVariavel * v); ///< Processa função iniclasselin

    bool FuncLin(TVariavel * v); ///< Processa função lin
    bool FuncNivel(TVariavel * v); ///< Processa função nivel
    bool FuncDepois(TVariavel * v); ///< Processa função depois
    bool FuncTexto(TVariavel * v); ///< Processa função texto

    void MudaConsulta(int valor); ///< Muda o valor da variável consulta
    unsigned char consulta; ///< O que consultar, 0=não está consultando nada
    TClasse * Classe;   ///< A classe que está sendo consultada
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
    TVarProg * Depois;          ///< Próximo objeto (se consulta!=0)
};

//----------------------------------------------------------------------------

#endif
