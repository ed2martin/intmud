#ifndef VAR_LOG_H
#define VAR_LOG_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarLog /// Vari�vel log
{
public:
    void Criar();           ///< Chamado ao criar objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TVarLog * destino); ///< Move para outro lugar
    int  getValor();        ///< Ler valor num�rico da vari�vel
    static int TempoEspera(int tempodecorrido);
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
private:
    void Fechar();      ///< Fecha arquivo
    static int Tempo;   ///< Quanto tempo para atualizar arquivos
    static TVarLog * Inicio; ///< Primeiro objeto com arq>=0
    TVarLog * Antes;    ///< Objeto anterior com arq>=0
    TVarLog * Depois;   ///< Pr�ximo objeto com arq>=0
    int  arq;           ///< Para acessar o arquivo
    int  pontlog;       ///< Ponteiro para anotar texto em buflog
    char buflog[2048];  ///< Texto a ser gravado
                        /**< @note Deve ser a �ltima vari�vel da classe */
};

//----------------------------------------------------------------------------

#endif
