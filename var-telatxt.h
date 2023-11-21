#ifndef VAR_TELATXT_H
#define VAR_TELATXT_H

class TVariavel;
class TVarInfo;
enum TVarTipo : unsigned char;
class TClasse;
class TObjeto;

//---------------------------------------------------------------------------
class TVarTelaTxt
{
private:
    bool FuncMsg(TVariavel * v);    ///< Processa função Msg
    bool FuncPosx(TVariavel * v);   ///< Processa função Posx
    bool FuncTecla(TVariavel * v);  ///< Processa função Tecla
    bool FuncProto(TVariavel * v);  ///< Processa função Proto
    bool FuncLimpa(TVariavel * v);  ///< Processa função Limpa
    bool FuncTexto(TVariavel * v);  ///< Processa função Texto
    bool FuncTotal(TVariavel * v);  ///< Processa função Total
    bool FuncLinha(TVariavel * v);  ///< Processa função Linha

    static void Escrever(const char * texto, int tamanho = -1);
        ///< Envia um texto para o console
    static void CursorEditor();
        ///< Posiciona o cursor na linha de edição
    static void ProcTecla(const char * texto);
        ///< Processa uma tecla, recebida com LerTecla()
    static void ProcTeclaCursor(int coluna);
        ///< Move o cursor para uma posição na linha de edição
        /**< Usado em ProcTecla */

    static unsigned int CorTela;
        ///< Cor dos textos, usada em Escrever()
        /**< - Bits 3-0 = fundo
         *   - Bits 6-4 = frente
         *   - Bit 7 = negrito, 0=desativado */
    static unsigned int CorBarraN;
        ///< Cor, ao processar \\n em Escrever()

    static unsigned int LinhaFinal;
        ///< Linha na tela correspondente ao campo de edição
    static unsigned int LinhaPosic;
        ///< Em que linha o usuário colocou o cursor, 0=no editor
    static unsigned int ColPosic;
        ///< Em que coluna o usuário colocou o cursor, se LinhaPosic!=0
    static unsigned int ColEscreve;
        ///< Coluna do texto sendo inserido na tela
        /**< O caracter \\n faz ColEscreve=0xFFFF para inserir
         *   nova linha quando chegar o próximo caracter */
    static unsigned int ColEditor;
        ///< Coluna do cursor na tela (texto sendo editado)
    static char * TelaBuf;
        ///< Buffer com cópia do conteúdo da tela
    static unsigned int TelaLin;
        ///< Número da linha em TelaBuf que é a última linha de texto na tela

    static const char * LerLinha() { txt_linha[tam_linha]=0; return txt_linha; }
        ///< Retorna o conteúdo da linha sendo editada
    static unsigned int col_linha;  ///< linha[col_linha] = primeiro caracter na tela
    static unsigned int tam_linha;  ///< Tamanho da linha, sem o byte =0 no final
    static unsigned int max_linha;  ///< Tamanho máximo da linha sendo editada
    static char * txt_linha; ///< Linha sendo editada

public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Criar();           ///< Cria objeto
            /**< Após criado, acertar defvar, indice e chamar EndObjeto() */
    void Apagar();          ///< Apaga objeto
    void Mover(TVarTelaTxt * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Func(TVariavel * v, const char * nome); ///< Função da variável

    static int FTamanho(const char * instr);
    static TVarTipo FTipo(TVariavel * v);
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
    void FOperadorAtribSub(TVariavel * v);

    int  getValor(int numfunc);
                            ///< Ler o valor numérico da variável
    void setValor(int numfunc, int valor);
                            ///< Mudar o valor numérico da variável
    const char * getTxt(int numfunc);
                            ///< Ler o texto
    void setTxt(int numfunc, const char * txt);
                            ///< Mudar o texto
    void addTxt(int numfunc, const char * txt);
                            ///< Adicionar o texto

    const char * defvar;    ///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

    static bool Inic();     ///< Inicializa consolse se Console!=0
    static void Fim();      ///< Encerra o console
    static void Processa(); ///< Processa teclas pressionadas
    static void ProcFim();  ///< Fim do processamento do console
    static bool FuncEvento(const char * evento, const char * texto);
                    ///< Executa uma função
                    /**< @param evento Nome do evento (ex. "msg")
                     *   @param texto Texto do primeiro argumento, 0=nenhum texto
                     *   @return true se não apagou o objeto, false se apagou */
    static TVarTelaTxt * ObjAtual; ///< Objeto atual, usado em FuncEvento()

    static TVarTelaTxt * Inicio; ///< Primeiro objeto
    TVarTelaTxt * Antes;    ///< Objeto anterior
    TVarTelaTxt * Depois;   ///< Próximo objeto
};

//---------------------------------------------------------------------------

#endif
