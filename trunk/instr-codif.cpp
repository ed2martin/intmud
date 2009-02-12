/* Este programa é software livre; você pode redistribuir e/ou
 * modificar nos termos da GNU General Public License V2
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details at www.gnu.org
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "instr.h"
#include "classe.h"
#include "misc.h"

/** @defgroup codif Instr::Codif - Algoritmo para codificar instruções

@par Princípio

1. Começa com uma das palavras reservadas seguido de espaço:
   - É definição de variável

2. Começa com const:
   - É constante

3. Começa com herda:
   - Instrução especial herda

4. Verifica instruções de controle de fluxo

5. Nenhum dos anteriores:
   - Processa como expressão numérica pura

@par Princípio - expressões

Uma expressão do tipo "1+2-3" é codificada da seguinte forma:\n
1 2 + 3 -

E guardada na memória da seguinte forma:
- Instr::ex_num1
- Instr::ex_num8p, 0x02
- Instr::exo_add
- Instr::ex_num8p, 0x03
- Instr::exo_sub
- Instr::ex_fim

Nesse caso, ao executar são realizadas as operações:
- Coloca 1 na pilha
- Coloca 2 na pilha
- Soma os dois valores do topo da pilha (vai ficar 3 na pilha)
- Coloca 3 na pilha
- Subtrai os dois valores do topo da pilha (vai ficar 0 na pilha)

@par Algoritmo - codifica de expressão

@verbatim
Variável modo
    Um valor qualquer de Instr::Expressao

Variável arg
    falso = espera operador unitário ou valor qualquer
    verdadeiro = espera operador binário

Pilha de variáveis
    PUSH alguma coisa -> coloca no topo da pilha
    POP alguma coisa -> tira do topo da pilha

arg=falso
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
      Anota ex_doispontos
      modo = ex_var2
      Vai para (1)
  Se *origem for '.':
    Anota ex_ponto
    modo = ex_var2
    Vai para (1)
  Se *origem for '[':
    Anota ex_abre
    arg=falso
    PUSH modo
    modo = ex_colchetes
    Vai para (1)
  Se *origem for '(':
    arg=falso
    PUSH modo
    modo = ex_parenteses
    Vai para (1)
  POP modo
  Anota modo
  arg=verdadeiro
  Vai para (1)

// Ignora espaço entre operadores e operandos
Se *origem for espaço:
  Avança até encontrar algo diferente de espaço

// Operando é texto
Se *origem for aspas duplas:
  Se arg=verdadeiro:
    Erro na expressão
  arg=verdadeiro
  Copia texto, avança até encontrar aspas duplas novamente
  Vai para (1)

// Operando numérico
Se *origem for um dígito de 0 a 9:
  Se arg=verdadeiro:
    Erro na expressão
  arg=verdadeiro
  Anota como constante
  Vai para (1)

Se *origem for um hífen seguido de um dígito de 0 a 9:
  Se arg=falso:
    arg=verdadeiro
    Anota como constante
    Vai para (1)

// Início de nome de variável
Se *origem for '$', '[' ou uma letra de A a Z:
  Se arg=verdadeiro:
    Erro na expressão
  arg=verdadeiro
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
  Se arg=verdadeiro
    Erro na expressão
  PUSH modo
  modo = ex_parenteses
  Vai para (1)

Se *origem for fecha parênteses:
  Se arg=falso
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
  Se arg=falso
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
  Se for operador '-' e arg=falso:
    É operador '-' unitário
  Se o tipo de operador (binário ou unitário) não combinar com arg:
    Erro na expressão
  Enquanto modo for operador e operador encontrado tiver menos prioridade
              que a variável modo:
    Anota modo
    POP modo
  PUSH modo
  modo=operador encontrado
  arg=falso
  Vai para (1)

// Fim da sentença
Se for \0:
  Enquanto modo for operador:
    Anota modo
    POP modo
  Se (modo=0 e arg=verdadeiro):
    Acabou a string
  Caso contrário:
    Erro na expressão
  Fim

Mensagem de erro:
  Caracter inválido na expressão
Fim
@endverbatim
*/

using namespace Instr;

//------------------------------------------------------------------------------
class TListaInstr {
public:
    const char * Nome;
    const char Instr;
};

// Lista de instruções, exceto cExpr
// Deve obrigatoriamente estar em ordem alfabética
static const TListaInstr ListaInstr[] = {
    { "const",     cConstExpr }, // Qualquer tipo de const
    { "continuar", cContinuar },
    { "efim",      cEFim },
    { "enquanto",  cEnquanto },
    { "fimse",     cFimSe },
    { "func",      cFunc },
    { "herda",     cHerda },
    { "indice",    cIndice },
    { "int1",      cInt1 },
    { "int16",     cInt16 },
    { "int32",     cInt32 },
    { "int8",      cInt8 },
    { "intdec",    cIntDec },
    { "intinc",    cIntInc },
    { "inttempo",  cIntTempo },
    { "listamsg",  cListaMsg },
    { "listaobj",  cListaObj },
    { "listatxt",  cListaTxt },
    { "log",       cLog },
    { "nomeobj",   cNomeObj },
    { "prog",      cProg },
    { "real",      cReal },
    { "ref",       cRef },
    { "ret",       cRet1 }, // Pode ser cRet1 ou cRet2
    { "sair",      cSair },
    { "salvar",    cSalvar },
    { "se",        cSe },
    { "senao",     cSenao1 }, // Pode ser cSenao1 ou cSenao2
    { "serv",      cServ },
    { "socket",    cSocket },
    { "terminar",  cTerminar },
    { "uint16",    cUInt16 },
    { "uint32",    cUInt32 },
    { "uint8",     cUInt8 },
    { "varfunc",   cVarFunc }
};

//------------------------------------------------------------------------------
static int comparaNome(const char * string1, const char * string2)
{
    for (;; string1++, string2++)
    {
        unsigned char ch1,ch2;
        ch1=(*string1==' ' ? 0 : tabNOMES[*(unsigned char *)string1]);
        ch2=(*string2==' ' ? 0 : tabNOMES[*(unsigned char *)string2]);
        if (ch1==0 || ch2==0)
            return (ch1 ? 1 : ch2 ? -1 : 0);
        if (ch1!=ch2)
            return (ch1<ch2 ? -1 : 1);
    }
}

//------------------------------------------------------------------------------
static char * anotaModo(char * destino, int modo)
{
    *destino++ = modo;
    switch (modo)
    {
    case exo_igualmul:
        *destino++ = exo_mul;
        *destino++ = exo_igual;
        break;
    case exo_igualdiv:
        *destino++ = exo_div;
        *destino++ = exo_igual;
        break;
    case exo_igualporcent:
        *destino++ = exo_porcent;
        *destino++ = exo_igual;
        break;
    case exo_igualadd:
        *destino++ = exo_add;
        *destino++ = exo_igual;
        break;
    case exo_igualsub:
        *destino++ = exo_sub;
        *destino++ = exo_igual;
        break;
    }
    return destino;
}

//------------------------------------------------------------------------------
/// Codifica uma instrução
/**
 *  @param destino Endereço destino (instrução codificada)
 *  @param origem  Endereço origem (string ASCIIZ)
 *  @param tamanho Tamanho do buffer em destino
 *  @retval true Conseguiu codificar com sucesso
 *  @retval false Erro, destino contém a mensagem de erro
 *  @sa codif
 */
bool Instr::Codif(char * destino, const char * origem, int tamanho)
{
    char * dest_ini = destino;
    char * dest_fim = destino + tamanho;
    bool proc_expr = false; // true = vai processar expressão numérica
                            // false = checar se tem comentário após instrução

// Verifica tamanho mínimo
    if (tamanho<10)
    {
        *dest_ini=0;
        return false;
    }

// Verifica se é comentário
    while (*origem==' ')
        origem++;
    if (*origem=='#')
    {
        destino[2] = cComent;
        for (origem++; *origem==' '; origem++);
        destino+=3;
        while (true)
        {
            if (destino >= dest_fim-1)
            {
                copiastr(dest_ini, "Instrução muito grande", tamanho);
                return false;
            }
            if (*origem==0)
                break;
            *destino++ = *origem++;
        }
        while (destino[-1]==' ')
            destino--;
        *destino++ = 0;
        dest_ini[0] = (destino - dest_ini);
        dest_ini[1] = (destino - dest_ini) >> 8;
        return true;
    }

// Obtém a variável/instrução em destino[2] a destino[5]
    destino[3]=0;
    destino[4]=0;
    destino[5]=0;
    while (true)
    {
    // Avança enquanto for espaço
        while (*origem==' ')
            origem++;
    // Verifica atributos de variáveis: comum e sav
        if (comparaNome(origem, "comum")==0)
        {
            destino[endProp] |= 1;
            origem += 5;
            continue;
        }
        if (comparaNome(origem, "sav")==0)
        {
            destino[endProp] |= 2;
            origem += 3;
            continue;
        }
    // Verifica se instrução não é nula
        if (*origem==0)
        {
            copiastr(dest_ini, "Faltou a instrução", tamanho);
            return false;
        }
    // Verifica instrução txt1 a txt512
        if ((origem[0]|0x20)=='t' && (origem[1]|0x20)=='x' &&
            (origem[2]|0x20)=='t' && origem[3]!='0')
        {
            int valor = 0;
            const char * p = origem+3;
            for (; *p>='0' && *p<='9'; p++)
            {
                valor = valor*10 + *p-'0';
                if (valor>0x1000)
                    break;
            }
            if (valor>=1 && comparaNome(p, "")==0)
            {
                if (valor<=256) // 1 a 256
                {
                    destino[2] = cTxt1;
                    destino[endIndice] = valor-1;
                    origem = p;
                    break;
                }
                if (valor<=512) // 257 a 512
                {
                    destino[2] = cTxt2;
                    destino[endIndice] = valor-257;
                    origem = p;
                    break;
                }
            }
        }
    // Obtém a instrução
        int ini = 0;
        int fim = sizeof(ListaInstr) / sizeof(ListaInstr[0]) - 1;
        while (true)
        {
            if (ini>fim) // Não encontrou
            {
                destino[2] = cExpr;
                if (destino[endProp]==0)
                    break;
                // Tem atribuição const ou sav: deveria ser tipo de variável
                char mens[30];
                char * p = mens;
                copiastr(mens, origem, sizeof(mens));
                while (*p && *p!=' ')
                    p++;
                *p=0;
                mprintf(dest_ini, tamanho, "Variável não existe: %s", mens);
                return false;
            }
            int meio = (ini+fim)/2;
            int resultado = comparaNome(origem, ListaInstr[meio].Nome);
        // Verifica se encontrou
            if (resultado==0)  // Se encontrou
            {
                origem += strlen(ListaInstr[meio].Nome);
                //while (*origem && *origem!=' ')
                //    origem++;
                destino[2] = ListaInstr[meio].Instr;
                if (destino[endProp] && (destino[2]<cVariaveis ||
                            destino[2]==cConstExpr ||
                            destino[2]==cFunc ||
                            destino[2]==cVarFunc))
                {
                    copiastr(dest_ini, "Atribuições comum e sav só podem "
                                    "ser usadas em variáveis", tamanho);
                    return false;
                }
                break;
            }
            if (resultado<0)
                fim = meio-1;
            else
                ini = meio+1;
        }
        break;
    }
    while (*origem==' ')
        origem++;

// Instrução Herda
    if (destino[2] == cHerda)
    {
        destino[3] = 0;
        destino+=4;
    // Obtém as classes
        TClasse * classes[20];
        unsigned int total = 0;
        while (*origem)
        {
            char nome[80];
            char * p = nome;
        // Avança enquanto for espaço
            while (*origem==' ')
                origem++;
            if (*origem==0)
                break;
        // Copia o nome
            for (; *origem && *origem!=','; origem++)
            {
                if (p < nome+sizeof(nome)-2)
                {
                    *p++ = *origem;
                    continue;
                }
                else if (*p==' ')
                    continue;
                p[-1]=0;
                mprintf(dest_ini, tamanho, "Classe não existe: %s", nome);
                return false;
            }
        // Apaga espaços no fim do nome
            while (p>nome)
            {
                if (p[-1]!=' ')
                    break;
                p--;
            }
            *p++=0;
        // Verifica se existe classe com o nome encontrado
            TClasse * obj = TClasse::Procura(nome);
            if (obj==0)
            {
                if (*nome)
                    mprintf(dest_ini, tamanho, "Classe não existe: %s", nome);
                else
                    copiastr(dest_ini, "Faltou o nome da classe", tamanho);
                return false;
            }
        // Verifica número de classes
            if (total >= sizeof(classes) / sizeof(classes[0]))
            {
                mprintf(dest_ini, tamanho,
                        "Deve definir no máximo %d classes",
                        sizeof(classes) / sizeof(classes[0]));
                return false;
            }
        // Verifica se classe repetida
            classes[total] = obj;
            for (unsigned int x=0; x<total; x++)
                if (classes[x] == obj)
                {
                    mprintf(dest_ini, tamanho, "Classe repetida: %s", nome);
                    return false;
                }
            total++;
        // Copia o nome da classe
            if (destino+strlen(obj->Nome)+1 > dest_fim)
            {
                copiastr(dest_ini, "Instrução muito grande", tamanho);
                return false;
            }
            destino = copiastr(destino, obj->Nome);
            destino++;
            dest_ini[3]++; // Aumenta o número de classes
        // Pula a vírgula
            if (*origem==',')
                origem++;
        }
    // Checa número de classes encontradas
        if (total==0)
        {
            copiastr(dest_ini, "Deve definir pelo menos uma classe", tamanho);
            return false;
        }
        dest_ini[0] = (destino - dest_ini);
        dest_ini[1] = (destino - dest_ini) >> 8;
        return true;
    }

// Processa variáveis
// Variáveis const: avança origem e destino
// Outras variáveis: acerta dados em destino e retorna
    if (destino[2] > cVariaveis)
    {
        char nome[80];
        char * p = nome;
        int  indice = -1;
    // Avança enquanto for espaço
        while (*origem==' ')
            origem++;
    // Copia o nome
        for (; *origem && *origem!='=' && *origem!='#'; origem++)
        {
            if (indice>=0)
            {
                if (*origem<'0' || *origem>'9')
                {
                    if (dest_ini[2] == cConstExpr)
                        copiastr(dest_ini, "Constantes não podem ser vetores",
                                 tamanho);
                    else
                        copiastr(dest_ini, "Tamanho inválido do vetor",
                                 tamanho);
                    return false;
                }
                indice = indice * 10 + *origem - '0';
                if (indice<0x100)
                    continue;
                copiastr(dest_ini, "Máximo 255 variáveis", tamanho);
                return false;
            }
            if (p < nome+sizeof(nome)-1)
            {
                if (*origem=='.')
                    indice = 0;
                else
                    *p++ = *origem;
                continue;
            }
            else if (*p==' ')
                continue;
            copiastr(dest_ini, "Nome de variável muito grande", tamanho);
            return false;
        }
        if (indice>=0)
        {
            dest_ini[endVetor] = indice;
            if (dest_ini[2] == cFunc || dest_ini[2] == cVarFunc)
            {
                copiastr(dest_ini, "Funções não podem ser vetores",
                                 tamanho);
                return false;
            }
        }
    // Apaga espaços no fim do nome
        while (p>nome)
        {
            if (p[-1]!=' ')
                break;
            p--;
        }
        *p++=0;
    // Verifica se nome válido
        if (*nome==0)
        {
            copiastr(dest_ini, "Faltou o nome da variável", tamanho);
            return false;
        }
        if (strlen(nome)>32)
        {
            copiastr(dest_ini, "Nome de variável muito grande", tamanho);
            return false;
        }
        for (const char * p=nome; *p; p++)
            if (tabNOMES[*(unsigned char*)p]==0)
            {
                copiastr(dest_ini, "Nome de variável inválido", tamanho);
                return false;
            }
    // Copia o nome da variável, avança destino
        if (destino+strlen(nome)+endNome+1 > dest_fim)
        {
            copiastr(dest_ini, "Instrução muito grande", tamanho);
            return false;
        }
        destino = copiastr(destino+endNome, nome);
        destino++;
    // Variável Const
        if (dest_ini[2] == cConstExpr)
        {
            if (*origem==0 || *origem=='#')
            {
                copiastr(dest_ini, "Faltou atribuir um valor à variável const",
                             tamanho);
                return false;
            }
            origem++;
            dest_ini[endIndice] = destino - dest_ini;
            proc_expr=true;
        }
    // Outros tipos de variáveis
        else if (*origem && *origem!='#')
        {
            copiastr(dest_ini,
                    "Variável não permite atribuição de valor", tamanho);
            return false;
        }
    }

// Verifica instruções de controle de fluxo
// Acerta destino e proc_expr
    switch (dest_ini[2])
    {
    case cSe:       // Requerem expressão numérica
    case cEnquanto:
        destino += 5;
        proc_expr=true;
        break;
    case cSenao1:   // Pode ter ou não expressão numérica
        if (*origem==0 || *origem=='#')
        {
            destino+=5;
            break;
        }
        destino[2] = cSenao2;
        destino += 5;
        proc_expr=true;
        break;
    case cRet1:     // Pode ter ou não expressão numérica
        if (*origem==0)
        {
            destino += 3;
            break;
        }
        destino[2] = cRet2;
        destino += 3;
        proc_expr=true;
        break;
    case cEFim:     // Dois bytes de dados
    case cSair:
    case cContinuar:
        destino += 5;
        break;
    case cFimSe:    // Nenhum byte de dados
    case cTerminar:
        destino += 3;
        break;
    case cExpr:  // Expressão numérica pura
        destino += 3;
        proc_expr=true;
        break;
    case cConstExpr: // Variável Const - já foi verificado acima
        break;
    default: // Nenhum dos anteriores
        if (dest_ini[2] > cVariaveis)
            break;
        copiastr(dest_ini, "Instrução desconhecida", tamanho);
        return false;
    }

// Anota comentário, se houver
    if (!proc_expr)
    {
        if (*origem=='#')
        {
            const char * final = destino;
            origem++;
            while (*origem==' ')
                origem++;
        // Verifica se tem espaço
            if (destino + strlen(origem) + 2 >= dest_fim)
            {
                copiastr(dest_ini, "Instrução muito grande", tamanho);
                return false;
            }
            destino = copiastr(destino, origem);
            while (destino>final && destino[-1]==' ')
                destino--;
        }
        dest_ini[0] = (destino - dest_ini);
        dest_ini[1] = (destino - dest_ini) >> 8;
        return true;
    }

// Processa expressão numérica
    char pilha[512];
    char * topo = pilha;
    int  modo=0;
    bool arg=false;
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
                origem++;
                *destino++ = ex_doispontos;
                modo = ex_var2;
            }
            else if (*origem=='.')
            {
                origem++;
                if (destino[-1] != ex_ponto)
                    *destino++ = ex_ponto;
                modo = ex_var2;
            }
            else if (*origem=='[')
            {
                origem++;
                *destino++ = ex_abre;
                arg=false;
                *topo++ = modo;
                modo = ex_colchetes;
            }
            else if (*origem=='(')
            {
                origem++;
                *destino++ = ex_arg;
                arg=false;
                *topo++ = modo;
                modo = ex_parenteses;
            }
            else
            {
                if (destino[-1] != ex_ponto)
                    *destino++ = ex_ponto;
                *destino++ = ex_varfim;
                modo = *--topo;
                arg=true;
            }
            continue;
        }

    // Ignora espaço entre operadores e operandos
        while (*origem == ' ')
            origem++;

    // Texto
        if (*origem=='\"')
        {
            if (arg)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            arg=true;
            *destino++ = ex_txt;
            for (origem++; destino<dest_fim-2; origem++)
            {
                if (*origem==0)
                {
                    copiastr(dest_ini, "Faltou fechar aspas", tamanho);
                    return false;
                }
                if (*origem=='\"')
                {
                    origem++;
                    while (*origem==' ')
                        origem++;
                    if (*origem=='\"')
                        continue;
                    if (*origem!='+')
                        break;
                    const char * p = origem+1;
                    while (*p==' ')
                        p++;
                    if (*p!='\"')
                        break;
                    origem=p;
                    continue;
                }
                if (*origem=='\\')
                {
                    origem++;
                    switch (*origem)
                    {
                    case 0:
                        copiastr(dest_ini, "Faltou fechar aspas", tamanho);
                        return false;
                    case 'N':
                    case 'n': *destino++ = ex_barra_n; break;
                    case 'B':
                    case 'b': *destino++ = ex_barra_b; break;
                    case 'C':
                    case 'c': *destino++ = ex_barra_c; break;
                    case 'D':
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
             (!arg && *origem=='-' && origem[1]>='0' && origem[1]<='9'))
        {
            if (arg)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            arg=true;
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
                        copiastr(dest_ini,
                             "Números não podem ter mais de um ponto", tamanho);
                        return false;
                    }
                    virgula=1;
                    continue;
                }
                if (*origem<'0' || *origem>'9')
                    break;
                if (virgula)
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
            if (valor==1 && negativo==false) // Um
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
                case 1:  *destino++ = ex_div1;  break;
                case 2:  *destino++ = ex_div2;  break;
                case 3:  *destino++ = ex_div3;  break;
                case 4:  *destino++ = ex_div4;  break;
                case 5:  *destino++ = ex_div5;  break;
                default: *destino++ = ex_div6;
                }
                virgula -= 6;
            }
            continue;
        }

    // Início de nome de variável
        if (*origem=='$' || *origem=='[' ||
                    tabNOMES[*(unsigned char*)origem]!=0)
        {
            if (arg)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            arg=true;
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
            *topo++ = modo;
            modo = ex_var1;
            if (*origem=='[')
            {
                *destino++ = ex_varabre;
                arg=false;
                *topo++ = modo;
                modo = ex_colchetes;
            }
            else
            {
                *destino++ = ex_varini;
                *destino++ = *origem;
            }
            origem++;
            continue;
        }

    // Parênteses
        if (*origem=='(')
        {
            origem++;
            if (arg)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            *topo++ = modo;
            modo = ex_parenteses;
            continue;
        }
        if (*origem==')')
        {
            origem++;
            if (!arg)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
        // Anota operadores de menor precedência
            while (modo>exo_ini && modo<exo_fim && destino < dest_fim-3)
            {
                destino = anotaModo(destino, modo);
                modo = *--topo;
            }
            if (destino >= dest_fim-1)
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
                else if (destino < dest_fim-1)
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
            origem++;
            if (!arg)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
        // Anota operadores de menor precedência
            while (modo>exo_ini && modo<exo_fim && destino < dest_fim-3)
            {
                destino = anotaModo(destino, modo);
                modo = *--topo;
            }
            if (destino >= dest_fim-1)
                continue;
        // Verifica se tinha abre colchetes
            if (modo != ex_colchetes)
            {
                mprintf(dest_ini, tamanho, "] sem [");
                return false;
            }
            modo = *--topo;
            *destino++ = ex_fecha;
            continue;
        }

    // Fim da sentença
        if (*origem==0 || *origem=='#')
        {
            while (modo>exo_ini && modo<exo_fim && destino < dest_fim-3)
            {
                destino = anotaModo(destino, modo);
                modo = *--topo;
            }
            if (destino >= dest_fim-1)
                continue;
            if (modo!=0 || !arg)
            {
                copiastr(dest_ini, "Erro na instrução", tamanho);
                return false;
            }
        // Verifica se tem comentário
            if (*origem=='#')
            {
                *destino++ = ex_coment;
                origem++;
                while (*origem==' ')
                    origem++;
                if (destino + strlen(origem) + 2 >= dest_fim)
                {
                    copiastr(dest_ini, "Instrução muito grande", tamanho);
                    return false;
                }
                destino = copiastr(destino, origem);
                while (destino[-1]==' ')
                    destino--;
            }
        // Marca o fim da instrução
            *destino++ = 0;
            dest_ini[0] = (destino - dest_ini);
            dest_ini[1] = (destino - dest_ini) >> 8;
        // Acerta variáveis const
            if (dest_ini[2] != cConstExpr)
                return true;
            const char * p = dest_ini + (unsigned char)dest_ini[4];
            int tipo = 0;
            while (true)
            {
                assert(p<destino);
                switch (*p)
                {
                case ex_fim:
                case ex_coment:
                    if (tipo)
                        dest_ini[2] = tipo;
                    return true;
                case ex_nulo:
                    if (tipo)
                        return true;
                    tipo = cConstNulo;
                    p++;
                    break;
                case ex_txt:
                    if (tipo)
                        return true;
                    tipo = cConstTxt;
                    while (*p++);
                    break;
                case ex_num32p:
                case ex_num32n:
                    p+=2;
                case ex_num16p:
                case ex_num16n:
                    p++;
                case ex_num8p:
                case ex_num8n:
                    p++;
                case ex_num0:
                case ex_num1:
                    p++;
                    if (tipo)
                        return true;
                    tipo = cConstNum;
                    break;
                case ex_div1:
                case ex_div2:
                case ex_div3:
                case ex_div4:
                case ex_div5:
                case ex_div6:
                    if (tipo!=cConstNum)
                        return true;
                    p++;
                    break;
                default:
                    return true;
                }
            }
        }

    // Verifica operadores
        unsigned char sinal=0; // Operador
        bool bsinal = true; // Verdadeiro se operador binário
        switch (*origem)
        {
        case ',': sinal=exo_virgula; break;

        case '!': if (origem[1]=='=')
                      sinal=exo_diferente,origem++;
                  else
                      sinal=exo_exclamacao,bsinal=false;
                  break;

        case '*': sinal=exo_mul;
                  if (origem[1]=='=')
                      sinal=exo_igualmul,origem++;
                  break;
        case '/': sinal=exo_div;
                  if (origem[1]=='=')
                      sinal=exo_igualdiv,origem++;
                  break;
        case '%': sinal=exo_porcent;
                  if (origem[1]=='=')
                      sinal=exo_igualporcent,origem++;
                  break;

        case '+': sinal=exo_add;
                  if (origem[1]=='=')
                      sinal=exo_igualadd,origem++;
                  break;
        case '-': if (origem[1]=='=')
                      sinal=exo_igualsub,origem++;
                  else if (arg)
                      sinal=exo_sub;
                  else
                      sinal=exo_neg,bsinal=false;
                  break;

        case '<': sinal=exo_menor;
                  if (origem[1]=='=')
                      sinal=exo_menorigual,origem++;
                  break;
        case '>': sinal=exo_maior;
                  if (origem[1]=='=')
                       sinal=exo_maiorigual,origem++;
                  break;

        case '=': sinal=exo_igual;
                  if (origem[1]=='=')
                      sinal=exo_igual2,origem++;
                  break;

        case '&': sinal=exo_e; break;
        case '|': sinal=exo_ou; break;

        default:
            mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
            return false;
        }
        origem++;

    // Verifica tipo de operador
        if (arg != bsinal)
        {
            mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
            return false;
        }

    // Anota operadores que têm mais prioridade sobre o operador encontrado
        int pri_sinal = Instr::Prioridade(sinal);
        while (modo>exo_ini && modo<exo_fim &&
               pri_sinal >= Instr::Prioridade(modo) &&
               destino < dest_fim-3)
        {
            destino = anotaModo(destino, modo);
            modo = *--topo;
        }
        if (destino >= dest_fim-2)
            continue;

        if (sinal==exo_e)
            *destino++ = exo_ee;
        if (sinal==exo_ou)
            *destino++ = exo_ouou;

        *topo++ = modo;
        modo = sinal;
        arg=false;
    }
}
