#ifndef MISC_H
#define MISC_H

extern char * arqnome; // Nome completo dos arquivos
extern char * arqinicio; // Nome dos arquivos sem o diret�rio
extern char * arqext;  // A partir de onde colocar a extens�o
                       // Tem espa�o para pelo menos 64 caracteres

        // Para transforma��o de caracteres
        // tabASCinic deve ser chamado no in�cio do programa
extern char tabNOMES[];
extern char tabCOMPLETO[];
void tabASCinic(void);

        // Monta string
        // Usar no lugar de snprintf(), que n�o est� presente em alguns UNIXes
char * mprintf(char * destino, int tamanho, const char * mens, ...);

        // Semelhante a strcpy(), mas retorna endere�o do byte =0 em destino
char * copiastr(char * destino, const char * origem);

        // Semelhante a strcpy(), mas retorna endere�o do byte =0 em destino
        // tamanho = n�mero de caracteres que pode escrever em destino
        // tamanho-1 = n�mero de caraceres que origem deve ter,
        //             se origem n�o for uma string em ASCIIZ
char * copiastr(char * destino, const char * origem, int tamanho);

        // Compara duas strings ASCIIZ
        // Retorna: 0=strings iguais  -1=string1<string2  1=string1>string2
int comparaZ(const char * string1, const char * string2);

        // Compara duas strings terminadas em espa�o ou 0
        // Retorna: 0=strings iguais  -1=string1<string2  1=string1>string2
int compara(const char * string1, const char * string2);

        // Codifica senha
void gerasenha(const char * senha, unsigned long codif[5]);

        // Verifica e/ou anota senha vazia
        // Entrada/Retorno: verdadeiro=senha vazia (sem senha)
bool senhavazia(bool limpar, unsigned long codif[5]);

        // Verifica se nome v�lido para apelido
        // Entrada: nome em ASCIIZ (termina com 0 ou ' ')
bool verifNome(const char * nome1);

        // Calcula o n�mero do dia a partir de uma data
long numdata(const char *);

unsigned short Num16(const char * x);
unsigned int Num24(const char * x);
unsigned int Num32(const char * x);

#endif
