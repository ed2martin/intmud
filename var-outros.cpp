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
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "instr.h"
#include "instr-enum.h"
#include "classe.h"
#include "objeto.h"
#include "var-outros.h"
#include "procurar.h"
#include "misc.h"

//------------------------------------------------------------------------------
bool FuncVetorTxt(TVariavel * v, const char * nome)
{
// Obtém número de variáveis e tamanho de uma variável
    int numvar = (unsigned char)v->defvar[Instr::endVetor];
    int tamvar = (unsigned char)v->defvar[Instr::endExtra] + 2;
    if (v->defvar[2] == Instr::cTxt2)
        tamvar += 256;
// Texto do vetor
    if (comparaZ(nome, "texto") == 0)
    {
    // Obtém índice inicial e a quantidade de variáveis
        int ini = 0;
        int total = numvar;
        if (Instr::VarAtual >= v + 1)
        {
            ini = v[1].getInt();
            if (ini < 0)
                ini = 0;
            if (Instr::VarAtual >= v + 2)
            {
                total = v[2].getInt() + 1;
                if (total > numvar)
                    total = numvar;
            }
        }
        total -= ini;
    // Cria variável
        const char * origem = v->end_char + ini * tamvar;
        Instr::ApagarVar(v);  // Não tem importância que v seja apagado aqui
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; total > 0; total--, origem += tamvar)
        {
            //printf("texto = ["); fflush(stdout);
            //for (const char * x = origem; *x; x++)
            //    if (*(signed char*)x >= ' ')
            //        putchar(*x);
            //    else
            //        printf("(%02X)", *(unsigned char*)x);
            //printf("]\n"); fflush(stdout);
            for (const char * o = origem; *o && destino < Instr::DadosFim - 1; )
                *destino++ = *o++;
        }
        *destino++ = 0;
    // Acerta a variável
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Divide em palavras
    if (comparaZ(nome, "palavras") == 0)
    {
    // Obtém número de palavras (parâmetro de entrada)
        int total = numvar;
        if (Instr::VarAtual < v + 1)
            return false;
        if (Instr::VarAtual >= v + 2)
        {
            total = v[2].getInt();
            if (total > numvar)
                total = numvar;
        }
    // Copia o texto obtendo o número de palavras
        int palavras = 0;
        const char * origem = v[1].getTxt();
        char * destino = v->end_char;
        for (; total>1; total--)
        {
            while (*origem == ' ' || *origem == Instr::ex_barra_n)
                origem++;
            if (*origem == 0)
                break;
            char * d = destino;
            destino += tamvar, palavras++;
            for (; *origem && *origem!=' ' &&
                    *origem != Instr::ex_barra_n; origem++)
                if (d < destino - 1)
                    *d++ = *origem;
            *d = 0;
        }
    // Copia o último texto
        while (*origem==' ' || *origem == Instr::ex_barra_n)
            origem++;
        if (*origem)
        {
            char * d = destino;
            destino += tamvar, palavras++;
            while (*origem && d < destino - 1)
                *d++ = *origem++;
            while (*origem == ' ' || *origem == Instr::ex_barra_n)
                origem++;
            if (*origem == 0)
            {
                for (d--; d >= destino - tamvar; d--)
                    if (*d != ' ' && *d != Instr::ex_barra_n)
                        break;
                d++;
            }
            *d = 0;
        }
    // Limpa próximas variáveis do vetor
        for (; palavras < numvar; destino += tamvar, numvar--)
            *destino = 0;
    // Retorna o número de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(palavras);
    }
// Divide em linhas
    if (comparaZ(nome, "linhas") == 0)
    {
    // Obtém número mínimo de colunas (parâmetro de entrada)
        int colunas = tamvar - 1;
        if (Instr::VarAtual < v + 1)
            return false;
        if (Instr::VarAtual >= v + 2)
        {
            colunas = v[2].getInt();
            if (colunas > tamvar - 1)
                colunas = tamvar - 1;
            if (colunas < 0)
                colunas = 0;
        }
    // Copia o texto
        const char * origem = v[1].getTxt();
        char * destino = v->end_char;
        int linha = 0;  // Número de linhas copiadas
        colunas -= tamvar - 1; // Quantas colunas recuar do final do texto
        while (linha < numvar)
        {
            char * d = destino;
            linha++, destino += tamvar;
        // Copia linha
            while (*origem && *origem != Instr::ex_barra_n && d < destino)
                *d++ = *origem++;
        // Linha cabe na variável
            if (d < destino)
            {
                *d = 0;
            // Fim da linha
                if (*origem == Instr::ex_barra_n)
                {
                    origem++;
                    continue;
                }
            // Fim do texto
                break;
            }
        // Obtém aonde deve dividir a linha
            d--, origem--; // Recua um caracter
            int x;
            for (x = -1; x >= colunas; x--)
                if (origem[x] == ' ')
                    break;
            if (x < colunas)
                for (x = -1; x >= colunas; x--)
                    if (origem[x] == '.')
                        break;
        // Divide a linha
            if (x >= colunas)
            {
                x++;
                origem += x;
                d += x;
            }
            *d = 0;
        }
    // Limpa próximas variáveis do vetor
        for (; linha < numvar; destino += tamvar, numvar--)
            *destino = 0;
    // Retorna o número de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(linha);
    }
// Limpa as variáveis do vetor
    if (comparaZ(nome, "limpar") == 0)
    {
        char * destino = v->end_char;
        const char * texto = "";
        if (Instr::VarAtual > v)
            texto = v[1].getTxt();
        for (; numvar > 0; numvar--, destino += tamvar)
            copiastr(destino, texto, tamvar);
        return false;
    }
// Junta texto
    if (comparaZ(nome, "juntar") == 0)
    {
    // Obtém o texto delimitador
        int total = numvar;
        char mens[4096];
        *mens = 0;
        if (Instr::VarAtual >= v + 1)
        {
            copiastr(mens, v[1].getTxt(), sizeof(mens));
            if (Instr::VarAtual >= v + 2)
            {
                total = v[2].getInt();
                if (total > numvar)
                    total = numvar;
            }
        }
    // Cria variável
        const char * origem = v->end_char;
        Instr::ApagarVar(v);  // Não tem importância que v seja apagado aqui
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; total > 0; total--, origem += tamvar)
        {
            for (const char * o = origem; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
            if (total<=1)
                break;
            for (const char * o = mens; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
        }
        *destino++ = 0;
    // Acerta a variável
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Separa texto
    int valor = 10;
    if (comparaZ(nome, "separar") == 0)
        valor = 0;
    else if (comparaZ(nome, "separarmai") == 0)
        valor = 1;
    else if (comparaZ(nome, "separardif") == 0)
        valor = 2;
    if (valor != 10)
    {
        int indice = 0;
        int indmax = numvar;
        char * destino = v->end_char;
        while (true)
        {
            if (Instr::VarAtual < v + 2)
                break;
            if (Instr::VarAtual >= v + 3)
            {
                indmax = v[3].getInt();
                if (indmax<1)
                    break;
                if (indmax > numvar)
                    indmax = numvar;
            }
        // Obtém delimitador
            TProcurar proc;
            const char * origem = v[2].getTxt();
            int tampadrao = strlen(origem);
            if (!proc.Padrao(origem, valor))
                break;
        // Obtém texto
            origem = v[1].getTxt();
            int tam = strlen(origem);
        // Anota texto
            while (true)
            {
                indice++;
                int posic = (indice >= indmax ? -1 : proc.Proc(origem, tam));
                if (posic < 0) // Não encontrou
                {
                    copiastr(destino, origem, tamvar);
                    destino += tamvar;
                    break;
                }
                int total = (posic < tamvar - 1 ? posic : tamvar - 1);
                memcpy(destino, origem, total);
                destino[total] = 0;
                origem += posic + tampadrao;
                tam -= posic + tampadrao;
                destino += tamvar;
            }
            break;
        }
        // Limpa próximas variáveis do vetor
        for (; indice < numvar; destino += tamvar, numvar--)
            *destino = 0;
        // Retorna o número de variáveis
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(indice);
    }
// TxtRemove
    if (comparaZ(nome, "txtremove") == 0)
    {
        if (Instr::VarAtual < v + 1)
            return false;
        int remove = txtRemove(v[1].getTxt()); // O que deve remover
        if (remove == 0)
            return false;
        char * destino = v->end_char;
        for (; numvar > 0; numvar--, destino += tamvar)
            txtRemove(destino, destino, tamvar, remove);
        return false;
    }
    return false;
}

//const char Int1_Valor[] = { 8, 0, Instr::cInt1, 0, 0, 1, '=', '0', 0 };

//------------------------------------------------------------------------------
bool FuncVetorInt1(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") == 0)
    {
        unsigned int total = (unsigned char)v->defvar[Instr::endVetor];
        unsigned char * p = (unsigned char*)v->end_char;
        unsigned char bitmask = 1 << v->numbit;
    // Preencher com 1
        if (Instr::VarAtual > v && v[1].getBool())
        {
            if (bitmask > 1)
            {
                for (; total > 0 && bitmask; total--, bitmask <<= 1)
                    *p |= bitmask;
                p++;
            }
            while (total >= 8)
                *p++ = 0xFF, total -= 8;
            for (bitmask = 1; total > 0; total--, bitmask <<= 1)
                *p |= bitmask;
            return false;
        }
    // Preencher com 0
        if (bitmask > 1)
        {
            for (; total > 0 && bitmask; total--, bitmask <<= 1)
                *p &= ~bitmask;
            p++;
        }
        while (total >= 8)
            *p++ = 0, total -= 8;
        for (bitmask = 1; total > 0; total--, bitmask <<= 1)
            *p &= ~bitmask;
        return false;
    }
    if (comparaZ(nome, "bits") == 0)
    {
        Instr::ApagarVar(v + 1);
        v->indice = 0;
        v->numfunc = 1;
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
int GetVetorInt1(TVariavel * v)
{
    unsigned int bitnum = v->numbit;
    unsigned char * p = (unsigned char *)v->end_char;
    int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (total > 32)
        total = 32;
// Primeiro byte
    if (total > 0 && bitnum)
    {
        valor = *p++ >> bitnum;
        bitnum = 8 - bitnum;
        switch (total)
        {
        case 1: valor &= 1; break;
        case 2: valor &= 3; break;
        case 3: valor &= 7; break;
        case 4: valor &= 15; break;
        case 5: valor &= 31; break;
        case 6: valor &= 63; break;
        case 7: valor &= 127; break;
        }
        total -= bitnum;
    }
// Demais bytes
    for (; total > 0 && bitnum < 32; total -= 8,bitnum += 8)
    {
        switch (total)
        {
        case 1: valor |= (*p & 1) << bitnum; break;
        case 2: valor |= (*p & 3) << bitnum; break;
        case 3: valor |= (*p & 7) << bitnum; break;
        case 4: valor |= (*p & 15) << bitnum; break;
        case 5: valor |= (*p & 31) << bitnum; break;
        case 6: valor |= (*p & 63) << bitnum; break;
        case 7: valor |= (*p & 127) << bitnum; break;
        default: valor |= *p++ << bitnum;
        }
    }
    return valor;
}

//------------------------------------------------------------------------------
void SetVetorInt1(TVariavel * v, int valor)
{
    unsigned int bitnum = v->numbit;
    unsigned char * p = (unsigned char *)v->end_char;
    int total = (unsigned char)v->defvar[Instr::endVetor];
    if (total > 32)
        total = 32;
// Altera as variáveis
    while (total > 0)
    {
        int bitmask = 255;
        switch (total)
        {
        case 1: bitmask = 1; break;
        case 2: bitmask = 3; break;
        case 3: bitmask = 7; break;
        case 4: bitmask = 15; break;
        case 5: bitmask = 31; break;
        case 6: bitmask = 63; break;
        case 7: bitmask = 127; break;
        }
    // Bits já estão alinhados
        if (bitnum == 0)
        {
            *p &= ~bitmask;
            *p |= (valor & bitmask);
            p++, total -= 8, valor >>= 8;
            continue;
        }
    // Bits não estão alinhados
        *p &= ~(bitmask << bitnum);
        *p |= (valor & bitmask) << bitnum;
        p++, total -= 8 - bitnum, valor >>= 8 - bitnum;
        bitnum = 0;
    }
}

//------------------------------------------------------------------------------
bool FuncVetorInt8(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    char * ender = v->end_char;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor < -0x80)
            valor = -0x80;
        else if (valor > 0x7F)
            valor = 0x7F;
    }
    memset(ender, valor, total);
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorUInt8(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    char * ender = v->end_char;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor < 0)
            valor = 0;
        else if (valor > 0xFF)
            valor = 0xFF;
    }
    memset(ender, valor, total);
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorInt16(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    short * ender = v->end_short;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor < -0x8000)
            valor = -0x8000;
        else if (valor > 0x7FFF)
            valor = 0x7FFF;
    }
    if (valor == 0)
        memset(ender, 0, 2 * total);
    else
        for (int x = 0; x < total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorUInt16(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    unsigned short * ender = v->end_ushort;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor < 0)
            valor = 0;
        else if (valor > 0xFFFF)
            valor = 0xFFFF;
    }
    if (valor == 0)
        memset(ender, 0, 2 * total);
    else
        for (int x = 0; x < total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorInt32(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    int * ender = v->end_int;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const int valor = (Instr::VarAtual <= v ? 0 : v[1].getInt());
    if (valor == 0)
        memset(ender, 0, 4 * total);
    else
        for (int x = 0; x < total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorUInt32(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    unsigned int * ender = v->end_uint;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    unsigned int valor = 0;
    if (Instr::VarAtual > v)
    {
        const double val_double = v[1].getDouble();
        if (val_double > 0xFFFFFFFFLL)
            valor = 0xFFFFFFFFLL;
        else if (val_double > 0)
            valor = (unsigned int)val_double;
    }
    if (valor == 0)
        memset(ender, 0, 4 * total);
    else
        for (int x = 0; x < total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorReal(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    float * ender = v->end_float;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const float valor = (Instr::VarAtual <= v ? 0 : v[1].getDouble());
    for (int x = 0; x < total; x++)
        ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
bool FuncVetorReal2(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    double * ender = v->end_double;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const double valor = (Instr::VarAtual <= v ? 0 : v[1].getDouble());
    for (int x = 0; x < total; x++)
        ender[x] = valor;
    return false;
}
