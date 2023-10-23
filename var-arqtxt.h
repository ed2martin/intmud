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
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
private:
    bool FuncLer(TVariavel * v);          ///< Processa função Ler
    bool FuncEscr(TVariavel * v);         ///< Processa função Escr
    bool FuncEof(TVariavel * v);          ///< Processa função Eof
    bool FuncPos(TVariavel * v);          ///< Processa função Pos
    bool FuncFechar(TVariavel * v);       ///< Processa função Fechar
    bool FuncFlush(TVariavel * v);        ///< Processa função Flush
    bool FuncValido(TVariavel * v);       ///< Processa função Valido
    bool FuncExiste(TVariavel * v);       ///< Processa função Existe
    bool FuncAbrir(TVariavel * v);        ///< Processa função Abrir
    bool FuncTruncar(TVariavel * v);      ///< Processa função Truncar

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);

    void Fechar();      ///< Fecha arquivo
    FILE * arq;         ///< Para acessar o arquivo
    bool ModoBinario;   ///< Se está acessando o arquivo em modo binário
};

//----------------------------------------------------------------------------

#endif
