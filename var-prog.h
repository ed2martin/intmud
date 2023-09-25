#ifndef VAR_PROG_H
#define VAR_PROG_H

#include "classe.h"
class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;
class TArqMapa;

//----------------------------------------------------------------------------
enum TProgConsulta ///< O que consultar, TVarProg::consulta
{
    prNada,     ///< Não está consultando nada
    prArquivo,  ///< iniarq() para arquivos
    prArqCl,    ///< iniarq() para classes de um arquivo
    prClasse,   ///< iniclasse()
    prFunc,     ///< inifunc()
    prFuncTudo, ///< inifunctudo()
    prFuncCl,   ///< inifunccl()
    prHerda,    ///< iniherda()
    prHerdaTudo,///< iniherdatudo()
    prHerdaInv, ///< iniherdainv()
    prLinhaCl,  ///< inilinha() - linhas de uma classe
    prLinhaFunc,///< inilinha() - linhas de uma função
    prLinhaVar  ///< inilinha() - uma variável (uma única linha)
};

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Prog */
class TVarProg /// Variáveis Prog
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Criar();           ///< Cria objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TVarProg * destino); ///< Move objeto para outro lugar
    static void LimparVar(); ///< Apaga referências do programa nas variáveis prog
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();    ///< Ler valor numérico da variável

private:
    bool FuncExiste(TVariavel * v);       ///< Processa função Existe
    bool FuncArquivo(TVariavel * v);      ///< Processa função Arquivo
    bool FuncArqNome(TVariavel * v);      ///< Processa função ArqNome
    bool FuncVarComum(TVariavel * v);     ///< Processa função VarComum
    bool FuncVarSav(TVariavel * v);       ///< Processa função VarSav
    bool FuncVarNum(TVariavel * v);       ///< Processa função VarNum
    bool FuncVarTexto(TVariavel * v);     ///< Processa função VarTexto
    bool FuncVarTipo(TVariavel * v);      ///< Processa função VarTipo
    bool FuncVarLugar(TVariavel * v);     ///< Processa função VarLugar
    bool FuncVarVetor(TVariavel * v);     ///< Processa função VarVetor
    bool FuncConst(TVariavel * v);        ///< Processa função Const
    bool FuncClasse(TVariavel * v);       ///< Processa função Classe

    bool FuncIniArq(TVariavel * v);       ///< Processa função IniArq
    bool FuncIniClasse(TVariavel * v);    ///< Processa função IniClasse
    bool FuncIniFunc(TVariavel * v);      ///< Processa função IniFunc
    bool FuncIniFuncTudo(TVariavel * v);  ///< Processa função IniFuncTudo
    bool FuncIniFuncCl(TVariavel * v);    ///< Processa função IniFuncCl
    bool FuncIniHerda(TVariavel * v);     ///< Processa função IniHerda
    bool FuncIniHerdaTudo(TVariavel * v); ///< Processa função IniHerdatudo
    bool FuncIniHerdaInv(TVariavel * v);  ///< Processa função IniHerdainv
    bool FuncIniLinha(TVariavel * v);     ///< Processa função IniLinha

    bool FuncLin(TVariavel * v);          ///< Processa função Lin
    bool FuncNivel(TVariavel * v);        ///< Processa função Nivel
    bool FuncDepois(TVariavel * v);       ///< Processa função Depois
    bool FuncTexto(TVariavel * v);        ///< Processa função Texto

    bool FuncApagar(TVariavel * v);       ///< Processa função Apagar
    bool FuncCriar(TVariavel * v);        ///< Processa função Criar
    bool FuncApagarLin(TVariavel * v);    ///< Processa função ApagarLin
    bool FuncCriarLin(TVariavel * v);     ///< Processa função CriarLin
    bool FuncFAntes(TVariavel * v);       ///< Processa função FAntes
    bool FuncFDepois(TVariavel * v);      ///< Processa função FDepois
    bool FuncMudar(TVariavel * v, int lugar); ///< Chamado por FuncF*
    bool FuncRenomear(TVariavel * v);     ///< Processa função Renomear
    bool FuncSalvar(TVariavel * v);       ///< Processa função Salvar
    bool FuncSalvarTudo(TVariavel * v);   ///< Processa função SalvarTudo

    bool FuncClIni(TVariavel * v);        ///< Processa função ClIni
    bool FuncClFim(TVariavel * v);        ///< Processa função ClFim
    bool FuncClAntes(TVariavel * v);      ///< Processa função ClAntes
    bool FuncClDepois(TVariavel * v);     ///< Processa função ClDepois

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);

    void MudaConsulta(TProgConsulta valor);
        ///< Muda o valor da variável consulta
    TProgConsulta consulta;     ///< O que consultar
    TClasse * Classe;           ///< A classe que está sendo consultada
    union {
        TArqMapa * ArqAtual;    ///< Valor atual da busca como arquivo
        TClasse ** PontAtual;   ///< Valor atual da busca
        TClasse * ClasseAtual;  ///< Valor atual da busca como classe
        const char * TextoAtual; ///< Valor atual da busca como texto
        int       ValorAtual;   ///< Valor atual da busca como int
    };
    union {
        TClasse ** PontFim;     ///< Valor final da busca
        TClasse * ClasseFim;    ///< Valor final da busca como classe
        const char * TextoFim;  ///< Valor final da busca como texto
        int       ValorFim;     ///< Valor final da busca como int
        TClasse * ClasseHerda[HERDA_TAM]; ///< Usado em IniHerda2
    };

    static TVarProg * Inicio;   ///< Primeiro objeto (com consulta!=0)
    TVarProg * Antes;           ///< Objeto anterior (se consulta!=0)
    TVarProg * Depois;          ///< Próximo objeto (se consulta!=0)
};

//----------------------------------------------------------------------------

#endif
