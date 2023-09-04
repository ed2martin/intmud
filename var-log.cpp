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
TVarLog * TVarLog::Inicio = nullptr;

//------------------------------------------------------------------------------
void TVarLog::Criar()
{
    arq = -1;
}

//------------------------------------------------------------------------------
void TVarLog::Apagar()
{
    Fechar();
}

//------------------------------------------------------------------------------
void TVarLog::Mover(TVarLog * destino)
{
    if (arq == -1)
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
    return (arq >= 0 ? 1 : 0);
}

//------------------------------------------------------------------------------
int TVarLog::TempoEspera(int tempodecorrido)
{
// Atualiza tempo
    Tempo -= tempodecorrido;
// Grava dados pendentes se tempo expirou
    if (Tempo <= 0)
    {
        for (TVarLog * obj = Inicio; obj; obj = obj->Depois)
            if (obj->pontlog)
            {
                safe_write(obj->arq, obj->buflog, obj->pontlog);
                obj->pontlog = 0;
            }
        Tempo = 20;
        return 600;
    }
// Obtém quanto tempo para gravar dados pendentes
    for (TVarLog * obj = Inicio; obj; obj = obj->Depois)
        if (obj->pontlog)
            return Tempo;
    Tempo = 20;
    return 600;
}

//------------------------------------------------------------------------------
void TVarLog::Fechar()
{
    if (arq < 0)
        return;
    if (pontlog)
        safe_write(arq, buflog, pontlog);
    close(arq);
    arq=-1;
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = nullptr;
}

//------------------------------------------------------------------------------
bool TVarLog::Func(TVariavel * v, const char * nome)
{
// Lista das funções de arqlog
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (TVarLog::*Func)(TVariavel * v); } ExecFunc[] = {
        { "abrir",     &TVarLog::FuncAbrir },
        { "existe",    &TVarLog::FuncExiste },
        { "fechar",    &TVarLog::FuncFechar },
        { "msg",       &TVarLog::FuncMsg },
        { "valido",    &TVarLog::FuncValido }  };
// Procura a função correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini + fim) / 2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado == 0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado < 0) fim = meio - 1; else ini = meio + 1;
    }
    return false;
}

//----------------------------------------------------------------------------
bool TVarLog::FuncMsg(TVariavel * v)
{
    if (arq < 0)
        return false;
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
        while (true)
        {
            while (*txt == Instr::ex_barra_n)
                txt++;
            if (*txt == 0)
                break;
            if (pontlog >= (int)sizeof(buflog) - 500)
            {
                safe_write(arq, buflog, pontlog);
                pontlog = 0;
            }
            for (int x = 0; x < 490 && *txt && *txt != Instr::ex_barra_n; x++)
                switch (*txt)
                {
                case Instr::ex_barra_b:
                    txt++;
                    break;
                case Instr::ex_barra_c:
                    if ((txt[1] >= '0' && txt[1] <= '9') ||
                            (txt[1] >= 'A' && txt[1] <= 'J') ||
                            (txt[1] >= 'a' && txt[1] <= 'j'))
                        txt += 2;
                    else
                        txt++;
                    break;
                case Instr::ex_barra_d:
                    if (txt[1] >= '0' && txt[1] <= '7')
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

//----------------------------------------------------------------------------
bool TVarLog::FuncFechar(TVariavel * v)
{
    Fechar();
    return false;
}

//----------------------------------------------------------------------------
bool TVarLog::FuncValido(TVariavel * v)
{
    char arqnome[300] = ""; // Nome do arquivo; nulo se não for válido
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome) - 4);
        if (!arqvalido(arqnome, false))
            *arqnome = 0;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(*arqnome != 0);
}

//----------------------------------------------------------------------------
bool TVarLog::FuncExiste(TVariavel * v)
{
    char arqnome[300] = ""; // Nome do arquivo; nulo se não for válido
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome) - 4);
        if (!arqvalido(arqnome, false))
            *arqnome = 0;
    }
    Instr::ApagarVar(v);
    if (*arqnome == 0)
        return Instr::CriarVarInt(0);
    int tam = strlen(arqnome);
    if (tam >= 4) // Tamanho suficiente
        if (comparaZ(arqnome + tam - 4, ".log") == 0) // Extensão correta
            tam -= 4;
    strcpy(arqnome + tam, ".log"); // Acerta a extensão
    struct stat buf;
    if (*arqnome && stat(arqnome, &buf) < 0)
        *arqnome = 0;
    return Instr::CriarVarInt(*arqnome != 0);
}

//----------------------------------------------------------------------------
bool TVarLog::FuncAbrir(TVariavel * v)
{
    char arqnome[300] = ""; // Nome do arquivo; nulo se não for válido
    if (Instr::VarAtual >= v + 1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome) - 4);
        if (!arqvalido(arqnome, false))
            *arqnome = 0;
    }
    if (*arqnome)
    {
    // Acerta o nome do arquivo
        int tam = strlen(arqnome);
        if (tam >= 4) // Tamanho suficiente
            if (comparaZ(arqnome + tam - 4, ".log") == 0) // Extensão correta
                tam -= 4;
        strcpy(arqnome + tam, ".log"); // Acerta a extensão
    // Abre arquivo
        int descr = open(arqnome, O_WRONLY|O_CREAT|O_APPEND, (mode_t)0666);
        if (descr < 0)
            *arqnome = 0;
        else
        {
        // Fecha arquivo
            Fechar();
        // Anota variáveis
            arq = descr;
            pontlog = 0;
        // Insere na lista ligada
            Antes = nullptr;
            Depois = Inicio;
            Inicio = this;
            if (Depois)
                Depois->Antes = this;
        }
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(*arqnome != 0);
}
