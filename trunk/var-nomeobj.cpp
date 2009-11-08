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
        Achou=false;
        if (Total <= 0)
        {
            // Retorna 0
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(0);
        }
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            const char * txt = v1->getTxt();
            while (true)
            {
            // Avan�a espa�os em branco
                while (*txt==' ')
                    txt++;
            // Verifica fim do texto
                if (*txt==0)
                    break;
            // Verifica pr�xima palavra
                int indice;
                for (indice=0; indice<NomeTam; indice++,txt++)
                    if (NomeObj[indice] !=
                            tabCOMPLETO[*(unsigned char*)txt])
                        break;
            // Se palavra n�o corresponde
                if (indice<NomeTam)
                {
                    while (*txt && *txt!=' ')
                        txt++;
                    continue;
                }
            // Se n�o � o item inicial
                if (Inicio>0)
                {
                    Inicio--;
                    // Retorna 0
                    Instr::ApagarVar(v);
                    return Instr::CriarVarInt(0);
                }
            // Diminui quantidade de itens
                Total--;
            // Retorna 1
                Achou=true;
                Instr::ApagarVar(v);
                return Instr::CriarVarInt(1);
            } // while (true)
        } // for
        // Retorna 0
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(0);
    }
// Inicializa��o
    if (comparaZ(nome, "ini")==0)
    {
        Inicio=0;
        Total=0;
    // Checa o n�mero de vari�veis
        if (Instr::VarAtual - v < 2)
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
        int x = v[2].getInt();
        if (Total>x)
            Total=x;
        // Copia o texto
        while (*txt==' ')
            txt++;
        for (x=0; x<(int)sizeof(NomeObj) && *txt; x++,txt++)
            NomeObj[x] = tabCOMPLETO[*(unsigned char*)txt];
        while (x>0)
        {
            if (NomeObj[x-1]!=' ')
                break;
            x--;
        }
        NomeTam = x;
        if (NomeTam==0)
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
