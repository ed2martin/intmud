#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instr.h"
#include "classe.h"
#include "misc.h"

/*** Processamento de instru��es

1. Come�a com uma das palavras reservadas seguido de espa�o:
� defini��o de vari�vel

2. Come�a com const:
� constante

3. Come�a com herda:
Instru��o especial herda

4. Verifica instru��es de controle de fluxo

5. Nenhum dos anteriores:
Processa como express�o num�rica pura

*** Princ�pio - express�es

Uma express�o do tipo "1+2-3" � codificada da seguinte forma:
1 2 + 3 -

E guardada na mem�ria da seguinte forma:
ex_num1, ex_num8p, 2, exo_soma, ex_num8p, 3, exo_sub, ex_fim

Nesse caso, ao executar s�o realizadas as opera��es:
Coloca 1 na pilha
Coloca 2 na pilha
Soma os dois valores do topo da pilha (vai ficar 3 na pilha)
Coloca 3 na pilha
Subtrai os dois valores do topo da pilha (vai ficar 0 na pilha)

*** Vari�veis - express�es

Vari�vel modo:
Um valor qualquer de TExpressao

Vari�vel arg:
0 = espera operador unit�rio ou valor qualquer
1 = espera operador bin�rio

Pilha:
PUSH alguma coisa -> coloca no topo da pilha
POP alguma coisa -> tira do topo da pilha

*** Algoritmo - codifica de express�o:

arg=0
modo=0

(1)
// Verificando nome de vari�vel
Se modo = ex_var1 ou modo = ex_var2
  Se *origem for uma letra de A a Z ou um n�mero de 0 a 9:
    Anota *origem, avan�a origem
    Vai para (1)
  Se *origem for ':':
    Se modo ! ex_var1:
      Erro na express�o
    Caso contr�rio:
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

// Ignora espa�o entre operadores e operandos
Se *origem for espa�o:
  Avan�a at� encontrar algo diferente de espa�o

// Operando � texto
Se *origem for aspas duplas:
  Se arg=1:
    Erro na express�o
  arg=1
  Copia texto, avan�a at� encontrar aspas duplas novamente
  Vai para (1)

// Operando num�rico
Se *origem for um d�gito de 0 a 9:
  Se arg=1:
    Erro na express�o
  arg=1
  Anota como constante
  Vai para (1)

Se *origem for um h�fen seguido de um d�gito de 0 a 9:
  Se arg=0:
    arg=1
    Anota como constante
    Vai para (1)

// In�cio de nome de vari�vel
Se *origem for '$', '[' ou uma letra de A a Z:
  Se arg=1:
    Erro na express�o
  arg=1
  Se for "nulo" seguido de alguma coisa diferente de A-Z, 0-9 e [:
    Anota ex_nulo
    Avan�a origem para depois da palavra "nulo"
    Vai para (1)
  Se *origem for '[':
    Anota ex_varabre
    Avan�a origem
  Caso contr�rio:
    Anota ex_varini
    Anota *origem, avan�a origem
  PUSH modo
  modo = ex_var1
  Vai para (1)

// Par�nteses
Se *origem for abre par�nteses:
  Se arg=1
    Erro na express�o
  PUSH modo
  modo = ex_parenteses
  Vai para (1)

Se *origem for fecha par�nteses:
  Se arg=0
    Erro na express�o
  Enquanto modo for operador:
    Anota modo
    POP modo
  Se modo n�o for ex_parenteses:
    Erro na express�o
  POP modo
  // Par�nteses com o significado de par�metros de uma fun��o
  Se modo for ex_var1 ou ex_var2:
    Anota ex_ponto
    Se *origem for '.':
      Avan�a origem
    Caso contr�rio:
      Anota ex_varfim
      POP modo
  Vai para (1)

// Fecha colchetes - volta a processar nome de vari�vel
Se *origem for fecha colchetes:
  Se arg=0
    Erro na express�o
  Enquanto modo for operador:
    Anota modo
    POP modo
  Se modo n�o for ex_colchetes:
    Erro na express�o
  POP modo
  Anota ex_fecha
  Vai para (1)

// Verifica operadores e preced�ncia de operadores
Se for operador:
  Se for operador '-' e arg=0:
    � operador '-' unit�rio
  Se o tipo de operador (bin�rio ou unit�rio) n�o combinar com arg:
    Erro na express�o
  Enquanto modo for operador e operador encontrado tiver menos prioridade
              que a vari�vel modo:
    Anota modo
    POP modo
  PUSH modo
  modo=operador encontrado
  arg=0
  Vai para (1)

// Fim da senten�a
Se for \0:
  Enquanto modo for operador:
    Anota modo
    POP modo
  Se (modo=0 e arg=1):
    Acabou a string
  Caso contr�rio:
    Erro na express�o
  Fim

Mensagem de erro:
  Caracter inv�lido na express�o
Fim

***/

//------------------------------------------------------------------------------
class TListaInstr {
public:
    const char * Nome;
    const char Instr;
};

// Lista de instru��es, exceto cExpr
// Deve obrigatoriamente estar em ordem alfab�tica
static const TListaInstr ListaInstr[] = {
    { "const",     cConstExpr }, // Qualquer tipo de const
    { "continuar", cContinuar },
    { "efim",      cEFim },
    { "enquanto",  cEnquanto },
    { "fimSe",     cFimSe },
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
// Retorna um n�mero que corresponde � prioridade do operador
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
// Codifica uma instru��o
// Retorna: true = conseguiu codificar com sucesso
//          false = erro, destino cont�m a mensagem de erro
bool InstrCodif(char * destino, const char * origem, int tamanho)
{
    char * dest_ini = destino;
    char * dest_fim = destino + tamanho;

// Verifica tamanho m�nimo
    if (tamanho<10)
    {
        *dest_ini=0;
        return false;
    }

// Obt�m a vari�vel/instru��o em destino[2] a destino[4]
    destino[3]=0;
    destino[4]=0;
    while (true)
    {
    // Avan�a enquanto for espa�o
        while (*origem==' ')
            origem++;
    // Verifica atributos de vari�veis: comum e sav
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
    // Verifica se instru��o n�o � nula
        if (*origem==0)
        {
            copiastr(dest_ini, "Faltou a instru��o", tamanho);
            return false;
        }
    // Verifica instru��o txt1 a txt512
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
    // Obt�m a instru��o
        int ini = 0;
        int fim = sizeof(ListaInstr) / sizeof(ListaInstr[0]) - 1;
        while (true)
        {
            if (ini>fim) // N�o encontrou
            {
                destino[2] = cExpr;
                if (destino[3]==0)
                    break;
                // Tem atribui��o const ou sav: deveria ser tipo de vari�vel
                char mens[30];
                char * p = mens;
                copiastr(mens, origem, sizeof(mens));
                while (*p && *p!=' ')
                    p++;
                *p=0;
                mprintf(dest_ini, tamanho, "Vari�vel n�o existe: %s", mens);
                return false;
            }
            int meio = (ini+fim)/2;
            int r = compara(origem, ListaInstr[meio].Nome);
            if (r==0)
            {
                destino[2] = ListaInstr[meio].Instr;
                break;
            }
            if (r<0)
                fim = meio-1;
            else
                ini = meio+1;
        }
        break;
    }

// Instru��o herda
    if (destino[2] == cHerda)
    {
        destino[3] = 0;
        destino+=4;
        origem+=5; // Avan�a ap�s "herda"
    // Obt�m as classes
        while (*origem)
        {
            char nome[80];
            char * p = nome;
        // Avan�a enquanto for espa�o
            while (*origem==' ')
                origem++;
            if (*origem==0)
                break;
        // Copia o nome
            while (*origem && *origem!=',' && p<nome+sizeof(nome)-1)
                *p++ = *origem++;
        // Apaga espa�os no fim do nome
            while (p>nome)
            {
                if (p[-1]!=' ')
                    break;
                p--;
            }
        // Verifica se existe classe com o nome encontrado
            *p++=0;
            TClasse * obj = TClasse::Procura(nome);
            if (obj==0)
            {
                if (*nome)
                    mprintf(dest_ini, tamanho, "Classe n�o existe: %s", nome);
                else
                    copiastr(dest_ini, "Faltou o nome da classe", tamanho);
                return false;
            }
        // Copia o nome da classe
            if (destino+strlen(nome)+1 > dest_fim)
            {
                copiastr(dest_ini, "Instru��o muito grande", tamanho);
                return false;
            }
            destino = copiastr(destino, nome);
            destino++;
            dest_ini[3]++; // Aumenta o n�mero de classes
        // Pula a v�rgula
            if (*origem==',')
                origem++;
        }
    // Checa n�mero de classes encontradas
        if (dest_ini[3]==0)
        {
            copiastr(dest_ini, "Deve definir pelo menos uma classe", tamanho);
            return false;
        }
        if ((unsigned char)dest_ini[3]>20)
        {
            copiastr(dest_ini, "Deve definir no m�ximo 20 classes", tamanho);
            return false;
        }
        dest_ini[0] = (destino - dest_ini);
        dest_ini[1] = (destino - dest_ini) >> 8;
        return true;
    }


***********
        Falta:

Se destino[0] == cComum:
    � do tipo comum; deve verificar express�o num�rica

Se destino[0] > cVariavel:
    � vari�vel

Se destino[0] == cExpr:
    � express�o num�rica pura

Caso contr�rio:
    Deve ser instru��o de controle de fluxo
***********



    destino += 2;

// Processa express�o num�rica
    char pilha[512];
    char * topo = pilha;
    int arg=0, modo=0;
    *topo++ = 0;
    while (true)
    {
    // Verifica se tem espa�o suficiente
        if (destino >= dest_fim - 2)
        {
            copiastr(dest_ini, "Instru��o muito grande", tamanho);
            return false;
        }

    // Processando nome de vari�vel
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

    // Ignora espa�o entre operadores e operandos
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

    // N�mero
        if (*origem>='0' && *origem<='9' ||
             (arg==0 && *origem=='-' && origem[1]>='0' && origem[1]<='9'))
        {
            if (arg==1)
            {
                mprintf(dest_ini, tamanho, "Erro a partir de: %s", origem);
                return false;
            }
            arg=1;
            bool negativo=false; // Se � negativo
            int  virgula=0;      // Casas ap�s a v�rgula
            long long valor=0;   // Valor lido
        // Obt�m o n�mero (negativo, virgula, valor)
            if (*origem=='-')
                negativo=true, origem++;
            for (;; origem++)
            {
                if (*origem=='.')
                {
                    if (virgula)
                    {
                        copiastr(dest_ini, "N�meros n�o podem ter mais de um ponto", tamanho);
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
                    copiastr(dest_ini, "N�mero muito grande", tamanho);
                    return false;
                }
            }
            if (virgula)
                virgula--;
        // Anota o n�mero
            if (valor==0) // zero � zero
            {
                *destino++ = ex_num0;
                continue;
            }
            if (valor==0 && negativo==false) // Um
                *destino++ = ex_num1;
            else if (valor<0x100) // N�mero de 8 bits
            {
                if (destino >= dest_fim-2)
                {
                    destino = dest_fim;
                    continue;
                }
                *destino++ = (negativo ? ex_num8n : ex_num8p);
                *destino++ = valor;
            }
            else if (valor<0x10000) // N�mero de 16 bits
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
            else // N�mero de 32 bits
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

    // In�cio de nome de vari�vel
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
        // Verifica se come�a com colchetes
            if (*origem=='[')
                *destino++ = ex_varabre;
            else
                *destino++ = *origem;
            origem++;
            *topo++ = modo;
            modo = ex_var1;
            continue;
        }

    // Par�nteses
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
        // Anota operadores de menor preced�ncia
            while (modo>exo_ini && modo<exo_fim && destino < dest_ini-1)
            {
                *destino++ = modo;
                modo = *--topo;
            }
            if (destino >= dest_ini-1)
                continue;
        // Verifica se tinha abre par�nteses
            if (modo != ex_parenteses)
            {
                mprintf(dest_ini, tamanho, ") sem (");
                return false;
            }
            modo = *--topo;
        // Par�nteses como par�metro de fun��o
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
        // Anota operadores de menor preced�ncia
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

    // Fim da senten�a
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
            copiastr(dest_ini, "Erro na instru��o", tamanho);
            return false;
        }

    // Verifica operadores
        unsigned char sinal=0; // Operador
        bool bsinal = true; // Verdadeiro se operador bin�rio
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

    // Anota operadores que t�m mais prioridade sobre o operador encontrado
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
