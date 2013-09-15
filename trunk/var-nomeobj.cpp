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
    // Prepara variáveis
        int valor = (Instr::VarAtual - v < 2 ? 1 : v[2].getInt());
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
    // Verifica se é o nome procurado
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
                if (*lista==0) // Fim da lista: não encontrou
                {
                    Instr::ApagarVar(v);
                    return Instr::CriarVarInt(0);
                }
            }
            while (*txt && *txt!=' ') txt++;
            while (*txt==' ') txt++;
        }
    // Checa se é o item inicial
        if (Inicio >= valor)
        {
            Inicio -= valor;
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(0);
        }
        valor -= Inicio;
        Inicio = 0;
    // Obtém a quantidade de itens
        if (valor >= Total)
            valor = Total, Total = 0;
        else
            Total -= valor;
        Achou = valor;
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(valor);
    }
// Inicialização
    if (comparaZ(nome, "ini")==0)
    {
        Inicio=0;
        Total=0;
    // Checa o número de variáveis
        if (Instr::VarAtual - v < 1)
            return false;
    // Obtém o texto
        Inicio = 0;
        const char * txt = v[1].getTxt();
        while (*txt==' ')
            txt++;
        while (*txt>='0' && *txt<='9')
        {
        // Obtém o número
            const char * p = txt;
            int num = 0;
            for (; *p>='0' && *p<='9'; p++)
                if (num<1000000)
                    num = num * 10 + *p - '0';
        // O número é zero
            if (num==0)
                break;
        // É a quantidade de itens
            if (*p==' ' && Total==0)
            {
                while (*p==' ')
                    p++;
                if (*p==0)  // Somente o número não pode ser quantidade
                    break;
                txt=p;
                Total=num;
                continue;
            }
        // É o número do primeiro item
            if (*p=='.')
                Inicio=num-1, txt=p+1;
        // É o nome do item
            break;
        }
        // Copia o texto
        while (*txt==' ')
            txt++;
        char * p = NomeObj;
        for (; *txt && p<NomeObj+sizeof(NomeObj)-1; p++,txt++)
            *p = tabNOMEOBJ[*(unsigned char*)(txt)];
        *p=0;
        // Obtém Total
        if (Total <= 0)
            Total=1;
        int x = (Instr::VarAtual - v < 2 ? 1 : v[2].getInt());
        if (Total>x)
            Total=x;
        if (*NomeObj==0)
            Total=0;
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
int TVarNomeObj::getValor()
{
    return Achou;
}
