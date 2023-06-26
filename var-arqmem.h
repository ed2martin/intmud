#ifndef VAR_ARQMEM_H
#define VAR_ARQMEM_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarArqMem
{
private:
    /// Cabeçalho de um bloco
    class TBloco
    {
    public:
        TBloco * Antes; ///< Objeto anterior ou NULL se não houver
        TBloco * Depois; ///< Próximo objeto ou NULL se não houver
        int Posicao; ///< Posição no arquivo do primeiro byte desse bloco
        int Tamanho; ///< Quantidade de bytes desse bloco
    };
    TBloco * Inicio; ///< Primeiro bloco
    TBloco * Fim; ///< Último bloco
    TBloco * PosBloco; ///< Bloco da posição atual
    int PosByte; ///< Byte do bloco da posição atual
    int ArqByte; ///< Quantos bytes usados no último bloco

    bool CriarBloco();
        ///< Uso interno: cria um bloco no final do arquivo
        /**< @return true se conseguiu criar, false se arquivo muito grande */
    void ApagarBloco();
        ///< Uso interno: apaga o último bloco do arquivo
    void Debug();
        ///< Verifica se variáveis de TArqMem estão corretas

    bool FuncAdd(TVariavel * v);          ///< Processa função Add
    bool FuncAddBin(TVariavel * v);       ///< Processa função AddBin
    bool FuncEof(TVariavel * v);          ///< Processa função Eof
    bool FuncEscr(TVariavel * v);         ///< Processa função Escr
    bool FuncEscrBin(TVariavel * v);      ///< Processa função EscrBin
    bool FuncLer(TVariavel * v);          ///< Processa função Ler
    bool FuncLerBin(TVariavel * v);       ///< Processa função LerBin
    bool FuncLerBinEsp(TVariavel * v);    ///< Processa função LerBinEsp
    bool FuncLerBinComum(TVariavel * v, bool espaco);
            ///< Usado por FuncLerBin e FuncLerBinEsp
    bool FuncLimpar(TVariavel * v);       ///< Processa função Limpar
    bool FuncPos(TVariavel * v);          ///< Processa função Pos
    bool FuncTamanho(TVariavel * v);      ///< Processa função Tamanho
    bool FuncTruncar(TVariavel * v);      ///< Processa função Truncar

public:
    TVarArqMem() { Criar(); }  ///< Construtor
    ~TVarArqMem() { Apagar(); } ///< Destrutor
    void Criar();           ///< Chamado ao criar objeto
    void Apagar();          ///< Apaga objeto
    bool Func(TVariavel * v, const char * nome); ///< Função da variável

    int Ler(char * buffer, int tamanho);
        ///< Lê a partir da posição atual, retorna quantos bytes lidos
    int Escrever(char * buffer, int tamanho);
        ///< Escreve a partir da posição atual, retorna quantos bytes escritos
    int Tamanho();
        ///< Retorna o tamanho do arquivo
    int Posicao();
        ///< Retorna a posição no arquivo
    void Posicao(int novapos);
        ///< Muda a posição no arquivo
    void TruncarZero();
        ///< Apaga tudo que tem no arquivo
    void TruncarPosicao();
        ///< Trunca o arquivo na posição atual (apaga tudo que vem depois)

    class TArqLer
    {
    public:
        TArqLer(TVarArqMem * arqmem, bool inicio);
            ///< Inicia a leitura, acerta as variáveis buffer e tamanho
        void Proximo();
            ///< Passa para o próximo bloco, acerta as variáveis buffer e tamanho
        void MudaPosicao(const char * posicao);
            ///< Muda posição no arquivo
        const char * buffer;
            ///< Buffer com os dados para serem lidos
        int tamanho;
            ///< Número de caracteres em buffer ou 0 se não houver
    private:
        TBloco * bloco; ///< Bloco atual
        TVarArqMem * arqmem; ///< Objeto ArqMem
    };
};

#endif
