#ifndef VAR_NOMEOBJ_H
#define VAR_NOMEOBJ_H

class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo NomeObj */
class TVarNomeObj /// Variáveis NomeObj
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();    ///< Ler valor numérico da variável
private:
    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);

    char NomeObj[64];   ///< Nome do item que está procurando
    int  Achou;         ///< Quantos itens achou na última busca
    int  Inicio;        ///< Número de itens antes
    int  Total;         ///< Número máximo de itens
};

//----------------------------------------------------------------------------

#endif
