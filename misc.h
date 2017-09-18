#ifndef MISC_H
#define MISC_H

//----------------------------------------------------------------------------
/** @defgroup misc_h Diversas funções */
/** @{ */

#define INT_EXT "int" ///< Extensão dos arquivos do programa interpretado
#define INT_NOME_TAM 0x200 ///< Tamanho máximo dos nomes dos arquivos ".int"

// Acima desse valor, double é mostrado com notação científica
#define DOUBLE_MAX 1000000000000000000.0

// Valor máximo de IntTempo é INTTEMPO_MAX*INTTEMPO_MAX-1
#define INTTEMPO_MAX 0x400

// Tabela de caracteres usada em comparaVar (para comparar variáveis)
#define TABELA_COMPARAVAR tabNOMES2

extern unsigned long TempoIni; ///< Tempo desde que o programa foi executado, 10=1seg
                         /**< @note Atualizado em main.cpp */

extern char * arqnome;   ///< Nome completo dos arquivos
extern char * arqinicio; ///< A partir de onde colocar os nomes dos arquivos
                         /** Tem espaço para pelo menos 0x200 caracteres. */
extern bool opcao_completo;///< Opção completo do arquivo int principal
extern bool opcao_log;     ///< Opção log do arquivo int principal

/// Prepara tabNOMES[] e tabCOMPLETO[]
/** Usado para transformação de caracteres.
 *  tabASCinic deve ser chamado no início do programa. */
void tabASCinic(void);

extern char * tabNOMES1;   ///< Apenas caracteres válidos em nomes de classes
extern char * tabNOMES2;   ///< Para comparar nomes de classes e funções
extern char * tabCOMPLETO; ///< Todos os caracteres
extern char * tabMAI;      ///< Todos os caracteres em letras maiúsculas
extern char * tabMIN;      ///< Todos os caracteres em letras minúsculas
extern char * tabMAIMIN;   ///< 1=letra minúscula, 2=maiúscula, 0=nenhum
extern char * tabTXTCOD;   ///< Usado para codificar caracteres: txtcod
extern char * tabTXTDEC;   ///< Usado para decodificar caracteres: txtdec
extern char * tab8B;       ///< txtremove sem filtro de letras acentuadas
extern char * tab7B;       ///< txtremove com filtro de letras acentuadas
extern char * tabTXTSEPARA;///< Usado em txtsepara
extern char * tabNOMEOBJ;  ///< Usado para converter nomes em nomeobj

int safe_read(int filedes, void *buffer, int size);
int safe_write(int filedes, const void *buffer, int size);

/** Monta string.
 *  Semelhante a sprintf(), exceto que:
 *  - Só processa caracteres %%, \%c, \%d, \%u e \%s
 *  - \%S = mensagem como em \%s, mas sem espaços finais
 *  - tamanho é o tamanho máximo do buffer destino
 *  .
 *  @param destino Endereço destino
 *  @param tamanho Tamanho do buffer em destino
 *  @param mens    Mensagem formatada, como em snprintf()
 *  @note Usar no lugar de snprintf(), que não está presente em alguns UNIXes */
char * mprintf(char * destino, int tamanho, const char * mens, ...);

/// Semelhante a strcpy(), mas retorna endereço do byte =0 em destino */
char * copiastr(char * destino, const char * origem);

/// Semelhante a strcpy(), mas retorna endereço do byte =0 em destino
/** @param destino Endereço destino
 *  @param origem Endereço origem
 *  @param tamanho Tamanho do buffer em destino */
char * copiastr(char * destino, const char * origem, int tamanho);

/// Semelhante a copiastr(), mas passa para letras minúsculas
/** @param destino Endereço destino
 *  @param origem Endereço origem
 *  @param tamanho Tamanho do buffer em destino */
char * copiastrmin(char * destino, const char * origem, int tamanho);

/// Compara dois nomes de classes, variáveis ou funções (strings ASCIIZ)
/** @retval -2 string1<string2; string2 contém string1
 *  @retval -1 string1<string2
 *  @retval 0 Strings iguais
 *  @retval 1 se string1>string2
 *  @retval 2 se string1>string2; string1 contém string2 */
int comparaVar(const char * string1, const char * string2);

/// Compara duas strings ASCIIZ
/** @retval -2 string1<string2; string2 contém string1
 *  @retval -1 string1<string2
 *  @retval 0 Strings iguais
 *  @retval 1 se string1>string2
 *  @retval 2 se string1>string2; string1 contém string2 */
int comparaZ(const char * string1, const char * string2);

/// Compara duas strings terminadas em espaço ou 0
/** @retval -2 string1<string2; string2 contém string1
 *  @retval -1 string1<string2
 *  @retval 0 Strings iguais
 *  @retval 1 se string1>string2
 *  @retval 2 se string1>string2; string1 contém string2 */
int compara(const char * string1, const char * string2);

/// Compara duas strings de tamanho fixo
/** @retval -1 string1<string2
 *  @retval 0 Strings iguais
 *  @retval 1 se string1>string2 */
int compara(const char * string1, const char * string2, int tam);

/// Verifica se nome de arquivo permitido e acerta barra normal/invertida
/** Considera nome inválido se não estiver no diretório do programa
 *  @param nome Nome do arquivo
 *  @return true se nome válido ou false se inválido
 *  @note Se opcao_completo for diferente de 0, sempre retorna true */
bool arqvalido(char * nome);

/// Verifica se nome de arquivo permitido e acerta barra normal/invertida
/** Checa se está no diretório do programa e o final do nome do arquivo
 *  @param nome Nome do arquivo
 *  @param somenteleitura Se vai apenas ler o arquivo
 *  @return true se nome válido ou false se inválido
 *  @note Se opcao_completo for diferente de 0, sempre retorna true */
bool arqvalido(char * nome, bool somenteleitura);

/// Verifica se nome válido para apelido
/** @param nome1 Texto em ASCIIZ
 *  @retval 0 Apelido válido
 *  @retval 1 Muito pequeno (menos de 2 caracteres)
 *  @retval 2 Contém caracteres inválidos */
int verifNome(const char * nome1);

/// Verifica se nome válido para senha
/** @param nome1 Texto em ASCIIZ
 *  @retval 0 Senha válida
 *  @retval 1 Muito pequena (menos de 5 caracteres)
 *  @retval 2 Contém caracteres inválidos
 *  @retval 3 Contém só letras ou só números */
int verifSenha(const char * nome1);

/// Codifica apelido para comparação
/** Dois apelidos iguais ou parecidos produzem o mesmo texto
 *  @param destino Endereço destino
 *  @param origem Endereço origem
 *  @param tamanho Tamanho do buffer em destino
 *  @return Endereço do byte =0 no fim do apelido */
char * txtNome(char * destino, const char * origem, int tamanho);

/// Codifica apelido, constituído só de letras, para comparação
/** O apelido codificado é sempre menor ou igual ao original
 *  @param nome Endereço do apelido
 *  @param tamanho Tamanho do apelido em caracteres
 *  @return Endereço de onde seria o byte =0 no final do nome
 *  @note Usado por txtNome() */
char * txtNomeLetras(char * nome, int tamanho);

/// Copia mensagem, como com copiastr(), mas filtrando a mensagem
/** @param destino Endereço destino
 *  @param origem Endereço origem
 *  @param tamanho Tamanho do buffer em destino */
char * txtFiltro(char * destino, const char * origem, int tamanho);

/// Obtém as opções para txtRemove abaixo
/** @param opcoes Texto com as opções
 *  @return Número que corresponde às opções escolhidas */
int txtRemove(const char * opcoes);

/// Converte texto conforme as opções
/** @param destino Aonde colocar o texto convertido
 *  @param origem Texto em ASCIIZ que será convertido
 *  @param tam Tamanho do buffer em destino
 *  @param opcoes Opções obtidas com TxtRemove(text0)
 *  @return Ponteiro para o 0 no final do texto convertido */
char * txtRemove(char * destino, const char * origem, int tam, int opcoes);

/// Calcula o número do dia a partir de uma data */
long numdata(const char *);

bool ClipboardMudar(const char * txt);
char * ClipboardLer();

///< Converte texto para número inteiro
int TxtToInt(const char * txt);

///< Converte texto para double
double TxtToDouble(const char * txt);

///< Converte double em texto
/**< @param txt char[80] aonde será colocado o número
 *   @param valor Número */
void DoubleToTxt(char * txt, double valor);

/// Passa de double para int arredondando para o valor mais próximo
/** - É necessário para passar valores de double para int.
 *  - Exemplo de conversão que não dá certo apenas com cast para int:
 *  - double v1 = 63250036212.0;
 *  - printf("%d\n", (int)v1); */
int DoubleToInt(double valor);

unsigned short Num16(const char * x); ///< Lê unsigned short de char[2]
unsigned int Num24(const char * x); ///< Lê unsigned int de char[3]
unsigned int Num32(const char * x); ///< Lê unsigned int de char[4]

//------------------------------------------------------------------------------
class TAddBufferBloco /// Usado em AddBuffer
{
public:
    TAddBufferBloco * Proximo; ///< Próximo bloco
    char buf[4080]; ///< Bytes de instruções
};

//------------------------------------------------------------------------------
class TAddBuffer /// Buffer expansível na memória
{
public:
    TAddBuffer();
    ~TAddBuffer();
    void Add(const char * origem, int tamanho);
        ///< Adiciona bytes no buffer
    int Anotar(char * destino, int tamanho);
        ///< Anota dados adicionados no buffer
    void AnotarBuf();
        ///< Aloca memória e coloca resultado em Buf
        /**< @note Quando o objeto for apagado, a memória em Buf é desalocada */
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
