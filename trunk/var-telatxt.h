#ifndef VAR_TELATXT_H
#define VAR_TELATXT_H

//----------------------------------------------------------------------------
class TVariavel;
class TClasse;
class TObjeto;

//---------------------------------------------------------------------------
class TVarTelaTxt
{
private:
    static void Escrever(const char * texto, int tamanho = -1);
        ///< Envia um texto para o console
    static void CursorEditor();
        ///< Posiciona o cursor na linha de edi��o
    static void ProcTecla(const char * texto);
        ///< Processa uma tecla, recebida com LerTecla()
    static void ProcTeclaCursor(int coluna);
        ///< Move o cursor para uma posi��o na linha de edi��o
        /**< Usado em ProcTecla */

    static unsigned int CorTela;
        ///< Cor dos textos, usada em Escrever()
        /**< - Bits 3-0 = fundo
         *   - Bits 6-4 = frente
         *   - Bit 7 = negrito, 0=desativado */
    static unsigned int CorBarraN;
        ///< Cor, ao processar \\n em Escrever()
    static unsigned char EditorPosic;
        ///< 0=cursor no editor, 1=cursor no texto, 2=na linha selecionada
    static unsigned int LinhaPosic;
        ///< Em que linha o usu�rio colocou o cursor, 0=no editor
    static unsigned int ColEscreve;
        ///< Coluna do texto sendo inserido na tela
        /**< O caracter \\n faz ColEscreve=0xFFFF para inserir
         *   nova linha quando chegar o pr�ximo caracter */
    static unsigned int ColEditor;
        ///< Coluna do cursor na tela (texto sendo editado)

    static const char * LerLinha() { txt_linha[tam_linha]=0; return txt_linha; }
        ///< Retorna o conte�do da linha sendo editada
    static unsigned int col_linha;  ///< linha[col_linha] = primeiro caracter na tela
    static unsigned int tam_linha;  ///< Tamanho da linha, sem o byte =0 no final
    static unsigned int max_linha;  ///< Tamanho m�ximo da linha sendo editada
    static char * txt_linha; ///< Linha sendo editada

public:
    void Criar();           ///< Cria objeto
            /**< Ap�s criado, acertar defvar, indice e chamar EndObjeto() */
    void Apagar();          ///< Apaga objeto
    void Mover(TVarTelaTxt * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor(const char * defvar1);
                            ///< Ler o valor num�rico da vari�vel
    void setValor(const char * defvar1, int valor);
                            ///< Mudar o valor num�rico da vari�vel
    const char * getTxt(const char * defvar1);
                            ///< Ler o texto
    void setTxt(const char * defvar1, const char * txt);
                            ///< Mudar o texto
    void addTxt(const char * defvar1, const char * txt);
                            ///< Adicionar o texto

    const char * defvar;    ///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

    static bool Inic();     ///< Inicializa consolse se Console!=0
    static void Fim();      ///< Encerra o console
    static void Processa(); ///< Processa teclas pressionadas
    static void ProcFim();  ///< Fim do processamento do console
    static bool FuncEvento(const char * evento, const char * texto);
                    ///< Executa uma fun��o
                    /**< @param evento Nome do evento (ex. "msg")
                     *   @param texto Texto do primeiro argumento, 0=nenhum texto
                     *   @return true se n�o apagou o objeto, false se apagou
                     */
    static TVarTelaTxt * ObjAtual; ///< Objeto atual, usado em FuncEvento()

    static TVarTelaTxt * Inicio; ///< Primeiro objeto
    TVarTelaTxt * Antes;    ///< Objeto anterior
    TVarTelaTxt * Depois;   ///< Pr�ximo objeto
};

//---------------------------------------------------------------------------

#endif
