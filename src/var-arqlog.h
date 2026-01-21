#ifndef VAR_ARQLOG_H
#define VAR_ARQLOG_H

class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
class TVarArqLog /// Variável arqlog
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Criar();           ///< Chamado ao criar objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TVarArqLog * destino); ///< Move para outro lugar
    int  getValor();        ///< Ler valor numérico da variável
    static int TempoEspera(int tempodecorrido);
private:
    static bool FuncMsg(TVariavel * v);    ///< Processa função Msg
    static bool FuncFechar(TVariavel * v); ///< Processa função Fechar
    static bool FuncValido(TVariavel * v); ///< Processa função Valido
    static bool FuncExiste(TVariavel * v); ///< Processa função Existe
    static bool FuncAbrir(TVariavel * v);  ///< Processa função Abrir

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
    static int Tempo;   ///< Quanto tempo para atualizar arquivos
    static TVarArqLog * Inicio; ///< Primeiro objeto com arq>=0
    TVarArqLog * Antes; ///< Objeto anterior com arq>=0
    TVarArqLog * Depois;///< Próximo objeto com arq>=0
    int  arq;           ///< Para acessar o arquivo
    int  pontlog;       ///< Ponteiro para anotar texto em buflog
    char buflog[2048];  ///< Texto a ser gravado
                        /**< @note Deve ser a última variável da classe */
};

//----------------------------------------------------------------------------

#endif
