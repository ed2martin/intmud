#ifndef MISC_H
#define MISC_H

//----------------------------------------------------------------------------
/** @defgroup misc_h Diversas fun��es */
/** @{ */

#define INT_EXT "int" ///< Extens�o dos arquivos do programa interpretado
#define INT_NOME_TAM 0x200 ///< Tamanho m�ximo dos nomes dos arquivos ".int"

extern unsigned long TempoIni; ///< Tempo desde que o programa foi executado, 10=1seg
                         /**< @note Atualizado em main.cpp */

extern char * arqnome;   ///< Nome completo dos arquivos
extern char * arqinicio; ///< A partir de onde colocar os nomes dos arquivos
                         /** Tem espa�o para pelo menos 0x200 caracteres. */

/// Prepara tabNOMES[] e tabCOMPLETO[]
/** Usado para transforma��o de caracteres.
 *  tabASCinic deve ser chamado no in�cio do programa. */
void tabASCinic(void);

extern char * tabNOMES1;   ///< Apenas caracteres v�lidos em nomes de classes
extern char * tabNOMES2;   ///< Para comparar nomes de classes e fun��es
extern char * tabCOMPLETO; ///< Todos os caracteres
extern char * tabMAI;      ///< Todos os caracteres em letras mai�sculas
extern char * tabMIN;      ///< Todos os caracteres em letras min�sculas
extern char * tabMAIMIN;   ///< 1=letra min�scula, 2=mai�scula, 0=nenhum
extern char * tabTXTCOD;   ///< Usado para codificar caracteres: txtcod
extern char * tabTXTDEC;   ///< Usado para decodificar caracteres: txtdec
extern char * tab8B;       ///< txtremove sem filtro de letras acentuadas
extern char * tab7B;       ///< txtremove com filtro de letras acentuadas
extern char * tabTXTSEPARA;///< Usado em txtsepara
extern char * tabNOMEOBJ;  ///< Usado para converter nomes em nomeobj

/** Monta string.
 *  Semelhante a sprintf(), exceto que:
 *  - S� processa caracteres %%, \%c, \%d, \%u e \%s
 *  - \%S = mensagem como em \%s, mas sem espa�os finais
 *  - tamanho � o tamanho m�ximo do buffer destino
 *  .
 *  @param destino Endere�o destino
 *  @param tamanho Tamanho do buffer em destino
 *  @param mens    Mensagem formatada, como em snprintf()
 *  @note Usar no lugar de snprintf(), que n�o est� presente em alguns UNIXes */
char * mprintf(char * destino, int tamanho, const char * mens, ...);

/// Semelhante a strcpy(), mas retorna endere�o do byte =0 em destino */
char * copiastr(char * destino, const char * origem);

/// Semelhante a strcpy(), mas retorna endere�o do byte =0 em destino
/** @param destino Endere�o destino
 *  @param origem Endere�o origem
 *  @param tamanho Tamanho do buffer em destino */
char * copiastr(char * destino, const char * origem, int tamanho);

/// Semelhante a copiastr(), mas passa para letras min�sculas
/** @param destino Endere�o destino
 *  @param origem Endere�o origem
 *  @param tamanho Tamanho do buffer em destino */
char * copiastrmin(char * destino, const char * origem, int tamanho);

/// Compara dois nomes de classes, vari�veis ou fun��es (strings ASCIIZ)
/** @retval -2 string1<string2; string2 cont�m string1
 *  @retval -1 string1<string2
 *  @retval 0 Strings iguais
 *  @retval 1 se string1>string2
 *  @retval 2 se string1>string2; string1 cont�m string2 */
int comparaVar(const char * string1, const char * string2);

/// Compara duas strings ASCIIZ
/** @retval -2 string1<string2; string2 cont�m string1
 *  @retval -1 string1<string2
 *  @retval 0 Strings iguais
 *  @retval 1 se string1>string2
 *  @retval 2 se string1>string2; string1 cont�m string2 */
int comparaZ(const char * string1, const char * string2);

/// Compara duas strings terminadas em espa�o ou 0
/** @retval -2 string1<string2; string2 cont�m string1
 *  @retval -1 string1<string2
 *  @retval 0 Strings iguais
 *  @retval 1 se string1>string2
 *  @retval 2 se string1>string2; string1 cont�m string2 */
int compara(const char * string1, const char * string2);

/// Compara duas strings de tamanho fixo
/** @retval -1 string1<string2
 *  @retval 0 Strings iguais
 *  @retval 1 se string1>string2 */
int compara(const char * string1, const char * string2, int tam);

/// Verifica se nome de arquivo permitido e acerta barra normal/invertida
/** Considera nome inv�lido se n�o estiver no diret�rio do programa
 *  @param nome Nome do arquivo
 *  @param ext Como o nome do arquivo deve terminar
 *  @return true se nome v�lido ou false se inv�lido */
bool arqvalido(char * nome, const char * ext);

/// Verifica se nome de arquivo permitido e acerta barra normal/invertida
/** Considera nome inv�lido se n�o estiver no diret�rio do programa
 *  @param nome Nome do arquivo
 *  @return true se nome v�lido ou false se inv�lido */
bool arqvalido(char * nome);

/// Verifica se nome v�lido para apelido
/** @param nome1 Texto em ASCIIZ
 *  @retval 0 Apelido v�lido
 *  @retval 1 Muito pequeno (menos de 2 caracteres)
 *  @retval 2 Cont�m caracteres inv�lidos */
int verifNome(const char * nome1);

/// Verifica se nome v�lido para senha
/** @param nome1 Texto em ASCIIZ
 *  @retval 0 Senha v�lida
 *  @retval 1 Muito pequena (menos de 5 caracteres)
 *  @retval 2 Cont�m caracteres inv�lidos
 *  @retval 3 Cont�m s� letras ou s� n�meros */
int verifSenha(const char * nome1);

/// Codifica apelido para compara��o
/** Dois apelidos iguais ou parecidos produzem o mesmo texto
 *  @param destino Endere�o destino
 *  @param origem Endere�o origem
 *  @param tamanho Tamanho do buffer em destino
 *  @return Endere�o do byte =0 no fim do apelido */
char * txtNome(char * destino, const char * origem, int tamanho);

/// Codifica apelido, constitu�do s� de letras, para compara��o
/** O apelido codificado � sempre menor ou igual ao original
 *  @param nome Endere�o do apelido
 *  @param tamanho Tamanho do apelido em caracteres
 *  @return Endere�o de onde seria o byte =0 no final do nome
 *  @note Usado por txtNome() */
char * txtNomeLetras(char * nome, int tamanho);

/// Copia mensagem, como com copiastr(), mas filtrando a mensagem
/** @param destino Endere�o destino
 *  @param origem Endere�o origem
 *  @param tamanho Tamanho do buffer em destino */
char * txtFiltro(char * destino, const char * origem, int tamanho);

/// Obt�m as op��es para txtRemove abaixo
/** @param opcoes Texto com as op��es
 *  @return N�mero que corresponde �s op��es escolhidas */
int txtRemove(const char * opcoes);

/// Converte texto conforme as op��es
/** @param destino Aonde colocar o texto convertido
 *  @param origem Texto em ASCIIZ que ser� convertido
 *  @param tam Tamanho do buffer em destino
 *  @param opcoes Op��es obtidas com TxtRemove(text0)
 *  @return Ponteiro para o 0 no final do texto convertido */
char * txtRemove(char * destino, const char * origem, int tam, int opcoes);

/// Calcula o n�mero do dia a partir de uma data */
long numdata(const char *);

bool ClipboardMudar(const char * txt);
char * ClipboardLer();

int NumInt(const char * txt); ///< Converte n�mero para inteiro

unsigned short Num16(const char * x); ///< L� unsigned short de char[2]
unsigned int Num24(const char * x); ///< L� unsigned int de char[3]
unsigned int Num32(const char * x); ///< L� unsigned int de char[4]

/// Passa de double para int arredondando para o valor mais pr�ximo
/** - � necess�rio para passar valores de double para int.
 *  - Exemplo de convers�o que n�o d� certo apenas com cast para int:
 *  - double v1 = 63250036212.0;
 *  - printf("%d\n", (int)v1); */
int DoubleToInt(double valor);

//------------------------------------------------------------------------------
class TAddBufferBloco /// Usado em AddBuffer
{
public:
    TAddBufferBloco * Proximo; ///< Pr�ximo bloco
    char buf[4080]; ///< Bytes de instru��es
};

//------------------------------------------------------------------------------
class TAddBuffer /// Buffer expans�vel na mem�ria
{
public:
    TAddBuffer();
    ~TAddBuffer();
    void Add(const char * origem, int tamanho);
        ///< Adiciona bytes no buffer
    int Anotar(char * destino, int tamanho);
        ///< Anota dados adicionados no buffer
    void AnotarBuf();
        ///< Aloca mem�ria e coloca resultado em Buf
        /**< @note Quando o objeto for apagado, a mem�ria em Buf � desalocada */
    void Limpar();
        ///< Limpa bytes adicionados no buffer
    unsigned int Total; ///< Quantos bytes adicionados
    char * Buf;         ///< Buffer gerado por AnotarBuf()
private:
    unsigned int PosAtual;  ///< Quantos bytes usados do bloco atual
    TAddBufferBloco * Inicio; ///< Primeiro bloco
    TAddBufferBloco * Atual;  ///< Bloco atual
};

/** @} */

#endif
