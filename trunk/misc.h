#ifndef MISC_H
#define MISC_H

//----------------------------------------------------------------------------
/** @defgroup misc_h Diversas fun��es */
/** @{ */

extern unsigned long TempoIni; ///< Tempo desde que o programa foi executado, 10=1seg
                         /**< @note Atualizado em main.cpp */

extern char * arqnome;   ///< Nome completo dos arquivos
extern char * arqinicio; ///< Nome dos arquivos sem o diret�rio
extern char * arqext;    ///< A partir de onde colocar a extens�o.
                         /** Tem espa�o para pelo menos 64 caracteres. */

    /// Prepara tabNOMES[] e tabCOMPLETO[]
    /** Usado para transforma��o de caracteres.
        tabASCinic deve ser chamado no in�cio do programa. */
void tabASCinic(void);

extern char tabNOMES[];    ///< Apenas caracteres v�lidos em nomes de classes
extern char tabCOMPLETO[]; ///< Todos os caracteres

    /** Monta string.
        Semelhante a sprintf(), exceto que:
        - S� processa caracteres %%, \%c, \%d, \%u e \%s
        - \%S = mensagem como em \%s, mas sem espa�os finais
        - tamanho � o tamanho m�ximo do buffer destino
        .
        @param destino Endere�o destino
        @param tamanho Tamanho do buffer em destino
        @param mens    Mensagem formatada, como em snprintf()
        @note Usar no lugar de snprintf(), que n�o est� presente em alguns UNIXes */
char * mprintf(char * destino, int tamanho, const char * mens, ...);

    /// Semelhante a strcpy(), mas retorna endere�o do byte =0 em destino */
char * copiastr(char * destino, const char * origem);

    /// Semelhante a strcpy(), mas retorna endere�o do byte =0 em destino
    /** @param destino Endere�o destino
        @param origem Endere�o origem
        @param tamanho Tamanho do buffer em destino */
char * copiastr(char * destino, const char * origem, int tamanho);

    /// Compara duas strings ASCIIZ
    /** @retval -2 string1<string2; string2 cont�m string1
        @retval -1 string1<string2
        @retval 0 Strings iguais
        @retval 1 se string1>string2
        @retval 2 se string1>string2; string1 cont�m string2 */
int comparaZ(const char * string1, const char * string2);

    /// Compara duas strings terminadas em espa�o ou 0
    /** @retval -2 string1<string2; string2 cont�m string1
        @retval -1 string1<string2
        @retval 0 Strings iguais
        @retval 1 se string1>string2
        @retval 2 se string1>string2; string1 cont�m string2 */
int compara(const char * string1, const char * string2);

    /// Codifica senha
void gerasenha(const char * senha, unsigned long codif[5]);

    /// Verifica e/ou anota senha vazia
    /** @param limpar Se deve limpar a senha (sem senha)
        @param codif Senha codificada
        @retval true Senha vazia (sem senha)
        @retval false Senha n�o vazia */
bool senhavazia(bool limpar, unsigned long codif[5]);

    /// Verifica se nome v�lido para apelido
    /** @param nome1 nome em ASCIIZ (termina com 0 ou ' ')
        @retval true Nome v�lido
        @retval false Nome inv�lido */
bool verifNome(const char * nome1);

    /// Calcula o n�mero do dia a partir de uma data */
long numdata(const char *);

unsigned short Num16(const char * x); ///< L� unsigned short de char[2]
unsigned int Num24(const char * x); ///< L� unsigned int de char[3]
unsigned int Num32(const char * x); ///< L� unsigned int de char[4]

/** @} */

#endif
