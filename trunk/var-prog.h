#ifndef VAR_PROG_H
#define VAR_PROG_H

//----------------------------------------------------------------------------
class TVariavel;
class TClasse;
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
    bool FuncExiste(TVariavel * v);
    bool FuncVarComum(TVariavel * v);
    bool FuncVarLocal(TVariavel * v);
    bool FuncVarSav(TVariavel * v);
    bool FuncVarTexto(TVariavel * v);
    bool FuncVarTipo(TVariavel * v);
    bool FuncVarVetor(TVariavel * v);

    unsigned char consulta; ///< O que consultar, 0=não está consultando nada
    static TVarProg * Inicio;   ///< Primeiro objeto (com consulta!=0)
    TVarProg * Antes;           ///< Objeto anterior (se consulta!=0)
    TVarProg * Depois;          ///< Próximo objeto (se consulta!=0)
};

//----------------------------------------------------------------------------

#endif
