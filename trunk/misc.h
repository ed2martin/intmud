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
extern char tabMAI[]; ///< Todos os caracteres em letras mai�sculas
extern char tabMIN[]; ///< Todos os caracteres em letras min�sculas

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

    /// Semelhante a memcpy()
void move_mem(void * destino, void * origem, unsigned int tamanho);

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

    /// Compara duas strings de tamanho fixo
    /** @retval -1 string1<string2
        @retval 0 Strings iguais
        @retval 1 se string1>string2 */
int compara(const char * string1, const char * string2, int tam);

    /// Codifica senha
void gerasenha(const char * senha, unsigned long codif[5]);

    /// Verifica e/ou anota senha vazia
    /** @param limpar Se deve limpar a senha (sem senha)
        @param codif Senha codificada
        @retval true Senha vazia (sem senha)
        @retval false Senha n�o vazia */
bool senhavazia(bool limpar, unsigned long codif[5]);

    /// Verifica se nome de arquivo permitido (est� no diret�rio do programa)
    /** @param nome Nome do arquivo
        @return true se nome v�lido ou false se inv�lido */
bool arqvalido(const char * nome);

    /// Verifica se nome v�lido para apelido
    /** @param nome1 Texto em ASCIIZ
        @retval 0 Apelido v�lido
        @retval 1 Muito pequeno (menos de 2 caracteres)
        @retval 2 Cont�m caracteres inv�lidos */
int verifNome(const char * nome1);

    /// Verifica se nome v�lido para senha
    /** @param nome1 Texto em ASCIIZ
        @retval 0 Senha v�lida
        @retval 1 Muito pequena (menos de 5 caracteres)
        @retval 2 Cont�m caracteres inv�lidos
        @retval 3 Cont�m s� letras ou s� n�meros */
int verifSenha(const char * nome1);

    /// Codifica apelido para compara��o
    /** Dois apelidos iguais ou parecidos produzem o mesmo texto
        @param destino Endere�o destino
        @param origem Endere�o origem
        @param tamanho Tamanho do buffer em destino
        @return Endere�o do byte =0 no fim do apelido */
char * txtNome(char * destino, const char * origem, int tamanho);

    /// Codifica apelido, constitu�do s� de letras, para compara��o
    /** O apelido codificado � sempre menor ou igual ao original
        @param nome Endere�o do apelido
        @param tamanho Tamanho do apelido em caracteres
        @return Endere�o de onde seria o byte =0 no final do nome
        @note Usado por txtNome() */
char * txtNomeLetras(char * nome, int tamanho);

    /// Copia mensagem, como com copiastr(), mas filtrando a mensagem
    /** @param destino Endere�o destino
        @param origem Endere�o origem
        @param tamanho Tamanho do buffer em destino */
char * txtFiltro(char * destino, const char * origem, int tamanho);

    /// Calcula o n�mero do dia a partir de uma data */
long numdata(const char *);

unsigned short Num16(const char * x); ///< L� unsigned short de char[2]
unsigned int Num24(const char * x); ///< L� unsigned int de char[3]
unsigned int Num32(const char * x); ///< L� unsigned int de char[4]

/** @} */

#endif
