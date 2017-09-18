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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
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
    memmove(destino, this, total);
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
                safe_write(obj->arq, obj->buflog, obj->pontlog);
                obj->pontlog = 0;
            }
        Tempo=20;
        return 600;
    }
// Obtém quanto tempo para gravar dados pendentes
    for (TVarLog * obj = Inicio; obj; obj=obj->Depois)
        if (obj->pontlog)
            return Tempo;
    Tempo=20;
    return 600;
}

//------------------------------------------------------------------------------
void TVarLog::Fechar()
{
    if (arq<0)
        return;
    if (pontlog)
        safe_write(arq, buflog, pontlog);
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
                    safe_write(arq, buflog, pontlog);
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
                                (txt[1]>='A' && txt[1]<='J') ||
                                (txt[1]>='a' && txt[1]<='j'))
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
// Fechar arquivo
    if (comparaZ(nome, "fechar")==0)
    {
        Fechar();
        return false;
    }
// Obtém o nome do arquivo
    char arqnome[512]; // Nome do arquivo; nulo se não for válido
    *arqnome=0;
    if (Instr::VarAtual >= v+1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome)-4);
    // Verifica se nome permitido
        if (!arqvalido(arqnome))
            *arqnome=0;
        else
        {
            int tam = strlen(arqnome);
            if (tam >= 4) // Tamanho suficiente
                if (comparaZ(arqnome+tam-4, ".log") == 0) // Extensão correta
                    tam -= 4;
            strcpy(arqnome + tam, ".log"); // Acerta a extensão
        }
    }
// Checa se nome de arquivo é válido
    if (comparaZ(nome, "valido")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(*arqnome != 0);
    }
// Checa se arquivo existe
    if (comparaZ(nome, "existe")==0)
    {
        struct stat buf;
        Instr::ApagarVar(v);
        if (*arqnome && stat(arqnome, &buf)<0)
            *arqnome = 0;
        return Instr::CriarVarInt(*arqnome != 0);
    }
// Abrir arquivo
    if (comparaZ(nome, "abrir")==0)
    {
        int descr = -1;
    // Variável int no topo da pilha
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
            return false;
    // Abre arquivo
        if (*arqnome)
            descr = open(arqnome, O_WRONLY|O_CREAT|O_APPEND, (mode_t)0666);
    // Se conseguiu abrir arquivo...
        if (descr >= 0)
        {
        // Fecha arquivo
            Fechar();
        // Anota variáveis
            arq = descr;
            pontlog = 0;
        // Insere na lista ligada
            Antes = 0;
            Depois = Inicio;
            Inicio = this;
            if (Depois)
                Depois->Antes = this;
        // Valor de retorno é 1
            Instr::VarAtual->setInt(1);
        }
        return true;
    }
    return false;
}
