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
#include <assert.h>
#include "var-socket.h"
#include "socket.h"
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

//#define DEBUG_CRIAR  // Mostra objetos criados e apagados
//#define DEBUG_MSG    // Mostra o que enviou

enum {
SocketProto = 1,
SocketCores = 2,
SocketAFlooder = 3,
SocketOpcTelnet = 4,
SocketPosX
};

TObjSocket * TObjSocket::sockObj = 0;
TVarSocket * TObjSocket::varObj = 0;

//------------------------------------------------------------------------------
TObjSocket::TObjSocket()
{
#ifdef DEBUG_CRIAR
    printf("new TObjSocket\n"); fflush(stdout);
#endif
// Acerta vari�veis
    CorInic=-1;
    CorEnvia=0x70;
    ColunaEnvia=0;
    Inicio=0;
}

//------------------------------------------------------------------------------
TObjSocket::~TObjSocket()
{
#ifdef DEBUG_CRIAR
    printf("delete TObjSocket\n"); fflush(stdout);
#endif
    RetiraVarSocket();
}

//------------------------------------------------------------------------------
void TObjSocket::RetiraVarSocket()
{
// Retira dos objetos TVarSocket
// Nota: n�o pode chamar Inicio->MudarSock(0) porque
// isso executaria um segundo delete[] em TObjSocket
    while (Inicio)
    {
        TVarSocket * s = Inicio->Depois;
        Inicio->Socket = 0;
        Inicio->Antes = 0;
        Inicio->Depois = 0;
        if (TObjSocket::varObj == Inicio)
            TObjSocket::varObj = 0;
        Inicio = s;
    }

// Acerta sockObj
    if (sockObj==this)
        sockObj=0;
}

//------------------------------------------------------------------------------
void TObjSocket::Endereco(int num, char * mens, int total)
{
    *mens = 0;
}

//------------------------------------------------------------------------------
/** @param mensagem Endere�o dos bytes a enviar
 *  @return true se conseguiu enviar, false se n�o conseguiu */
bool TObjSocket::Enviar(const char * mensagem)
{
    int coluna = ColunaEnvia; // Coluna atual
    int agora = CorEnvia;
    int antes = CorEnvia;
    char mens[4096]; // Mensagem decodificada
    char * destino = mens; // Aonde colocar a mensagem
    const char * fim = mens + sizeof(mens) - 6; // Aonde termina

// Mostra o que est� enviando
#ifdef DEBUG_MSG
    printf(">>>>>>> Enviou: ");
    for (int x=0; mensagem[x]; x++)
        switch (mensagem[x])
        {
        case Instr::ex_barra_n: printf("\\n"); break;
        case Instr::ex_barra_b: printf("\\b"); break;
        case Instr::ex_barra_c: printf("\\c"); break;
        case Instr::ex_barra_d: printf("\\d"); break;
        case '\\':  printf("\\\\"); break;
        default: putchar(mensagem[x]);
        }
    putchar('\n');
#endif

// Decodifica mensagem
    for (; *mensagem; mensagem++)
        switch (*mensagem)
        {
        case Instr::ex_barra_b: // Branco com fundo preto
            agora = 0x70;
            break;
        case Instr::ex_barra_c: // Cor dos caracteres
            {
                unsigned char ch = *(++mensagem);
                if (ch>='0' && ch<='9')
                {
                    agora = (agora & 15) | (ch-'0') * 16;
                    break;
                }
                ch |= 0x20;
                if (ch < 'a' || ch > 'j')
                {
                    mensagem--;
                    break;
                }
                if (ch <= 'f')
                {
                    agora = (agora & 15) | (ch-'a'+10) * 16;
                    break;
                }
                if (agora != antes)
                {
                    destino[0] = 1;
                    destino[1] = agora;
                    destino+=2;
                    antes = agora;
                }
                if (destino > fim)
                    return false;
                *destino++ = ch + 2 - 'g';
                break;
            }
        case Instr::ex_barra_d: // Cor de fundo
            if (mensagem[1]>='0' && mensagem[1]<='7')
            {
                agora = (agora & 0xF0) | (mensagem[1]-'0');
                mensagem++;
            }
            break;
        case Instr::ex_barra_n: // Pr�xima linha
            if (agora != antes)
            {
                destino[0] = 1;
                destino[1] = agora;
                destino+=2;
                antes = agora;
            }
            if (destino > fim)
                return false;
            *destino++ = '\n';
            coluna=0;
            if (CorInic>=0)
                agora = antes = CorInic;
            break;
        default:
            if (*(unsigned char*)mensagem < ' ')
                break;
            if (agora != antes)
            {
                destino[0] = 1;
                destino[1] = agora;
                destino+=2;
                antes = agora;
            }
            if (destino > fim)
                return false;
            coluna++;
            *destino++ = *mensagem;
            break;
        }
// Acrescenta cor e 0 no final
    if (agora != antes)
    {
        if (destino > fim)
            return false;
        destino[0] = 1;
        destino[1] = agora;
        destino += 2;
    }
    *destino=0;

// Mostra o que est� enviando
#ifdef DEBUG_MSG
    for (const char * x=mens; *x; x++)
        if (*x!=1)
            putchar(*x);
        else
            { x++; printf("[%d]", (unsigned char)x[0]); }
    putchar('\n'); fflush(stdout);
#endif

// Tenta enviar mensagem
    if (EnvMens(mens) == false)
        return false;

// Conseguiu - acerta vari�veis
    ColunaEnvia = coluna;
    CorEnvia = agora;
    return true;
}

//------------------------------------------------------------------------------
void TObjSocket::FuncFechou(const char * txt)
{
    for (varObj = Inicio; varObj;)
    {
        if (varObj->b_objeto)
        {
            TObjeto * end = varObj->endobjeto;
            char mens[80];
            int indice = varObj->indice;
            sprintf(mens, "%s_fechou", varObj->defvar+Instr::endNome);
            varObj->MudarSock(0);
                // A partir daqui varObj pode ser nulo
            if (Instr::ExecIni(end, mens)==false)
                end->MarcarApagar();
            else
            {
                Instr::ExecArg(txt);
                Instr::ExecArg(indice);
                Instr::ExecX();
            }
            Instr::ExecFim();
        }
        else if (varObj->endclasse)
        {
            TClasse * end = varObj->endclasse;
            char mens[80];
            int indice = varObj->indice;
            sprintf(mens, "%s_fechou", varObj->defvar+Instr::endNome);
            varObj->MudarSock(0);
                // A partir daqui varObj pode ser nulo
            if (Instr::ExecIni(end, mens)==false)
                continue;
            Instr::ExecArg(indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
        else
            varObj->MudarSock(0);
    }
}

//------------------------------------------------------------------------------
bool TObjSocket::FuncEvento(const char * evento, const char * texto, int valor)
{
    //printf("FuncEvento [%s] [%s]\n", evento, texto); fflush(stdout);
    sockObj = this;
    for (TVarSocket * vobj = Inicio; vobj;)
    {
        bool prossegue = false;
    // Definido em objeto: prepara para executar
        if (vobj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar+Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endobjeto, mens);
        }
    // Definido em classe: prepara para executar
        else if (vobj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar+Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endclasse, mens);
        }
    // Executa
        varObj = vobj;
        if (prossegue)
        {
            if (texto)
                Instr::ExecArg(texto);
            if (valor>=0)
                Instr::ExecArg(valor);
            Instr::ExecArg(vobj->indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
    // Verifica se objeto foi apagado
    // Passa para pr�ximo objeto
        if (vobj == varObj)
            vobj = vobj->Depois;
        else if (sockObj)
            vobj = varObj;
        else
            return false;
    } // for (TVarSocket ...
    return true;
}

//------------------------------------------------------------------------------
void TVarSocket::MudarSock(TObjSocket * obj)
{
// Verifica se o endere�o vai mudar
    if (obj == Socket)
        return;
// Retira da lista
    if (Socket)
    {
        (Antes ? Antes->Depois : Socket->Inicio) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        if (Socket->Inicio == 0)
            Socket->Fechar();
    // Acerta TObjSocket::varObj
        if (TObjSocket::varObj == this)
            TObjSocket::varObj =Depois;
    }
// Coloca na lista
    if (obj)
    {
        Antes = 0;
        Depois = obj->Inicio;
        if (Depois)
            Depois->Antes = this;
        obj->Inicio = this;
    }
// Atualiza ponteiro
    Socket = obj;
}

//------------------------------------------------------------------------------
void TVarSocket::Mover(TVarSocket * destino)
{
    if (Socket)
    {
        (Antes ? Antes->Depois : Socket->Inicio) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    memmove(destino, this, sizeof(TVarSocket));
}

//------------------------------------------------------------------------------
void TVarSocket::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto=o, b_objeto=true;
    else
        endclasse=c, b_objeto=false;
}

//------------------------------------------------------------------------------
void TVarSocket::Igual(TVarSocket * v)
{
    MudarSock(v->Socket);
}

//------------------------------------------------------------------------------
bool TVarSocket::Func(TVariavel * v, const char * nome)
{
// Envia mensagem
    if (comparaZ(nome, "msg")==0)
    {
        if (Socket==0)
            return false;
        bool enviou = true;
        for (TVariavel * obj=v+1; obj<=Instr::VarAtual; obj++)
            if (Socket->Enviar(obj->getTxt())==false)
            {
                enviou = false;
                break;
            }
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(enviou);
    }
// Conecta
    int tipoabrir = 0;
    if (comparaZ(nome, "abrir")==0)
        tipoabrir = 1;
    else if (comparaZ(nome, "abrirssl")==0)
        tipoabrir = 2;
    if (tipoabrir)
    {
        MudarSock(0);
        if (Instr::VarAtual - v < 2)
            return false;
        int porta = v[2].getInt();
        const char * ender = v[1].getTxt();
#ifdef DEBUG_MSG
        printf(">>>>>>> Abrir(%s, %d)\n", ender, porta);
#endif
        if (tipoabrir == 2)
        {
            const char * err = AbreClienteSSL();
            if (err)
            {
                Instr::ApagarVar(v);
                return Instr::CriarVarInt(0);
            }
        }
        TSocket * s = TSocket::Conectar(ender, porta, tipoabrir==2);
        if (s)
            MudarSock(s);
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(s!=0);
    }
// Fecha Socket
    if (comparaZ(nome, "fechar")==0)
    {
        if (Socket)
            Socket->Fechar();
        return false;
    }
// Endere�o IP
    int x = 0;
    if (comparaZ(nome, "iplocal")==0)
        x = 1;
    else if (comparaZ(nome, "ip")==0)
        x = 2;
    else if (comparaZ(nome, "txtmd5")==0)
        x = 3;
    else if (comparaZ(nome, "txtsha1")==0)
        x = 4;
    if (x)
    {
        char mens[50];
        *mens=0;
        if (Socket)
            Socket->Endereco(x-1, mens, sizeof(mens));
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(mens);
    }
// Vari�veis
    x = 0;
    if (comparaZ(nome, "proto")==0)
        x = SocketProto;
    else if (comparaZ(nome, "cores")==0)
        x = SocketCores;
    else if (comparaZ(nome, "aflooder")==0)
        x = SocketAFlooder;
    else if (comparaZ(nome, "opctelnet")==0)
        x = SocketOpcTelnet;
    else if (comparaZ(nome, "posx")==0)
        x = SocketPosX;
    if (x)
    {
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = x;
        return true;
    }
// Abre biblioteca SSL
    if (comparaZ(nome, "inissl")==0)
    {
        const char * err = AbreClienteSSL();
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(err ? err : "");
    }
// Fun��o ou vari�vel desconhecida
    return false;
}

//------------------------------------------------------------------------------
int TVarSocket::getTipo(int numfunc)
{
    return (numfunc ? varInt : varOutros);
}

//------------------------------------------------------------------------------
int TVarSocket::getValor(int numfunc)
{
    if (Socket==0)
        return 0;
    switch (numfunc)
    {
    case 0:             return 1;
    case SocketPosX:    return Socket->ColunaEnvia;
    }
    return Socket->Variavel(numfunc, -1);
}

//------------------------------------------------------------------------------
void TVarSocket::setValor(int numfunc, int valor)
{
    if (Socket==0)
        return;
    switch (numfunc)
    {
    case 0:
        MudarSock(0);
        break;
    case SocketPosX:
        break;
    default:
        Socket->Variavel(numfunc, valor<0 ? 0 : valor);
    }
}
