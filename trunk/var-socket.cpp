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
#include "var-socket.h"
#include "variavel.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

#define MOSTRA_MSG  // Mostra o que enviou e o que recebeu

TSocket * TSocket::sInicio = 0;
TSocket * TSocket::sockObj = 0;
TVarSocket * TSocket::varObj = 0;
bool TSocket::boolenvpend = false;

//------------------------------------------------------------------------------
/// Conecta a um endereço
/**
 @param socknum Aonde colocará o socket objtido
 @param ender  Endereço a conectar
 @param porta  Porta
 @param conSock2 Se !=0, copia struct sockaddr_in para conSock2
 @retval 0 se conseguiu conectar, colocou o socket em socknum
 @retval !=0 se não conseguiu; retorna a mensagem de erro
 */
static const char * Conectar(int * socknum, const char * ender, int porta,
                            struct sockaddr_in * conSock2)
{
    int  conManip;
    struct sockaddr_in conSock;
    struct hostent *hnome;

    *socknum = 0;

#ifdef __WIN32__
    memset(&conSock,0,sizeof(conSock));
    conSock.sin_addr.s_addr=inet_addr(ender);
    if ( conSock.sin_addr.s_addr == INADDR_NONE )
    {
        if ( (hnome=gethostbyname(ender)) == NULL )
            return "Problema ao obter endereço IP";
        conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
    }
    conSock.sin_family=AF_INET;
    conSock.sin_port=htons( (u_short)porta );
    if ( (conManip = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        return "Problema em socket()";
    if ( connect(conManip, (struct sockaddr *)&conSock,
                                      sizeof(struct sockaddr)) == SOCKET_ERROR)
    {
        closesocket(conManip);
        return "Não consegui conectar";
    }
    unsigned long argp=1; // 0=bloquear  1=nao bloquear
    if ( ioctlsocket(conManip, FIONBIO, &argp) !=0 )
    {
        closesocket(conManip);
        return "ioctlsocket";
    }
#else
    memset(&conSock.sin_zero, 0, 8);
    conSock.sin_family=AF_INET;
    conSock.sin_port=htons(porta);
    conSock.sin_addr.s_addr=inet_addr(ender);
    if ( (conSock.sin_addr.s_addr) == (unsigned long)-1 )
    {
        if ( (hnome=gethostbyname(ender)) == NULL )
            return "Problema ao obter endereço IP";
        conSock.sin_addr = (*(struct in_addr *)hnome->h_addr);
    }
    if ( (conManip=socket(PF_INET,SOCK_STREAM,0)) ==-1)
        return "Problema em socket()";
    //printf("Conectando a %s porta %d\n",
    //            inet_ntoa(conSock.sin_addr), porta);
    if ( connect(conManip, (struct sockaddr *)&conSock,
                                      sizeof(struct sockaddr))==-1)
    {
        close(conManip);
        return "Não consegui conectar";
    }
    fcntl(conManip, F_SETFL, O_NONBLOCK);
#endif
    *socknum = conManip;
    if (conSock2)
        memcpy(conSock2, &conSock, sizeof(conSock));
    return 0;
}

//------------------------------------------------------------------------------
TSocket::TSocket()
{
// Coloca na lista ligada
    sAntes = 0;
    sDepois = sInicio;
    if (sInicio)
        sInicio->sAntes = this;
    sInicio = this;
// Acerta variáveis
    Inicio=0;
    sock=-1;
    pontEnv=0;
    pontTelnet=0;
    pontESC=0;
    pontRec=0;
    dadoRecebido=0;
    CorAtual=0x70;
}

//------------------------------------------------------------------------------
TSocket::~TSocket()
{
// Fecha Socket
    Fechar();
// Retira dos objetos TVarSocket
    while (Inicio)
        Inicio->MudarSock(0);
// Retira da lista ligada
    (sAntes ? sAntes->sDepois : sInicio) = sDepois;
    if (sDepois)
        sDepois->sAntes = sAntes;
// Acerta sObj
    if (sockObj==this)
        sockObj=sDepois;
}

//------------------------------------------------------------------------------
void TSocket::Fechar()
{
    if (sock<0)
        return;
    close(sock);
    sock=-1;
    pontRec = 0;
    pontEnv = 0;
}

//------------------------------------------------------------------------------
/** @param mensagem Endereço dos bytes a enviar
 *  @return true se conseguiu enviar, false se não conseguiu */
bool TSocket::EnvMens(const char * mensagem)
{
    if (sock<0)
        return false;

// Verifica se tem espaço no buffer para enviar
    int tamanho = strlen(mensagem);
    if (tamanho > SOCKET_ENV)
        return false;
    if (tamanho > SOCKET_ENV - (int)pontEnv)
    {
        EnvPend();
        if (tamanho > SOCKET_ENV - (int)pontEnv)
            return false;
    }
    boolenvpend = true;

// Mostra o que está enviando
#ifdef MOSTRA_MSG
    printf(">>>>>>> Enviou(%d): ", tamanho);
    for (int x=0; x<tamanho; x++)
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

// Coloca no buffer
    for (; *mensagem; mensagem++)
        switch (*mensagem)
        {
        case Instr::ex_barra_n:
            bufEnv[pontEnv++] = '\n';
            break;
        case Instr::ex_barra_b:
        case Instr::ex_barra_c:
        case Instr::ex_barra_d:
            if (mensagem[1])
                mensagem++;
            break;
        default:
            bufEnv[pontEnv++] = *mensagem;
        }
    return true;
}

//------------------------------------------------------------------------------
/** @param mensagem Endereço dos bytes a enviar
 *  @param tamanho Tamanho da mensagem
 *  @return true se conseguiu enviar, false se não conseguiu */
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
    int resposta;
#ifdef MOSTRA_MSG
    printf(">>> ENV \"");
    for (unsigned int x=0; x<pontEnv; x++)
        putchar(bufEnv[x]);
    printf("\"\n");
#endif
#ifdef __WIN32__
    resposta=send(sock, bufEnv, pontEnv, 0);
    if (resposta==0)
        resposta=-1;
    else if (resposta==SOCKET_ERROR)
    {
        resposta=WSAGetLastError();
        resposta = (resposta==WSAEWOULDBLOCK ? 0 : -1);
    }
#else
    resposta=send(sock, bufEnv, pontEnv, 0);
    if (resposta<=0)
    {
        if (resposta<0 && (errno==EINTR || errno==EWOULDBLOCK || errno==ENOBUFS))
            resposta=0;
        else
            resposta=-1;
    }
#endif
    if (resposta<0)
        Fechar();
    else if (resposta>=(int)pontEnv)
        pontEnv=0;
    else if (resposta>0)
    {
        pontEnv -= resposta;
        memcpy(bufEnv, bufEnv+resposta, pontEnv);
    }
}

//------------------------------------------------------------------------------
bool TSocket::Aberto()
{
    return sock>=0;
}

//------------------------------------------------------------------------------
void TSocket::Fd_Set(fd_set * set_entrada, fd_set * set_saida)
{
    while (true)
    {
        boolenvpend = false;
    // Envia dados pendentes
    // Verifica se algum socket fechou
        for (TSocket * obj = sInicio; obj;)
        {
            obj->EnvPend();
            if (obj->sock >= 0)
            {
                obj=obj->sDepois;
                continue;
            }
            sockObj = obj;
        // Gera evento
            for (varObj = obj->Inicio; varObj;)
            {
                if (varObj->objeto)
                {
                    TObjeto * end = varObj->objeto;
                    char mens[80];
                    sprintf(mens, "%s_fechou", varObj->defvar+5);
                    varObj->MudarSock(0);
                    if (Instr::ExecIni(end, mens)==false)
                        end->MarcarApagar();
                    else
                        Instr::ExecX();
                    Instr::ExecFim();
                }
                else if (varObj->classe)
                {
                    TClasse * end = varObj->classe;
                    char mens[80];
                    sprintf(mens, "%s_fechou", varObj->defvar+5);
                    varObj->MudarSock(0);
                    if (Instr::ExecIni(end, mens)==false)
                        continue;
                    Instr::ExecX();
                    Instr::ExecFim();
                }
                else
                    varObj->MudarSock(0);
            }
        // Passa para próximo objeto TSocket
            if (sockObj==obj) // Se objeto Socket não foi apagado, apaga
                delete obj; // Fará automaticamente: sockObj=sockObj->Depois
            obj=sockObj;  // Passa para o próximo objeto
        }
        if (!boolenvpend)
            break;
    }
    for (TSocket * obj = sInicio; obj; obj=obj->sDepois)
    {
        FD_SET(obj->sock, set_entrada);
        if (obj->pontEnv)
            FD_SET(obj->sock, set_saida);
    }
}

//------------------------------------------------------------------------------
void TSocket::ProcEventos(fd_set * set_entrada, fd_set * set_saida)
{
    for (TSocket * obj = sInicio; obj; )
    {
    // Verifica se tem dados pendentes para ler
        if (obj->sock<0 || FD_ISSET(obj->sock, set_entrada)==0)
        {
            obj = obj->sDepois;
            continue;
        }
    // Lê e processa dados pendentes
        while (true)
        {
        // Lê do socket
            char mens[1024];
            int resposta;
#ifdef __WIN32__
            resposta = recv(obj->sock, mens, sizeof(mens), 0);
            if (resposta==0)
                resposta=-1;
            else if (resposta==SOCKET_ERROR)
            {
                resposta=WSAGetLastError();
                resposta = (resposta==WSAEWOULDBLOCK ? 0 : -1);
            }
#else
            resposta = recv(obj->sock, mens, sizeof(mens), 0);
            if (resposta<=0)
            {
                if (resposta<0 && (errno==EINTR || errno==EWOULDBLOCK || errno==ENOBUFS))
                    resposta=0;
                else
                    resposta=-1;
            }
#endif
        // Verifica se socket fechou
            if (resposta<0)
            {
                obj->Fechar();
                obj = obj->sDepois;
                break;
            }
        // Processa dados recebidos
            sockObj = obj;
            obj->Processa(mens, resposta);
        // Verifica se objeto socket foi apagado
            if (obj != sockObj)
            {
                obj = sockObj;
                break;
            }
        // Verifica se fim da leitura
            if (resposta != sizeof(mens))
            {
                obj = obj->sDepois;
                break;
            }
        } // while (true)
    } // for
}

//------------------------------------------------------------------------------
/// Processa dados recebidos em TSocket::ProcEventos
/** @param mensagem Endereço do buffer
 *  @param tamanho Tamanho do buffer
 *  @return Número de bytes recebidos */
void TSocket::Processa(const char * buffer, int tamanho)
{
    while (tamanho>0)
    {
        bool sair=true;
        char dado;
        while (tamanho)
        {
        // Obtém próximo caracter
            dado=*buffer++;
            tamanho--;
 //------- Para mostrar o que chegou
 //printf("[%d;%d] ",dado,dadoRecebido); fflush(stdout);
 //printf(" %d ", (unsigned char)dado);
 //if ((unsigned char)dado>=32 || dado=='\n')
 //   putchar(dado); else printf("\\%02d", dado); fflush(stdout);
        // Protocolo do Telnet

        // Primeiro caracter  - IAC
            if (pontTelnet==0)
            {
                if (dado==0)
                    continue;
                if (dado==(char)255)
                {
                    bufTelnet[pontTelnet++]=dado;
                    continue;
                }
            }
        // Mensagem comprida (SB + SE)
        // 255 250 ... 255 240 = mensagem
            if (pontTelnet>=3)
                if (bufTelnet[pontTelnet-1]==(char)255 && dado==(char)240)
                {
                    if (bufTelnet[2]==24 && bufTelnet[3]==1) // Quer saber o terminal
                    {
                        EnvMens("\xFF\xFA\x18\x00" // Comando
                               "TERM" // Nome
                               "\xFF\xF0", 10); // Fim do comando
                    }
                    pontTelnet=0;
                    continue;
                }
        // Anota caracter no buffer
            if (pontTelnet>=(int)sizeof(bufTelnet))
                pontTelnet--;
            if (pontTelnet)
                bufTelnet[pontTelnet++]=dado;
        // Mensagem de 2 bytes
        // 255 255 = caracter 255
        // 255 240 = mensagem 240
            if (pontTelnet==2)
            {
                if (dado==(char)255 || dado==(char)240)
                    pontTelnet=0;
                continue;
            }
        // Mensagem de 3 bytes
        // 255 x y = mensagem x y
            if (pontTelnet==3 && bufTelnet[1]!=(char)250)
            {
                if (bufTelnet[1]==(char)251 || bufTelnet[1]==(char)253)
                {
                    bufTelnet[1]^=6;
                    switch (bufTelnet[2])
                    {
                    case 1:
                    case 3:
                    case 5:
                    case 6:
                    case 24:
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                    case 36:
                        break;
                    default: bufTelnet[1]++;
                    }
                    EnvMens(bufTelnet, 3);
                }
                if (bufTelnet[2]==31) // Quer saber o tamanho da janela
                    EnvMens("\xFF\xFA\x1F" // Comando
                           "\x00\x50\x00\x19" // Tamanho
                           "\xFF\xF0", 9); // Fim do comando
                pontTelnet=0;
                continue;
            }
            if (pontTelnet)
                continue;
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
                char * x = bufESC+1;
                while (*x)
                {
                    if (memcmp(x, "0;", 2)==0)
                        CorAtual=0x70;
                    else if (memcmp(x, "1;", 2)==0)
                        CorAtual|=0x80;
                    else if (memcmp(x, "22;", 2)==0)
                        CorAtual&=0x7F;
                    else if (x[0]=='3' && x[1]>='0' && x[1]<='9' && x[2]==';')
                    {
                        CorAtual &= 0x8F;
                        if (x[1]>='8')
                            CorAtual |= 0x70;
                        else
                            CorAtual += (x[1]-'0')*16;
                    }
                    else if (x[0]=='4' && x[1]>='0' && x[1]<='9' && x[2]==';')
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
 //printf(" CH(%d,%c) ", (unsigned char)dado, (unsigned char)dado>=32?dado:' '); fflush(stdout);
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
                    const char convirc[] = {
                        15, // 0=white -> 7=branco
                        0, // 1=black -> 0=preto
                        4, // 2=blue  -> 4=azul
                        2, // 3=green -> 2=verde
                        1, // 4=red   -> 1=vermelho
                        3, // 5=brown -> 3=marrom
                        5, // 6=purple -> 5=magenta ???
                        9, // 7=orange -> vermelho forte (não tinha outra cor)
                        11, // 8=yellow
                        10, // 9=lt.green
                        6,  // 10=teal (a kinda green/blue cyan)
                        14, // 11=lt.cyan
                        12, // 12=lt.blue
                        13, // 13=pink
                        8,  // 14=grey
                        7 };  // 15=lt.grey
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
        // Sequências de controle - caracter ESC
            if (dado==27)
            {
                dadoRecebido=2;
                pontESC=0;
            }
        // Próxima linha
            else if (dado==10 || dado==13)
            {
                if (dado==13 ? dadoRecebido==10 : dadoRecebido==13)
                    dadoRecebido=0;
                else
                {
                    dadoRecebido=dado;
                    sair=false;
                    break;
                }
            }
        // BackSpace
            else if (dado==8 || dado==127)
            {
                if (pontRec>2)
                    pontRec-=2;
            }
        // Caracteres imprimíveis
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
        } // while (tamanho)
        if (sair)
            return;

    // Prepara mensagem
        char texto[2048];
        char * dest = texto;
        char * fim = texto + sizeof(texto) - 6;
        int cor = 0xFFF;
        for (unsigned int x=0; x<pontRec && dest<fim; x+=2)
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
            *dest++ = bufRec[x];
        }
        *dest=0;
        pontRec=0;
#ifdef MOSTRA_MSG
        printf(">>>>>>> Recebeu %s\n", texto);
#endif

    // Processa a mensagem
        for (TVarSocket * vobj = Inicio; vobj;)
        {
        // Definido em objeto: prepara para executar
            if (vobj->objeto)
            {
                char mens[80];
                sprintf(mens, "%s_msg", vobj->defvar+5);
                if (Instr::ExecIni(vobj->objeto, mens)==false)
                {
                    vobj = vobj->Depois;
                    continue;
                }
            }
        // Definido em classe: prepara para executar
            else if (vobj->classe)
            {
                char mens[80];
                sprintf(mens, "%s_msg", vobj->defvar+5);
                if (Instr::ExecIni(vobj->classe, mens)==false)
                {
                    vobj = vobj->Depois;
                    continue;
                }
            }
        // Variável local: não executa nenhuma função
            else
                vobj = vobj->Depois;
        // Executa
            varObj = vobj;
            Instr::ExecArg(texto);
            Instr::ExecX();
            Instr::ExecFim();
        // Verifica se objeto foi apagado
        // Passa para próximo objeto
            if (vobj == varObj)
                vobj = vobj->Depois;
            else if (sockObj == this)
                vobj = varObj;
            else
                return;
        } // for (TVarSocket ...
    } // while (tamanho>0)
}

//------------------------------------------------------------------------------
void TVarSocket::MudarSock(TSocket * obj)
{
// Verifica se o endereço vai mudar
    if (obj == Socket)
        return;
// Retira da lista
    if (Socket)
    {
        (Antes ? Antes : Socket->Inicio) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        if (Socket->Inicio == 0)
            delete Socket;
    // Acerta TSocket::varObj
        if (TSocket::varObj == this)
            TSocket::varObj =Depois;
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
    if (Socket==0)
        return;
    (Antes ? Antes->Depois : Socket->Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
}

//------------------------------------------------------------------------------
void TVarSocket::Igual(TVarSocket * v)
{
    MudarSock(v->Socket);
}

//------------------------------------------------------------------------------
bool TVarSocket::Func(TVariavel * v, const char * nome)
{
// Fecha Socket
    if (comparaZ(nome, "fechar")==0)
    {
        if (Socket)
            delete Socket;
        return false;
    }
// Envia mensagem
    if (comparaZ(nome, "msg")==0)
    {
        if (Socket==0)
            return false;
        int enviou = true;
        for (TVariavel * obj=v+1; obj<=Instr::VarAtual; obj++)
            if (Socket->EnvMens(obj->getTxt())==false)
            {
                enviou = false;
                break;
            }
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarInt))
            return false;
        if (enviou)
            Instr::VarAtual->setInt(1);
        return true;
    }
// Conecta
    if (comparaZ(nome, "abrir")==0)
    {
        MudarSock(0);
        if (Instr::VarAtual - v < 2)
            return false;
        int porta = v[2].getInt();
        const char * ender = v[1].getTxt();
    printf(">>>>>>> Abrir(%s, %d)\n", ender, porta);
        int sock=-1;
        if (porta<1 || porta>65535)
            return false;
        ender = Conectar(&sock, ender, porta, 0);
        if (ender==0)
        {
            Socket = new TSocket();
            Socket->Inicio = this;
            Socket->sock = sock;
        }
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        Instr::VarAtual->endfixo = (ender ? ender : "");
        return true;
    }
// Função ou variável desconhecida
    return false;
}

//------------------------------------------------------------------------------
int TVarSocket::getValor()
{
    return Socket != 0;
}
