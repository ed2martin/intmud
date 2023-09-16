#ifndef VAR_ARQLOG_H
#define VAR_ARQLOG_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarArqLog /// Variável arqlog
{
public:
    void Criar();           ///< Chamado ao criar objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TVarArqLog * destino); ///< Move para outro lugar
    int  getValor();        ///< Ler valor numérico da variável
    static int TempoEspera(int tempodecorrido);
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
private:
    bool FuncMsg(TVariavel * v);    ///< Processa função Msg
    bool FuncFechar(TVariavel * v); ///< Processa função Fechar
    bool FuncValido(TVariavel * v); ///< Processa função Valido
    bool FuncExiste(TVariavel * v); ///< Processa função Existe
    bool FuncAbrir(TVariavel * v);  ///< Processa função Abrir

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
