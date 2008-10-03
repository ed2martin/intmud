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
#include "instr.h"
#include "classe.h"
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
    { "int16",     cInt16 },
    { "int32",     cInt32 },
    { "int8",      cInt8 },
    { "intb0",     cIntb0 }, // Pode ser cIntb0 a cIntb7
    { "intdec",    cIntDec },
    { "intinc",    cIntInc },
    { "listamsg",  cListaMsg },
    { "listaobj",  cListaObj },
    { "listatxt",  cListaTxt },
    { "log",       cLog },
    { "prog",      cProg },
    { "real",      cReal },
    { "ref",       cRef },
    { "ret",       cRet1 }, // Pode ser cRet1 ou cRet2
    { "sair",      cSair },
    { "salvar",    cSalvar },
    { "se",        cSe },
    { "Senao",     cSenao1 }, // Pode ser cSenao1 ou cSenao2
    { "serv",      cServ },
    { "socket",    cSocket },
    { "terminar",  cTerminar },
    { "uint16",    cUInt16 },
    { "uint32",    cUInt32 },
    { "uint8",     cUInt8 },
    { "vartempo",  cVarTempo }
};

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
bool Instr::Codif(char * destino, const char * origem, int tamanho)
{
    char * dest_ini = destino;
    char * dest_fim = destino + tamanho;

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

// Obtém a variável/instrução em destino[2] a destino[4]
    destino[3]=0;
    destino[4]=0;
    while (true)
    {
    // Avança enquanto for espaço
        while (*origem==' ')
            origem++;
    // Verifica atributos de variáveis: comum e sav
        if (compara(origem, "comum")==0)
        {
            destino[3] |= 1;
            origem += 5;
            continue;
        }
        if (compara(origem, "sav")==0)
        {
            destino[3] |= 2;
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
            if (valor<=256) // 1 a 256
            {
                destino[2] = cTxt1;
                destino[4] = valor-1;
                origem = p;
                break;
            }
            if (valor<=512) // 257 a 512
            {
                destino[2] = cTxt2;
                destino[4] = valor-257;
                origem = p;
                break;
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
                if (destino[3]==0)
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
            int r = compara(origem, ListaInstr[meio].Nome);
            if (r==0)  // Se encontrou
            {
                destino[2] = ListaInstr[meio].Instr;
                if (destino[3] && destino[2]<cVariaveis)
                {
                    copiastr(dest_ini, "Atribuições comum e sav só podem "
                                    "ser usadas em variáveis", tamanho);
                    return false;
                }
                while (*origem && *origem!=' ')
                    origem++;
                break;
            }
            if (r<0)
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
        if (dest_ini[3]==0)
        {
            copiastr(dest_ini, "Deve definir pelo menos uma classe", tamanho);
            return false;
        }
        if ((unsigned char)dest_ini[3]>20)
        {
            copiastr(dest_ini, "Deve definir no máximo 20 classes", tamanho);
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
    // Avança enquanto for espaço
        while (*origem==' ')
            origem++;
    // Copia o nome
        for (; *origem && *origem!='='; origem++)
        {
            if (p < nome+sizeof(nome)-1)
            {
                *p++ = *origem;
                continue;
            }
            else if (*p==' ')
                continue;
            copiastr(dest_ini, "Nome de variável muito grande", tamanho);
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
        if (destino+strlen(nome)+6 > dest_fim)
        {
            copiastr(dest_ini, "Instrução muito grande", tamanho);
            return false;
        }
        destino = copiastr(destino+5, nome);
        destino++;
        dest_ini[4] = destino - dest_ini;
    // Verifica se não é variável Const
        if (destino[2] != cConstExpr)
        {
            if (*origem)
            {
                copiastr(dest_ini,
                         "Variável não permite atribuição de valor", tamanho);
                return false;
            }
            dest_ini[0] = (destino - dest_ini);
            dest_ini[1] = (destino - dest_ini) >> 8;
            return true;
        }
    // Variável Const
        if (*origem==0)
        {
            copiastr(dest_ini, "Faltou atribuir um valor à variável const",
                     tamanho);
            return false;
        }
        origem++;
    }

// Verifica instruções de controle de fluxo
    switch (destino[2])
    {
    case cSe:       // Requerem expressão numérica
    case cEnquanto:
        destino += 5;
        break;
    case cSenao1:   // Pode ter ou não expressão numérica
        if (*origem==0)
        {
            dest_ini[0]=3;
            dest_ini[1]=0;
            return true;
        }
        destino[2] = cSenao2;
        destino += 5;
        break;
    case cRet1:     // Pode ter ou não expressão numérica
        if (*origem==0)
        {
            dest_ini[0]=3;
            dest_ini[1]=0;
            return true;
        }
        destino[2] = cRet2;
        destino += 3;
        break;
    case cEFim:     // Dois bytes de dados
    case cSair:
    case cContinuar:
        dest_ini[0]=5;
        dest_ini[1]=0;
        return true;
    case cFimSe:    // Nenhum byte de dados
    case cTerminar:
        dest_ini[0]=3;
        dest_ini[1]=0;
        return true;
    case cExpr:  // Expressão numérica pura
        destino += 3;
        break;
    case cConstExpr: // Variável Const - já foi verificado acima
        break;
    default: // Nenhum dos anteriores; provavelmente nunca chegará aqui
        copiastr(dest_ini, "Instrução desconhecida", tamanho);
        return false;
    }

// Processa expressão numérica
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
            while (modo>exo_ini && modo<exo_fim && destino < dest_fim-1)
            {
                *destino++ = modo;
                modo = *--topo;
            }
            if (destino >= dest_fim-1)
                continue;
        // Verifica se tinha abre parênteses
            if (modo != ex_parenteses)
            {
                mprintf(dest_fim, tamanho, ") sem (");
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
            if (arg==0)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
        // Anota operadores de menor precedência
            while (modo>exo_ini && modo<exo_fim && destino < dest_fim-1)
            {
                *destino++ = modo;
                modo = *--topo;
            }
            if (destino >= dest_fim-1)
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
        if (*origem==0 || *origem=='#')
        {
            while (modo>exo_ini && modo<exo_fim && destino < dest_fim-1)
            {
                *destino++ = modo;
                modo = *--topo;
            }
            if (destino >= dest_fim-1)
                continue;
            if (modo!=0 || arg!=1)
            {
                copiastr(dest_ini, "Erro na instrução", tamanho);
                return false;
            }
        // Verifica se tem comentário
            if (*origem=='#')
            {
                *destino++ = ex_coment;
                origem++;
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
            while (*p)
            {
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
                    p++;
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
               destino < dest_fim-1)
        {
            *destino++ = modo;
            modo = *--topo;
        }
        if (destino >= dest_fim-1)
            continue;
        *topo++ = modo;
        modo = sinal;
        arg=0;
    }
}
