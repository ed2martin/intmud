#ifndef MISC_H
#define MISC_H

//----------------------------------------------------------------------------
/** @defgroup misc_h Diversas funções */
/** @{ */

#define INT_EXT "int" // Extensão dos arquivos do programa interpretado

extern unsigned long TempoIni; ///< Tempo desde que o programa foi executado, 10=1seg
                         /**< @note Atualizado em main.cpp */

extern char * arqnome;   ///< Nome completo dos arquivos
extern char * arqinicio; ///< Nome dos arquivos sem o diretório
extern char * arqext;    ///< A partir de onde colocar a extensão.
                         /** Tem espaço para pelo menos 64 caracteres. */

    /// Prepara tabNOMES[] e tabCOMPLETO[]
    /** Usado para transformação de caracteres.
        tabASCinic deve ser chamado no início do programa. */
void tabASCinic(void);

extern char * tabNOMES;    ///< Apenas caracteres válidos em nomes de classes
extern char * tabCOMPLETO; ///< Todos os caracteres
extern char * tabMAI;      ///< Todos os caracteres em letras maiúsculas
extern char * tabMIN;      ///< Todos os caracteres em letras minúsculas
extern char * tabTXTCOD;   ///< Usado para codificar caracteres: txtcod
extern char * tabTXTDEC;   ///< Usado para decodificar caracteres: txtdec
extern char * tab8B;       ///< txtremove sem filtro de letras acentuadas
extern char * tab7B;       ///< txtremove com filtro de letras acentuadas
extern char * tabTXTSEPARA;///< Usado em txtsepara

    /** Monta string.
        Semelhante a sprintf(), exceto que:
        - Só processa caracteres %%, \%c, \%d, \%u e \%s
        - \%S = mensagem como em \%s, mas sem espaços finais
        - tamanho é o tamanho máximo do buffer destino
        .
        @param destino Endereço destino
        @param tamanho Tamanho do buffer em destino
        @param mens    Mensagem formatada, como em snprintf()
        @note Usar no lugar de snprintf(), que não está presente em alguns UNIXes */
char * mprintf(char * destino, int tamanho, const char * mens, ...);

    /// Semelhante a memcpy()
void move_mem(void * destino, void * origem, unsigned int tamanho);

    /// Semelhante a strcpy(), mas retorna endereço do byte =0 em destino */
char * copiastr(char * destino, const char * origem);

    /// Semelhante a strcpy(), mas retorna endereço do byte =0 em destino
    /** @param destino Endereço destino
        @param origem Endereço origem
        @param tamanho Tamanho do buffer em destino */
char * copiastr(char * destino, const char * origem, int tamanho);

    /// Compara duas strings ASCIIZ
    /** @retval -2 string1<string2; string2 contém string1
        @retval -1 string1<string2
        @retval 0 Strings iguais
        @retval 1 se string1>string2
        @retval 2 se string1>string2; string1 contém string2 */
int comparaZ(const char * string1, const char * string2);

    /// Compara duas strings terminadas em espaço ou 0
    /** @retval -2 string1<string2; string2 contém string1
        @retval -1 string1<string2
        @retval 0 Strings iguais
        @retval 1 se string1>string2
        @retval 2 se string1>string2; string1 contém string2 */
int compara(const char * string1, const char * string2);

    /// Compara duas strings de tamanho fixo
    /** @retval -1 string1<string2
        @retval 0 Strings iguais
        @retval 1 se string1>string2 */
int compara(const char * string1, const char * string2, int tam);

    /// Verifica se nome de arquivo permitido e acerta barra normal/invertida
    /** Considera nome inválido se não estiver no diretório do programa
        @param nome Nome do arquivo
        @param ext Como o nome do arquivo deve terminar
        @return true se nome válido ou false se inválido */
bool arqvalido(char * nome, const char * ext);

    /// Verifica se nome de arquivo permitido e acerta barra normal/invertida
    /** Considera nome inválido se não estiver no diretório do programa
        @param nome Nome do arquivo
        @return true se nome válido ou false se inválido */
bool arqvalido(char * nome);

    /// Verifica se nome válido para apelido
    /** @param nome1 Texto em ASCIIZ
        @retval 0 Apelido válido
        @retval 1 Muito pequeno (menos de 2 caracteres)
        @retval 2 Contém caracteres inválidos */
int verifNome(const char * nome1);

    /// Verifica se nome válido para senha
    /** @param nome1 Texto em ASCIIZ
        @retval 0 Senha válida
        @retval 1 Muito pequena (menos de 5 caracteres)
        @retval 2 Contém caracteres inválidos
        @retval 3 Contém só letras ou só números */
int verifSenha(const char * nome1);

    /// Codifica apelido para comparação
    /** Dois apelidos iguais ou parecidos produzem o mesmo texto
        @param destino Endereço destino
        @param origem Endereço origem
        @param tamanho Tamanho do buffer em destino
        @return Endereço do byte =0 no fim do apelido */
char * txtNome(char * destino, const char * origem, int tamanho);

    /// Codifica apelido, constituído só de letras, para comparação
    /** O apelido codificado é sempre menor ou igual ao original
        @param nome Endereço do apelido
        @param tamanho Tamanho do apelido em caracteres
        @return Endereço de onde seria o byte =0 no final do nome
        @note Usado por txtNome() */
char * txtNomeLetras(char * nome, int tamanho);

    /// Copia mensagem, como com copiastr(), mas filtrando a mensagem
    /** @param destino Endereço destino
        @param origem Endereço origem
        @param tamanho Tamanho do buffer em destino */
char * txtFiltro(char * destino, const char * origem, int tamanho);

    /// Calcula o número do dia a partir de uma data */
long numdata(const char *);

unsigned short Num16(const char * x); ///< Lê unsigned short de char[2]
unsigned int Num24(const char * x); ///< Lê unsigned int de char[3]
unsigned int Num32(const char * x); ///< Lê unsigned int de char[4]

    /// Passa de double para int arredondando para o valor mais próximo
    /** - É necessário para passar valores de double para int.
     *  - Exemplo de conversão que não dá certo apenas com cast para int:
     *  - double v1 = 63250036212.0;
     *  - printf("%d\n", (int)v1); */
int DoubleToInt(double valor);

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
    char * Buf;         ///< Buffer gerado por AlocaMem()
private:
    unsigned int PosAtual;  ///< Quantos bytes usados do bloco atual
    TAddBufferBloco * Inicio; ///< Primeiro bloco
    TAddBufferBloco * Atual;  ///< Bloco atual
};

/** @} */

#endif
