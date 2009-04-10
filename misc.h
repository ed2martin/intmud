#ifndef MISC_H
#define MISC_H

//----------------------------------------------------------------------------
/** @defgroup misc_h Diversas funções */
/** @{ */

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

extern char tabNOMES[];    ///< Apenas caracteres válidos em nomes de classes
extern char tabCOMPLETO[]; ///< Todos os caracteres

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

    /// Codifica senha
void gerasenha(const char * senha, unsigned long codif[5]);

    /// Verifica e/ou anota senha vazia
    /** @param limpar Se deve limpar a senha (sem senha)
        @param codif Senha codificada
        @retval true Senha vazia (sem senha)
        @retval false Senha não vazia */
bool senhavazia(bool limpar, unsigned long codif[5]);

    /// Verifica se nome válido para apelido
    /** @param nome1 nome em ASCIIZ (termina com 0 ou ' ')
        @retval true Nome válido
        @retval false Nome inválido */
bool verifNome(const char * nome1);

    /// Calcula o número do dia a partir de uma data */
long numdata(const char *);

unsigned short Num16(const char * x); ///< Lê unsigned short de char[2]
unsigned int Num24(const char * x); ///< Lê unsigned int de char[3]
unsigned int Num32(const char * x); ///< Lê unsigned int de char[4]

/** @} */

#endif
