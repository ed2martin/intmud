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
#include "exec.h"
#include "var-exec.h"
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

TArqExec * TArqExec::Inicio = nullptr;
bool TObjExec::boolenvpend = false;
TObjExec * TObjExec::Inicio = nullptr;

//------------------------------------------------------------------------------
TArqExec::TArqExec(const char * texto)
{
    Anterior = nullptr;
    Proximo = Inicio;
    if (Proximo)
        Proximo->Anterior = this;
    Inicio = this;
    int tam = strlen(texto) + 1;
    Nome = new char[tam];
    memcpy(Nome, texto, tam);
}

//------------------------------------------------------------------------------
TArqExec::~TArqExec()
{
    (Anterior ? Anterior->Proximo : Inicio) = Proximo;
    if (Proximo)
        Proximo->Anterior = Anterior;
}

//------------------------------------------------------------------------------
TObjExec::TObjExec(TVarExec * var)
{
    assert(var->ObjExec == nullptr);
    var->ObjExec = this;
    VarExec = var;
    pontEnv = 0;
    dadoRecebido = 0;
// Coloca na lista ligada
    Antes = nullptr;
    Depois = Inicio;
    if (Inicio)
        Inicio->Antes = this;
    Inicio = this;
}

//------------------------------------------------------------------------------
TObjExec::~TObjExec()
{
    if (VarExec)
        VarExec->ObjExec = nullptr;
// Retira da lista ligada
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
}

//------------------------------------------------------------------------------
bool TObjExec::Enviar(const char * txt)
{
    char buf[EXEC_ENV];
    int  tamanho = 0;
// Prepara mensagem
    for (; *txt && tamanho < EXEC_ENV - 4; txt++)
    {
        switch (*txt)
        {
        case Instr::ex_barra_b:
            break;
        case Instr::ex_barra_c:
            if ((txt[1] >= '0' && txt[1] <= '9') ||
                    (txt[1] >= 'A' && txt[1] <= 'J') ||
                    (txt[1] >= 'a' && txt[1] <= 'j'))
                txt++;
            break;
        case Instr::ex_barra_d:
            if (txt[1] >= '0' && txt[1] <= '7')
                txt++;
            break;
        case Instr::ex_barra_n:
#ifdef __WIN32__
            buf[tamanho++] = 13;
#endif
            buf[tamanho++] = 10;
            break;
        default:
            buf[tamanho++] = *txt;
        }
    }
// Envia a mensagem
    boolenvpend = true;
    if (tamanho > EXEC_ENV - (int)pontEnv)
    {
        EnvPend();
        if (tamanho > EXEC_ENV - (int)pontEnv)
            return false;
    }
    memcpy(bufEnv + pontEnv, buf, tamanho);
    pontEnv += tamanho;
    return true;
}

//------------------------------------------------------------------------------
void TObjExec::EnvPend()
{
    int resposta = Escrever(bufEnv, pontEnv);
    if (resposta > 0)
    {
        pontEnv -= resposta;
        memmove(bufEnv, bufEnv+resposta, pontEnv);
    }
}

//------------------------------------------------------------------------------
void TObjExec::Receber()
{
    char bufRec[4096];  // Contém a mensagem recebida
    unsigned int pontRec = 0; // Número do byte que está lendo

    while (true)
    {
        char buf[4096];
        int lido = Ler(buf, sizeof(buf));
        for (int cont = 0; cont < lido; cont++)
        {
            unsigned char dado = buf[cont];
            //if (dado < ' ' && pontRec < sizeof(bufRec)-5)
            //{
            //    sprintf(bufRec+pontRec, "\\%02X", dado);
            //    pontRec+=3;
            //}
            if (dado >= ' ')
            {
                dadoRecebido = 0;
                bufRec[pontRec++] = dado;
                if (pontRec < sizeof(bufRec) - 1)
                    continue;
            }
            else if (dado != 10 && dado != 13)
            {
                dadoRecebido = 0;
                continue;
            }
            else if (dado == 13 ? dadoRecebido == 10 : dadoRecebido == 13)
            {
                dadoRecebido = 0;
                continue;
            }
            dadoRecebido = dado;
            bufRec[pontRec] = 0;
            pontRec = 0;
            if (VarExec)
                VarExec->FuncEvento("msg", bufRec, dado == 13 || dado == 10);
        }
        if (lido != sizeof(buf))
            break;
    }

    if (pontRec != 0 && VarExec != nullptr)
    {
        bufRec[pontRec] = 0;
        VarExec->FuncEvento("msg", bufRec, 0);
    }
}

//------------------------------------------------------------------------------
void TObjExec::Fd_Set(fd_set * set_entrada, fd_set * set_saida)
{
    while (true)
    {
        boolenvpend = false;

    // Envia dados pendentes; apaga objeto se necessário
        for (TObjExec * obj = Inicio; obj; )
        {
        // Verifica se tem dados pendentes para transmitir
            bool ev_env = false;
            if (obj->pontEnv)
            {
                obj->EnvPend();
                ev_env = (obj->pontEnv <= 0);
            }
        // Verifica evento env
            if (ev_env && obj->VarExec)
            {
                obj->VarExec->FuncEvento("env", nullptr, -1);
                continue;
            }
        // Checa se deve apagar o objeto, passa para o próximo
            if (obj->VarExec == nullptr)
            {
                TObjExec * proximo = obj->Depois;
                delete obj;
                obj = proximo;
                continue;
            }
            else
                obj = obj->Depois;
        }

        if (!boolenvpend)
            break;
    }

#ifndef __WIN32__
    for (TObjExec * obj = Inicio; obj; obj = obj->Depois)
    {
        int entra = obj->pipein();
        int sai = obj->pipeout();
        if (entra >= 0)
            FD_SET(entra, set_entrada);
        if (obj->pontEnv && sai >= 0)
            FD_SET(entra, set_saida);
    }
#endif
}

//------------------------------------------------------------------------------
void TObjExec::ProcEventos(fd_set * set_entrada, fd_set * set_saida)
{
    for (TObjExec * obj = Inicio; obj; obj = obj->Depois)
    {
    // Checa se chegou alguma coisa
        obj->Receber();
        if (obj->VarExec == nullptr)
            continue;

    // Checa se o programa fechou
        if (obj->InfoProg() == 1)
            continue;
        obj->Receber(); // Recebe dados pendentes
        if (obj->VarExec)
            obj->VarExec->FuncEvento("fechou", nullptr, obj->CodRetorno);
        if (obj->VarExec)
        {
            obj->VarExec->ObjExec = nullptr; // Indica que o programa fechou
            obj->VarExec = nullptr;
        }
    }
}

//------------------------------------------------------------------------------
void TVarExec::Apagar()
{
    if (ObjExec)
        ObjExec->VarExec = nullptr;
}

//------------------------------------------------------------------------------
void TVarExec::Mover(TVarExec * destino)
{
    if (ObjExec)
        ObjExec->VarExec = destino;
    memmove(destino, this, sizeof(TVarExec));
}

//------------------------------------------------------------------------------
void TVarExec::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto = o, b_objeto = true;
    else
        endclasse = c, b_objeto = false;
}

//------------------------------------------------------------------------------
void TVarExec::FuncEvento(const char * evento, const char * texto, int valor)
{
    //printf("FuncEvento [%s] [%s]\n", evento, texto); fflush(stdout);
    bool prossegue = false;
// Definido em objeto: prepara para executar
    if (b_objeto)
    {
        char mens[80];
        sprintf(mens, "%s_%s", defvar+Instr::endNome, evento);
        prossegue = Instr::ExecIni(endobjeto, mens);
    }
// Definido em classe: prepara para executar
    else if (endclasse)
    {
        char mens[80];
        sprintf(mens, "%s_%s", defvar+Instr::endNome, evento);
        prossegue = Instr::ExecIni(endclasse, mens);
    }
// Executa
    if (prossegue)
    {
        if (texto)
            Instr::ExecArg(texto);
        if (valor >= 0)
            Instr::ExecArg(valor);
        Instr::ExecArg(indice);
        Instr::ExecX();
        Instr::ExecFim();
    }
}

//------------------------------------------------------------------------------
bool TVarExec::Func(TVariavel * v, const char * nome)
{
// Envia mensagem
    if (comparaZ(nome, "msg") == 0)
    {
        if (ObjExec == nullptr || Instr::VarAtual != v + 1)
            return false;
        bool enviou = ObjExec->Enviar(v[1].getTxt());
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(enviou);
    }
// Começa a executar
    if (comparaZ(nome, "abrir") == 0)
    {
        if (Instr::VarAtual < v + 1)
            return false;
        if (ObjExec)
        {
            ObjExec->VarExec = nullptr;
            ObjExec = nullptr;
        }
        const char * cmd = v[1].getTxt();
        if (cmd == nullptr || *cmd == 0)
            return false;
        if (!opcao_completo)
        {
            int tamcmd = strlen(cmd);
            TArqExec * obj = TArqExec::ExecIni();
            for (; obj; obj = obj->ExecProximo())
            {
                const char * nome = obj->ExecNome();
                int tamnome = strlen(nome);
                if (tamnome == 0)
                    continue;
                if (nome[tamnome - 1] == '*')
                {
                    tamnome--;
                    if (tamcmd < tamnome)
                        continue;
                }
                else if (tamcmd != tamnome)
                    continue;
                if (memcmp(cmd, nome, tamnome) == 0)
                    break;
            }
            if (obj == nullptr)
            {
                Instr::ApagarVar(v);
                return Instr::CriarVarTexto("ArqExec não pode executar isso");
            }
        }
        int visivel = (Instr::VarAtual >= v+2 ? v[2].getInt() : 0);
        ObjExec = new TObjExec(this);
        const char * err = ObjExec->Abrir(cmd, visivel == 1);
        Instr::ApagarVar(v);
        if (err)
            delete ObjExec;
        return Instr::CriarVarTexto(err ? err : "");
    }
// Encerra a execução
    if (comparaZ(nome, "fechar") == 0)
    {
        if (ObjExec)
        {
            ObjExec->VarExec = nullptr;
            ObjExec = nullptr;
        }
        return false;
    }
// Checa se está aberto
    if (comparaZ(nome, "aberto") == 0)
    {
        bool aberto = (ObjExec != nullptr);
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(aberto);
    }
    return false;
}
