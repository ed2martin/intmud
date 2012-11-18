#ifndef VAR_NOMEOBJ_H
#define VAR_NOMEOBJ_H

//----------------------------------------------------------------------------
class TVariavel;

/** Trata das vari�veis do tipo NomeObj */
class TVarNomeObj /// Vari�veis NomeObj
{
public:
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();    ///< Ler valor num�rico da vari�vel
private:
    char NomeObj[64];   ///< Nome do item que est� procurando
    int  Achou;         ///< Quantos itens achou na �ltima busca
    int  Inicio;        ///< N�mero de itens antes
    int  Total;         ///< N�mero m�ximo de itens
};

//----------------------------------------------------------------------------

#endif
