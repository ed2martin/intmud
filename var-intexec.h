#ifndef VAR_INTEXEC_H
#define VAR_INTEXEC_H

class TVariavel;
class TVarInfo;
class TObjeto;

//----------------------------------------------------------------------------
/** Executa eventos */
class TVarIntExec /// Variáveis intexec
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    static void ProcEventos(); ///< Processa eventos pendentes
    int  getValor(); ///< Ler valor da variável
    void setValor(int valor); ///< Mudar valor da variável
    void Mover(TVarIntExec * destino); ///< Move para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);

    const char * defvar;///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

private:
    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static void FMoverDef(TVariavel * v);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
    static void FOperadorAtrib(TVariavel * v);

    TVarIntExec * Antes;        ///< Objeto anterior na lista ligada
    TVarIntExec * Depois;        ///< Próximo objeto na lista ligada
    static TVarIntExec * Inicio; ///< Primeiro item da lista ligada
    static TVarIntExec * Fim; ///< Último item da lista ligada
};

//----------------------------------------------------------------------------

#endif
