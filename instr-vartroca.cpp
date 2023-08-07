/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos das licenças GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "instr.h"
#include "objeto.h"
#include "variavel.h"
#include "random.h"
#include "misc.h"

//----------------------------------------------------------------------------
// Retorna o caracter para a função vartroca
static inline int CaractTxtVar(const char * str)
{
    return TABELA_COMPARAVAR[*(unsigned char*)str];
}

//----------------------------------------------------------------------------
// Retorna o caracter para a função vartrocacod - do texto
static inline int CaractOrigem(const char * str)
{
    unsigned char ch = tabTXTCOD[*(unsigned char*)str];
    if (ch)
        return TABELA_COMPARAVAR['@'] * 0x100 + TABELA_COMPARAVAR[ch];
    return TABELA_COMPARAVAR[*(unsigned char*)str] * 0x100;
}

//----------------------------------------------------------------------------
// Retorna o caracter para a função vartrocacod - do nome da função
static inline int CaractFunc(const char * str)
{
    int ch = TABELA_COMPARAVAR[*(unsigned char*)str] * 0x100;
    if (str[0] != '@')
        return ch;
    return ch + TABELA_COMPARAVAR[*(unsigned char*)(str+1)];
}

//----------------------------------------------------------------------------
// Função vartroca se valor=0 ou vartrocacod se valor!=0
static inline bool StaticVarTroca(TVariavel * v, int valor)
{
    if (Instr::VarAtual < v + 3)
        return false;
    if (Instr::FuncAtual >= Instr::FuncFim - 2 ||
            Instr::FuncAtual->este == nullptr)
        return false;

// Variáveis
    TClasse * c = Instr::FuncAtual->este->Classe;
    const char * origem; // Primeiro argumento - texto original
    char mens[BUF_MENS]; // Aonde jogar o texto codificado
    int porcent = 100; // Porcentagem da possibilidade de troca
    int espaco = 0; // Quantas trocas deve ignorar após efetuar uma
    int espacocont = 0; // Contador de espaços entre trocas

// Obtém porcentagem e espaço entre trocas
    if (Instr::VarAtual >= v + 4)
    {
        porcent = v[4].getInt();
        if (porcent <= 0) // Nenhuma possibilidade de troca
        {
            char * p = copiastr(mens, v[1].getTxt(), sizeof(mens));
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto(mens, p-mens);
        }
        if (Instr::VarAtual >= v+5)
        {
            espaco = v[5].getInt();
            if (espaco < 0)
                espaco=0;
        }
    }

// Obtém argumento - padrão que deve procurar no texto
    char txtpadrao[40]; // Texto
    int  tampadrao = 0;   // Tamanho do texto sem o zero
    origem = v[2].getTxt();
    while (*origem && tampadrao < (int)sizeof(txtpadrao))
        txtpadrao[tampadrao++] = TABELA_COMPARAVAR[*(unsigned char*)origem++];
#if 0
    printf("Padrão = [");
    for (int x=0; x<tampadrao; x++)
        putchar(txtpadrao[x]);
    puts("]"); fflush(stdout);
#endif

// Obtém índices das variáveis que serão procuradas
    origem = v[3].getTxt();
    int tamvar = strlen(origem); // Tamanho do texto sem o zero
    int indini = 0;              // Índice inicial
    int indfim = c->NumVar - 1;    // Índice final
    if (tamvar)
    {
        indini = c->IndiceNomeIni(origem);
        indfim = c->IndiceNomeFim(origem);
    }

// Verifica se índices válidos
    if (indini < 0)   // Índice inválido
        indini = 1, indfim = 0;
    else if (tampadrao == 0) // Texto a procurar é nulo
        if (c->InstrVar[indini][tamvar + Instr::endNome] == 0)
                        // Se primeira variável é nula
            indini++; // Passa para próxima variável
#if 0
    printf("Variáveis(%d): ", tamvar);
    for (int x=indini; x<=indfim; x++)
        printf("[%s]", c->InstrVar[x]+Instr::endNome);
    putchar('\n'); fflush(stdout);
#endif

// Cabeçalho da instrução
    mens[2] = Instr::cConstExpr; // Tipo de instrução
    mens[Instr::endAlin] = 0;
    mens[Instr::endProp] = 0;
    mens[Instr::endIndice] = Instr::endNome + 2; // Aonde começam os dados da constante
    mens[Instr::endVetor] = 0; // Não é vetor
    mens[Instr::endExtra] = 0;
    mens[Instr::endNome] = '+'; // Nome da variável
    mens[Instr::endNome + 1] = 0;
    mens[Instr::endNome + 2] = Instr::ex_txt;
    char * destino = mens + Instr::endNome + 3;
    char * dest_ini = 0;// Endereço do início da variável ex_txt em destino
                        // 0=não anotou nenhuma variável

// Monta instrução
    tamvar += Instr::endNome;  // Para comparação
    origem = v[1].getTxt();
    while (*origem)
    {
        int i;
    // Verifica padrão
        for (i = 0; i < tampadrao; i++)
            if (TABELA_COMPARAVAR[(unsigned char)origem[i]] != txtpadrao[i])
                break;
    // Não achou - anota caracter
        if (i<tampadrao)
        {
            if (destino >= mens+sizeof(mens) - 4)
                break;
            *destino++ = *origem++;
            continue;
        }
     // Achou - procura variável
        int ini = indini; // Índice inicial
        int fim = indfim; // Índice final
        int indvar = -1; // Índice da variável ou <0 se não encontrou
        const char * pontvar = 0; // Variável origem para a variável em indvar

        const char * p1 = origem + tampadrao;
        for (int cont = tamvar;;)
        {
            int xini = -1, xfim = fim;
        // Obtém o caracter que está procurando
            int ch1 = (valor ? CaractOrigem(p1) : CaractTxtVar(p1));
            if (ch1 == 0) // Se for o fim do texto
                break;
        // Obtém: xini = primeira palavra com a letra
            while (ini<=fim)
            {
                int meio = (ini + fim) / 2;
                int ch2 = (valor ?
                        CaractFunc(c->InstrVar[meio] + cont) :
                        CaractTxtVar(c->InstrVar[meio] + cont));
#if 0
                int comp = (ch1 == ch2 ? 0 : ch1 < ch2 ? -1 : 1);
                printf("cmp1  ini=%d fim=%d  [%s] [%s] = %d\n",
                       ini, fim, origem+tampadrao,
                       c->InstrVar[meio] + tamvar, comp); fflush(stdout);
#endif
                if (ch2 == ch1)
                    fim = meio - 1, xini = meio;
                else if (ch2 < ch1)
                    ini = meio + 1;
                else
                    xfim = fim = meio - 1;
            }
        // Checa se tem alguma palavra com a letra
            if (xini < 0)
                break;
            assert(xini <= xfim);
        // Obtém: xfim = última palavra com a letra
            ini = xini, fim = xfim;
            while (ini <= fim)
            {
                int meio = (ini + fim) / 2;
                int ch2 = (valor ?
                        CaractFunc(c->InstrVar[meio] + cont) :
                        CaractTxtVar(c->InstrVar[meio] + cont));
#if 0
                int comp = (ch1==ch2 ? 0 : ch1<ch2 ? -1 : 1);
                printf("cmp2  ini=%d fim=%d  [%s] [%s] = %d\n",
                       ini, fim, origem+tampadrao,
                       c->InstrVar[meio] + tamvar, comp); fflush(stdout);
#endif
                if (ch2 == ch1)
                    ini = meio + 1, xfim = meio;
                else if (ch2 < ch1)
                    ini = meio + 1;
                else
                    fim = meio - 1;
            }
        // Avança caracteres iguais de xini e xfim
            const char * p2 = c->InstrVar[xini] + cont;
            const char * p3 = c->InstrVar[xfim] + cont;
            if (valor)
                while (*p1)
                {
                    int ch3 = CaractOrigem(p1);
                    if (ch3 != CaractFunc(p2) || ch3 != CaractFunc(p3))
                        break;
                    if (p2[0] == '@' && p2[1] && p3[0] == '@' && p3[1])
                        p2++, p3++, cont++;
                    p1++, p2++, p3++, cont++;
                }
            else
                while (*p1)
                {
                    int ch3 = CaractTxtVar(p1);
                    if (ch3 != CaractTxtVar(p2) || ch3 != CaractTxtVar(p3))
                        break;
                    p1++, p2++, p3++, cont++;
                }
        // Checa se função em xini é uma possível resposta
            if (*p2==0)
                if (porcent >= 100 || circle_random() % 100 <
                    (unsigned int)porcent)
                {
                    indvar = xini;
                    pontvar = p1;
                }
#if 0
            printf("caracteres [%d] faixa de [%s] a [%s] candidato [%s]\n",
                        cont-tamvar, c->InstrVar[xini] + tamvar,
                        c->InstrVar[xfim] + tamvar,
                        indvar<0 ? "" : c->InstrVar[indvar] + tamvar); fflush(stdout);
#endif
        // Checa se deve continuar procurando
            if (*p1 == 0 || *p3 == 0)
                break;
            ini = xini, fim = xfim; // Procurar na faixa de xini a xfim
        }
    // Checa espaço entre trocas
        if (indvar >= 0 && espacocont)
            indvar =- 1,espacocont--;
    // Não achou variável - anota caracter
        if (indvar < 0)
        {
            if (destino >= mens + sizeof(mens) - 4)
                break;
            *destino++ = *origem++;
            continue;
        }
    // Achou variável
        espacocont = espaco;
    // Acerta origem
        const char * defvar = c->InstrVar[indvar];
        int tamtxt = pontvar - origem - tampadrao;
#if 0
        printf("Texto %d [%s] [%s] [", tamtxt, origem, pontvar);
        for (const char * pp = origem-tamtxt; pp<origem; pp++)
            putchar(*pp);
        printf("]\n");
        printf("Variável [%s]\n", defvar+Instr::endNome); fflush(stdout);
#endif
        origem = pontvar;
    // Se for variável, copia texto
        if (defvar[2] != Instr::cFunc && defvar[2] != Instr::cVarFunc &&
                defvar[2] != Instr::cConstExpr && defvar[2] != Instr::cConstVar)
        {
            TVariavel v;
            indvar = c->IndiceVar[indvar];
            v.defvar = defvar;
            v.tamanho = 0;
            v.numbit = indvar >> 24;
            if (defvar[2] == Instr::cConstTxt || // Constante
                    defvar[2] == Instr::cConstNum)
                v.endvar = 0;
            else if (indvar & 0x400000) // Variável da classe
                v.endvar = c->Vars + (indvar & 0x3FFFFF);
            else    // Variável do objeto
                v.endvar = Instr::FuncAtual->este->Vars + (indvar & 0x3FFFFF);
            const char * o = v.getTxt();
            while (*o && destino < mens + sizeof(mens) - 4)
                *destino++ = *o++;
            continue;
        }
    // Verifica se espaço suficiente
        if (destino + strlen(defvar + Instr::endNome) + tamtxt + 15
                >= mens + sizeof(mens))
            break;
    // Anota fim do texto
        if (dest_ini == 0) // Início
            *destino++ = 0;
        else if (destino == dest_ini) // Texto vazio
            destino--;
        else   // Não é texto vazio
        {
            *destino++ = 0;
            *destino++ = Instr::exo_add;
        }
    // Anota variável
        destino[0] = Instr::ex_varini;
        destino = copiastr(destino + 1, defvar + Instr::endNome);
        destino[0] = Instr::ex_arg;
        destino[1] = Instr::ex_txt;
        destino += 2;
        if (tamtxt)
        {
            memcpy(destino, origem - tamtxt, tamtxt);
            destino += tamtxt;
        }
        destino[0] = 0;
        destino[1] = Instr::ex_ponto;
        destino[2] = Instr::ex_varfim;
        destino[3] = Instr::exo_add;
        destino[4] = Instr::ex_txt;
        destino += 5;
        dest_ini = destino;
    }

    destino[0] = 0;
    destino[1] = Instr::exo_add;
    destino[2] = Instr::ex_fim;
    destino += 3;

// Texto puro, sem substituições
    if (dest_ini == 0)
    {
        const char * texto = mens + Instr::endNome + 3;
        Instr::ApagarVar(v);
        bool b = Instr::CriarVarTexto(texto, destino - texto - 3);
#if 0
        printf("vartxt txt: %s\n", (char*)VarAtual->endvar);
        fflush(stdout);
#endif
        return b;
    }

// Acerta tamanho da instrução
    int total = destino - mens;
    mens[0] = total;    // Tamanho da instrução
    mens[1] = total >> 8;

// Acerta variáveis
    Instr::ApagarVar(v);
    if (Instr::DadosFim - Instr::DadosTopo < total)
        return false;
    Instr::VarAtual++;
    Instr::VarAtual->Limpar();
    Instr::VarAtual->defvar = Instr::DadosTopo;
    Instr::VarAtual->endvar = Instr::DadosTopo;
    Instr::VarAtual->tamanho = total;
    memcpy(Instr::DadosTopo, mens, total);
    Instr::DadosTopo += total;

// Mostra o que codificou
#if 0
    putchar('\n');
    const char * pp = VarAtual->defvar;
    const char * pp2 = pp + Num16(pp);
    for (; pp<pp2; pp++)
    {
        printf(" %02X", (unsigned char)pp[0]);
        if (pp[0] >= 32 && pp[0] < 127)
            printf("/%c", pp[0]);
    }
    putchar('\n');
    Instr::Decod(mens, VarAtual->defvar, sizeof(mens));
    printf("vartxt exec: %s\n", mens); fflush(stdout);
#endif

// Acerta função
    Instr::FuncAtual++;
    Instr::FuncAtual->nome = Instr::VarAtual->defvar;
    Instr::FuncAtual->linha = Instr::VarAtual->defvar;
    Instr::FuncAtual->este = Instr::FuncAtual[-1].este;
    Instr::FuncAtual->expr = Instr::VarAtual->defvar + Instr::endNome + 2;
    Instr::FuncAtual->inivar = Instr::VarAtual + 1;
    Instr::FuncAtual->fimvar = Instr::VarAtual + 1;
    Instr::FuncAtual->numarg = 0;
    Instr::FuncAtual->tipo = 0;
    Instr::FuncAtual->indent = 0;
    Instr::FuncAtual->objdebug = Instr::FuncAtual[-1].objdebug;
    Instr::FuncAtual->funcdebug = Instr::FuncAtual[-1].funcdebug;
    return true;
}

//----------------------------------------------------------------------------
bool Instr::FuncVarTroca(TVariavel * v, int valor)
{
    return StaticVarTroca(v, 0);
}

//----------------------------------------------------------------------------
bool Instr::FuncVarTrocaCod(TVariavel * v, int valor)
{
    return StaticVarTroca(v, 1);
}
