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
 #include <netinet/in.h> // Cont�m a estrutura sockaddr_in
 #include <sys/time.h>
 #include <sys/types.h>
 #include <sys/socket.h> // Comunica��o
 #include <signal.h>
 #include <netinet/in.h> // Cont�m a estrutura sockaddr_in
 #include <sys/un.h>     // Cont�m a estrutura sockaddr_un
 #include <errno.h>
#endif
#include "config.h"
#include "socket.h"
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"
#include "sha1.h"
#include "md5.h"

//#define DEBUG_CRIAR  // Mostra objetos criados e apagados
//#define DEBUG_MSG    // Mostra o que enviou e recebeu
//#define DEBUG_SSL    // Mostra informa��es de conex�o SSL

TSocket * TSocket::sockAtual = 0;
TSocket * TSocket::sInicio = 0;
bool TSocket::boolenvpend = false;

// Conex�o de socket, vide tamb�m:
// http://www.muq.org/~cynbe/ref/nonblocking-connects.html

//----------------------------------------------------------------------------
enum TSocketProto /// Valores de TSocket::proto
{
    spConnect1,  ///< Conectando, conex�o normal
    spConnect2,  ///< Conectando, conex�o SSL
    spSslConnect,///< Conectando via SSL - SslConnect
    spSslAccept, ///< Recebendo conex�o via SSL - SslAccept
    spTelnet1,   ///< Telnet, s� mensagens inteiras
    spTelnet2,   ///< Telnet, recebe mensagens incompletas (sem o \\n)
    spIRC,       ///< IRC
    spPapovox    ///< Papovox
};

//------------------------------------------------------------------------------
// Converter do IRC para Telnet
static const char convirc[] = {
    15, // 0=white -> 7=branco
    0, // 1=black -> 0=preto
    4, // 2=blue  -> 4=azul
    2, // 3=green -> 2=verde
    1, // 4=red   -> 1=vermelho
    3, // 5=brown -> 3=marrom
    5, // 6=purple -> 5=magenta ???
    9, // 7=orange -> vermelho forte (n�o tinha outra cor)
    11, // 8=yellow
    10, // 9=lt.green
    6,  // 10=teal (a kinda green/blue cyan)
    14, // 11=lt.cyan
    12, // 12=lt.blue
    13, // 13=pink
    8,  // 14=grey
    7 };  // 15=lt.grey

// Converter do Telnet para IRC
static const char desconvirc[] = {
    1, // 0
    4, // 1
    3, // 2
    5, // 3
    2, // 4
    6, // 5
    10, // 6
    15, // 7
    14, // 8
    7, // 9
    9, // 10
    8, // 11
    12, // 12
    13, // 13
    11, // 14
    0 }; // 15

//------------------------------------------------------------------------------
TSocket * TSocket::Conectar(const char * ender, int porta, bool ssl)
{
    struct sockaddr_in conSock;
    struct hostent *hnome;
    int conManip;

    if (porta<0 || porta>65535)
        return 0;
#ifdef __WIN32__
    memset(&conSock,0,sizeof(conSock));
    conSock.sin_addr.s_addr=inet_addr(ender);
    if ( conSock.sin_addr.s_addr == INADDR_NONE )
    {
        if ( (hnome=gethostbyname(ender)) == NULL )
            return 0;
        conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
    }
    conSock.sin_family=AF_INET;
    conSock.sin_port=htons( (u_short)porta );
    if ( (conManip = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        return 0;
    SockConfig(conManip);
    if ( connect(conManip, (struct sockaddr *)&conSock,
                                      sizeof(struct sockaddr)) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            closesocket(conManip);
            return 0;
        }
    }
#else
    memset(&conSock.sin_zero, 0, 8);
    conSock.sin_family=AF_INET;
    conSock.sin_port=htons(porta);
    conSock.sin_addr.s_addr=inet_addr(ender);
    if ( (conSock.sin_addr.s_addr) == (unsigned long)-1 )
    {
        if ( (hnome=gethostbyname(ender)) == NULL )
            return 0;
        conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
    }
    if ( (conManip=socket(PF_INET,SOCK_STREAM,0)) ==-1)
        return 0;
    SockConfig(conManip);
    if ( connect(conManip, (struct sockaddr *)&conSock,
                                      sizeof(struct sockaddr)) <0 )
    {
        if (errno != EINPROGRESS)
        {
            close(conManip);
            return 0;
        }
    }
#endif
    TSocket * s = new TSocket(conManip, 0);
    s->proto = (ssl ? spConnect2 : spConnect1);
    s->conSock = conSock;
#ifdef DEBUG_SSL
    if (ssl) { puts("CONECTADO"); fflush(stdout); }
#endif
    return s;
}

//------------------------------------------------------------------------------
const char * TSocket::TxtErro(int erro)
{
#ifdef __WIN32__
    // Lista de erros de: http://www.sockets.com/err_lst1.htm
    switch (erro)
    {
    case WSABASEERR:
        return "";
    case WSAEINTR: // (10004) Interrupted system call
        return "Chamada do sistema interrompida";
    case WSAEBADF: // (10009) Bad file number
        return "N�mero de arquivo inv�lido";
    case WSAEACCES: // (10013) Permission denied
        return "Permiss�o negada";
    case WSAEFAULT: // (10014) Bad address
        return "Endere�o incorreto";
    case WSAEINVAL: // (10022) Invalid argument
        return "Argumento inv�lido";
    case WSAEMFILE: // (10024) Too many open files
        return "Muitos arquivos abertos";
    case WSAEWOULDBLOCK: // (10035) Operation would block
        return "Opera��o deveria bloquear";
    case WSAEINPROGRESS: // (10036) Operation now in progress
        return "Opera��o em progresso";
    case WSAEALREADY: // (10037) Operation already in progress
        return "Opera��o j� est� em progresso";
    case WSAENOTSOCK: // (10038) Socket operation on non-socket
        return "Opera��o socket em n�o socket";
    case WSAEDESTADDRREQ: // (10039) Destination address required
        return "Faltou o endere�o destino";
    case WSAEMSGSIZE: // (10040) Message too long
        return "Mensagem longa demais";
    case WSAEPROTOTYPE: // (10041) Protocol wrong type for socket
        return "Protocolo errado para socket";
    case WSAENOPROTOOPT: // (10042) Bad protocol option
        return "Op��o de protocolo inv�lida";
    case WSAEPROTONOSUPPORT: // (10043) Protocol not supported
        return "Protocolo n�o suportado";
    case WSAESOCKTNOSUPPORT: // (10044) Socket type not supported
        return "Tipo de socket n�o suportado";
    case WSAEOPNOTSUPP: // (10045) Operation not supported on socket
        return "Opera��o n�o suportada em socket";
    case WSAEPFNOSUPPORT: // (10046) Protocol family not supported
        return "Protocolo n�o suportado";
    case WSAEAFNOSUPPORT: // (10047) Address family not supported by protocol family
        return "Tipo de endere�o n�o suportado pelo protocolo";
    case WSAEADDRINUSE: // (10048) Address already in use
        return "Endere�o j� est� em uso";
    case WSAEADDRNOTAVAIL: // (10049) Can't assign requested address
        return "Imposs�vel designar endere�o requisitado";
    case WSAENETDOWN: // (10050) Network is down
        return "A rede n�o est� funcionando";
    case WSAENETUNREACH: // (10051) Network is unreachable
        return "A rede est� inacess�vel";
    case WSAENETRESET: // (10052) Net dropped connection or reset
        return "A rede derrubou a conex�o";
    case WSAECONNABORTED: // (10053) Software caused connection abort
        return "Software cancelou a conex�o";
    case WSAECONNRESET: // (10054) Connection reset by peer
        return "Conex�o fechada remotamente";
    case WSAENOBUFS: // (10055) No buffer space available
        return "Sem espa�o dispon�vel no buffer";
    case WSAEISCONN: // (10056) Socket is already connected
        return "Socket j� est� conectado";
    case WSAENOTCONN: // (10057) Socket is not connected
        return "Socket n�o est� conectado";
    case WSAESHUTDOWN: // (10058) Can't send after socket shutdown
        return "Imposs�vel enviar ap�s socket fechar";
    case WSAETOOMANYREFS: // (10059) Too many references, can't splice
        return "Muitas refer�ncias";
    case WSAETIMEDOUT: // (10060) Connection timed out
        return "Conex�o expirou por excesso de tempo";
    case WSAECONNREFUSED: // (10061) Connection refused
        return "Conex�o recusada";
    case WSAELOOP: // (10062) Too many levels of symbolic links
        return "Muitos n�veis de links simb�licos";
    case WSAENAMETOOLONG: // (10063) File name too long
        return "Nome de arquivo muito longo";
    case WSAEHOSTDOWN: // (10064) Host is down
        return "Host n�o est� funcionando";
    case WSAEHOSTUNREACH: // (10065) No Route to Host
        return "Nenhuma rota para o host";
    case WSAENOTEMPTY: // (10066) Directory not empty
        return "Diret�rio n�o est� vazio";
    case WSAEPROCLIM: // (10067) Too many processes
        return "Muitos processos";
    case WSAEUSERS: // (10068) Too many users
        return "Muitos usu�rios";
    case WSAEDQUOT: // (10069) Disc Quota Exceeded
        return "Quota de disco excedida";
    case WSAESTALE: // (10070) Stale NFS file handle
        return "Arquivo n�o dispon�vel";
    case WSASYSNOTREADY: // (10091) Network SubSystem is unavailable
        return "Sistema de rede indispon�vel";
    case WSAVERNOTSUPPORTED: // (10092) WINSOCK DLL Version out of range
        return "Vers�o inv�lida da DLL Winsock";
    case WSANOTINITIALISED: // (10093) Successful WSASTARTUP not yet performed
        return "N�o foi feito um WSASTARTUP com sucesso";
    case WSAEREMOTE: // (10071) Too many levels of remote in path
        return "Muitos n�veis remotos no caminho";
    case WSAHOST_NOT_FOUND: // (11001) Host not found
        return "Host n�o encontrado";
    case WSATRY_AGAIN: // (11002) Non-Authoritative Host not found
        return "Host n�o autorizado n�o encontrado";
    case WSANO_RECOVERY: // (11003) Non-Recoverable errors: FORMERR, REFUSED, NOTIMP
        return "Erro n�o recuper�vel";
    //case WSANO_DATA: // (11004)* Valid name, no data record of requested type
    case WSANO_ADDRESS: // (11004)* No address, look for MX record
        return "Endere�o n�o existe";
    }
    static char mens[40];
    sprintf(mens, "Erro %d", erro);
    return mens;
#else
    return strerror(erro);
#endif
}

//------------------------------------------------------------------------------
void TSocket::SockConfig(int localSocket)
{
#ifdef __WIN32__
    int sopcoes;
    int stamanho;
    unsigned long argp=1; // 0=bloquear  1=nao bloquear
    ioctlsocket(localSocket, FIONBIO, &argp);
    sopcoes=2048;
    stamanho=sizeof(sopcoes);
    setsockopt(localSocket, SOL_SOCKET, SO_RCVBUF,
            (const char*)(void*)&sopcoes, stamanho);
    sopcoes=2048;
    stamanho=sizeof(sopcoes);
    setsockopt(localSocket, SOL_SOCKET, SO_SNDBUF,
            (const char*)(void*)&sopcoes, stamanho);
#else
    size_t sopcoes;
    ACCEPT_TYPE_ARG3 stamanho;
    fcntl(localSocket, F_SETFL, O_NONBLOCK);
    sopcoes=2048;
    stamanho=sizeof(sopcoes);
    setsockopt(localSocket, SOL_SOCKET, SO_RCVBUF, &sopcoes, stamanho);
    sopcoes=2048;
    stamanho=sizeof(sopcoes);
    setsockopt(localSocket, SOL_SOCKET, SO_SNDBUF, &sopcoes, stamanho);
#endif
}

//------------------------------------------------------------------------------
TSocket::TSocket(int socknum, SSL * sslnum)
{
#ifdef DEBUG_CRIAR
    printf("new TSocket(%d)\n", socknum); fflush(stdout);
#endif
// Coloca na lista ligada
    sAntes = 0;
    sDepois = sInicio;
    if (sInicio)
        sInicio->sAntes = this;
    sInicio = this;
// Acerta vari�veis
    sock=socknum;
    sockssl=sslnum;
    pontEnv=0;
    pontTelnet=0;
    pontESC=0;
    memset(telnetopc, 0, sizeof(telnetopc));
    telnetecho=0;
    pontRec=0;
    dadoRecebido=0;
    CorAtual=0x70;
    CorAnterior=0x70;
    proto=spTelnet1;
    cores=0;
    eventoenv=false;
    usaropctelnet=false;
    sockenvrec=0;
    sockerro=0;
    AFlooder=0;
}

//------------------------------------------------------------------------------
TSocket::~TSocket()
{
#ifdef DEBUG_CRIAR
    printf("delete TSocket(%d)\n", sock); fflush(stdout);
#endif
// Fecha Socket
    FecharSock(0, 0);
// Verifica se apagou objeto sockAtual
    if (sockAtual==this)
        sockAtual=sDepois;
// Retira da lista ligada
    (sAntes ? sAntes->sDepois : sInicio) = sDepois;
    if (sDepois)
        sDepois->sAntes = sAntes;
}

//------------------------------------------------------------------------------
void TSocket::Fechar()
{
    delete this;
}

//------------------------------------------------------------------------------
void TSocket::FecharSock(int erro, bool env)
{
    if (sock<0)
        return;
    if (!env)
        EnvPend();
    if (sockssl)
    {
#ifdef DEBUG_SSL
        puts("SSL_SHUTDOWN"); fflush(stdout);
#endif
        SslShutdown(sockssl);
        SslFree(sockssl);
        sockssl = 0;
    }
    close(sock);
    sock=-1;
    pontRec = 0;
    pontEnv = 0;
    sockerro = erro;
    sockenvrec = env;
}

//------------------------------------------------------------------------------
void TSocket::CriaSSL()
{
    if (sockssl || sock<0)
        return;
    sockssl = SslNew(ssl_ctx_cliente);
    SslSetFd(sockssl, sock);
    acaossl = 0;
#ifdef DEBUG_SSL
    puts("SSL_NEW"); fflush(stdout);
#endif
}

//------------------------------------------------------------------------------
bool TSocket::Conectado()
{
    if (sock<0)
        return false;
    switch (proto)
    {
    case spTelnet1:
    case spTelnet2:
    case spIRC:
    case spPapovox:
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
int TSocket::Variavel(char num, int valor)
{
    switch (num)
    {
    case 1: // proto
        if (sock<0)
            return 0;
        if (valor >= 0)
        {
            bool mudar = false;
            switch (proto)
            {
            case spTelnet1:
            case spTelnet2:
            case spIRC:
            case spPapovox:
                if (valor<2) valor=2;
                if (valor>5) valor=5;
                switch (valor)
                {
                case 2:
                    mudar = (proto!=spTelnet1 && proto!=spTelnet2);
                    proto = spTelnet1;
                    break;
                case 3:
                    mudar = (proto!=spTelnet1 && proto!=spTelnet2);
                    proto = spTelnet2;
                    break;
                case 4:
                    mudar = (proto!=spIRC);
                    proto = spIRC;
                    break;
                case 5:
                    mudar = (proto!=spPapovox);
                    proto = spPapovox;
                    break;
                }
                if (mudar)
                    pontRec=0, pontESC=0, pontTelnet=0;
            }
        }
        switch (proto)
        {
        case spTelnet1: return 2;
        case spTelnet2: return 3;
        case spIRC:     return 4;
        case spPapovox: return 5;
        }
        return 1;
    case 2: // cores
        if (valor>=0)
            cores = (valor>3 ? 3 : valor);
        return cores;
    case 3: // aflooder
        if (valor>=0)
        {
            if (valor==0)
                AFlooder=0;
            else if (AFlooder==0)
                AFlooder=TempoIni+1;
        }
        return (AFlooder!=0);
    case 4: // opctelnet
        if (proto!=spTelnet1 && proto!=spTelnet2)
            return 1;
        if (valor>=0)
            usaropctelnet = valor;
        return usaropctelnet;
    }
    return 0;
}

//------------------------------------------------------------------------------
void TSocket::Endereco(int num, char * mens, int tam)
{
    *mens = 0;
    if (sock<0)
        return;
    if (num < 2)
    {
        struct sockaddr_in conSock;
#ifdef __WIN32__
        int tam = sizeof(conSock);
#else
        ACCEPT_TYPE_ARG3 tam = sizeof(conSock);
#endif
        tam = sizeof(struct sockaddr);
        int r;
        if (num)
            r = getpeername(sock, (struct sockaddr *)&conSock, &tam);
        else
            r = getsockname(sock, (struct sockaddr *)&conSock, &tam);
        if (r>=0)
            copiastr(mens, inet_ntoa(conSock.sin_addr), tam);
        return;
    }
    if (num < 4)
    {
    // Obt�m objeto X509
        if (sockssl==0 || sock<0)
            return;
        X509 * obj509 = SslGetPeerCertificate(sockssl);
        if (obj509==0)
            return;
    // Obt�m o tamanho do certificado
        int total = SslX509i2d(obj509, 0);
        if (total<=0)
        {
            SslX509free(obj509);
            return;
        }
    // Obt�m o certificado
        unsigned char * buf = new unsigned char[total];
        unsigned char * p = buf;
        SslX509i2d(obj509, &p);
    // Obt�m o hash
        char texto[50];
        unsigned char digest[20];
        if (num == 2)
        {
            cvs_MD5Context md5Info;
            cvs_MD5Init(&md5Info);
            cvs_MD5Update(&md5Info, buf, total);
            cvs_MD5Final(digest, &md5Info);
            total = 16;
        }
        else
        {
            SHA_CTX shaInfo;
            SHAInit(&shaInfo);
            SHAUpdate(&shaInfo, buf, total);
            SHAFinal(digest, &shaInfo);
            total = 20;
        }
    // Monta o texto
        for (int x=0; x<total; x++)
        {
            int valor = digest[x] >> 4;
            texto[x*2] = (valor<10 ? valor+'0' : valor+'a'-10);
            valor = digest[x] & 0x0F;
            texto[x*2+1] = (valor<10 ? valor+'0' : valor+'a'-10);
        }
        texto[total*2] = 0;
        copiastr(mens, texto, tam);
    // Desaloca os recursos
        delete[] buf;
        SslX509free(obj509);
        return;
    }
}

//------------------------------------------------------------------------------
bool TSocket::EnvMens(const char * mensagem)
{
    if (sock<0)
        return false;

// Papovox
    if (proto==spPapovox)
    {
        char mens[SOCKET_ENV];
        char * ini = mens;
        char * destino = mens + 3;
        for (; *mensagem; mensagem++)
        {
            if (destino >= mens + sizeof(mens) - 4)
                return false;
            switch (*mensagem)
            {
            case 1:
                mensagem++;
            case 2:
            case 3:
            case 4:
            case 5:
                break;
            case '\n':
                ini[0] = 1;
                ini[1] = (destino - ini - 3);
                ini[2] = (destino - ini - 3) >> 8;
                ini = destino;
                destino += 3;
                break;
            default:
                *destino++ = *mensagem;
            }
        }
        int total = destino - ini - 3;
        if (total>0)
        {
            ini[0] = 1;
            ini[1] = total;
            ini[2] = total >> 8;
        }
        else
            destino -= 3;
        if (destino != mens)
            eventoenv = true;
        return EnvMens(mens, destino - mens);
    }

// Sem cores
    if ((cores&2)==0)
    {
        if (!Conectado())
            return false;
        char mens[SOCKET_ENV];
        char * destino = mens;
        for (; *mensagem; mensagem++)
        {
            if (destino >= mens + sizeof(mens)-2)
                return false;
            switch (*(unsigned char*)mensagem)
            {
            case 1:
                mensagem++;
                break;
            case 2: // echo off
                memcpy(destino, "\xFF\xFB\x01", 3);
                destino += 3;
                break;
            case 3: // echo on
                memcpy(destino, "\xFF\xFC\x01", 3);
                destino += 3;
                break;
            case 4: // go ahead
                memcpy(destino, "\xFF\xF9", 2);
                destino += 2;
                break;
            case 5: // bipe
                *destino++ = 7;
                break;
            case '\n':
                *destino++ = 13;
                *destino++ = 10;
                break;
            case 255:
                memcpy(destino, "\xFF\xFF", 2);
                destino += 2;
                break;
            default:
                *destino++ = *mensagem;
            }
        }
        if (destino != mens)
            eventoenv = true;
        return EnvMens(mens, destino - mens);
    }

// Telnet
    if (proto==spTelnet1 || proto==spTelnet2)
    {
        char mens[SOCKET_ENV];
        char * destino = mens;
        int cor = CorEnvia;
        int ecototal = 0; // Quantidade de comandos ECHO na mensagem
        for (; *mensagem; mensagem++)
        {
        // Verifica se espa�o suficiente
            if (destino >= mens + sizeof(mens)-15)
                return false;
        // Caracter normal
            if (*mensagem!=1)
            {
                switch (*(unsigned char*)mensagem)
                {
                case 2: // echo off
                    ecototal++;
                    memcpy(destino, "\xFF\xFB\x01", 3);
                    destino += 3;
                    break;
                case 3: // echo on
                    ecototal++;
                    memcpy(destino, "\xFF\xFC\x01", 3);
                    destino += 3;
                    break;
                case 4: // go ahead
                    memcpy(destino, "\xFF\xF9", 2);
                    destino += 2;
                    break;
                case 5: // bipe
                    *destino++ = 7;
                    break;
                case '\n':
                    *destino++ = 13;
                    *destino++ = 10;
                    break;
                case 255:
                    memcpy(destino, "\xFF\xFF", 2);
                    destino += 2;
                    break;
                default:
                    *destino++ = *mensagem;
                }
                continue;
            }
        // Defini��o de cor
            mensagem++;
            int antes = cor;
            cor = *(unsigned char*)mensagem;
            char ini='[';
            //sprintf(destino, "%02x", cor); destino+=2;
            *destino++ = 0x1B;
                                // Tirar negrito
            if ((antes&0x80) && (cor&0x80)==0)
            {
                destino[0] = ini;
                destino[1] = '0';
                destino+=2, ini=';';
                if ((antes&0x70)!=0x70) // Se frente != branco
                {                       // Deve acertar frente
                    antes = ((antes&15) | (cor&0xF0)) ^ 0x70;
                }
                if (antes&15)    // Se fundo != preto
                    antes |= 15; // Deve acertar fundo
            }
            else if ((antes&0x80)==0 && (cor&0x80)) // Negrito
            {
                destino[0] = ini;
                destino[1] = '1';
                destino+=2, ini=';';
            }
            if ((antes^cor)&0x70) // Frente
            {
                destino[0] = ini;
                destino[1] = '3';
                destino[2] = ((cor>>4)&7)+'0';
                destino+=3, ini=';';
            }
            if ((antes^cor)&15) // Fundo
            {
                destino[0] = ini;
                destino[1] = '4';
                destino[2] = (cor&7)+'0';
                destino+=3, ini=';';
            }
            *destino++ = 'm';
            //*destino++ = '\\';
            //*destino++ = (novacor>>8)+'0';
            //*destino++ = ((novacor>>4)&15)+'0';
            //*destino++ = (novacor&15)+'0';
        }
        if (destino != mens)
            eventoenv = true;
        if (!EnvMens(mens, destino - mens))
            return false;
        telnetecho += ecototal;
        return true;
    }

// IRC
    if (proto==spIRC)
    {
        char mens[SOCKET_ENV];
        char * destino = mens;
        int cor = CorEnvia;
        for (; *mensagem; mensagem++)
        {
        // Verifica se espa�o suficiente
            if (destino >= mens + sizeof(mens)-10)
                return false;
        // Pr�xima linha
            if (*mensagem=='\n')
            {
                *destino++ = 13;
                *destino++ = 10;
                continue;
            }
        // Caracter normal
            if (*mensagem!=1)
            {
                if (*(unsigned char*)mensagem >= ' ')
                    *destino++ = *mensagem;
                continue;
            }
        // Defini��o de cor
            mensagem++;
            bool numero =   // Se pode ser interpretado como n�mero
                    (mensagem[1]==0 ||
                    (mensagem[1]>='0' && mensagem[1]<='9'));
            bool virgula =  // Se pode ser interpretado como v�rgula
                    (mensagem[1]==0 || mensagem[1]==',');
            int antes = cor ^ *(unsigned char*)mensagem;
            cor = *(unsigned char*)mensagem;
            *destino++ = 3;
            if (cor==0x70) // Cor padr�o
            {
                if (numero==false)
                    *destino=0;
                else if (virgula==false)
                    sprintf(destino, "%02d",
                            desconvirc[(cor>>4)&15]);
                else
                    sprintf(destino, "%d,%02d",
                            desconvirc[(cor>>4)&15],
                            desconvirc[cor&15]);
            }
            else if ((antes & 15)==0) // Fundo n�o mudou
            {
                if (virgula)
                    sprintf(destino, "%d,%02d",
                            desconvirc[(cor>>4)&15],
                            desconvirc[cor&15]);
                else if (numero)
                    sprintf(destino, "%02d",
                            desconvirc[(cor>>4)&15]);
                else
                    sprintf(destino, "%d",
                            desconvirc[(cor>>4)&15]);
            }
            else // Fundo mudou
            {
                if (numero)
                    sprintf(destino, "%d,%02d",
                            desconvirc[(cor>>4)&15],
                            desconvirc[cor&15]);
                else
                    sprintf(destino, "%d,%d",
                            desconvirc[(cor>>4)&15],
                            desconvirc[cor&15]);
            }
            while (*destino)
                destino++;
        }
        if (destino != mens)
            eventoenv = true;
        return EnvMens(mens, destino - mens);
    }

    assert(0); // Protocolo desconhecido
    return false;
}

//------------------------------------------------------------------------------
bool TSocket::EnvMens(const char * mensagem, int tamanho)
{
    if (sock<0)
        return false;
    if (tamanho > SOCKET_ENV)
        return false;
    boolenvpend = true;
    if (tamanho > SOCKET_ENV - (int)pontEnv)
    {
        EnvPend();
        if (tamanho > SOCKET_ENV - (int)pontEnv)
            return false;
    }
    memcpy(bufEnv + pontEnv, mensagem, tamanho);
    pontEnv += tamanho;
    return true;
}

//------------------------------------------------------------------------------
void TSocket::EnvPend()
{
    if (sock<0 || pontEnv<=0)
        return;
    int resposta, coderro=0;
#ifdef DEBUG_MSG
    printf(">>> ENV \"");
    for (unsigned int x=0; x<pontEnv; x++)
        putchar(bufEnv[x]);
    printf("\"\n");
#endif
    if (sockssl)
    {
        resposta=SslWrite(sockssl, bufEnv, pontEnv);
        if (resposta <= 0)
            switch (SslGetError(sockssl, resposta))
            {
            case SSL_ERROR_NONE:
                resposta = 0;
                break;
            case SSL_ERROR_WANT_READ:
                resposta = 0, acaossl = 0;
                break;
            case SSL_ERROR_WANT_WRITE:
                resposta = 0, acaossl = 1;
                break;
            default:
                resposta = -1;
            }
#ifdef DEBUG_SSL
        printf("SSL_WRITE = %d (acaossl=%d)\n", resposta, acaossl);
        fflush(stdout);
#endif
    }
    else
    {
#ifdef __WIN32__
        resposta=send(sock, bufEnv, pontEnv, 0);
        //if (resposta==0)
        //    resposta=-1, coderro=0;
        //else
        if (resposta==SOCKET_ERROR)
        {
            coderro = WSAGetLastError();
            resposta = (coderro==WSAEWOULDBLOCK ? 0 : -1);
        }
#else
        resposta=send(sock, bufEnv, pontEnv, 0);
        if (resposta<=0)
        {
            coderro = errno;
            if (resposta<0 && (errno==EINTR || errno==EWOULDBLOCK || errno==ENOBUFS))
                resposta=0;
            else if (resposta==0)
                resposta=-1,coderro=ECONNRESET;
        }
#endif
    }
    if (resposta<0)
        FecharSock(coderro, 1);
    else if (resposta>=(int)pontEnv)
        pontEnv=0;
    else if (resposta>0)
    {
        pontEnv -= resposta;
        memcpy(bufEnv, bufEnv+resposta, pontEnv);
    }
}

//------------------------------------------------------------------------------
void TSocket::SairPend()
{
    for (TSocket * obj = sInicio; obj; obj=obj->sDepois)
        obj->EnvPend();
}

//------------------------------------------------------------------------------
void TSocket::Fd_Set(fd_set * set_entrada, fd_set * set_saida, fd_set * set_err)
{
    while (true)
    {
        boolenvpend = false;
    // Envia dados pendentes
    // Verifica se algum socket fechou
        for (TSocket * obj = sInicio; obj;)
        {
        // Verifica se tem dados pendentes para transmitir
            bool ev_env = false;
            if (obj->pontEnv)
            {
                obj->EnvPend();
                ev_env = (obj->pontEnv<=0) && obj->eventoenv;
            }
        // Verifica socket fechou
            if (obj->sock < 0)
            {
                char mens[100];
                mprintf(mens, sizeof(mens), "%s %s",
                    obj->sockenvrec ? "Ao enviar" : "Ao receber",
                    obj->sockerro<0 ? "protocolo incorreto" :
                    obj->sockerro==0 ? "" :
                    TxtErro(obj->sockerro));
                sockAtual = obj;
            // Gera evento
                obj->FuncFechou(mens);
            // Passa para pr�ximo objeto TSocket
                if (sockAtual==obj) // Se objeto Socket n�o foi apagado, apaga
                    delete obj; // Faz automaticamente:
                                // sockAtual = sockAtual->Depois
                obj=sockAtual;  // Passa para o pr�ximo objeto
                continue;
            }
        // Verifica se socket fechado
            if (!obj->Conectado())
            {
                obj=obj->sDepois;
                continue;
            }
        // Verifica evento env
            if (ev_env)
            {
                obj->eventoenv = false;
                sockAtual = obj->sDepois;
                obj->FuncEvento("env", 0);
                obj = sockAtual;
                continue;
            }
        // Passa para pr�ximo socket
            obj = obj->sDepois;
        }
        if (!boolenvpend)
            break;
    }
    for (TSocket * obj = sInicio; obj; obj=obj->sDepois)
    {
        if (obj->proto == spConnect1 || obj->proto == spConnect2)
        {
#ifdef __WIN32__
            FD_SET(obj->sock, set_err);
            FD_SET(obj->sock, set_saida);
#else
            FD_SET(obj->sock, set_entrada);
            FD_SET(obj->sock, set_saida);
#endif
        }
        else if (obj->sockssl)
        {
#ifdef DEBUG_SSL
            printf("SSL_SELECT (acaossl=%d)\n", obj->acaossl);
            fflush(stdout);
#endif
            if (obj->acaossl)
                FD_SET(obj->sock, set_saida);
            else
                FD_SET(obj->sock, set_entrada);
        }
        else
        {
            FD_SET(obj->sock, set_entrada);
            if (obj->pontEnv)
                FD_SET(obj->sock, set_saida);
        }
    }
}

//------------------------------------------------------------------------------
void TSocket::ProcEventos(fd_set * set_entrada,
                          fd_set * set_saida, fd_set * set_err)
{
    for (TSocket * obj = sInicio; obj; )
    {
    // Verifica se socket aberto
        if (obj->sock<0)
        {
            obj = obj->sDepois;
            continue;
        }
    // Verifica socket conectando
        if (obj->proto==spConnect1 || obj->proto==spConnect2)
        {
            int coderro = 0;
#ifdef __WIN32__
            if (! FD_ISSET(obj->sock, set_saida))
            {
                if (! FD_ISSET(obj->sock, set_err))
                {
                    obj = obj->sDepois;
                    continue;
                }
                SOCK_SOCKLEN_T len = sizeof(coderro);
                if (getsockopt(obj->sock, SOL_SOCKET, SO_ERROR,
                            (char*)&coderro, &len) != 0)
                {
                    coderro = WSAGetLastError();
                    if (coderro==0)
                        coderro=-1;
                }
                else if (coderro==0) // Timeout no select
                {
                    obj = obj->sDepois;
                    continue;
                }
            }
#else
        // Checa se conex�o pendente
            if (FD_ISSET(obj->sock, set_entrada)==0 &&
                    FD_ISSET(obj->sock, set_saida)==0)
            {
                obj = obj->sDepois;
                continue;
            }
        // Checa se est� conectado
            if ( connect(obj->sock, (struct sockaddr *)&obj->conSock,
                                      sizeof(struct sockaddr)) < 0 )
            {
                coderro = errno;
                if (coderro==EISCONN)
                    coderro=0;
            }
        // Se erro: obt�m o c�digo de erro
            if (coderro)
            {
                int err = 0;
                SOCK_SOCKLEN_T len = sizeof(err);
                if (getsockopt(obj->sock, SOL_SOCKET, SO_ERROR,
                    (void*)&err, &len)==0)
                if (err)
                    coderro = err;
            }
#endif
        // Erro ao conectar
            if (coderro)
            {
                sockAtual = obj;
                char mens[80] = "Erro ao conectar";
                if (coderro != -1)
                    copiastr(mens, TxtErro(coderro), sizeof(mens));
                if (obj->FuncEvento("err", mens))
                    delete obj; // Apaga objeto se n�o foi apagado
                                // socket � fechado ao apagar objeto
                obj = sockAtual;
                continue;
            }
        // Conseguiu conectar, checa se � conex�o normal (n�o SSL)
            if (obj->proto!=spConnect2)
            {
                sockAtual = obj->sDepois;
                obj->proto = spTelnet2;
                obj->FuncEvento("con", 0);
                obj = sockAtual;
                continue;
            }
        // Conex�o SSL
            obj->proto = spSslConnect;
            obj->CriaSSL();
        // Erro ao conectar
        }
    // Verifica conex�o SSL
        if (obj->proto==spSslConnect)
        {
            int resposta = SslConnect(obj->sockssl);
            if (resposta > 0)
            {
                sockAtual = obj->sDepois;
                obj->proto = spTelnet2;
                obj->FuncEvento("con", 0);
                obj = sockAtual;
                continue;
            }
            switch (SslGetError(obj->sockssl, resposta))
            {
            case SSL_ERROR_WANT_READ:
                resposta = 0, obj->acaossl = 0;
                break;
            case SSL_ERROR_WANT_WRITE:
                resposta = 0, obj->acaossl = 1;
                break;
            default:
                resposta = -1;
            }
#ifdef DEBUG_SSL
            printf("SSL_CONNECT = %d (acaossl=%d)\n", resposta, obj->acaossl);
            fflush(stdout);
#endif
            if (resposta >= 0) // Conex�o pendente
            {
                obj = obj->sDepois;
                continue;
            }
            sockAtual = obj;
            if (obj->FuncEvento("err", "Erro ao negociar SSL"))
                delete obj; // Apaga objeto se n�o foi apagado
                            // socket � fechado ao apagar objeto
            obj = sockAtual;
            continue;
        }
    // L� e processa dados pendentes
        if (obj->sockssl==0 && FD_ISSET(obj->sock, set_entrada)==0)
        {
            obj = obj->sDepois;
            continue;
        }
        while (true)
        {
        // L� do socket
            char mens[4096];
            int resposta, coderro=0;
            if (obj->sockssl)
            {
                coderro = 0;
                resposta=SslRead(obj->sockssl, mens, sizeof(mens));
                if (resposta <= 0)
                    switch (SslGetError(obj->sockssl, resposta))
                    {
                    case SSL_ERROR_NONE:
                        resposta = 0;
                        break;
                    case SSL_ERROR_WANT_READ:
                        resposta = 0, obj->acaossl = 0;
                        break;
                    case SSL_ERROR_WANT_WRITE:
                        resposta = 0, obj->acaossl = 1;
                        break;
                    default:
                        resposta = -1;
                    }
#ifdef DEBUG_SSL
                printf("SSL_READ = %d (acaossl=%d)\n", resposta, obj->acaossl);
                fflush(stdout);
#endif
            }
            else
            {
#ifdef __WIN32__
                resposta = recv(obj->sock, mens, sizeof(mens), 0);
                if (resposta==0)
                    resposta=-1;
                else if (resposta==SOCKET_ERROR)
                {
                    coderro = WSAGetLastError();
                    resposta = (coderro==WSAEWOULDBLOCK ? 0 : -1);
                }
#else
                resposta = recv(obj->sock, mens, sizeof(mens), 0);
                if (resposta<=0)
                {
                    coderro = errno;
                    if (resposta<0 && (errno==EINTR || errno==EWOULDBLOCK || errno==ENOBUFS))
                        resposta=0;
                    else
                        resposta=-1;
                }
#endif
            }
        // Verifica se socket fechou
            if (resposta<0)
            {
                obj->FecharSock(coderro, 0);
                obj = obj->sDepois;
                break;
            }
        // Processa dados recebidos
            if (resposta)
            {
                sockAtual = obj;
                obj->Processa(mens, resposta);
                if (obj != sockAtual) // Se objeto socket foi apagado
                {
                    obj = sockAtual;
                    break;
                }
            }
        // Verifica se fim da leitura
            if (resposta == sizeof(mens))
                continue;
            if (obj->proto == spTelnet2 && obj->pontRec)
            {
                sockAtual = obj;
                if (!obj->EventoMens(false))
                {
                    obj = sockAtual;
                    break;
                }
            }
            obj = obj->sDepois;
            break;
        } // while (true)
    } // for
}

//------------------------------------------------------------------------------
void TSocket::Processa(const char * buffer, int tamanho)
{
    while (tamanho>0)
    {
    // Ignora CR/LF, quando estiver mudando de protocolo
        if ((*buffer==10 && dadoRecebido==13) ||
            (*buffer==13 && dadoRecebido==10))
            buffer++, tamanho--, dadoRecebido=0;
    // Protocolo Telnet
        if (proto==spTelnet1 || proto==spTelnet2)
        {
            unsigned char dado;
            while (tamanho)
            {
            // Obt�m pr�ximo caracter
                dado=*buffer++;
                tamanho--;
    //------- Para mostrar o que chegou
    //printf("[%d;%d] ",dado,dadoRecebido); fflush(stdout);
    //if (dado>=32 || dado=='\n')
    //   putchar(dado); else printf("\\%02x", dado); fflush(stdout);
    //printf(": %d %02x\n", dadoRecebido, CorAtual); fflush(stdout);
    //printf(": %d %02x\n", dado, pontTelnet); fflush(stdout);
            // Protocolo do Telnet

            // Primeiro caracter  - IAC
                if (pontTelnet==0)
                {
                    if (dado<7)
                        continue;
                    if (dado==255)
                    {
                        bufTelnet[pontTelnet++]=dado;
                        continue;
                    }
                    goto telnet_cor;
                }
            // Telnet: Mensagem comprida (SB + SE)
            // 255 250 ... 255 240 = mensagem
                if (pontTelnet>=3 && dado==240 &&
                        bufTelnet[pontTelnet-1]==255)
                {
                    if (bufTelnet[3]==1)
                        switch (bufTelnet[2])
                        {
                        case 24: // Quer saber o terminal
                            EnvMens("\xFF\xFA\x18\x00" // Comando
                                "INTMUD" // Nome
                                "\xFF\xF0", 12); // Fim do comando
                            break;
                        case 32: // Quer saber velocidade do terminal
                            EnvMens("\xFF\xFA\x20\x00"
                                "38400,38400\xFF\xF0", 17);
                            break;
                        case 35: // Quer saber display X
                            EnvMens("\xFF\xFA\x23\x00"
                                "x.y\xFF\xF0", 9);
                            break;
                        case 39: // Quer saber vari�veis de ambiente
                            EnvMens("\xFF\xFA\x27\x00\xFF\xF0", 6);
                            break;
                        }
                    pontTelnet=0;
                    continue;
                }
            // Telnet: Mensagem de 2 bytes
            // 255 255 = caracter 255
            // 255 240 = mensagem 240
            // 255 249 = go ahead
                if (pontTelnet==1)
                {
                    if (dado==255)
                    {
                        pontTelnet=0;
                        goto telnet_cor;
                    }
                    if (dado==249)
                    {
                        pontTelnet=0, dado=6;
                        goto telnet_cor;
                    }
                    if (dado==240)
                    {
                        pontTelnet=0;
                        continue;
                    }
                }
            // Telnet: Anota caracter no buffer
                if (pontTelnet>=(int)sizeof(bufTelnet))
                    pontTelnet--;
                bufTelnet[pontTelnet++]=dado;
            // Telnet: Mensagem de 3 bytes
            // 255 x y = mensagem x y
                if (pontTelnet!=3 || bufTelnet[1]==250)
                    continue;
                pontTelnet=0;
                if (bufTelnet[1]>=251 && bufTelnet[1]<=254)
                {
                    int ind = -1;
                    switch (bufTelnet[2])
                    {
                    case 1: ind=0; break; // Echo
                    case 3: ind=1; break; // Supress Go Ahead
                    case 5: ind=2; break; // Status
                    case 6: ind=3; break; // Timing Mark
                    case 24: ind=4; break; // Terminal Type
                    case 31: ind=5; break; // Window Size
                    case 32: ind=6; break; // Terminal Speed
                    case 33: ind=7; break; // Remote Flow Control
                    case 34: ind=8; break; // Linemode
                    case 36: ind=9; break; // Environment Variables
                    default:
                        bufTelnet[1] = (bufTelnet[1]<243 ? 254 : 252);
                    }
                    if (ind>=0) switch (bufTelnet[1])
                    {
                    case 251: // WILL -> responde DO
                        if ((telnetopc[ind] & 3) == 2)
                            ind = -1;
                        else
                            telnetopc[ind] = (telnetopc[ind] & 0xF0) | 2;
                        bufTelnet[1] = 253;
                        break;
                    case 252: // WONT -> responde DONT
                        if ((telnetopc[ind] & 3) == 3)
                            ind = -1;
                        else
                            telnetopc[ind] = (telnetopc[ind] & 0xF0) | 3;
                        bufTelnet[1] = 254;
                        break;
                    case 253: // DO -> responde WILL
                        if (telnetecho)
                            telnetecho--, ind=-1;
                        else if ((telnetopc[ind] & 0x30) == 0x20)
                            ind = -1;
                        else
                            telnetopc[ind] = (telnetopc[ind] & 15) | 0x20;
                        bufTelnet[1] = 251;
                        break;
                    case 254: // DONT -> responde WONT
                        if (telnetecho)
                            telnetecho--, ind=-1;
                        else if ((telnetopc[ind] & 0x30) == 0x30)
                            ind = -1;
                        else
                            telnetopc[ind] = (telnetopc[ind] & 15) | 0x30;
                        bufTelnet[1] = 252;
                        break;
                    }
                    if (ind < 0)
                        continue;
                    EnvMens((char*)bufTelnet, 3);
                    if (bufTelnet[2]==31) // Quer saber o tamanho da janela
                        EnvMens("\xFF\xFA\x1F" // Comando
                            "\x00\x50\x00\x18" // Tamanho
                            "\xFF\xF0", 9); // Fim do comando
                    if (bufTelnet[1]==253 && dado==1) // 255 251 1 = echo off
                    {
                        dado=4;
                        goto telnet_cor;
                    }
                    if (bufTelnet[1]==254 && dado==1) // 255 252 1 = echo on
                    {
                        dado=5;
                        goto telnet_cor;
                    }
                }
                continue;
telnet_cor:
            // Filtra cores do Telnet
                if (dadoRecebido==2)
                {
                    if (dado>' ' && ((dado|0x20)<'a' || (dado|0x20)>'z'))
                    {
                        if (pontESC<sizeof(bufESC)-2)
                            bufESC[pontESC++]=dado;
                        continue;
                    }
                    dadoRecebido=0;
                    if (dado!='m' || bufESC[0]!='[' || pontESC==0)
                        continue;
                    bufESC[pontESC]=';';
                    bufESC[pontESC+1]=0;
                    unsigned char * x = bufESC+1;
                    while (*x)
                    {
                        if (memcmp(x, "0;", 2)==0)
                            CorAtual=0x70;
                        else if (memcmp(x, "1;", 2)==0)
                            CorAtual|=0x80;
                        else if (memcmp(x, "22;", 2)==0)
                            CorAtual&=0x7F;
                        else if (x[0]=='3' && x[1]>='0' &&
                                 x[1]<='9' && x[2]==';')
                        {
                            CorAtual &= 0x8F;
                            if (x[1]>='8')
                                CorAtual |= 0x70;
                            else
                                CorAtual += (x[1]-'0')*16;
                        }
                        else if (x[0]=='4' && x[1]>='0' &&
                                 x[1]<='9' && x[2]==';')
                        {
                            CorAtual &= 0xF0;
                            if (x[1]<'8')
                                CorAtual += (x[1]-'0');
                        }
                        x++;
                        while (*x && *x!=';')
                            x++;
                        if (*x==';')
                            x++;
                    }
                    continue;
                }
            // Sequ�ncias de controle - caracter ESC
                if (dado==27)
                {
                    dadoRecebido=2;
                    pontESC=0;
                }
            // Pr�xima linha
                else if (dado==10 || dado==13)
                {
                    if (dado==13 ? dadoRecebido==10 : dadoRecebido==13)
                        dadoRecebido=0;
                    else
                    {
                        dadoRecebido = dado;
                        bufRec[pontRec+1] = CorAtual;
                    // Envia mensagem
                        if (!EventoMens(true))
                            return;
                        break;
                    }
                }
            // BackSpace
                else if (dado==8 || dado==127)
                {
                    if (pontRec>2)
                        pontRec-=2;
                }
            // Caracteres imprim�veis
                else if (dado>=' ' || (usaropctelnet && dado>=4 && dado<=7))
                {
                    if (pontRec<sizeof(bufRec)-4)
                    {
                        bufRec[pontRec] = dado;
                        bufRec[pontRec+1] = CorAtual;
                        if ((CorAtual & 15) == (CorAtual>>4))
                            bufRec[pontRec+1] ^= 0x80;
                        pontRec += 2;
                    }
                }
            } // while (tamanho)
        }

    // Protocolo IRC
        else if (proto==spIRC)
        {
            char dado;
            while (tamanho)
            {
            // Obt�m pr�ximo caracter
                dado=*buffer++;
                tamanho--;
    //printf(" CH(%d,%c) ", (unsigned char)dado,
    //            (unsigned char)dado>=32?dado:' '); fflush(stdout);
            // Filtra cores do IRC
                if (dado==3)
                {
                    dadoRecebido=21;
                    CorIRC1 = CorIRC2 = 0;
                    continue;
                }
                if (dadoRecebido>=21)
                {
                    dadoRecebido++;
                    if (dadoRecebido==23 && dado==',')
                    {
                        dadoRecebido++;
                        continue;
                    }
                    else if (dadoRecebido==24)
                    {
                        if (dado==',')
                            continue;
                    }
                    else if (dadoRecebido<=26 && dado>='0' && dado<='9')
                    {
                        if (dadoRecebido<=24)
                            CorIRC1 = CorIRC1*10 + dado-'0';
                        else
                            CorIRC2 = CorIRC2*10 + dado-'0';
                        continue;
                    }
                    if (dadoRecebido==22)
                        CorAtual=0x70;
                    else
                    {
                    // Muda a frente
                        if (dadoRecebido<=25)
                        {
                            CorAtual &= 15;
                            CorAtual |= convirc[CorIRC1&15] * 16;
                        }
                    // Muda frente e fundo
                        else
                            CorAtual = convirc[CorIRC1&15] * 16 +
                                    (convirc[CorIRC2&15] & 7);
                    }
                    dadoRecebido=0;
                }
            // Pr�xima linha
                else if (dado==10)
                {
                    bufRec[pontRec+1] = CorAtual;
                // Envia mensagem
                    if (!EventoMens(true))
                        return;
                }
            // BackSpace
                else if (dado==8 || dado==127)
                {
                    if (pontRec>2)
                        pontRec-=2;
                }
            // Caracteres imprim�veis
                else if ((unsigned char)dado>=' ')
                {
                    if (pontRec<sizeof(bufRec)-4)
                    {
                        bufRec[pontRec] = dado;
                        bufRec[pontRec+1] = CorAtual;
                        if ((CorAtual & 15) == (CorAtual>>4))
                            bufRec[pontRec+1] ^= 0x80;
                        pontRec += 2;
                    }
                }
            }
        }
    // Protocolo Papovox
        else if (proto==spPapovox)
        {
            dadoRecebido=0;
            for (; pontRec<3 && tamanho>0; tamanho--,buffer++) // Cabe�alho
                bufRec[pontRec++]=*buffer;
            if (pontRec && bufRec[0]!=1) // Verifica tipo de mensagem
            {
                FecharSock(-1, 0);
                return;
            }
            if (pontRec < 3) // Verifica se o cabe�alho j� chegou
                return;
            unsigned int mensagem =      // Tamanho da mensagem
                            3 + (unsigned char)bufRec[1]
                            + ((unsigned char)bufRec[2]<<8);
            if (mensagem >= SOCKET_REC) // Verifica tamanho da mensagem
            {
                FecharSock(-1, 0);
                return;
            }
            for (; tamanho>0 && pontRec<mensagem; tamanho--)
                bufRec[pontRec++] = *buffer++;
            if (pontRec<mensagem)
                return;
        // Envia mensagem
            if (!EventoMens(true))
                return;
        }
        else
            assert(0);
    } // while (tamanho>0)
}

//------------------------------------------------------------------------------
bool TSocket::EventoMens(bool completo)
{
// Prepara mensagem
    char texto[2048];
    char * dest = texto;
    char * fim = texto + sizeof(texto) - 7;

// Prepara - Telnet e IRC
    if (proto==spTelnet1 || proto==spTelnet2 || proto==spIRC)
    {
        if (cores&1) // Com cor
        {
            int cor = CorAnterior;
            bufRec[pontRec+1] = CorAtual;
            if ((CorAtual & 15) == (CorAtual>>4))
                bufRec[pontRec+1] ^= 0x80;
            for (unsigned int x=0; dest<fim; x+=2)
            {
                if (cor!=bufRec[x+1])
                {
                    if (bufRec[x+1]==0x70)
                        *dest++ = Instr::ex_barra_b;
                    else
                    {
                        cor ^= bufRec[x+1];
                        if (cor & 0xF0)
                        {
                            *dest++ = Instr::ex_barra_c;
                            *dest = ((bufRec[x+1] >> 4) & 15) + '0';
                            if (*dest > '9')
                                (*dest) += 7;
                            dest++;
                        }
                        if (cor & 7)
                        {
                            *dest++ = Instr::ex_barra_d;
                            *dest++ = (bufRec[x+1] & 7) + '0';
                        }
                    }
                    cor=bufRec[x+1];
                }
                if (x>=pontRec)
                    break;
                unsigned char ch = bufRec[x];
                if (ch>=4 && ch<=7)
                {
                    *dest++ = Instr::ex_barra_c;
                    *dest++ = ch + 'C';
                }
                else
                    *dest++ = ch;
            }
            CorAnterior = cor;
        }
        else // Sem cor
            for (unsigned int x=0; x<pontRec && dest<fim; x+=2)
            {
                unsigned char ch = bufRec[x];
                if (ch>=' ')
                    *dest++=ch;
            }
        if (proto==spIRC) // IRC
            CorAtual = 0x70;
        *dest=0;
    }
// Prepara - Papovox
    else if (proto==spPapovox)
    {
        int mensagem = 3+(unsigned char)bufRec[1] // Tamanho da mensagem
                    +((unsigned char)bufRec[2]<<8);
        assert(mensagem < SOCKET_REC);
        for (int x=3; x<mensagem && dest<fim; x++)
            if ((unsigned char)bufRec[x] >= ' ')
                *dest++ = bufRec[x];
        *dest=0;
    }
// Protocolo desconhecido ou n�o est� conectado
    else
    {
        pontRec = 0;
        return false;
    }

// Acerta vari�veis
    pontRec=0;
#ifdef DEBUG_MSG
    printf(">>>>>>> Recebeu %s\n", texto);
#endif

// Anti-flooder
    if (AFlooder && completo) // Verifica se anti-flooder ativado
    {
        if (AFlooder >= TempoIni + 60) // Condi��o de flooder
            return true;
        AFlooder += strlen(texto)/8+5; // Acerta AFlooder
        if (AFlooder <= TempoIni)  // Acerta valor m�nimo de AFlooder
            AFlooder = TempoIni + 1;
    }

// Processa a mensagem
    return FuncEvento("msg", texto, completo);
}
