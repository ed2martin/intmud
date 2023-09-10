#ifndef VAR_INTEXEC_H
#define VAR_INTEXEC_H

class TVariavel;
class TObjeto;

//----------------------------------------------------------------------------
/** Executa eventos */
class TVarIntExec /// Variáveis intexec
{
public:
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
    TVarIntExec * Antes;        ///< Objeto anterior na lista ligada
    TVarIntExec * Depois;        ///< Próximo objeto na lista ligada
    static TVarIntExec * Inicio; ///< Primeiro item da lista ligada
    static TVarIntExec * Fim; ///< Último item da lista ligada
};

//----------------------------------------------------------------------------

#endif
