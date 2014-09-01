/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos das licen�as GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var-nomeobj.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//------------------------------------------------------------------------------
bool TVarNomeObj::Func(TVariavel * v, const char * nome)
{
// Checa texto
    if (comparaZ(nome, "nome")==0)
    {
        Achou = 0;
        if (Total <= 0 || Instr::VarAtual - v < 1)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(0);
        }
    // Prepara vari�veis
        if (*NomeObj)
        {
            char mens[4096];
            const char * txt = v[1].getTxt();
            while (*txt==' ') txt++;
            char * p = mens;
            for (; *txt && p<mens+sizeof(mens)-1; p++,txt++)
                *p = tabNOMEOBJ[*(unsigned char*)(txt)];
            *p=0;
            // Checa fim do nome
            if (*mens==0)
            {
                Instr::ApagarVar(v);
                return Instr::CriarVarInt(0);
            }
    // Verifica se � o nome procurado
    // Todas as palavras de NomeObj devem estar em mens
            txt = NomeObj;
            while (*txt)
            {
                const char * lista = mens;
                while (true)
                {
                    const char * str = txt;
                    while (*str && *str!=' ' && *str==*lista)
                        str++, lista++;
                    if (*str==0 || *str==' ') // Encontrou
                        break;
                    while (*lista && *lista!=' ') lista++;
                    while (*lista==' ') lista++;
                    if (*lista==0) // Fim da lista: n�o encontrou
                    {
                        Instr::ApagarVar(v);
                        return Instr::CriarVarInt(0);
                    }
                }
                while (*txt && *txt!=' ') txt++;
                while (*txt==' ') txt++;
            }
        }
    // Checa se � o item inicial
        int valor = (Instr::VarAtual - v < 2 ? 1 : v[2].getInt());
        if (Inicio >= valor)
        {
            Inicio -= valor;
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(0);
        }
        valor -= Inicio;
        Inicio = 0;
    // Obt�m a quantidade de itens
        if (valor >= Total)
            valor = Total, Total = 0;
        else
            Total -= valor;
        Achou = valor;
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(valor);
    }
// Inicializa��o
    if (comparaZ(nome, "ini")==0)
    {
        Inicio=0;
        Total=0;
    // Checa o n�mero de vari�veis
        if (Instr::VarAtual - v < 1)
            return false;
    // Obt�m o texto que se refere a todos os itens
        char nometudo[200];
        *nometudo = 0;
        if (Instr::VarAtual >= v+3)
        {
            const char * o = v[3].getTxt();
            char * d = nometudo;
            for (; *o && d < nometudo + sizeof(nometudo) - 1; o++,d++)
                *d = tabNOMEOBJ[*(unsigned char*)o];
            *d = 0;
        }
    // Obt�m o texto
        Inicio = 0;
        const char * txt = v[1].getTxt();
        while (*txt==' ')
            txt++;
        if (*nometudo)
        {
            const char * p1 = txt;
            const char * p2 = nometudo;
            while (*p1 && tabNOMEOBJ[*(unsigned char*)p1] == *p2)
                p1++,p2++;
            if (*p1=='.' && *p2==0)
                Total=0x7FFFFFFF, txt=p1+1;
        }
        if (Total == 0)
            while (*txt>='0' && *txt<='9')
            {
            // Obt�m o n�mero
                const char * p = txt;
                int num = NumInt(p);
                while (*p>='0' && *p<='9')
                    p++;
            // O n�mero � zero
                if (num==0)
                    break;
            // � a quantidade de itens
                if (*p==' ' && Total==0)
                {
                    while (*p==' ')
                        p++;
                    if (*p==0)  // Somente o n�mero n�o pode ser quantidade
                        break;
                    txt=p;
                    Total=num;
                    continue;
                }
            // � o n�mero do primeiro item
                if (*p=='.')
                    Inicio=num-1, txt=p+1;
            // � o nome do item
                break;
            }
    // Copia o texto
        {
            while (*txt==' ')
                txt++;
            char * p = NomeObj;
            for (; *txt && p<NomeObj+sizeof(NomeObj)-1; p++,txt++)
                *p = tabNOMEOBJ[*(unsigned char*)(txt)];
            *p=0;
        }
    // Checa se o texto � nulo
        if (*NomeObj == 0)
        {
            Total = 0;
            return false;
        }
    // Checa se digitou a palavra que corresponde a todos os itens
        if (Instr::VarAtual >= v+3)
            if (strcmp(NomeObj, nometudo)==0)
            {
                *NomeObj = 0;
                if (Total <= 0)
                    Total = 0x7FFFFFFF;
            }
    // Obt�m Total
        if (Total <= 0)
            Total = 1;
        int x = (Instr::VarAtual - v < 2 ? 1 : v[2].getInt());
        if (Total > x)
            Total = x;
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
int TVarNomeObj::getValor()
{
    return Achou;
}
