#ifndef VAR_ARQTXT_H
#define VAR_ARQTXT_H

#include <stdio.h>

class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
class TVarArqTxt /// Variável arqtxt
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Criar();           ///< Chamado ao criar objeto
    void Apagar();          ///< Apaga objeto
    int  getValor();        ///< Ler valor numérico da variável
private:
    static bool FuncLer(TVariavel * v);          ///< Processa função Ler
    static bool FuncEscr(TVariavel * v);         ///< Processa função Escr
    static bool FuncEof(TVariavel * v);          ///< Processa função Eof
    static bool FuncPos(TVariavel * v);          ///< Processa função Pos
    static bool FuncFechar(TVariavel * v);       ///< Processa função Fechar
    static bool FuncFlush(TVariavel * v);        ///< Processa função Flush
    static bool FuncValido(TVariavel * v);       ///< Processa função Valido
    static bool FuncExiste(TVariavel * v);       ///< Processa função Existe
    static bool FuncAbrir(TVariavel * v);        ///< Processa função Abrir
    static bool FuncTruncar(TVariavel * v);      ///< Processa função Truncar

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

    void Fechar();      ///< Fecha arquivo
    FILE * arq;         ///< Para acessar o arquivo
    bool ModoBinario;   ///< Se está acessando o arquivo em modo binário
};

//----------------------------------------------------------------------------

#endif
