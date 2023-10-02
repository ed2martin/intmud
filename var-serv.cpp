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
#ifdef __WIN32__
 #include <windows.h>
 #include <winsock.h>
 #include <fcntl.h>
 #include <io.h>
 #define close closesocket
#else
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <netinet/in.h> // Contém a estrutura sockaddr_in
 #include <string.h>
 #include <sys/time.h>
 #include <sys/types.h>
 #include <sys/socket.h> // Comunicação
 #include <signal.h>
 #include <netinet/in.h> // Contém a estrutura sockaddr_in
 #include <sys/un.h>     // Contém a estrutura sockaddr_un
 #include <errno.h>
#endif
#include <assert.h>
#include "config.h"
#include "var-serv.h"
#include "var-socket.h"
#include "socket.h"
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "instr-enum.h"
#include "misc.h"

//#define DEBUG_CRIAR  // Mostra objetos criados e apagados

TVarServ * TVarServ::Inicio = nullptr;
TVarServ * TVarServ::varObj = nullptr;

//------------------------------------------------------------------------------
TVarServObj::TVarServObj(TVarServ * serv, int s)
{
#ifdef DEBUG_CRIAR
    puts("new TVarServObj"); fflush(stdout);
#endif
// Coloca na lista ligada
    Serv = serv;
    Antes = Serv->ServFim;
    Depois = nullptr;
    (Antes ? Antes->Depois : Serv->ServInicio) = this;
    Serv->ServFim = this;
// Acerta outras variáveis
    sock = s;
    acaossl = 0;
    tempo = 100;
// Conexão segura
    sockssl = SslNew(ssl_ctx_servidor);
    SslSetFd(sockssl, s);
}

//------------------------------------------------------------------------------
TVarServObj::~TVarServObj()
{
// Tira da lista ligada
    (Antes ? Antes->Depois : Serv->ServInicio) = Depois;
    (Depois ? Depois->Antes : Serv->ServFim) = Antes;
// Apaga objeto SSL e fecha socket
    if (sockssl)
    {
        SslShutdown(sockssl);
        SslFree(sockssl);
    }
    if (sock >= 0)
        close(sock);
#ifdef DEBUG_CRIAR
    printf("delete TVarServObj%s%s\n", sock >= 0 ? " socket" : "",
         sockssl ? " ssl" : ""); fflush(stdout);
#endif
}

//----------------------------------------------------------------------------
const TVarInfo * TVarServ::Inicializa()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

//------------------------------------------------------------------------------
void TVarServ::Criar()
{
#ifdef DEBUG_CRIAR
    puts("new TVarServ"); fflush(stdout);
#endif
    sock = -1;
}

//------------------------------------------------------------------------------
void TVarServ::Apagar()
{
#ifdef DEBUG_CRIAR
    puts("delete TVarServ"); fflush(stdout);
#endif
    Fechar();
}

//------------------------------------------------------------------------------
void TVarServ::Fechar()
{
    if (sock < 0)
        return;
    close(sock);
    sock = -1;
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    if (varObj == this)
        varObj = Depois;
    while (ServInicio)
        delete ServInicio;
}

//------------------------------------------------------------------------------
void TVarServ::Mover(TVarServ * destino)
{
    if (sock >= 0)
    {
        (Antes ? Antes->Depois : Inicio) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    if (varObj == this)
        varObj = destino;
    memmove(destino, this, sizeof(TVarServ));
}

//------------------------------------------------------------------------------
void TVarServ::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto = o, b_objeto = true;
    else
        endclasse = c, b_objeto = false;
}

//------------------------------------------------------------------------------
bool TVarServ::Abrir(const char * ender, unsigned short porta)
{
    struct sockaddr_in strSock;
    struct hostent *hnome;

    while (true)
    {
// Fecha socket
        Fechar();

#ifdef __WIN32__
    // Windows: Cria socket
        if ( (sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET )
        {
            sock = -1;
            break;
        }
        //BOOL iopcoes = 1;
        //int  tamanho = sizeof(iopcoes);
        //setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, (const char*)(void*)&iopcoes, tamanho);

    // Windows: Seleciona o endereço
        memset(&(strSock.sin_zero), 0, 8);
        strSock.sin_family = AF_INET;
        strSock.sin_addr.s_addr = htonl(INADDR_ANY);
        if (*ender)
        {
            strSock.sin_addr.s_addr = inet_addr(ender);
            if ( strSock.sin_addr.s_addr == INADDR_NONE )
            {
                if ( (hnome=gethostbyname(ender)) == NULL )
                    break;
                strSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
            }
        }

    // Windows: Seleciona a porta
        strSock.sin_port = htons((u_short)porta);
        if ( bind(sock, (LPSOCKADDR)&strSock, sizeof(strSock)) )
            break;

    // Windows: Escuta na porta selecionada
        if (listen(sock, 15))
            break;

    // Windows: Modo de não bloqueio
        unsigned long argp = 1; // 0=bloquear  1=nao bloquear
        if ( ioctlsocket(sock, FIONBIO, &argp) != 0 )
            break;
#else
    // Unix: Cria socket
        int  iopcoes = 1;
        ACCEPT_TYPE_ARG3 tamanho = sizeof(iopcoes);
        if ( (sock = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
            break;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &iopcoes, tamanho);

    // Unix: Seleciona o endereço
        memset(&(strSock.sin_zero), 0, 8);
        strSock.sin_family = AF_INET;
        strSock.sin_addr.s_addr = htonl(INADDR_ANY);
        if (*ender)
            if (inet_aton(ender, &strSock.sin_addr) == 0)
            {
                if ( (hnome = gethostbyname(ender)) == nullptr )
                    break;
                strSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
            }

    // Unix: Seleciona a porta
        strSock.sin_port = htons(porta);
        if ( bind(sock, (struct sockaddr *)&strSock,
                            sizeof(struct sockaddr)) < 0)
            break;

    // Unix: Escuta na porta selecionada
        if (listen(sock, 15) < 0)
            break;

    // Unix: Modo de não bloqueio
        fcntl(sock, F_SETFL, O_NONBLOCK);
#endif

    // Coloca na lista ligada
        Antes = nullptr;
        Depois = Inicio;
        if (Depois)
            Depois->Antes = this;
        Inicio = this;
        return true;
    }

// Ocorreu algum erro
    // UNIX: o erro está em strerror(errno)
    // Windows: chamar WSAGetLastError() e obter o erro de uma tabela
    if (sock >= 0)
        close(sock);
    sock = -1;
    return false;
}

//------------------------------------------------------------------------------
int TVarServ::Fd_Set(fd_set * set_entrada, fd_set * set_saida)
{
    int tempo = 1000;
    for (TVarServ * s = Inicio; s; s=s->Depois)
    {
        FD_SET(s->sock, set_entrada);
        for (TVarServObj * x = s->ServInicio; x; x = x->Depois)
        {
            FD_SET(x->sock, x->acaossl ? set_saida : set_entrada);
            if (tempo > x->tempo)
                tempo = x->tempo;
        }
    }
    return tempo;
}

//------------------------------------------------------------------------------
void TVarServ::ProcEventos(fd_set * set_entrada, int tempo)
{
// Recebe conexões
    for (TVarServ * obj = Inicio; obj; )
    {
    // Verifica se tem conexões pendentes
        if (FD_ISSET(obj->sock, set_entrada) == 0)
        {
            obj = obj->Depois;
            continue;
        }
    // Recebe conexões
        varObj = obj;
        while (obj == varObj)
        {
            struct sockaddr_in SockStr_in;
#ifdef __WIN32__
            int tamsock = sizeof(SockStr_in);
            unsigned int localSocket = accept(obj->sock,
                (struct sockaddr *)&SockStr_in, &tamsock);
            if (localSocket == INVALID_SOCKET)
                break;
#else
            ACCEPT_TYPE_ARG3 tamsock = sizeof(SockStr_in);
            int localSocket = accept(obj->sock,
                (ACCEPT_TYPE_ARG2)&SockStr_in, &tamsock);
            if (localSocket < 0)
                break;
#endif
            TSocket::SockConfig(localSocket); // Acerta socket

            if (obj->modossl)
                new TVarServObj(obj, localSocket); // Conexão segura
            else
                obj->ExecEvento(localSocket, 0); // Gera evento
        }
    // Fecha o laço
        if (obj == varObj)
            obj = obj->Depois;
        else
            obj = varObj;
    }
// Negocia conexões SSL
    for (TVarServ * obj = Inicio; obj; )
    {
        varObj = obj;
        for (TVarServObj * x = obj->ServInicio; x; )
        {
        // Checa tempo tentando conectar
            if (x->tempo <= tempo)
            {
                TVarServObj * x2 = x->Depois;
                delete x;
                x = x2;
                continue;
            }
            x->tempo -= tempo;
        // Prossegue com a conexão
            int resposta = SslAccept(x->sockssl);
            if (resposta > 0) // Conexão realizada
            {
            // Apaga objeto x sem fechar a conexão
                int local_sock = x->sock;
                SSL * local_ssl = x->sockssl;
                TVarServObj * x2 = x->Depois;
                x->sock = -1;
                x->sockssl = nullptr;
                delete x;
            // Passa para o próximo objeto
                x = x2;
            // Executa evento
                obj->ExecEvento(local_sock, local_ssl);
                if (varObj == obj)
                    continue;
                break;
            }
            switch (SslGetError(x->sockssl, resposta))
            //switch (SslGetError(x->sockssl, resposta))
            {
            case SSL_ERROR_WANT_READ: // Quer ler
                x->acaossl = 0;
                x = x->Depois;
                break;
            case SSL_ERROR_WANT_WRITE: // Quer enviar
                x->acaossl = 1;
                x = x->Depois;
                break;
            default: // Ocorreu um erro qualquer
              {
                TVarServObj * x2 = x->Depois;
                delete x;
                x = x2;
                continue;
              }
            } // switch
        } // while
        if (varObj == obj)
            obj = obj->Depois;
        else
            obj = varObj;
    } // for
}

//------------------------------------------------------------------------------
// Gera evento
void TVarServ::ExecEvento(int localSocket, SSL * sslSocket)
{
    bool prossegue = false;
    if (b_objeto)
    {
        char mens[80];
        sprintf(mens, "%s_socket", defvar+Instr::endNome);
        prossegue = Instr::ExecIni(endobjeto, mens);
    }
    else if (endclasse)
    {
        char mens[80];
        sprintf(mens, "%s_socket", defvar+Instr::endNome);
        prossegue = Instr::ExecIni(endclasse, mens);
    }
    if (prossegue == false)
    {
        if (sslSocket)
        {
            SslShutdown(sslSocket);
            SslFree(sslSocket);
        }
        close(localSocket);
        return;
    }
        // Cria argumento: TVarSocket
    Instr::ExecArgCriar(Instr::InstrSocket);
    TVariavel * v = Instr::VarAtual;
        // Cria argumento: índice
    Instr::ExecArg(indice);
        // Cria TObjSocket com o socket
    TSocket * s = new TSocket(localSocket, sslSocket);
        // Coloca TObjSocket em TVarSocket
    v->end_socket->MudarSock(s);
        // Executa
    Instr::ExecX();
    Instr::ExecFim();
}

//------------------------------------------------------------------------------
bool TVarServ::Func(TVariavel * v, const char * nome)
{
// Lista das funções de serv
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (TVarServ::*Func)(TVariavel * v); } ExecFunc[] = {
        { "abrir",        &TVarServ::FuncAbrir },
        { "abrirssl",     &TVarServ::FuncAbrirSSL },
        { "fechar",       &TVarServ::FuncFechar },
        { "inissl",       &TVarServ::FuncIniSSL }  };
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

//------------------------------------------------------------------------------
bool TVarServ::FuncFechar(TVariavel * v)
{
    Fechar();
    return false;
}

//------------------------------------------------------------------------------
bool TVarServ::FuncAbrir(TVariavel * v)
{
    if (Instr::VarAtual - v < 2)
        return false;
    int porta = v[2].getInt();
    const char * ender = v[1].getTxt();
    int valor = 0;
    if (porta >= 0 && porta <= 65535)
    {
        modossl = false;
        valor = Abrir(ender, porta);
//printf("%s %d -> %d\n\n", ender, porta, valor); fflush(stdout);
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(valor);
}

//------------------------------------------------------------------------------
bool TVarServ::FuncAbrirSSL(TVariavel * v)
{
    if (Instr::VarAtual - v < 2)
        return false;
    int porta = v[2].getInt();
    const char * ender = v[1].getTxt();
    int valor = 0;
    if (porta >= 0 && porta <= 65535 && ssl_ctx_servidor)
    {
        modossl = true;
        valor = Abrir(ender, porta);
//printf("%s %d -> %d\n\n", ender, porta, valor); fflush(stdout);
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(valor);
}

//------------------------------------------------------------------------------
bool TVarServ::FuncIniSSL(TVariavel * v)
{
    if (Instr::VarAtual - v < 2)
        return false;
    if (ssl_ctx_servidor)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto("");
    }
    char arq_crt[0x100];
    char arq_key[0x100];
    copiastr(arq_crt, v[1].getTxt(), sizeof(arq_crt));
    copiastr(arq_key, v[2].getTxt(), sizeof(arq_key));
    if (!arqvalido(arq_crt, true))
        return "Nome do arquivo CRT não é permitido";
    if (!arqvalido(arq_key, true))
        return "Nome do arquivo KEY não é permitido";
    const char * err = AbreServidorSSL(arq_crt, arq_key);
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(err ? err : "");
}

//------------------------------------------------------------------------------
int TVarServ::getValor()
{
    return sock >= 0;
}

//------------------------------------------------------------------------------
int TVarServ::FTamanho(const char * instr)
{
    return sizeof(TVarServ);
}

//------------------------------------------------------------------------------
int TVarServ::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarServ);
}

//------------------------------------------------------------------------------
void TVarServ::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarServ * ref = reinterpret_cast<TVarServ*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].defvar = v->defvar;
        ref[antes].indice = antes;
        ref[antes].EndObjeto(c, o);
        ref[antes].Criar();
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}
