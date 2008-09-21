#include "instr.h"
#include "misc.h"

/*** Processamento de instruções

1. Começa com uma das palavras reservadas seguido de espaço:
É definição de variável

2. Começa com const:
É constante

3. Começa com herda:
Instrução especial herda

4. Verifica instruções de controle de fluxo

5. Nenhum dos anteriores:
Processa como expressão numérica pura

*** Princípio - expressões

Uma expressão do tipo "1+2-3" é codificada da seguinte forma:
1 2 + 3 -

E guardada na memória da seguinte forma:
ex_num1, ex_num8p, 2, exo_soma, ex_num8p, 3, exo_sub, ex_fim

Nesse caso, ao executar são realizadas as operações:
Coloca 1 na pilha
Coloca 2 na pilha
Soma os dois valores do topo da pilha (vai ficar 3 na pilha)
Coloca 3 na pilha
Subtrai os dois valores do topo da pilha (vai ficar 0 na pilha)

*** Variáveis - expressões

Variável modo:
Um valor qualquer de TExpressao

Variável arg:
0 = espera operador unitário ou valor qualquer
1 = espera operador binário

Pilha:
PUSH alguma coisa -> coloca no topo da pilha
POP alguma coisa -> tira do topo da pilha

*** Algoritmo - codifica de expressão:

arg=0
modo=0

(1)
// Verificando nome de variável
Se modo = ex_var1 ou modo = ex_var2
  Se *origem for uma letra de A a Z ou um número de 0 a 9:
    Anota *origem, avança origem
    Vai para (1)
  Se *origem for ':':
    Se modo ! ex_var1:
      Erro na expressão
    Caso contrário:
      Anota ':'
      modo = ex_var2
      Vai para (1)
  Se *origem for '.':
    Anota ex_ponto
    modo = ex_var2
    Vai para (1)
  Se *origem for '[':
    Anota ex_abre
    arg=0
    PUSH modo
    modo = ex_colchetes
    Vai para (1)
  Se *origem for '(':
    arg=0
    PUSH modo
    modo = ex_parenteses
    Vai para (1)
  POP modo
  Anota modo
  arg=1
  Vai para (1)

// Ignora espaço entre operadores e operandos
Se *origem for espaço:
  Avança até encontrar algo diferente de espaço

// Operando é texto
Se *origem for aspas duplas:
  Se arg=1:
    Erro na expressão
  arg=1
  Copia texto, avança até encontrar aspas duplas novamente
  Vai para (1)

// Operando numérico
Se *origem for um dígito de 0 a 9:
  Se arg=1:
    Erro na expressão
  arg=1
  Anota como constante
  Vai para (1)

Se *origem for um hífen seguido de um dígito de 0 a 9:
  Se arg=0:
    arg=1
    Anota como constante
    Vai para (1)

// Início de nome de variável
Se *origem for '$', '[' ou uma letra de A a Z:
  Se arg=1:
    Erro na expressão
  arg=1
  Se for "nulo" seguido de alguma coisa diferente de A-Z, 0-9 e [:
    Anota ex_nulo
    Avança origem para depois da palavra "nulo"
    Vai para (1)
  Se *origem for '[':
    Anota ex_varabre
    Avança origem
  Caso contrário:
    Anota ex_varini
    Anota *origem, avança origem
  PUSH modo
  modo = ex_var1
  Vai para (1)

// Parênteses
Se *origem for abre parênteses:
  Se arg=1
    Erro na expressão
  PUSH modo
  modo = ex_parenteses
  Vai para (1)

Se *origem for fecha parênteses:
  Se arg=0
    Erro na expressão
  Enquanto modo for operador:
    Anota modo
    POP modo
  Se modo não for ex_parenteses:
    Erro na expressão
  POP modo
  // Parênteses com o significado de parâmetros de uma função
  Se modo for ex_var1 ou ex_var2:
    Anota ex_ponto
    Se *origem for '.':
      Avança origem
    Caso contrário:
      Anota ex_varfim
      POP modo
  Vai para (1)

// Fecha colchetes - volta a processar nome de variável
Se *origem for fecha colchetes:
  Se arg=0
    Erro na expressão
  Enquanto modo for operador:
    Anota modo
    POP modo
  Se modo não for ex_colchetes:
    Erro na expressão
  POP modo
  Anota ex_fecha
  Vai para (1)

// Verifica operadores e precedência de operadores
Se for operador:
  Se for operador '-' e arg=0:
    É operador '-' unitário
  Se o tipo de operador (binário ou unitário) não combinar com arg:
    Erro na expressão
  Enquanto modo for operador e operador encontrado tiver menos prioridade
              que a variável modo:
    Anota modo
    POP modo
  PUSH modo
  modo=operador encontrado
  arg=0
  Vai para (1)

// Fim da sentença
Se for \0:
  Enquanto modo for operador:
    Anota modo
    POP modo
  Se (modo=0 e arg=1):
    Acabou a string
  Caso contrário:
    Erro na expressão
  Fim

Mensagem de erro:
  Caracter inválido na expressão
Fim

***/

//------------------------------------------------------------------------------
// Retorna um número que corresponde à prioridade do operador
// Usado em InstrCodif
static int prioridade(int operador)
{
    switch (operador)
    {
    case exo_virgula:    return 1;
    case exo_neg:        return 2;
    case exo_exclamacao: return 2;
    case exo_mul:        return 3;
    case exo_div:        return 3;
    case exo_porcent:    return 3;
    case exo_add:        return 4;
    case exo_sub:        return 4;
    case exo_menor:      return 5;
    case exo_menorigual: return 5;
    case exo_maior:      return 5;
    case exo_maiorigual: return 5;
    case exo_igual:      return 6;
    case exo_diferente:  return 6;
    case exo_e:          return 7;
    case exo_ou:         return 8;
    }
    return 0;
}

//------------------------------------------------------------------------------
// Codifica uma instrução
// Retorna: true = conseguiu codificar com sucesso
//          false = erro, destino contém a mensagem de erro
bool InstrCodif(char * destino, const char * origem, int tamanho)
{
    char * dest_ini = destino;
    char * dest_fim = destino + tamanho;


    destino += 2;
    tamanho -= 2;



***********
        Falta:

1. Começa com uma das palavras reservadas seguido de espaço:
É definição de variável

2. Começa com const:
É constante

3. Começa com herda:
Instrução especial herda

4. Verifica instruções de controle de fluxo

5. Nenhum dos anteriores:
Processa como expressão numérica pura
***********



    char pilha[512];
    char * topo = pilha;
    int arg=0, modo=0;
    *topo++ = 0;
    while (true)
    {
    // Verifica se tem espaço suficiente
        if (destino >= dest_fim - 2)
        {
            copiastr(dest_ini, "Instrução muito grande", tamanho);
            return false;
        }

    // Processando nome de variável
        if (modo==ex_var1 || modo==ex_var2)
        {
            if (tabNOMES[*(unsigned char*)origem] && *origem!=' ')
                *destino++ = *origem++;
            else if (*origem==':')
            {
                if (modo!=ex_var1)
                {
                    mprintf(dest_ini, tamanho, "Dois pontos duas vezes");
                    return false;
                }
                *destino++ = *origem++;
                modo = ex_var2;
            }
            else if (*origem=='.')
            {
                *destino++ = ex_ponto;
                modo = ex_var2;
            }
            else if (*origem=='[')
            {
                *destino++ = ex_abre;
                arg=0;
                *topo++ = modo;
                modo = ex_colchetes;
            }
            else if (*origem=='(')
            {
                arg=0;
                *topo++ = modo;
                modo = ex_parenteses;
            }
            else
            {
                modo = *--topo;
                arg=1;
            }
            continue;
        }

    // Ignora espaço entre operadores e operandos
        while (*origem == ' ')
            origem++;

    // Texto
        if (*origem=='\"')
        {
            if (arg==1)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            arg=1;
            *destino++ = ex_txt;
            for (origem++; *origem!='\"' && destino<dest_fim-2; origem++)
            {
                if (*origem==0)
                {
                    copiastr(dest_ini, "Faltou fechar aspas", tamanho);
                    return false;
                }
                if (*origem=='\\')
                {
                    origem++;
                    switch (*origem)
                    {
                    case 0:
                        copiastr(dest_ini, "Faltou fechar aspas", tamanho);
                        return false;
                    case 'n': *destino++ = ex_barra_n; break;
                    case 'b': *destino++ = ex_barra_b; break;
                    case 'c': *destino++ = ex_barra_c; break;
                    case 'd': *destino++ = ex_barra_d; break;
                    default:  *destino++ = *origem;
                    }
                    continue;
                }
                *destino++ = *origem;
            }
            if (destino < dest_fim-1)
                *destino++ = 0;
            continue;
        }

    // Número
        if (*origem>='0' && *origem<='9' ||
             (arg==0 && *origem=='-' && origem[1]>='0' && origem[1]<='9'))
        {
            if (arg==1)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            arg=1;
            bool negativo=false; // Se é negativo
            int  virgula=0;      // Casas após a vírgula
            long long valor=0;   // Valor lido
        // Obtém o número (negativo, virgula, valor)
            if (*origem=='-')
                negativo=true, origem++;
            for (;; origem++)
            {
                if (*origem=='.')
                {
                    if (virgula)
                    {
                        copiastr(dest_ini, "Números não podem ter mais de um ponto", tamanho);
                        return false;
                    }
                    virgula=1;
                    continue;
                }
                if (*origem<'0' || *origem>'9')
                    break;
                virgula++;
                valor = (valor*10+*origem-'0');
                if (valor > 0xFFFFFFFFLL)
                {
                    copiastr(dest_ini, "Número muito grande", tamanho);
                    return false;
                }
            }
            if (virgula)
                virgula--;
        // Anota o número
            if (valor==0) // zero é zero
            {
                *destino++ = ex_num0;
                continue;
            }
            if (valor==0 && negativo==false) // Um
                *destino++ = ex_num1;
            else if (valor<0x100) // Número de 8 bits
            {
                if (destino >= dest_fim-2)
                {
                    destino = dest_fim;
                    continue;
                }
                *destino++ = (negativo ? ex_num8n : ex_num8p);
                *destino++ = valor;
            }
            else if (valor<0x10000) // Número de 16 bits
            {
                if (destino >= dest_fim-3)
                {
                    destino = dest_fim;
                    continue;
                }
                *destino++ = (negativo ? ex_num16n : ex_num16p);
                *destino++ = valor;
                *destino++ = valor >> 8;
            }
            else // Número de 32 bits
            {
                if (destino >= dest_fim-5)
                {
                    destino = dest_fim;
                    continue;
                }
                *destino++ = (negativo ? ex_num32n : ex_num32p);
                *destino++ = valor;
                *destino++ = valor >> 8;
                *destino++ = valor >> 16;
                *destino++ = valor >> 24;
            }
        // Anota as casas decimais
            while (virgula>0 && destino < dest_fim-1)
            {
                switch (virgula)
                {
                case 1:  *destino += ex_div1;  break;
                case 2:  *destino += ex_div2;  break;
                case 3:  *destino += ex_div3;  break;
                case 4:  *destino += ex_div4;  break;
                case 5:  *destino += ex_div5;  break;
                default: *destino += ex_div6;
                }
                virgula -= 6;
            }
            continue;
        }

    // Início de nome de variável
        if (*origem=='$' || *origem=='[' ||
                    tabNOMES[*(unsigned char*)origem]!=0)
        {
            if (arg==1)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            arg=1;
        // Verifica "nulo"
            if ((origem[0]|0x20)=='n' && (origem[1]|0x20)=='u' &&
                (origem[2]|0x20)=='l' && (origem[3]|0x20)=='o' &&
                 tabNOMES[*(unsigned char*)(origem+4)]==0 && origem[4]!='.')
            {
                *destino++ = ex_nulo;
                origem += 4;
                continue;
            }
        // Verifica se começa com colchetes
            if (*origem=='[')
                *destino++ = ex_varabre;
            else
                *destino++ = *origem;
            origem++;
            *topo++ = modo;
            modo = ex_var1;
            continue;
        }

    // Parênteses
        if (*origem=='(')
        {
            if (arg==1)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            arg=1;
            *topo++ = modo;
            modo = ex_parenteses;
            continue;
        }
        if (*origem==')')
        {
            if (arg==0)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
        // Anota operadores de menor precedência
            while (modo>exo_ini && modo<exo_fim && destino < dest_ini-1)
            {
                *destino++ = modo;
                modo = *--topo;
            }
            if (destino >= dest_ini-1)
                continue;
        // Verifica se tinha abre parênteses
            if (modo != ex_parenteses)
            {
                mprintf(dest_ini, tamanho, ") sem (");
                return false;
            }
            modo = *--topo;
        // Parênteses como parâmetro de função
            if (modo==ex_var1 || modo==ex_var2)
            {
                *destino++ = ex_ponto;
                if (*origem=='.')
                    origem++;
                else if (destino < dest_ini-1)
                {
                    *destino++ = ex_varfim;
                    modo = *--topo;
                }
            }
            continue;
        }

    // Fecha colchetes
        if (*origem==']')
        {
            if (arg==0)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
        // Anota operadores de menor precedência
            while (modo>exo_ini && modo<exo_fim && destino < dest_ini-1)
            {
                *destino++ = modo;
                modo = *--topo;
            }
            if (destino >= dest_ini-1)
                continue;
        // Verifica se tinha abre colchetes
            if (modo != ex_colchetes)
            {
                mprintf(dest_ini, tamanho, ") sem (");
                return false;
            }
            modo = *--topo;
            *destino++ = ex_fecha;
            continue;
        }

    // Fim da sentença
        if (*origem==0)
        {
        // Anota operadores pendentes
            while (modo>exo_ini && modo<exo_fim && destino < dest_ini-1)
            {
                *destino++ = modo;
                modo = *--topo;
            }
            if (destino >= dest_ini-1)
                continue;
            if (modo==0 && arg==1)
            {
                *destino++ = 0;
                dest_ini[0] = (destino - dest_ini);
                dest_ini[1] = (destino - dest_ini) >> 8;
                return true;
            }
            copiastr(dest_ini, "Erro na instrução", tamanho);
            return false;
        }

    // Verifica operadores
        unsigned char sinal=0; // Operador
        bool bsinal = true; // Verdadeiro se operador binário
        switch (*origem)
        {
        case ',': sinal=exo_virgula, bsinal=false; break;

        case '!': if (origem[1]=='=')
                      sinal=exo_diferente,origem++;
                  else
                      sinal=exo_exclamacao,bsinal=false;
                  break;

        case '*': sinal=exo_mul; break;
        case '/': sinal=exo_div; break;
        case '%': sinal=exo_porcent; break;

        case '+': sinal=exo_add; break;
        case '-': sinal=(arg ? exo_sub : exo_neg); break;

        case '<': sinal=exo_menor;
                  if (origem[1]=='=')
                      sinal=exo_menorigual,origem++;
                  break;
        case '>': sinal=exo_maior;
                  if (origem[1]=='=')
                       sinal=exo_maiorigual,origem++;
                  break;

        case '=': sinal=exo_igual; break;

        case '&': sinal=exo_e; break;
        case '|': sinal=exo_ou; break;

        default:
            mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
            return false;
        }

    // Verifica tipo de operador
        if (arg != bsinal)
        {
            mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
            return false;
        }

    // Anota operadores que têm mais prioridade sobre o operador encontrado
        int pri_sinal = prioridade(sinal);
        while (modo>exo_ini && modo<exo_fim &&
               pri_sinal >= prioridade(modo) &&
               destino < dest_ini-1)
        {
            *destino++ = modo;
            modo = *--topo;
        }
        if (destino >= dest_ini-1)
            continue;
        *topo++ = modo;
        modo = sinal;
        arg=0;
    }
}
