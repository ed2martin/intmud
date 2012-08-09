/* Este programa � software livre; voc� pode redistribuir e/ou
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
        int valor = (Instr::VarAtual - v < 2 ? 1 : v[2].getInt());
        char mens[4096];
        const char * txt = v[1].getTxt();
        while (*txt==' ') txt++;
        copiastrmin(mens, txt, sizeof(mens));
        txt = mens;
    // Verifica se � o nome procurado
        while (true)
        {
        // Checa fim do nome
            if (*txt==0)
            {
                Instr::ApagarVar(v);
                return Instr::CriarVarInt(0);
            }
        // Checa se � o nome procurado
            if (memcmp(txt, NomeObj, NomeTam) == 0)
                break;
            while (*txt && *txt!=' ') txt++;
            while (*txt==' ') txt++;
        }
    // Checa se � o item inicial
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
    // Obt�m o texto
        Inicio = 0;
        const char * txt = v[1].getTxt();
        while (*txt==' ')
            txt++;
        while (*txt>='0' && *txt<='9')
        {
        // Obt�m o n�mero
            const char * p = txt;
            int num = 0;
            for (; *p>='0' && *p<='9'; p++)
                if (num<1000000)
                    num = num * 10 + *p - '0';
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
        // Obt�m Total
        if (Total <= 0)
            Total=1;
        int x = (Instr::VarAtual - v < 2 ? 1 : v[2].getInt());
        if (Total>x)
            Total=x;
        // Copia o texto
        while (*txt==' ')
            txt++;
        for (x=0; x<(int)sizeof(NomeObj) && *txt; x++)
        {
            NomeObj[x] = tabCOMPLETO[*(unsigned char*)txt];
            if (*txt++==' ')
                while (*txt==' ')
                    txt++;
        }
        if (x>0 && NomeObj[x-1]==' ')
            x--;
        NomeTam = x;
        if (x==0)
            Total=0;
        for (; x>=0; x--)
            if (NomeObj[x]==' ')
                NomeObj[x]='_';
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
int TVarNomeObj::getValor()
{
    return Achou;
}
