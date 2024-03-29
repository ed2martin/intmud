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
    static bool FuncMsg(TVariavel * v);    ///< Processa fun��o Msg
    static bool FuncPosx(TVariavel * v);   ///< Processa fun��o Posx
    static bool FuncTecla(TVariavel * v);  ///< Processa fun��o Tecla
    static bool FuncProto(TVariavel * v);  ///< Processa fun��o Proto
    static bool FuncLimpa(TVariavel * v);  ///< Processa fun��o Limpa
    static bool FuncTexto(TVariavel * v);  ///< Processa fun��o Texto
    static bool FuncTotal(TVariavel * v);  ///< Processa fun��o Total
    static bool FuncLinha(TVariavel * v);  ///< Processa fun��o Linha

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

    static unsigned int LinhaFinal;
        ///< Linha na tela correspondente ao campo de edi��o
    static unsigned int LinhaPosic;
        ///< Em que linha o usu�rio colocou o cursor, 0=no editor
    static unsigned int ColPosic;
        ///< Em que coluna o usu�rio colocou o cursor, se LinhaPosic!=0
    static unsigned int ColEscreve;
        ///< Coluna do texto sendo inserido na tela
        /**< O caracter \\n faz ColEscreve=0xFFFF para inserir
         *   nova linha quando chegar o pr�ximo caracter */
    static unsigned int ColEditor;
        ///< Coluna do cursor na tela (texto sendo editado)
    static char * TelaBuf;
        ///< Buffer com c�pia do conte�do da tela
    static unsigned int TelaLin;
        ///< N�mero da linha em TelaBuf que � a �ltima linha de texto na tela

    static const char * LerLinha() { txt_linha[tam_linha]=0; return txt_linha; }
        ///< Retorna o conte�do da linha sendo editada
    static unsigned int col_linha;  ///< linha[col_linha] = primeiro caracter na tela
    static unsigned int tam_linha;  ///< Tamanho da linha, sem o byte =0 no final
    static unsigned int max_linha;  ///< Tamanho m�ximo da linha sendo editada
    static char * txt_linha; ///< Linha sendo editada

    void Criar();           ///< Cria objeto
            /**< Ap�s criado, acertar defvar, indice e chamar EndObjeto() */
    void Apagar();          ///< Apaga objeto
    void Mover(TVarTelaTxt * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);

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
    static void FOperadorAtrib(TVariavel * v1, TVariavel * v2);
    void FOperadorAtribSub(int numfunc, TVariavel * v2);
    static bool FOperadorAdd(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorCompara(TVariavel * v1, TVariavel * v2);

    int  getValor(int numfunc);
                            ///< Ler o valor num�rico da vari�vel
    const char * getTxt(int numfunc);
                            ///< Ler o texto
    void addTxt(int numfunc, const char * txt);
                            ///< Adicionar o texto

    const char * defvar;    ///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

public:
    static const TVarInfo * Inicializa();
        ///< Inicializa vari�vel e retorna informa��es
    static bool Inic();     ///< Inicializa console se Console!=0
    static void Fim();      ///< Encerra o console
    static void Processa(); ///< Processa teclas pressionadas
    static void ProcFim();  ///< Fim do processamento do console
    static bool FuncEvento(const char * evento, const char * texto);
                    ///< Executa uma fun��o
                    /**< @param evento Nome do evento (ex. "msg")
                     *   @param texto Texto do primeiro argumento, 0=nenhum texto
                     *   @return true se n�o apagou o objeto, false se apagou */

private:
    static TVarTelaTxt * ObjAtual; ///< Objeto atual, usado em FuncEvento()

    static TVarTelaTxt * Inicio; ///< Primeiro objeto
    TVarTelaTxt * Antes;    ///< Objeto anterior
    TVarTelaTxt * Depois;   ///< Pr�ximo objeto
};

//---------------------------------------------------------------------------

#endif
