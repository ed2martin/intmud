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

#include <stdio.h>      // Entrada/Sa�da padr�o
#include <stdlib.h>     // Biblioteca padr�o
#include <string.h>     // Fun��es string
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#ifdef __WIN32__
    // O n�mero de sockets padr�o no Windows � 64
    // Se precisar mudar, definir FD_SETSIZE aqui, antes de winsock.h
 #define FD_SETSIZE 128
 #include <windows.h>
 #include <winsock.h>
 #include <fcntl.h>
 #include <io.h>
#else
 #include <sys/types.h>
 #include <fcntl.h>
 #include <signal.h>
 #include <unistd.h>     // read e write
 #include <sys/time.h>
 #include <ulimit.h>
 #include <sys/resource.h>
 #include <sys/vtimes.h>
#endif
#include <time.h>       // Obter data/hora do sistema
#include "config.h"
#include "arqler.h"
#include "classe.h"
#include "arqmapa.h"
#include "instr.h"
#include "variavel.h"
#include "socket.h"
#include "var-sav.h"
#include "var-serv.h"
#include "var-outros.h"
#include "var-listaobj.h"
#include "var-log.h"
#include "random.h"
#include "misc.h"

//#define CORE    // Para gerar arquivos core
#define DEBUG   // Para n�o colocar o programa em segundo plano

void Inicializa(const char * arg);
void Termina();

//------------------------------------------------------------------------------
#ifdef __WIN32__
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int)
#else
int main(int argc, char *argv[])
#endif
{
// Inicializa��o
#ifdef __WIN32__
    Inicializa(lpCmdLine);
#else
    Inicializa(argc<=1 ? "" : argv[1]);
#endif

// Coloca o programa em segundo plano
#if !defined DEBUG && !defined __WIN32__
    {
        fflush(stdout);
        switch (fork())
        {
        case -1:
            perror("Erro em fork()");
            exit(EXIT_FAILURE);
        case 0:
            // Processo "filho" retorna com _exit(0)
            break;
        default:
            exit(EXIT_SUCCESS);
        }
        setsid();
        switch (fork())
        {
        case -1:
            exit(EXIT_FAILURE);
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
        }
        int arq=open("/dev/null",O_RDWR);
        dup2(arq,STDIN_FILENO);
        dup2(arq,STDOUT_FILENO);
        dup2(arq,STDERR_FILENO);
        close(arq);
    }
#endif

// Processamento de eventos
    struct timeval tselect;
    unsigned short atual=0,tempo=0; // Para calcular tempo decorrido
    unsigned short espera;
    fd_set set_entrada;
    fd_set set_saida;
    fd_set set_err;
    FD_ZERO(&set_entrada);
    FD_ZERO(&set_saida);
    FD_ZERO(&set_err);
    while (true)
    {
    // Obt�m: espera = tempo decorrido
        espera = atual;
#ifdef __WIN32__
        atual = timeGetTime()/100;
#else
        gettimeofday(&tselect,0);
        atual = (tselect.tv_sec&0xFFFF)*10 + tselect.tv_usec/100000;
#endif
        espera = atual - espera;
        if (espera>0x800) espera=0;
        if (espera>TESPERA_MAX) espera=TESPERA_MAX;

    // Eventos de IntTempo
        if (espera>0)
            TempoIni += espera;
        TVarIntTempo::ProcEventos(espera);

    // Chama eventos serv e socket
        TVarServ::ProcEventos(&set_entrada);
        TSocket::ProcEventos(espera, &set_entrada, &set_saida, &set_err);

    // Chama eventos de arqsav
        TVarSav::ProcEventos(espera);

    // Limpa objetos de listaobj e listaitem
        TGrupoX::ProcEventos();

    // Grava logs pendentes
    // Obt�m tempo de espera conforme TVarLog
        espera = TVarLog::TempoEspera(espera);

    // Prepara vari�veis para select()
    // Acerta tempo de espera conforme TSocket
        FD_ZERO(&set_entrada);
        FD_ZERO(&set_saida);
        FD_ZERO(&set_err);
        TVarServ::Fd_Set(&set_entrada);
        int esp = TSocket::Fd_Set(&set_entrada, &set_saida, &set_err);
        if (espera>esp)
            espera=esp;

    // Acerta tempo de espera conforme TVarIntTempo
        esp = TVarIntTempo::TempoEspera();
        if (espera>esp)
            espera=esp;

    // Acerta tempo de espera conforme TVarSavDir
        if (espera>10 && TVarSavDir::ChecaPend())
            espera=10;

#ifdef __WIN32__
        tempo = timeGetTime()/100;
#else
        gettimeofday(&tselect,0);
        tempo = (tselect.tv_sec&0xFFFF)*10 + tselect.tv_usec/100000;
#endif
        tempo-=atual;
        if (espera<tempo)
            espera=0;
        else
            espera-=tempo;

    // Espera
        tselect.tv_sec = espera/10;
        tselect.tv_usec = espera%10*100000;
#ifdef __WIN32__
        // Nota:
        // Se n�o especificar nenhum socket, o Windows retorna SOCKET_ERROR
        // Se acontecer da rede falhar, tamb�m pode retornar SOCKET_ERROR
        // Nesse caso, usa-se Sleep() para esperar 100 milissegundos
        if (select(FD_SETSIZE, &set_entrada, &set_saida, 0, &tselect)
                == SOCKET_ERROR)
            Sleep(100);
#else
        select(FD_SETSIZE, &set_entrada, &set_saida, 0, &tselect);
#endif
    }

// Fim
    Termina();
    return 0;
}

//------------------------------------------------------------------------------
/// Inicializa��o do programa
/// @param arg Nome do arquivo .map, ou "" se n�o foi especificado
void Inicializa(const char * arg)
{
// Vari�veis
    char mens[2048];
    bool erro = false;
    FILE * log = stdout; // Para onde enviar as mensagens
    TArqLer arq;

// Gerar arquivos core
#if defined CORE && !defined __WIN32__
    struct rlimit limites;
    limites.rlim_cur = 4000000;
    limites.rlim_max = 4000000;
    if (setrlimit(RLIMIT_CORE, &limites)<0)
    {
        perror("ULIMIT");
        exit(EXIT_FAILURE);
    }
    // Muda prioridade, para o programa executar o mais
    // lento poss�vel
    //setpriority(PRIO_PROCESS, (int)getpid(), PRIO_MAX);
#endif

// Inicializa vari�veis
    tabASCinic();   // Prepara tabela ASCII
    circle_srandom(time(0)); // Para gerar n�meros aleat�rios
    TVarIntTempo::PreparaIni(); // Vari�veis inttempo

// Obt�m nome do programa: arqnome, arqinicio e arqext
    {
        char nome[0xC000];
        char * pnome = nome;
        const char * endfim = nome + sizeof(nome) - 1;

    // Verifica se nome � nulo
        while (*arg==' ')
            arg++;
        if (*arg==0)
            arg = PACKAGE;

    // Obt�m primeiro argumento em nome[]
    // Obt�m: pnome = endere�o do 0 no final
#ifdef __WIN32__
        bool aspas = false;
        for (; *arg && pnome<endfim; arg++)
        {
            if (*arg=='\"')
                aspas = !aspas;
            else if (*arg==' ' && !aspas)
                break;
            else
                *pnome++ = *arg;
        }
        *pnome = 0;
#else
        pnome = copiastr(nome, arg, sizeof(nome));
#endif

    // Muda para o diret�rio do nome
    // Obt�m: pnome = endere�o do nome do arquivo sem o diret�rio
#ifdef __WIN32__
        for (; pnome>=nome; pnome--)
            if (*pnome=='\\')
            {
                *pnome = 0;
                if (!SetCurrentDirectory(nome))
                    exit(EXIT_FAILURE);
                break;
            }
        pnome++;
#else
        for (; pnome>=nome; pnome--)
            if (*pnome=='/')
            {
                *pnome = 0;
                if (chdir(nome)<0)
                    exit(EXIT_FAILURE);
                break;
            }
        pnome++;
#endif

    // Copia o nome do arquivo para o in�cio de nome[]
    // Obt�m: pnome = endere�o do primeiro byte ap�s o 0
        if (pnome > nome)
            pnome = copiastr(nome, pnome);
        else
            for (pnome = nome; *pnome;)
                pnome++;
        pnome++;

    // Acerta nomearq
#ifdef __WIN32__
        if (GetCurrentDirectory((DWORD)(endfim-pnome), pnome) <= 0)
            exit(EXIT_FAILURE);
        arqnome = new char[strlen(nome) + strlen(pnome) + 70];
        sprintf(arqnome, "%s\\%s", pnome, nome);
#else
        if (getcwd(pnome, endfim-pnome) == 0)
            exit(EXIT_FAILURE);
        arqnome = new char[strlen(nome) + strlen(pnome) + 70];
        sprintf(arqnome, "%s/%s", pnome, nome);
#endif

    // Obt�m arqinicio
        arqinicio = arqnome;
        if (*nome)
            arqinicio += 1 + strlen(pnome);

    // Obt�m arqext
        arqext = arqinicio + strlen(arqinicio);
        if (arqext > arqinicio + 4)
            if (arqext[-4]=='.' && (arqext[-3]|0x20)=='m' &&
                    (arqext[-2]|0x20)=='a' && (arqext[-1]|0x20)=='p')
                arqext -= 4;
        *arqext = 0;
    }

// Obt�m arquivo de log
#ifdef __WIN32__
    strcpy(arqext, ".log");
    log=fopen(arqnome, "w");
    if (log==0)
        exit(EXIT_FAILURE);
#endif
    fprintf(log, "IntMUD vers�o " VERSION " (Interpretador MUD)\n");

// L� arquivo de configura��o
    strcpy(arqext, ".cfg");
    if (!arq.Abrir(arqnome))
    {
        fprintf(log, "Abrindo arquivo %s: %s\n", arqinicio, strerror(errno));
        exit(EXIT_FAILURE);
    }
    while (true)
    {
        int linha = arq.Linha(mens, sizeof(mens));
        if (linha<0) // Erro
        {
            fprintf(log, "Lendo arquivo %s: %s\n", arqinicio, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (linha==0) // Fim do arquivo
            break;
        if (*mens=='#') // Coment�rio
            continue;
        char * valor = mens;
        while (*valor && *valor!='=') // Procura um igual
            valor++;
        if (*valor=='=')
        {
            char * p = valor;
            for (; p>mens; p--) // Retira espa�os antes do igual
                if (p[-1]!=' ')
                    break;
            *p=0;
            for (valor++; *valor==' '; valor++);
        }
        //printf("%d [%s] [%s]\n", linha, mens, valor);
        if (comparaZ(mens, "exec")==0)
            Instr::VarExecIni = atoi(valor);

    }

// Cria classes a partir dos arquivos .map
    TArqMapa * mapa = new TArqMapa(""); // Arquivo intmud.map
    mapa->Existe = true;

    // Abre intmud.map
    strcpy(arqext, ".map");
    if (!arq.Abrir(arqnome))
    {
        fprintf(log, "Abrindo arquivo %s: %s\n", arqinicio, strerror(errno));
        exit(EXIT_FAILURE);
    }
    while (true)
    {
    // L� pr�xima linha
        int linhanum = arq.Linha(mens, sizeof(mens));
        if (linhanum<0) // Erro
        {
            fprintf(log, "Lendo arquivo %s: %s\n", arqinicio, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (linhanum==0) // Fim do arquivo
        {
            mapa = mapa->Proximo;
            if (mapa==0)
                break;
            mprintf(arqext, 60, "-%s.map", mapa->Arquivo);
            if (!arq.Abrir(arqnome))
            {
                fprintf(log, "Abrindo arquivo %s: %s\n",
                        arqinicio, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }

    // Verifica linha MAPAGRANDE, que indica v�rios arquivos
        if (comparaZ(mens, "MAPAGRANDE")==0)
        {
            if (TArqMapa::MapaGrande) // Se j� obteve a lista de arquivos
            {
                fprintf(log, "%s:%d: Instru��o repetida - MAPAGRANDE\n",
                            arqinicio, linhanum);
                erro=true;
                continue;
            }
            if (TClasse::RBfirst()) // Se j� criou alguma classe
            {
                fprintf(log, "%s:%d: Instru��o MAPAGRANDE n�o pode "
                             "pertencer a uma classe\n", arqinicio, linhanum);
                erro=true;
                continue;
            }

        // Abre diret�rio
            TArqMapa::MapaGrande = true;
            DIR * dir=opendir(".");
            dirent * sdir;
            if (dir==0)
            {
                fprintf(log, "Procurando arquivos: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

        // Obt�m nomes dos arquivos
            copiastr(arqext, "-");
            int tam = strlen(arqinicio);
            while ( (sdir=readdir(dir))!=0 )
            {
                char * pont=sdir->d_name;
                if (strlen(pont)<=4 || memcmp(arqinicio, pont, tam)!=0)
                    continue;
                for (; *pont; pont++);
                pont-=4;
                if ( pont[0]!='.' || (pont[1]|0x20)!='m' ||
                     (pont[2]|0x20)!='a' || (pont[3]|0x20)!='p' )
                    continue;
                copiastr(mens, sdir->d_name+tam, sizeof(mens));
                for (pont=mens; *pont; pont++);
                if (pont<mens+4)
                    continue;
                pont[-4]=0;
                if (TArqMapa::NomeValido(mens))
                {
                    TArqMapa * m = new TArqMapa(mens);
                    m->Existe = true;
                }
                //printf("%s\n", sdir->d_name); fflush(stdout);
            }
            closedir(dir);
            copiastr(arqext, ".map");
            continue;
        }

    // Verifica se � in�cio de classe
        if (*mens!='[')
            continue;

    // Verifica nova classe
        char * p = mens+1;
        while (*p) p++;
        if (p[-1]!=']')
            continue;
        for (p--; p[-1]==' '; p--);
        *p=0;

    // Verifica se classe � v�lida ou j� existe
        if (TClasse::NomeValido(mens+1)==false)
        {
            fprintf(log, "%s:%d: Classe inv�lida: [%s]\n",
                            arqinicio, linhanum, mens+1);
            erro=true;
            continue;
        }
        if (TClasse::Procura(mens+1))
        {
            fprintf(log, "%s:%d: Classe repetida: [%s]\n",
                            arqinicio, linhanum, mens+1);
            erro=true;
            continue;
        }

    // Cria classe
        new TClasse(mens+1, mapa);
        mapa->Mudou = false;
    }

// Verifica se ocorreu erro no mapa
    if (erro)
        exit(EXIT_FAILURE);

// Para testes - mostra arquivos e classes
#if 0
    printf("Classes:");
    for (TClasse * obj = TClasse::RBfirst(); obj; obj=TClasse::RBnext(obj))
        printf(" [%s]", obj->Nome);
    putchar('\n');
    if (TArqMapa::Inicio)
    {
        printf("Arquivos:");
        for (TArqMapa * obj = TArqMapa::Inicio; obj; obj=obj->Proximo)
        {
            if (*obj->Arquivo==0)
                strcpy(arqext, ".map");
            else
                mprintf(arqext, 60, "-%s.map", obj->Arquivo);
            printf(" %s", arqinicio);
        }
        putchar('\n');
    }
#endif

// Obt�m instru��es das classes
// Acerta TClasse::Comandos
    TClasse * classeatual = 0;
    Instr::ChecaLinha checalinha;
    char comando[65000];
    unsigned int pcomando=0;

    // Abre intmud.map
    strcpy(arqext, ".map");
    if (!arq.Abrir(arqnome))
    {
        fprintf(log, "Abrindo arquivo %s: %s\n", arqinicio, strerror(errno));
        exit(EXIT_FAILURE);
    }
    mapa = TArqMapa::Inicio;
    while (true)
    {
    // L� pr�xima linha
        int linhanum = arq.Linha(mens, sizeof(mens));
        if (linhanum<0) // Erro
        {
            fprintf(log, "Lendo arquivo %s: %s\n", arqinicio, strerror(errno));
            exit(EXIT_FAILURE);
        }
        // Verifica se � nome de classe
        char * pclasse = 0;
        if (linhanum>0 && *mens=='[')
        {
            for (pclasse = mens+1; *pclasse; pclasse++);
            if (pclasse[-1]!=']')
                pclasse=0;
        }
        if (linhanum==0 || pclasse) // Fim do arquivo ou pr�xima classe
        {
            if (classeatual) // Anota instru��es da classe
            {
                comando[pcomando]=0;    // Marca fim da lista de comandos
                comando[pcomando+1]=0;
                pcomando += 2;
                classeatual->Comandos = new char[pcomando]; // Anota comandos
                memcpy(classeatual->Comandos, comando, pcomando);
            }
            pcomando = 0;
            classeatual = 0;
            checalinha.Inicio();
        }
        if (linhanum==0) // Fim do arquivo
        {
        // Avan�a para pr�ximo arquivo, se existir
            if (mapa)
                mapa = mapa->Proximo;
            if (mapa==0)
                break;
            mprintf(arqext, 60, "-%s.map", mapa->Arquivo);
            if (!arq.Abrir(arqnome))
            {
                fprintf(log, "Abrindo arquivo %s: %s\n",
                        arqinicio, strerror(errno));
                exit(EXIT_FAILURE);
            }
            continue;
        }
    // Verifica linha MAPAGRANDE
        if (comparaZ(mens, "MAPAGRANDE")==0)
            continue;
    // Verifica classe
        if (pclasse)
        {
            for (pclasse--; pclasse[-1]==' '; pclasse--);
            *pclasse=0;
            classeatual = TClasse::Procura(mens+1);
            if (classeatual==0)
            {
                fprintf(log, "%s:%d: Classe n�o encontrada: [%s]\n",
                            arqinicio, linhanum, mens+1);
                erro=true;
                break;
            }
            continue;
        }
    // Instru��es antes da defini��o da classe
        if (classeatual==0)
        {
            fprintf(log, "%s:%d: Instru��es n�o pertencem a nenhuma classe\n",
                        arqinicio, linhanum);
            erro=true;
            continue;
        }
    // Codifica instru��o
        //fprintf(log, "= %s\n", mens);
        if (pcomando + sizeof(mens) >= sizeof(comando))
        {
            fprintf(log, "%s:%d: Espa�o insuficiente; classe muito grande\n",
                        arqinicio, linhanum);
            erro=true;
            continue;
        }
        if (!Instr::Codif(comando+pcomando, mens,
                            sizeof(comando)-pcomando))
        {
            fprintf(log, "%s:%d: %s\n",
                        arqinicio, linhanum, comando+pcomando);
            erro=true;
            continue;
        }
    // Informa��es sobre o que codificou
#if 0
        unsigned tam = Num16(comando+pcomando);
        if (tam>sizeof(comando)-pcomando)
            tam=sizeof(comando)-pcomando;
        for (unsigned x=0; x<tam; x++)
            fprintf(log, " %02X", (unsigned char)comando[pcomando+x]);
        fprintf(log, "\n");

        if (Instr::Mostra(mens, comando+pcomando, sizeof(mens)))
            fprintf(log, "+ %s\n", mens);
        else
            fprintf(log, "- %s\n", mens);
        if (Instr::Decod(mens, comando+pcomando, sizeof(mens)))
            fprintf(log, "++ %s\n", mens);
        else
            fprintf(log, "-- %s\n", mens);
#endif
    // Verifica instru��o
        if (comando[pcomando+2]==Instr::cHerda)
        {
            const char * p = comando+pcomando+4;
            for (unsigned char x = comando[pcomando+3]; x; x--)
            {
                TClasse * obj = TClasse::Procura(p);
                assert(obj!=0);
                while (*p++);
                if (obj!=classeatual)
                    continue;
                fprintf(log, "%s:%d: Imposs�vel herdar a pr�pria classe\n",
                            arqinicio, linhanum);
                erro=true;
                continue;
            }
        }
        const char * p = checalinha.Instr(comando+pcomando);
        if (p)
        {
            fprintf(log, "%s:%d: %s\n", arqinicio, linhanum, p);
            erro=true;
            continue;
        }
        pcomando += Num16(comando+pcomando);
    }

// Verifica se ocorreu erro no mapa
    if (erro)
        exit(EXIT_FAILURE);

// Verifica se todas as classes foram anotadas
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        if (cl->Comandos==0)
        {
            fprintf(log, "Erro de fase; provavelmente algum arquivo mudou\n");
            exit(EXIT_FAILURE);
        }

// Acerta lista de classes derivadas
    TClasse::AcertaDeriv();

// Para testes - mostra classes e instru��es
#if 0
    printf("Classes:\n");
    for (TClasse * obj = TClasse::RBfirst(); obj; obj=TClasse::RBnext(obj))
    {
        printf("\n[%s]\n", obj->Nome);
        if (obj->NumDeriv)
        {
            printf("*** Herdada em:");
            for (int x=0; x<obj->NumDeriv; x++)
                printf("%s%s", x==0 ? " " : ", ", obj->ListaDeriv[x]->Nome);
            putchar('\n');
        }
        for (const char * p = obj->Comandos; Num16(p); p+=Num16(p))
        {
            char mens[2048];
            if (Instr::Decod(mens, p, sizeof(mens)))
                printf("%s\n", mens);
            else
            {
                printf("**** Erro\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    putchar('\n');
#endif

// Acerta vari�veis das classes
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        cl->AcertaVar();

#ifdef __WIN32__
// Inicializa WinSock
    WSADATA info;
    bool xinic=(WSAStartup(MAKELONG(1, 1), &info) != SOCKET_ERROR);
    if (!xinic)
    {
        fprintf(log, "Problema inicializando WinSock\n");
        exit(EXIT_FAILURE);
    }
    fclose(log);
#else
        // Ignora sinal PIPE
    signal(SIGPIPE, SIG_IGN);
#endif

// Executa fun��es iniclasse das classes
    for (TClasse * cl = TClasse::RBfirst(); cl;)
    {
        TClasse::ClInic = TClasse::RBnext(cl);
        if (Instr::ExecIni(cl, "iniclasse"))
        {
            Instr::ExecArg(cl->Nome);
            Instr::ExecX();
            Instr::ExecFim();
        }
        cl = TClasse::ClInic;
    }
}

//------------------------------------------------------------------------------
/// Encerra o programa
void Termina()
{
#ifdef __WIN32__
    WSACleanup();
#endif
    exit(EXIT_SUCCESS);
}
