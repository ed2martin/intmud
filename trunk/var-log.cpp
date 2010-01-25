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
#include <fcntl.h>
#include <unistd.h>
#include "var-log.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

int TVarLog::Tempo = 20;
TVarLog * TVarLog::Inicio = 0;

//------------------------------------------------------------------------------
void TVarLog::Criar()
{
    arq=-1;
}

//------------------------------------------------------------------------------
void TVarLog::Apagar()
{
    Fechar();
}

//------------------------------------------------------------------------------
void TVarLog::Mover(TVarLog * destino)
{
    if (arq==-1)
    {
        destino->arq = -1;
        return;
    }
    (Antes ? Antes->Depois : Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
    int total = sizeof(TVarLog) - sizeof(buflog) + pontlog;
    move_mem(destino, this, total);
}

//------------------------------------------------------------------------------
int TVarLog::getValor()
{
    return (arq>=0 ? 1 : 0);
}

//------------------------------------------------------------------------------
int TVarLog::TempoEspera(int tempodecorrido)
{
// Atualiza tempo
    Tempo -= tempodecorrido;
// Grava dados pendentes se tempo expirou
    if (Tempo<=0)
    {
        for (TVarLog * obj = Inicio; obj; obj=obj->Depois)
            if (obj->pontlog)
            {
                write(obj->arq, obj->buflog, obj->pontlog);
                obj->pontlog = 0;
            }
        Tempo=20;
        return 100;
    }
// Obt�m quanto tempo para gravar dados pendentes
    for (TVarLog * obj = Inicio; obj; obj=obj->Depois)
        if (obj->pontlog)
            return Tempo;
    Tempo=20;
    return 100;
}

//------------------------------------------------------------------------------
void TVarLog::Fechar()
{
    if (arq<0)
        return;
    if (pontlog)
        write(arq, buflog, pontlog);
    close(arq);
    arq=-1;
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = 0;
}

//------------------------------------------------------------------------------
bool TVarLog::Func(TVariavel * v, const char * nome)
{
// Mensagem no log
    if (comparaZ(nome, "msg")==0)
    {
        if (arq<0)
            return false;
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            const char * txt = v1->getTxt();
            while (true)
            {
                while (*txt==Instr::ex_barra_n)
                    txt++;
                if (*txt==0)
                    break;
                if (pontlog>=(int)sizeof(buflog)-500)
                {
                    write(arq, buflog, pontlog);
                    pontlog=0;
                }
                for (int x=0; x<490 && *txt && *txt!=Instr::ex_barra_n; x++)
                    switch (*txt)
                    {
                    case Instr::ex_barra_b:
                        txt++;
                        break;
                    case Instr::ex_barra_c:
                        if ((txt[1]>='0' && txt[1]<='9') ||
                                (txt[1]>='A' && txt[1]<='F') ||
                                (txt[1]>='a' && txt[1]<='f'))
                            txt += 2;
                        else
                            txt++;
                        break;
                    case Instr::ex_barra_d:
                        if (txt[1]>='0' && txt[1]<='7')
                            txt += 2;
                        else
                            txt++;
                        break;
                    default:
                        buflog[pontlog++] = *txt++;
                    }
                buflog[pontlog++] = '\n';
            }
        }
        return false;
    }
// Abrir arquivo
    if (comparaZ(nome, "abrir")==0)
    {
        int descr = -1;
    // Obt�m nome do arquivo
        while (Instr::VarAtual == v+1)
        {
            char nome[300]; // Nome do arquivo
            copiastr(nome, v[1].getTxt(), sizeof(nome)-4);
    // Verifica se nome permitido
            if (!arqvalido(nome, "log"))
                break;
    // Abre arquivo
            descr = open(nome, O_WRONLY|O_CREAT|O_APPEND, (mode_t)0666);
            break;
        }
    // Vari�vel int no topo da pilha
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
        {
            if (descr>=0)
                close(descr);
            return false;
        }
    // Se conseguiu abrir arquivo...
        if (descr >= 0)
        {
        // Fecha arquivo
            Fechar();
        // Anota vari�veis
            arq = descr;
            pontlog = 0;
        // Insere na lista ligada
            Antes = 0;
            Depois = Inicio;
            Inicio = this;
            if (Depois)
                Depois->Antes = this;
        // Valor de retorno � 1
            Instr::VarAtual->setInt(1);
        }
        return true;
    }
// Fechar arquivo
    if (comparaZ(nome, "fechar")==0)
    {
        Fechar();
        return false;
    }
    return false;
}
