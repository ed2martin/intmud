/* Este programa é software livre; você pode redistribuir e/ou
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
#include "config.h"
#include "var-serv.h"
#include "var-socket.h"
#include "socket.h"
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

//#define DEBUG_CRIAR  // Mostra objetos criados e apagados

TVarServ * TVarServ::Inicio = 0;
TVarServ * TVarServ::varObj = 0;

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
    if (sock<0)
        return;
    close(sock);
    sock=-1;
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    if (varObj==this)
        varObj=Depois;
}

//------------------------------------------------------------------------------
void TVarServ::Mover(TVarServ * destino)
{
    if (sock<0)
        return;
    (Antes ? Antes->Depois : Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
}

//------------------------------------------------------------------------------
void TVarServ::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto=o, b_objeto=true;
    else
        endclasse=c, b_objeto=false;
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

// Cria socket
#ifdef __WIN32__
        if ( (sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET )
        {
            sock=-1;
            break;
        }
        //BOOL iopcoes = 1;
        //int  tamanho = sizeof(iopcoes);
        //setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, (const char*)(void*)&iopcoes, tamanho);
#else
        int  iopcoes = 1;
        ACCEPT_TYPE_ARG3 tamanho = sizeof(iopcoes);
        if ( (sock = socket(PF_INET, SOCK_STREAM, 0)) <0 )
            break;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &iopcoes, tamanho);
#endif

// Seleciona o endereço
        memset(&(strSock.sin_zero), 0, 8);
        strSock.sin_family = AF_INET;
        strSock.sin_addr.s_addr = htonl(INADDR_ANY);
        if (*ender)
        {
            strSock.sin_addr.s_addr=inet_addr(ender);
            if ( (strSock.sin_addr.s_addr) == (unsigned long)-1 )
            {
                if ( (hnome=gethostbyname(ender)) == NULL )
                    break;
                strSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
            }
        }

// Seleciona a porta
#ifdef __WIN32__
        strSock.sin_port = htons((u_short)porta);
        if ( bind(sock, (LPSOCKADDR)&strSock, sizeof(strSock)) )
            break;
#else
        strSock.sin_port = htons(porta);
        if ( bind(sock, (struct sockaddr *)&strSock,
                            sizeof(struct sockaddr))<0)
            break;
#endif

// Escuta na porta selecionada
#ifdef __WIN32__
        if (listen(sock, 15))
            break;
#else
        if (listen(sock, 15)<0)
            break;
#endif

// Modo de não bloqueio
#ifdef __WIN32__
        unsigned long argp=1; // 0=bloquear  1=nao bloquear
        if ( ioctlsocket(sock, FIONBIO, &argp) !=0 )
            break;
#else
        fcntl(sock, F_SETFL, O_NONBLOCK);
#endif

// Coloca na lista ligada
        Antes = 0;
        Depois = Inicio;
        if (Depois)
            Depois->Antes = this;
        Inicio = this;
        return true;
    }

// Ocorreu algum erro
    // UNIX: o erro está em strerror(errno)
    // Windows: chamar WSAGetLastError() e obter o erro de uma tabela
    if (sock>=0)
        close(sock);
    sock=-1;
    return false;
}

//------------------------------------------------------------------------------
void TVarServ::Fd_Set(fd_set * set_entrada)
{
    for (TVarServ * s = Inicio; s; s=s->Depois)
        FD_SET(s->sock, set_entrada);
}

//------------------------------------------------------------------------------
void TVarServ::ProcEventos(fd_set * set_entrada)
{
    for (TVarServ * obj = Inicio; obj; )
    {
    // Verifica se tem conexões pendentes
        if (FD_ISSET(obj->sock, set_entrada)==0)
        {
            obj = obj->Depois;
            continue;
        }
    // Recebe conexão
        varObj = obj;
        while (obj == varObj)
        {
            struct sockaddr_in SockStr_in;
#ifdef __WIN32__
            int tamsock = sizeof(SockStr_in);
            unsigned int localSocket=accept(obj->sock,
                (struct sockaddr *)&SockStr_in, &tamsock);
            if (localSocket==INVALID_SOCKET)
                break;
#else
            ACCEPT_TYPE_ARG3 tamsock = sizeof(SockStr_in);
            int localSocket=accept(obj->sock,
                (ACCEPT_TYPE_ARG2)&SockStr_in, &tamsock);
            if (localSocket<0)
                break;
#endif
            TSocket::SockConfig(localSocket);
        // Gera evento
            bool prossegue = false;
            if (obj->b_objeto)
            {
                char mens[80];
                sprintf(mens, "%s_socket", obj->defvar+Instr::endNome);
                prossegue = Instr::ExecIni(obj->endobjeto, mens);
            }
            else if (obj->endclasse)
            {
                char mens[80];
                sprintf(mens, "%s_socket", obj->defvar+Instr::endNome);
                prossegue = Instr::ExecIni(obj->endclasse, mens);
            }
            if (prossegue==false)
            {
                close(localSocket);
                continue;
            }
                // Cria argumento: TVarSocket
            Instr::ExecArgCriar(Instr::InstrSocket);
            TVariavel * v = Instr::VarAtual;
                // Cria argumento: índice
            Instr::ExecArg(obj->indice);
                // Cria TObjSocket com o socket
            TSocket * s = new TSocket(localSocket);
                // Coloca TObjSocket em TVarSocket
            v->end_socket->MudarSock(s);
                // Executa
            Instr::ExecX();
            Instr::ExecFim();
        }
    // Fecha o laço
        if (obj == varObj)
            obj = obj->Depois;
        else
            obj = varObj;
    }
}

//------------------------------------------------------------------------------
bool TVarServ::Func(TVariavel * v, const char * nome)
{
// Fecha Socket
    if (comparaZ(nome, "fechar")==0)
    {
        Fechar();
        return false;
    }
// Abre porta
    if (comparaZ(nome, "abrir")==0)
    {
        if (Instr::VarAtual - v < 2)
            return false;
        int porta = v[2].getInt();
        const char * ender = v[1].getTxt();
        if (porta<0 || porta>65535)
            return false;
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        Instr::VarAtual->setInt(Abrir(ender, porta));
    //printf("%s %d -> %d\n", ender, porta, Instr::VarAtual->getInt()); fflush(stdout);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
int TVarServ::getValor()
{
    return sock>=0;
}
