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

TVarSocket * TObjSocket::varObj = 0;
TDNSSocket * TDNSSocket::Inicio = 0;

//------------------------------------------------------------------------------
TObjSocket::TObjSocket()
{
#ifdef DEBUG_CRIAR
    printf("new TObjSocket\n"); fflush(stdout);
#endif
// Acerta variáveis
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
// Nota: não pode chamar Inicio->MudarSock(0) porque
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

// Mostra o que está enviando
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
                    agora = (agora & ~0xF0) | (ch-'0') * 16;
                    break;
                }
                switch (ch | 0x20)
                {
                case 'a': agora=(agora|0xF0)-0x50; break;
                case 'b': agora=(agora|0xF0)-0x40; break;
                case 'c': agora=(agora|0xF0)-0x30; break;
                case 'd': agora=(agora|0xF0)-0x20; break;
                case 'e': agora=(agora|0xF0)-0x10; break;
                case 'f': agora=(agora|0xF0);      break;
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
                        destino+=3;
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
            if (mensagem[1]>='0' && mensagem[1]<='7')
            {
                agora = (agora & ~0x0F) | (mensagem[1]-'0');
                mensagem++;
            }
            break;
        case Instr::ex_barra_n: // Próxima linha
            if (agora != antes)
            {
                destino[0] = 1;
                destino[1] = agora;
                destino[2] = agora >> 8;
                destino+=3;
                antes = agora;
            }
            if (destino > fim)
                return false;
            *destino++ = '\n';
            coluna=0;
            break;
        default:
            if (*(unsigned char*)mensagem < ' ')
                break;
            if (agora != antes)
            {
                destino[0] = 1;
                destino[1] = agora;
                destino[2] = agora >> 8;
                destino+=3;
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
    *destino=0;

// Mostra o que está enviando
#ifdef DEBUG_MSG
    for (const char * x=mens; *x; x++)
        if (*x!=1)
            putchar(*x);
        else
            { x++; printf("[%d]", (unsigned char)x[0]); }
    putchar('\n'); fflush(stdout);
#endif

// Tenta enviar mensagem
    if (EnvMens(mens, codigo) == false)
        return false;

// Conseguiu - acerta variáveis
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
                Instr::ExecFim();
            }
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
            Instr::ExecArg(txt);
            Instr::ExecArg(indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
        else
            varObj->MudarSock(0);
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
        varObj = vobj->Depois;
        if (prossegue)
        {
            if (texto)
                Instr::ExecArg(texto);
            if (v1>=0)
                Instr::ExecArg(v1);
            if (v2>=0)
                Instr::ExecArg(v2);
            Instr::ExecArg(vobj->indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
    // Passa para próximo objeto
        vobj = varObj;
    } // for (TVarSocket ...
}

//------------------------------------------------------------------------------
void TVarSocket::Apagar()
{
    for (TDNSSocket * obj = TDNSSocket::Inicio; obj; obj=obj->Depois)
        if (obj->Socket == this)
            obj->Socket = 0;
    MudarSock(0);
}

//------------------------------------------------------------------------------
void TVarSocket::MudarSock(TObjSocket * obj)
{
// Verifica se o endereço vai mudar
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
            TObjSocket::varObj = Depois;
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
    if (TObjSocket::varObj == this)
        TObjSocket::varObj = destino;
    for (TDNSSocket * obj = TDNSSocket::Inicio; obj; obj=obj->Depois)
        if (obj->Socket == this)
            obj->Socket = destino;
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
// Envia mensagem: de longe é a função mais usada
    if (comparaZ(nome, "msg")==0)
    {
        if (Socket==0)
            return false;
        bool enviou = true;
        int codigo = (Instr::VarAtual > v+1 ? v[2].getInt() : 1);
        if (Instr::VarAtual >= v+1)
            enviou = Socket->Enviar(v[1].getTxt(), codigo);
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(enviou);
    }
// Lista das funções de socket
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        int valor;
        bool (TVarSocket::*Func)(TVariavel * v, int valor); } ExecFunc[] = {
        { "abrir",        0, &TVarSocket::FuncAbrir },
        { "abrirssl",     1, &TVarSocket::FuncAbrir },
        { "aflooder",     SocketAFlooder,  &TVarSocket::FuncVariaveis },
        { "cores",        SocketCores,     &TVarSocket::FuncVariaveis },
        { "eventoip",     0, &TVarSocket::FuncEventoIP },
        { "fechar",       0, &TVarSocket::FuncFechar },
        { "inissl",       0, &TVarSocket::FuncIniSSL },
        { "ip",           1, &TVarSocket::FuncEndereco },
        { "iplocal",      0, &TVarSocket::FuncEndereco },
        { "ipnome",       0, &TVarSocket::FuncIPNome },
        { "ipvalido",     0, &TVarSocket::FuncIPValido },
        { "nomeip",       0, &TVarSocket::FuncNomeIP },
        { "opctelnet",    SocketOpcTelnet, &TVarSocket::FuncVariaveis },
        { "posx",         SocketPosX,      &TVarSocket::FuncVariaveis },
        { "proto",        SocketProto,     &TVarSocket::FuncVariaveis },
        { "txtmd5",       2, &TVarSocket::FuncEndereco },
        { "txtsha1",      3, &TVarSocket::FuncEndereco }  };
// Procura a função correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini+fim)/2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado==0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v, ExecFunc[meio].valor);
        if (resultado<0) fim=meio-1; else ini=meio+1;
    }
    return false;
}

//----------------------------------------------------------------------------
/// Conecta
bool TVarSocket::FuncAbrir(TVariavel * v, int valor)
{
    MudarSock(0);
    if (Instr::VarAtual - v < 2)
        return false;
    int porta = v[2].getInt();
    const char * ender = v[1].getTxt();
#ifdef DEBUG_MSG
    printf(">>>>>>> Abrir(%s, %d)\n", ender, porta);
#endif
    if (valor == 1)
    {
        const char * err = AbreClienteSSL();
        if (err)
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarInt(0);
        }
    }
    TSocket * s = TSocket::Conectar(ender, porta, valor==1);
    if (s)
        MudarSock(s);
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(s!=0);
}

//----------------------------------------------------------------------------
/// Fecha Socket
bool TVarSocket::FuncFechar(TVariavel * v, int valor)
{
    if (Socket)
        Socket->Fechar();
    return false;
}

//----------------------------------------------------------------------------
/// Endereço IP
bool TVarSocket::FuncEndereco(TVariavel * v, int valor)
{
    char mens[50];
    *mens=0;
    if (Socket)
        Socket->Endereco(valor, mens, sizeof(mens));
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
/// Variáveis
bool TVarSocket::FuncVariaveis(TVariavel * v, int valor)
{
    Instr::ApagarVar(v+1);
    Instr::VarAtual->numfunc = valor;
    return true;
}

//----------------------------------------------------------------------------
/// Obter nome e gerar evento
bool TVarSocket::FuncEventoIP(TVariavel * v, int valor)
{
    if (Instr::VarAtual - v < 1)
        return false;
    TDNSSocket * obj = new TDNSSocket(this, v[1].getTxt());
    Instr::ApagarVar(v);
    if (*obj->nomeini)
        return Instr::CriarVarInt(1);
    else
    {
        delete obj;
        return Instr::CriarVarInt(0);
    }
}

//----------------------------------------------------------------------------
/// Endereço IP a partir do nome ou IP
bool TVarSocket::FuncNomeIP(TVariavel * v, int valor)
{
    if (Instr::VarAtual - v < 1)
        return false;
    struct sockaddr_in conSock;
    struct hostent *hnome;
    const char * ender = v[1].getTxt();
#ifdef __WIN32__
    memset(&conSock,0,sizeof(conSock));
    conSock.sin_addr.s_addr=inet_addr(ender);
    if ( conSock.sin_addr.s_addr == INADDR_NONE )
    {
        if ( (hnome=gethostbyname(ender)) == NULL )
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
        conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
    }
#else
    memset(&conSock.sin_zero, 0, 8);
    if (inet_aton(ender, &conSock.sin_addr) == 0)
    {
        if ( (hnome=gethostbyname(ender)) == NULL )
        {
            Instr::ApagarVar(v);
            return Instr::CriarVarTexto("");
        }
        conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
    }
#endif
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(inet_ntoa(conSock.sin_addr));
}

//----------------------------------------------------------------------------
/// Nome a partir do nome ou IP
bool TVarSocket::FuncIPNome(TVariavel * v, int valor)
{
    if (Instr::VarAtual - v < 1)
        return false;
    struct sockaddr_in conSock;
    struct hostent *hnome;
    const char * ender = v[1].getTxt();
#ifdef __WIN32__
    memset(&conSock,0,sizeof(conSock));
    conSock.sin_addr.s_addr=inet_addr(ender);
    if ( conSock.sin_addr.s_addr == INADDR_NONE )
        hnome = gethostbyname(ender);
    else
        hnome = gethostbyaddr( (char *) &conSock.sin_addr,
                            sizeof(conSock.sin_addr), AF_INET );
#else
    memset(&conSock.sin_zero, 0, 8);
    if (inet_aton(ender, &conSock.sin_addr) == 0)
        hnome = gethostbyname(ender);
    else
        hnome = gethostbyaddr( (char *) &conSock.sin_addr,
                            sizeof(conSock.sin_addr), AF_INET );
#endif
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(hnome == NULL ? "" : hnome->h_name);
}

//----------------------------------------------------------------------------
/// Checa se é um endereço IP válido
bool TVarSocket::FuncIPValido(TVariavel * v, int valor)
{
    if (Instr::VarAtual - v < 1)
        return false;
    int valido = 0;
    const char * ender = v[1].getTxt();
#ifdef __WIN32__
    if ( inet_addr(ender) != INADDR_NONE )
        valido = 1;
#else
    struct sockaddr_in conSock;
    if (inet_aton(ender, &conSock.sin_addr) != 0)
        valido = 1;
#endif
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(valido);
}

//----------------------------------------------------------------------------
/// Abre biblioteca SSL
bool TVarSocket::FuncIniSSL(TVariavel * v, int valor)
{
    const char * err = AbreClienteSSL();
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(err ? err : "");
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

//------------------------------------------------------------------------------
#ifdef __WIN32__
/// Resolve endereço em segundo plano no Windows
static DWORD WINAPI TDNSSocket_Resolve(LPVOID lpParam)
{
    TDNSSocket * obj = static_cast<TDNSSocket *>(lpParam);
    struct sockaddr_in conSock;
    struct hostent *hnome;
    memset(&conSock,0,sizeof(conSock));
    conSock.sin_addr.s_addr=inet_addr(obj->nomeini);
    if ( conSock.sin_addr.s_addr == INADDR_NONE )
    {
        hnome = gethostbyname(obj->nomeini);
        if (hnome)
        {
            conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
            copiastr(obj->ip, inet_ntoa(conSock.sin_addr));
        }
    }
    else
    {
        hnome = gethostbyaddr( (char *) &conSock.sin_addr,
                            sizeof(conSock.sin_addr), AF_INET );
        copiastr(obj->ip, inet_ntoa(conSock.sin_addr));
    }
    copiastr(obj->nome, hnome==NULL ? "" : hnome->h_name, sizeof(obj->nome));
    return 0;
}
#endif

//------------------------------------------------------------------------------
#ifndef __WIN32__
/// Interpreta sinal SIGCHLD
// Nota: Já está sendo processado em TExec::proc_sigchld_handler()
/*static void dns_sigchld_handler(int signum)
{
    int pid, status, serrno;
    serrno = errno;
    while (true)
    {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid<0 && errno!=ECHILD) // -1 significa que ocorreu algum erro
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
    Antes = 0;
    Depois = Inicio;
    if (Depois)
        Depois->Antes = this;
    Inicio = this;
    if (*ender == 0)
        return;
    *nomeini = 0;
    *nome = 0;
    *ip = 0;
    copiastr(nomeini, ender, sizeof(nomeini));
    if (*nomeini == 0)
        return;
#ifdef __WIN32__
    hthread = CreateThread(NULL, 0, &TDNSSocket_Resolve,
                           reinterpret_cast<DWORD*>(this), 0, NULL);
    if (hthread == NULL)
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
        struct sockaddr_in conSock;
        struct hostent *hnome;
        memset(&conSock.sin_zero, 0, 8);
        if (inet_aton(nomeini, &conSock.sin_addr) == 0)
        {
            hnome = gethostbyname(nomeini);
            if (hnome)
            {
                conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
                copiastr(ip, inet_ntoa(conSock.sin_addr));
            }
        }
        else
        {
            hnome = gethostbyaddr( (char *) &conSock.sin_addr,
                                sizeof(conSock.sin_addr), AF_INET );
            copiastr(ip, inet_ntoa(conSock.sin_addr));
        }
        char mens[1024];
        char * p = mprintf(mens, sizeof(mens), "%s%c%s",
                hnome==NULL ? "" : hnome->h_name, 0, ip);
        safe_write(descrpipe[1], mens, p-mens+1);
        _exit(EXIT_SUCCESS);
    }
    close(descrpipe[1]);
    fcntl(descrpipe[0], F_SETFL, O_NONBLOCK);
    //signal(SIGCHLD,dns_sigchld_handler); // Processar sinal SIGCHLD
    // Nota: Já está sendo processado em TExec::proc_sigchld_handler()
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
    for (TDNSSocket * obj = Inicio; obj; obj=obj->Depois)
        FD_SET(obj->recdescr, set_entrada);
#endif
}

//------------------------------------------------------------------------------
void TDNSSocket::ProcEventos(fd_set * set_entrada)
{
    for (TDNSSocket * obj = Inicio; obj;)
    {
        bool prossegue = false;
    // Checa se já tem todas as informações
#ifdef __WIN32__
        if (WaitForSingleObject(obj->hthread, 0) == WAIT_TIMEOUT)
        {
            obj = obj->Depois;
            continue;
        }
        CloseHandle(obj->hthread);
#else
        char mens[1024];
        int resposta =  read(obj->recdescr, mens, sizeof(mens)-2);
        if (resposta<0 && (errno==EINTR || errno==EWOULDBLOCK))
        {
            obj = obj->Depois;
            continue;
        }
        close(obj->recdescr);
        if (resposta<0)
            resposta=0;
        memset(mens + resposta, 0, 2);
        copiastr(obj->nome, mens, sizeof(obj->nome));
        copiastr(obj->ip, mens+strlen(mens)+1, sizeof(obj->ip));
#endif
    // Checa se pode gerar evento
        TVarSocket * vobj = obj->Socket;
        if (vobj == 0)
            ;
    // Definido em objeto: prepara para executar
        else if (vobj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_eventoip", vobj->defvar+Instr::endNome);
            prossegue = Instr::ExecIni(vobj->endobjeto, mens);
        }
    // Definido em classe: prepara para executar
        else if (vobj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_eventoip", vobj->defvar+Instr::endNome);
            prossegue = Instr::ExecIni(vobj->endclasse, mens);
        }
    // Executa
        if (prossegue)
        {
            Instr::ExecArg(obj->nomeini);
            Instr::ExecArg(obj->nome);
            Instr::ExecArg(obj->ip);
            Instr::ExecArg(vobj->indice);
            Instr::ExecX();
            Instr::ExecFim();
        }
    // Passa para o próximo objeto
        TDNSSocket * obj2 = obj;
        obj = obj->Depois;
        delete obj2;
    }
}
