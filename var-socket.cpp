/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
#else
 #include <errno.h>
 #include <sys/types.h>
 #include <sys/wait.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <fcntl.h>
 #include <netinet/in.h>
 #include <netdb.h>
#endif
#include "var-socket.h"
#include "socket.h"
#include "variavel.h"
#include "variavel-def.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "instr-enum.h"
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

TVarSocket * TObjSocket::varObj = nullptr;
TDNSSocket * TDNSSocket::Inicio = nullptr;

//------------------------------------------------------------------------------
TObjSocket::TObjSocket()
{
#ifdef DEBUG_CRIAR
    printf("new TObjSocket\n"); fflush(stdout);
#endif
// Acerta vari�veis
    CorEnvia = 0x70;
    ColunaEnvia = 0;
    Inicio = nullptr;
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
// Nota: n�o pode chamar Inicio->MudarSock(nullptr) porque
// isso executaria um segundo delete[] em TObjSocket
    while (Inicio)
    {
        TVarSocket * s = Inicio->Depois;
        Inicio->Socket = nullptr;
        Inicio->Antes = nullptr;
        Inicio->Depois = nullptr;
        if (TObjSocket::varObj == Inicio)
            TObjSocket::varObj = nullptr;
        Inicio = s;
    }
}

//------------------------------------------------------------------------------
void TObjSocket::Endereco(int num, char * mens, int total)
{
    *mens = 0;
}

//------------------------------------------------------------------------------
bool TObjSocket::Enviar(const char * mensagem, int codigo)
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
                if (ch >= '0' && ch <= '9')
                {
                    agora = (agora & ~0xF0) | (ch-'0') * 16;
                    break;
                }
                switch (ch | 0x20)
                {
                case 'a': agora = (agora | 0xF0) - 0x50; break;
                case 'b': agora = (agora | 0xF0) - 0x40; break;
                case 'c': agora = (agora | 0xF0) - 0x30; break;
                case 'd': agora = (agora | 0xF0) - 0x20; break;
                case 'e': agora = (agora | 0xF0) - 0x10; break;
                case 'f': agora = (agora | 0xF0); break;
                case 'g': agora |=  0x100; break;
                case 'h': agora &= ~0x100; break;
                case 'i': agora |=  0x200; break;
                case 'j': agora &= ~0x200; break;
                case 'k': agora |=  0x400; break;
                case 'l': agora &= ~0x400; break;
                case 'm':
                case 'n':
                case 'o':
                case 'p':
                    if (agora != antes)
                    {
                        destino[0] = 1;
                        destino[1] = agora;
                        destino[2] = agora >> 8;
                        destino += 3;
                        antes = agora;
                    }
                    if (destino > fim)
                        return false;
                    *destino++ = ch + 2 - 'm';
                    break;
                default:
                    mensagem--;
                }
                break;
            }
        case Instr::ex_barra_d: // Cor de fundo
            if (mensagem[1] >= '0' && mensagem[1] <= '7')
            {
                agora = (agora & ~0x0F) | (mensagem[1] - '0');
                mensagem++;
            }
            break;
        case Instr::ex_barra_n: // Pr�xima linha
            if (agora != antes)
            {
                destino[0] = 1;
                destino[1] = agora;
                destino[2] = agora >> 8;
                destino += 3;
                antes = agora;
            }
            if (destino > fim)
                return false;
            *destino++ = '\n';
            coluna = 0;
            break;
        default:
            if (*(unsigned char*)mensagem < ' ')
                break;
            if (agora != antes)
            {
                destino[0] = 1;
                destino[1] = agora;
                destino[2] = agora >> 8;
                destino += 3;
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
        destino[2] = agora >> 8;
        destino += 3;
    }
    *destino = 0;

// Mostra o que est� enviando
#ifdef DEBUG_MSG
    for (const char * x=mens; *x; x++)
        if (*x != 1)
            putchar(*x);
        else
            { x++; printf("[%d]", (unsigned char)x[0]); }
    putchar('\n'); fflush(stdout);
#endif

// Tenta enviar mensagem
    if (EnvMens(mens, codigo) == false)
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
            snprintf(mens, sizeof(mens), "%s_fechou", varObj->defvar + Instr::endNome);
            varObj->MudarSock(nullptr);
                // A partir daqui varObj pode ser nulo
            if (Instr::ExecIni(end, mens) == false)
                end->MarcarApagar();
            else
            {
                Instr::ExecArg(txt);
                Instr::ExecArg(indice);
                Instr::ExecX();
                Instr::ExecFim();
            }
        }
        else if (varObj->endclasse)
        {
            TClasse * end = varObj->endclasse;
            char mens[80];
            int indice = varObj->indice;
            snprintf(mens, sizeof(mens), "%s_fechou", varObj->defvar + Instr::endNome);
            varObj->MudarSock(nullptr);
                // A partir daqui varObj pode ser nulo
            if (Instr::ExecIni(end, mens) == false)
                continue;
            Instr::ExecArg(txt);
            Instr::ExecArg(indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
        else
            varObj->MudarSock(nullptr);
    }
}

//------------------------------------------------------------------------------
void TObjSocket::FuncEvento(const char * evento, const char * texto, int v1, int v2)
{
    //printf("FuncEvento [%s] [%s]\n", evento, texto); fflush(stdout);
    for (TVarSocket * vobj = Inicio; vobj;)
    {
        bool prossegue = false;
    // Definido em objeto: prepara para executar
        if (vobj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar + Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endobjeto, mens);
        }
    // Definido em classe: prepara para executar
        else if (vobj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar + Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endclasse, mens);
        }
    // Executa
        varObj = vobj->Depois;
        if (prossegue)
        {
            if (texto)
                Instr::ExecArg(texto);
            if (v1 >= 0)
                Instr::ExecArg(v1);
            if (v2 >= 0)
                Instr::ExecArg(v2);
            Instr::ExecArg(vobj->indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
    // Passa para pr�ximo objeto
        vobj = varObj;
    } // for (TVarSocket ...
}

//----------------------------------------------------------------------------
const TVarInfo * TVarSocket::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "abrir",        &TVarSocket::FuncAbrir },
        { "abrirssl",     &TVarSocket::FuncAbrirSsl },
        { "aflooder",     &TVarSocket::FuncAFlooder },
        { "cores",        &TVarSocket::FuncCores },
        { "eventoip",     &TVarSocket::FuncEventoIP },
        { "fechar",       &TVarSocket::FuncFechar },
        { "inissl",       &TVarSocket::FuncIniSSL },
        { "ip",           &TVarSocket::FuncIp },
        { "iplocal",      &TVarSocket::FuncIpLocal },
        { "ipnome",       &TVarSocket::FuncIPNome },
        { "ipvalido",     &TVarSocket::FuncIPValido },
        { "msg",          &TVarSocket::FuncMsg },
        { "nomeip",       &TVarSocket::FuncNomeIP },
        { "opctelnet",    &TVarSocket::FuncOpcTelnet },
        { "posx",         &TVarSocket::FuncPosX },
        { "proto",        &TVarSocket::FuncProto },
        { "txtmd5",       &TVarSocket::FuncMd5 },
        { "txtsha1",      &TVarSocket::FuncSha1 }  };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        FTipo,
        FRedim,
        FMoverEnd,
        FMoverDef,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        FOperadorIgual2,
        FOperadorCompara,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);

    return &var;
}

//------------------------------------------------------------------------------
void TVarSocket::Apagar()
{
    for (TDNSSocket * obj = TDNSSocket::Inicio; obj; obj = obj->Depois)
        if (obj->Socket == this)
            obj->Socket = nullptr;
    MudarSock(nullptr);
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
        if (Socket->Inicio == nullptr)
            Socket->Fechar();
    // Acerta TObjSocket::varObj
        if (TObjSocket::varObj == this)
            TObjSocket::varObj = Depois;
    }
// Coloca na lista
    if (obj)
    {
        Antes = nullptr;
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
    if (TObjSocket::varObj == this)
        TObjSocket::varObj = destino;
    for (TDNSSocket * obj = TDNSSocket::Inicio; obj; obj = obj->Depois)
        if (obj->Socket == this)
            obj->Socket = destino;
    memmove(destino, this, sizeof(TVarSocket));
}

//------------------------------------------------------------------------------
void TVarSocket::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto = o, b_objeto = true;
    else
        endclasse = c, b_objeto = false;
}

//----------------------------------------------------------------------------
// Envia mensagem: de longe � a fun��o mais usada
bool TVarSocket::FuncMsg(TVariavel * v)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    if (ref->Socket == nullptr)
        return false;
    bool enviou = true;
    int codigo = (Instr::VarAtual > v + 1 ? v[2].getInt() : 1);
    if (Instr::VarAtual >= v + 1)
        enviou = ref->Socket->Enviar(v[1].getTxt(), codigo);
    return Instr::CriarVarInt(v, enviou);
}

//----------------------------------------------------------------------------
/// Conecta
bool TVarSocket::FuncAbrir(TVariavel * v)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    ref->MudarSock(nullptr);
    if (Instr::VarAtual - v < 2)
        return false;
    int porta = v[2].getInt();
    const char * ender = v[1].getTxt();
#ifdef DEBUG_MSG
    printf(">>>>>>> Abrir(%s, %d)\n", ender, porta);
#endif
    TSocket * s = TSocket::Conectar(ender, porta, false);
    if (s)
        ref->MudarSock(s);
    return Instr::CriarVarInt(v, s != nullptr);
}

//----------------------------------------------------------------------------
/// Conecta
bool TVarSocket::FuncAbrirSsl(TVariavel * v)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    ref->MudarSock(nullptr);
    if (Instr::VarAtual - v < 2)
        return false;
    int porta = v[2].getInt();
    const char * ender = v[1].getTxt();
#ifdef DEBUG_MSG
    printf(">>>>>>> AbrirSsl(%s, %d)\n", ender, porta);
#endif
    const char * err = AbreClienteSSL();
    if (err)
        return Instr::CriarVarInt(v, 0);
    TSocket * s = TSocket::Conectar(ender, porta, true);
    if (s)
        ref->MudarSock(s);
    return Instr::CriarVarInt(v, s != nullptr);
}


//----------------------------------------------------------------------------
/// Fecha Socket
bool TVarSocket::FuncFechar(TVariavel * v)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    if (ref->Socket)
        ref->Socket->Fechar();
    return false;
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncIpLocal(TVariavel * v)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    char mens[50] = "";
    if (ref->Socket)
        ref->Socket->Endereco(0, mens, sizeof(mens));
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncIp(TVariavel * v)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    char mens[50] = "";
    if (ref->Socket)
        ref->Socket->Endereco(1, mens, sizeof(mens));
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncMd5(TVariavel * v)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    char mens[50] = "";
    if (ref->Socket)
        ref->Socket->Endereco(2, mens, sizeof(mens));
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncSha1(TVariavel * v)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    char mens[50] = "";
    if (ref->Socket)
        ref->Socket->Endereco(3, mens, sizeof(mens));
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncAFlooder(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = SocketAFlooder;
    return true;
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncCores(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = SocketCores;
    return true;
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncOpcTelnet(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = SocketOpcTelnet;
    return true;
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncPosX(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = SocketPosX;
    return true;
}

//----------------------------------------------------------------------------
bool TVarSocket::FuncProto(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = SocketProto;
    return true;
}

//----------------------------------------------------------------------------
/// Obter nome e gerar evento
bool TVarSocket::FuncEventoIP(TVariavel * v)
{
    if (Instr::VarAtual - v < 1)
        return false;
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar) + v->indice;
    TDNSSocket * obj = new TDNSSocket(ref, v[1].getTxt());
    if (*obj->nomeini)
        return Instr::CriarVarInt(v, 1);
    delete obj;
    return Instr::CriarVarInt(v, 0);
}

//----------------------------------------------------------------------------
/// Endere�o IP a partir do nome ou IP
bool TVarSocket::FuncNomeIP(TVariavel * v)
{
    if (Instr::VarAtual - v < 1)
        return false;
    char buf[4096];
    if (TSocket::NomeParaIps(v[1].getTxt(), buf, sizeof(buf)) != 0)
        return Instr::CriarVarTxtFixo(v, "");
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(buf);
}

//----------------------------------------------------------------------------
/// Nome a partir do nome ou IP
bool TVarSocket::FuncIPNome(TVariavel * v)
{
    if (Instr::VarAtual - v < 1)
        return false;
    char buf[4096];
    if (TSocket::IpParaNome(v[1].getTxt(), buf, sizeof(buf)) != 0)
        return Instr::CriarVarTxtFixo(v, "");
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(buf);
}

//----------------------------------------------------------------------------
/// Checa se � um endere�o IP v�lido
bool TVarSocket::FuncIPValido(TVariavel * v)
{
    if (Instr::VarAtual - v < 1)
        return false;
    int valido = TSocket::IpValido(v[1].getTxt());
    return Instr::CriarVarInt(v, valido);
}

//----------------------------------------------------------------------------
/// Abre biblioteca SSL
bool TVarSocket::FuncIniSSL(TVariavel * v)
{
    const char * err = AbreClienteSSL();
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(err ? err : "");
}

//------------------------------------------------------------------------------
TVarTipo TVarSocket::FTipo(TVariavel * v)
{
    return (v->numfunc ? varInt : varOutros);
}

//------------------------------------------------------------------------------
int TVarSocket::getValor(int numfunc)
{
    if (Socket == nullptr)
        return 0;
    switch (numfunc)
    {
    case 0:             return 1;
    case SocketPosX:    return Socket->ColunaEnvia;
    }
    return Socket->Variavel(numfunc, -1);
}

//------------------------------------------------------------------------------
int TVarSocket::FTamanho(const char * instr)
{
    return sizeof(TVarSocket);
}

//------------------------------------------------------------------------------
int TVarSocket::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarSocket);
}

//------------------------------------------------------------------------------
void TVarSocket::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarSocket * ref = reinterpret_cast<TVarSocket*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].Socket = nullptr;
        ref[antes].defvar = v->defvar;
        ref[antes].indice = antes;
        ref[antes].EndObjeto(c, o);
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TVarSocket::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_COMPLETO(TVarSocket)
}

//------------------------------------------------------------------------------
void TVarSocket::FMoverDef(TVariavel * v)
{
    VARIAVEL_MOVERDEF(TVarSocket)
}

//------------------------------------------------------------------------------
bool TVarSocket::FGetBool(TVariavel * v) VARIAVEL_FGETINT1(TVarSocket)
int TVarSocket::FGetInt(TVariavel * v) VARIAVEL_FGETINT1(TVarSocket)
double TVarSocket::FGetDouble(TVariavel * v) VARIAVEL_FGETINT1(TVarSocket)
const char * TVarSocket::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT1(TVarSocket)

//------------------------------------------------------------------------------
void TVarSocket::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TVarSocket * r1 = reinterpret_cast<TVarSocket*>(v1->endvar) + v1->indice;
    if (v1->numfunc == 0)
    {
        if (v1->defvar[2] != v2->defvar[2])
            return;
        TVarSocket * r2 = reinterpret_cast<TVarSocket*>(v2->endvar) + v2->indice;
        if (r1 != r2)
            r1->MudarSock(r2->Socket);
    }
    else if (v1->numfunc != SocketPosX && r1->Socket != nullptr)
    {
        int valor = v2->getInt();
        r1->Socket->Variavel(v1->numfunc, valor < 0 ? 0 : valor);
    }
}

//------------------------------------------------------------------------------
bool TVarSocket::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    TVarSocket * ref1 = reinterpret_cast<TVarSocket*>(v1->endvar) + v1->indice;
    if (v1->numfunc != 0)
        return v2->TipoNumerico() && ref1->getValor(v1->numfunc) == v2->getDouble();
    if (ref1->Socket == nullptr)
        return false;
    TVarSocket * ref2 = reinterpret_cast<TVarSocket*>(v2->endvar) + v2->indice;
    return v1->defvar[2] == v2->defvar[2] && v2->numfunc == 0 &&
            ref1->Socket == ref2->Socket;
}

//------------------------------------------------------------------------------
unsigned char TVarSocket::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    TVarSocket * ref1 = reinterpret_cast<TVarSocket*>(v1->endvar) + v1->indice;
    if (v1->numfunc != 0)
        return TVarInfo::ComparaInt(ref1->getValor(v1->numfunc), v2);
    if (ref1->Socket == nullptr)
        return 0;
    TVarSocket * ref2 = reinterpret_cast<TVarSocket*>(v2->endvar) + v2->indice;
    return v1->defvar[2] == v2->defvar[2] && v2->numfunc == 0 &&
            ref1->Socket == ref2->Socket ? 2 : 0;
}

//------------------------------------------------------------------------------
void TDNSSocket::ResolveDNS()
{
    if (TSocket::IpValido(nomeini))
    {
        if (TSocket::IpParaNome(nomeini, nomefim + 1, sizeof(nomefim) - 1) == 0)
        {
            nomefim[0] = 'N';
            return;
        }
    }
    else
    {
        if (TSocket::NomeParaIps(nomeini, nomefim + 1, sizeof(nomefim) - 1) == 0)
        {
            nomefim[0] = 'I';
            return;
        }
    }
    nomefim[0] = ' ';
    nomefim[1] = 0;
}

//------------------------------------------------------------------------------
#ifdef __WIN32__
/// Resolve endere�o em segundo plano no Windows
static DWORD WINAPI TDNSSocket_Resolve(LPVOID lpParam)
{
    TDNSSocket * obj = static_cast<TDNSSocket *>(lpParam);
    obj->ResolveDNS();
    return 0;
}
#endif

//------------------------------------------------------------------------------
#ifndef __WIN32__
/// Interpreta sinal SIGCHLD
// Nota: J� est� sendo processado em TExec::proc_sigchld_handler()
/*static void dns_sigchld_handler(int signum)
{
    int pid, status, serrno;
    serrno = errno;
    while (true)
    {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid < 0 && errno != ECHILD) // -1 significa que ocorreu algum erro
        {
            //perror("waitpid");
            break;
        }
        if (pid <= 0)
            break;
        //notice_termination(pid, status);
    }
    errno = serrno;
    signal(SIGCHLD,dns_sigchld_handler); // Processar sinal SIGCHLD
}*/
#endif

//------------------------------------------------------------------------------
TDNSSocket::TDNSSocket(TVarSocket * var, const char * ender)
{
    Socket = var;
    Antes = nullptr;
    Depois = Inicio;
    if (Depois)
        Depois->Antes = this;
    Inicio = this;
    if (*ender == 0)
        return;
    *nomeini = 0;
    nomefim[0] = 0;
    nomefim[1] = 0;
    copiastr(nomeini, ender, sizeof(nomeini));
    if (*nomeini == 0)
        return;
#ifdef __WIN32__
    hthread = CreateThread(nullptr, 0, &TDNSSocket_Resolve,
                           reinterpret_cast<DWORD*>(this), 0, nullptr);
    if (hthread == nullptr)
        *nomeini = 0;
#else
        // Cria pipes
    int descrpipe[2];
    if (pipe(descrpipe) < 0)
    {
        *nomeini = 0;
        return;
    }
        // Cria um novo processo
    pid_t pid = fork();
    if (pid < 0) // Erro
    {
        close(descrpipe[0]);
        close(descrpipe[1]);
        *nomeini = 0;
        return;
    }
    if (pid == 0) // Processo filho
    {
        close(descrpipe[0]);
        ResolveDNS();
        safe_write(descrpipe[1], nomefim, strlen(nomefim) + 1);
        _exit(EXIT_SUCCESS);
    }
    close(descrpipe[1]);
    fcntl(descrpipe[0], F_SETFL, O_NONBLOCK);
    //signal(SIGCHLD,dns_sigchld_handler); // Processar sinal SIGCHLD
    // Nota: J� est� sendo processado em TExec::proc_sigchld_handler()
    recdescr = descrpipe[0];
#endif
}

//------------------------------------------------------------------------------
TDNSSocket::~TDNSSocket()
{
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
}

//------------------------------------------------------------------------------
void TDNSSocket::Fd_Set(fd_set * set_entrada)
{
#ifndef __WIN32__
    for (TDNSSocket * obj = Inicio; obj; obj = obj->Depois)
        FD_SET(obj->recdescr, set_entrada);
#endif
}

//------------------------------------------------------------------------------
void TDNSSocket::ProcEventos(fd_set * set_entrada)
{
    for (TDNSSocket * obj = Inicio; obj;)
    {
        bool prossegue = false;
    // Checa se j� tem todas as informa��es
#ifdef __WIN32__
        if (WaitForSingleObject(obj->hthread, 0) == WAIT_TIMEOUT)
        {
            obj = obj->Depois;
            continue;
        }
        CloseHandle(obj->hthread);
#else
        int resposta =  read(obj->recdescr, &obj->nomefim, sizeof(obj->nomefim));
        if (resposta < 0 && (errno==EINTR || errno==EWOULDBLOCK))
        {
            obj = obj->Depois;
            continue;
        }
        close(obj->recdescr);
        if (resposta < 0)
            resposta = 0;
        if (resposta >= (int)sizeof(obj->nomefim))
            resposta = (int)sizeof(obj->nomefim) - 1;
        obj->nomefim[resposta] = 0;
#endif
    // Checa se pode gerar evento
        TVarSocket * vobj = obj->Socket;
        if (vobj == nullptr)
            ;
    // Definido em objeto: prepara para executar
        else if (vobj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_eventoip", vobj->defvar + Instr::endNome);
            prossegue = Instr::ExecIni(vobj->endobjeto, mens);
        }
    // Definido em classe: prepara para executar
        else if (vobj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_eventoip", vobj->defvar + Instr::endNome);
            prossegue = Instr::ExecIni(vobj->endclasse, mens);
        }
    // Executa
        if (prossegue)
        {
            Instr::ExecArg(obj->nomeini); // Nome pesquisado
            if (obj->nomefim[0] == 'N')
            {
                Instr::ExecArg(strcmp(obj->nomeini, obj->nomefim + 1) == 0 ?
                        "" : obj->nomefim + 1);
                Instr::ExecArg(obj->nomeini);
            }
            else if (obj->nomefim[0] == 'I')
            {
                Instr::ExecArg(obj->nomeini);
                Instr::ExecArg(obj->nomefim + 1);
            }
            else
            {
                Instr::ExecArg("");
                Instr::ExecArg("");
            }
            Instr::ExecArg(vobj->indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
    // Passa para o pr�ximo objeto
        TDNSSocket * obj2 = obj;
        obj = obj->Depois;
        delete obj2;
    }
}
