/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "procurar.h"
#include "misc.h"

//----------------------------------------------------------------------------
/// Construtor
TProcurar::TProcurar()
{
    tampadrao = 0;
    *troca = 0;
    dest = nullptr;
}

//----------------------------------------------------------------------------
/// Define o padrão que se quer procurar
/** @param p Padrão a procurar
 *  @param e Vide TProcurar::exato
 *  @return verdadeiro se padrão válido, falso se padrão vazio
 */
bool TProcurar::Padrao(const char * p, int e)
{
    TProcurar::exato = e;
// Obtém tamanho do padrão
    tampadrao = strlen(p);
    if (tampadrao > (int)sizeof(padrao))
        tampadrao = (int)sizeof(padrao);
// 0 ou 1 caracteres
    if (tampadrao < 2)
    {
        *padrao = (e == 0 ? tabCOMPLETO[*(unsigned char*)p] :
                   e == 1 ? tabMAI[*(unsigned char*)p] : *p);
        return tampadrao >= 1;
    }
// Prepara tabela
    for (int x = 0; x < 0x100; x++)
        tabela[x] = -1;
// Copia padrão e acerta tabela
    int x;
    switch (e)
    {
    case 0: // Não diferencia
        for (x = 0; x < tampadrao; x++, p++)
        {
            unsigned char ch = tabCOMPLETO[*(unsigned char*)p];
            padrao[x] = ch;
            tabela[ch] = x;
        }
        break;
    case 1: // maiúsculas = minúsculas
        for (x = 0; x < tampadrao; x++, p++)
        {
            unsigned char ch = tabMAI[*(unsigned char*)p];
            padrao[x] = ch;
            tabela[ch] = x;
        }
        break;
    default: // Exato
        for (x = 0; x < tampadrao; x++, p++)
        {
            padrao[x] = *p;
            tabela[*(unsigned char*)p] = x;
        }
    }
    return true;
}

//----------------------------------------------------------------------------
/// Procura o padrão em um texto
/** @param texto Texto onde procurar o padrão
 *  @param tamanho Tamanho do texto (=strlen(texto))
 *  @return índice aonde encontrou ou -1 se não encontrou ou TProcurar::dest!=0
 *  @note  Usa algoritmo Boyer-Moore, mas com uma só tabela
 */
int TProcurar::Proc(const char * texto, int tamanho)
{
    lidoatual = texto;
    lidofim = texto + tamanho;
    destatual = dest;
    destfim = dest + tamdest - 1;
    if (tamanho < tampadrao)
    {
        AddTroca();
        return -1;
    }
    if (tampadrao <= 1)
    {
        const char * ini = texto;
        const char * fim = texto+tamanho;
        switch (exato)
        {
        case 0: // Não diferencia
            for (; texto<fim; texto++)
            {
                if (tabCOMPLETO[*(unsigned char*)texto] != padrao[0])
                    continue;
                if (destatual == nullptr)
                    return texto - ini;
                AddTroca(texto);
            }
            break;
        case 1: // maiúsculas = minúsculas
            for (; texto<fim; texto++)
            {
                if (tabMAI[*(unsigned char*)texto] != padrao[0])
                    continue;
                if (destatual == nullptr)
                    return texto - ini;
                AddTroca(texto);
            }
            break;
        default: // Exato
            for (; texto < fim; texto++)
            {
                if (*texto != padrao[0])
                    continue;
                if (destatual == nullptr)
                    return texto-ini;
                AddTroca(texto);
            }
        }
        AddTroca();
        return -1;
    }
    int i = tampadrao - 1, j = 0;
    switch (exato)
    {
    case 0: // Não diferencia
        while (true)
        {
            char ch = tabCOMPLETO[(unsigned char)texto[i+j]];
            if (padrao[i] == ch)
            {
                if (i--)    // Passa para próximo caracter
                    continue;
                        // Encontrou
                if (destatual == nullptr)
                    return j;
                AddTroca(texto + j);
                j += tampadrao;
            }
            else
            {
                i -= tabela[(unsigned char)ch]; // Obtém deslocamento
                j += (i > 1 ? i : 1); // Desloca j
            }
            i = tampadrao - 1;    // Inicializa i
            if (i + j >= tamanho) // Verifica fim da string
                break;
        }
        break;
    case 1: // maiúsculas = minúsculas
        while (true)
        {
            char ch = tabMAI[(unsigned char)texto[i+j]];
            if (padrao[i] == ch)
            {
                if (i--)    // Passa para próximo caracter
                    continue;
                        // Encontrou
                if (destatual == 0)
                    return j;
                AddTroca(texto + j);
                j += tampadrao;
            }
            else
            {
                i -= tabela[(unsigned char)ch]; // Obtém deslocamento
                j += (i > 1 ? i : 1); // Desloca j
            }
            i = tampadrao - 1;    // Inicializa i
            if (i + j >= tamanho) // Verifica fim da string
                break;
        }
        break;
    default: // Exato
        while (true)
        {
            char ch = texto[i + j];
            if (padrao[i] == ch)
            {
                if (i--)    // Passa para próximo caracter
                    continue;
                        // Encontrou
                if (destatual == nullptr)
                    return j;
                AddTroca(texto + j);
                j += tampadrao;
            }
            else
            {
                i -= tabela[(unsigned char)ch]; // Obtém deslocamento
                j += (i > 1 ? i : 1); // Desloca j
            }
            i = tampadrao - 1;    // Inicializa i
            if (i + j >= tamanho) // Verifica fim da string
                break;
        }
    }
    AddTroca();
    return -1;
}

//----------------------------------------------------------------------------
/// Procura o padrão em um texto
/** @param funcler Função que lê próximos caracteres do texto
 *                 - Recebe endereço e tamanho do buffer (sempre > 0)
 *                 - Retorna quantidade de bytes lidos ou <=0 se fim do texto
 *  @return índice aonde encontrou ou -1 se não encontrou ou TProcurar::dest!=0
 *  @note  Usa algoritmo Boyer-Moore, mas com uma só tabela
 *  @note  Não faz substituições (não anota texto em dest)
 */
int TProcurar::Proc(int (*funcler)(char * buf, int tambuf))
{
    char buftxt[0x4000];
    unsigned int buflido = 0;
    unsigned int bufini = 0;
    if (tampadrao <= 1)
    {
        if (tampadrao <= 0)
            return -1;
        switch (exato)
        {
        case 0: // Não diferencia
            while (true)
            {
                int lido = funcler(buftxt, 1024);
                if (lido <= 0)
                    return -1;
                for (int i = 0; i < lido; i++)
                    if (tabCOMPLETO[(unsigned char)buftxt[i]] == padrao[0])
                        return i + bufini;
                bufini += lido;
            }
            break;
        case 1: // maiúsculas = minúsculas
            while (true)
            {
                int lido = funcler(buftxt, 1024);
                if (lido <= 0)
                    return -1;
                for (int i = 0; i < lido; i++)
                    if (tabMAI[(unsigned char)buftxt[i]] == padrao[0])
                        return i + bufini;
                bufini += lido;
            }
            break;
        default: // Exato
            while (true)
            {
                int lido = funcler(buftxt, 1024);
                if (lido <= 0)
                    return -1;
                for (int i = 0; i < lido; i++)
                    if (buftxt[i] == padrao[0])
                        return i + bufini;
                bufini += lido;
            }
        }
        return -1;
    }
    int i = tampadrao - 1, j = 0;
    while (true)
    {
        while (i + j >= (int)(bufini+buflido))
        {
            int ler = sizeof(buftxt) - buflido;
            ler = funcler(buftxt + buflido, ler > 1024 ? 1024 : ler);
            if (ler <= 0)
                return -1;
            buflido += ler;
            if (buflido >= sizeof(buftxt))
                buflido = 0, bufini += sizeof(buftxt);
        }
        char ch;
        while (true)
        {
            ch = buftxt[(i + j - bufini) % sizeof(buftxt)];
            if (exato == 0) // Não diferencia
                ch = tabCOMPLETO[(unsigned char)ch];
            else if (exato == 1) // maiúsculas = minúsculas
                ch = tabMAI[(unsigned char)ch];
            if (padrao[i]!=ch)
                break;
            if (i--)    // Passa para próximo caracter
                continue;
                    // Encontrou
            return j;
        }
        i -= tabela[(unsigned char)ch]; // Obtém deslocamento
        j += (i > 1 ? i : 1); // Desloca j
        i = tampadrao - 1;    // Inicializa i
    }
    return -1;
}

//----------------------------------------------------------------------------
/// Anota string em dest
void TProcurar::AddTroca()
{
    if (destatual == nullptr)
        return;
    int tam = lidofim - lidoatual;
    if (tam > destfim - destatual)
        tam = destfim - destatual;
    if (tam < 0)
        tam = 0;
    destatual[tam] = 0;
    if (tam > 0)
        memcpy(destatual, lidoatual, tam);
}

//----------------------------------------------------------------------------
/// Anota parte da string (até texto) + troca[] em dest
void TProcurar::AddTroca(const char * texto)
{
    int tam = texto - lidoatual;
    if (tam > destfim - destatual)
        tam = destfim - destatual;
    if (tam > 0)
    {
        memcpy(destatual, lidoatual, tam);
        destatual += tam;
    }
    lidoatual = texto + tampadrao;
    for (const char * p = troca; *p && destatual < destfim; p++)
        *destatual++ = *p;
}
