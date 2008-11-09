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

#include <stdio.h>      // Entrada/Saída padrão
#include <stdlib.h>     // Biblioteca padrão
#include <string.h>     // Funções string
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#ifdef __WIN32__
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
#include "misc.h"

#define CORE    // Para gerar arquivos core
#define DEBUG   // Para não colocar o programa em segundo plano

void Inicializa();

//------------------------------------------------------------------------------
#ifdef __WIN32__
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#else
int main(int argc, char *argv[])
{
#endif
// Inicialização
    Inicializa();

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





// Fim
#ifdef __WIN32__
    WSACleanup();
#endif
    exit(EXIT_SUCCESS);
    return 0;
}

//------------------------------------------------------------------------------
// Inicialização
void Inicializa()
{
// Variáveis
    char mens[500];
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
    // lento possível
    //setpriority(PRIO_PROCESS, (int)getpid(), PRIO_MAX);
#endif

// Prepara tabela ASCII
    tabASCinic();

// Obtém o diretório atual + "intmud"
#ifdef __WIN32__
    int x = GetCurrentDirectory((DWORD)10, mens);
    if (x<=0)
        exit(EXIT_FAILURE);
    x += strlen(PACKAGE);
    arqnome = new char[x+70];
    GetCurrentDirectory((DWORD)x, arqnome); // Obtém o diretório atual
    for (arqinicio=arqnome; *arqinicio; arqinicio++);
    *arqinicio++ = '\\';
    strcpy(arqinicio, PACKAGE);
#else
    int x = 100;
    while (true)
    {
        arqnome = new char[x+70+strlen(PACKAGE)];
        if (getcwd(arqnome, x) == arqnome)
            break;
        delete[] arqnome;
        if (errno != ERANGE)
            exit(EXIT_FAILURE);
        x *= 2;
    }
    for (arqinicio=arqnome; *arqinicio; arqinicio++);
    *arqinicio++ = '/';
    strcpy(arqinicio, PACKAGE);
#endif
    arqext = arqinicio;
    while (*arqext) arqext++;

// Obtém arquivo de log
#ifdef __WIN32__
    strcpy(arqext, ".log");
    log=fopen(arqnome, "w");
    if (log==0)
        exit(EXIT_FAILURE);
#endif
    fprintf(log, "IntMUD versão " VERSION " (Interpretador MUD)\n");

// Lê arquivo de configuração
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
        if (*mens=='#') // Comentário
            continue;
        char * valor = mens;
        while (*valor && *valor!='=') // Procura um igual
            valor++;
        if (*valor=='=')
        {
            char * p = valor;
            for (; p>mens; p--) // Retira espaços antes do igual
                if (p[-1]!=' ')
                    break;
            *p=0;
            for (valor++; *valor==' '; valor++);
        }
        //printf("%d [%s] [%s]\n", linha, mens, valor);


    }

// Cria classes a partir dos arquivos .map
    TArqMapa * mapa = 0;

    // Abre intmud.map
    strcpy(arqext, ".map");
    if (!arq.Abrir(arqnome))
    {
        fprintf(log, "Abrindo arquivo %s: %s\n", arqinicio, strerror(errno));
        exit(EXIT_FAILURE);
    }
    while (true)
    {
    // Lê próxima linha
        int linhanum = arq.Linha(mens, sizeof(mens));
        if (linhanum<0) // Erro
        {
            fprintf(log, "Lendo arquivo %s: %s\n", arqinicio, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (linhanum==0) // Fim do arquivo
        {
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
        }

    // Verifica linha MAPAGRANDE, que indica vários arquivos
        if (comparaZ(mens, "MAPAGRANDE")==0)
        {
            if (TArqMapa::Inicio) // Se já obteve a lista de arquivos
            {
                fprintf(log, "%s:%d: Instrução repetida - MAPAGRANDE\n",
                            arqinicio, linhanum);
                erro=true;
                continue;
            }
            if (TClasse::RBfirst()) // Se já criou alguma classe
            {
                fprintf(log, "%s:%d: Instrução MAPAGRANDE não pode "
                             "pertencer a uma classe\n", arqinicio, linhanum);
                erro=true;
                continue;
            }

        // Abre diretório
            DIR * dir=opendir(".");
            dirent * sdir;
            if (dir==0)
            {
                fprintf(log, "Procurando arquivos: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

        // Obtém nomes dos arquivos
            mapa = new TArqMapa(""); // Arquivo intmud.map
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
                new TArqMapa(mens);
                //printf("%s\n", sdir->d_name); fflush(stdout);
            }
            closedir(dir);
            copiastr(arqext, ".map");
            continue;
        }

    // Verifica se é início de classe
        if (*mens!='[')
            continue;

    // Verifica nova classe
        char * p = mens+1;
        while (*p) p++;
        if (p[-1]!=']')
        {
            fprintf(log, "%s:%d: [ sem ]\n", arqinicio, linhanum);
            erro=true;
            continue;
        }
        for (p--; p[-1]==' '; p--);
        *p=0;

    // Verifica se classe é válida ou já existe
        if (TClasse::NomeClasse(mens+1)==false)
        {
            fprintf(log, "%s:%d: Classe inválida: [%s]\n",
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

    // Verifica se classe está no arquivo certo
        if (mapa)
        {
        // Obtém parte do nome do arquivo, sendo que "" é intmud.map
            for (p=mens+1; *p; p++)
                if (*p==' ')
                {
                    p=mens+1;
                    break;
                }
        // Verifica se bate com o nome do arquivo
            if (compara(mapa->Arquivo, p)!=0)
            {
                fprintf(log, "%s:%d: Classe [%s] deveria estar "
                            "no arquivo ", arqinicio, linhanum, mens+1);
                for (p=mens+1; *p && *p!=' '; p++);
                char x = *arqext;
                *arqext=0;
                if (*p==0)
                    fprintf(log, "%s.map\n", arqinicio);
                else
                {
                    *p=0;
                    fprintf(log, "%s-%s.map\n", arqinicio, mens+1);
                }
                *arqext = x;
                erro=true;
                continue;
            }
        }

    // Cria classe
        new TClasse(mens+1);
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

// Obtém instruções das classes
// Acerta TClasse::Comandos e TClasse::NumDeriv
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
    // Lê próxima linha
        int linhanum = arq.Linha(mens, sizeof(mens));
        if (linhanum<0) // Erro
        {
            fprintf(log, "Lendo arquivo %s: %s\n", arqinicio, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (linhanum==0 || *mens=='[') // Fim do arquivo ou próxima classe
        {
            if (classeatual) // Anota instruções da classe
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
        // Avança para próximo arquivo, se existir
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
        }
    // Verifica linha MAPAGRANDE
        if (comparaZ(mens, "MAPAGRANDE")==0)
            continue;
    // Verifica classe
        if (*mens=='[')
        {
            char * p = mens+1;
            while (*p) p++;
            if (p[-1]!=']')
            {
                fprintf(log, "%s:%d: [ sem ]\n", arqinicio, linhanum);
                erro=true;
                continue;
            }
            for (p--; p[-1]==' '; p--);
            *p=0;
            classeatual = TClasse::Procura(mens+1);
            if (classeatual==0)
            {
                fprintf(log, "%s:%d: Classe não encontrada: [%s]\n",
                            arqinicio, linhanum, mens+1);
                erro=true;
                break;
            }
            continue;
        }
    // Instruções antes da definição da classe
        if (classeatual==0)
        {
            fprintf(log, "%s:%d: Instruções não pertencem a nenhuma classe\n",
                        arqinicio, linhanum);
            erro=true;
            continue;
        }
    // Codifica instrução
        //fprintf(log, "= %s\n", mens);
        if (pcomando + 500 >= sizeof(comando))
        {
            fprintf(log, "%s:%d: Espaço insuficiente; classe muito grande\n",
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
    // Informações sobre o que codificou
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
    // Verifica instrução
        if (comando[pcomando+2]==Instr::cHerda)
        {
            const char * p = comando+pcomando+4;
            for (unsigned char x = comando[pcomando+3]; x; x--)
            {
                TClasse * obj = TClasse::Procura(p);
                assert(obj!=0);
                obj->NumDeriv++;
                while (*p++);
                if (obj!=classeatual)
                    continue;
                fprintf(log, "%s:%d: Impossível herdar a própria classe\n",
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
// Aloca memória em TClasse::ListaDeriv
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
    {
        if (cl->NumDeriv)
        {
            cl->ListaDeriv = new TClasse* [cl->NumDeriv];
            cl->NumDeriv = 0;
        }
        if (cl->Comandos==0)
        {
            fprintf(log, "Erro de fase; provavelmente algum arquivo mudou\n");
            exit(EXIT_FAILURE);
        }
    }

// Acerta lista de classes derivadas:
// TClasse::ListaDeriv e TClasse::NumDeriv
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
    {
    // Verifica se tem instrução herda
        if (cl->Comandos[0]==0 && cl->Comandos[1]==0)
            continue;
        if (cl->Comandos[2]!=Instr::cHerda)
            continue;
    // Checa classes herdadas
        const char * p = cl->Comandos+4;
        for (unsigned char x = cl->Comandos[3]; x; x--)
        {
            TClasse * obj = TClasse::Procura(p);
            assert(obj!=0);
            obj->ListaDeriv[ obj->NumDeriv++ ] = cl;
            while (*p++);
        }
    }

// Para testes - mostra classes e instruções
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
            char mens[500];
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

// Acerta lista de funções e variáveis das classes
// Primeiro das classes herdadas em outras
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
    {
        cl->AcertaComandos();
        if (cl->NumDeriv)
            cl->AcertaVar();
    }
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        cl->AcertaVar();



  // ***************************
  // Falta: executar as funções ini das classes
  // ***************************




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
#endif
}
