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
#include <stdarg.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#ifdef __WIN32__
    // O número de sockets padrão no Windows é 64
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
#endif
#include <time.h>       // Obter data/hora do sistema
#include "config.h"
#include "arqler.h"
#include "classe.h"
#include "arqmapa.h"
#include "instr.h"
#include "variavel.h"
#include "socket.h"
#include "console.h"
#include "var-sav.h"
#include "var-serv.h"
#include "var-outros.h"
#include "var-listaobj.h"
#include "var-texto.h"
#include "var-log.h"
#include "var-telatxt.h"
#include "random.h"
#include "misc.h"

// ulimit -S -c 20000
//#define CORE    // Para gerar arquivos core
//#define DEBUG   // Para não colocar o programa em segundo plano

//------------------------------------------------------------------------------
#if defined CORE && !defined __WIN32__
 #include <sys/resource.h>
 #include <sys/vtimes.h>
#endif

void Inicializa(const char * arg);
void Termina();
void TerminaSign(int sig);

static FILE * err_log = 0;
int err_tipo = 0;

//------------------------------------------------------------------------------
// Semelhante a sprintf(), exceto que:
// Só processa caracteres %%, %c, %d, %u e %s
void err_printf(const char * mens, ...)
{
    char txt[512];
    char * destino = txt;
    const char * fim = txt + sizeof(txt) - 1;
    const char * p;
    char * p2;
    char numero[20];
    int tamanho; // Para %d
    unsigned int utamanho; // Para %u
    va_list argp;

// Prepara mensagem
    va_start(argp, mens);
    for (; *mens; mens++)
    {
        if (*mens!='%')
        {
            if (destino<fim)
                *(destino++)=*mens;
            continue;
        }
        mens++;
        switch (*mens)
        {
        case '%':
            if (destino<fim)
                *(destino++)='%';
            break;
        case 'c':
            if (destino<fim)
                *(destino++)=va_arg(argp, int);
            break;
        case 's':
            for (p=va_arg(argp, char *); *p && destino<fim; p++,destino++)
                *destino=*p;
            break;
        case 'd':
            tamanho=va_arg(argp, int);
            if (destino>=fim)
                break;
            if (tamanho<0)
            {
                *destino++='-';
                tamanho=-tamanho;
            }
            for (p2=numero; p2<&numero[sizeof(numero)] && tamanho; tamanho/=10)
                *p2++=tamanho%10+'0';
            if (p2==numero)
                *p2++='0';
            for (p=p2-1; p>=numero && destino<fim; p--)
                *destino++=*p;
            break;
        case 'u':
            utamanho=(unsigned int)va_arg(argp, unsigned int);
            if (destino>=fim)
                break;
            for (p2=numero; p2<&numero[sizeof(numero)] && utamanho; utamanho/=10)
                *p2++=utamanho%10+'0';
            if (p2==numero)
                *p2++='0';
            for (p=p2-1; p>=numero && destino<fim; p--)
                *destino++=*p;
            break;
        default:
            mens--;
        }
    }
    *destino=0;
    va_end(argp);

// Envia mensagem
    const char msg1[] = "IntMUD versão " VERSION " (Interpretador MUD)\n";
    switch (err_tipo)
    {
    case 0:
        if (Console==0)
        {
            Console = new TConsole;
            if (!Console->Inic(false))
                exit(EXIT_FAILURE);
            Console->EnvTxt(msg1, strlen(msg1));
        }
        Console->EnvTxt(txt, destino-txt);
        break;
    case 1:
        if (err_log == 0)
        {
            strcpy(arqext, ".log");
            err_log = fopen(arqnome, "w");
            if (err_log==0)
                exit(EXIT_FAILURE);
            fwrite(msg1, 1, strlen(msg1), err_log);
        }
        fwrite(txt, 1, destino-txt, err_log);
        break;
    default:
        exit(EXIT_FAILURE);
    }
}

//------------------------------------------------------------------------------
void err_fim()
{
#ifdef __WIN32__
    if (Console==0)
        exit(EXIT_FAILURE);
    const char msg1[] = "Pressione ENTER para fechar\n";
    Console->EnvTxt(msg1, strlen(msg1));
    Console->CursorLin(-1);
    int total = Console->LinAtual;
    while (true)
    {
        Console->Flush();
        const char * tecla = Console->LerTecla();
        if (tecla==0)
        {
            Sleep(100);
            //struct timeval tselect;
            //tselect.tv_sec = 0;
            //tselect.tv_usec = 100000;
            //select(FD_SETSIZE, 0, 0, 0, &tselect);
            continue;
        }
        if (strcmp(tecla, "ENTER")==0 || strcmp(tecla, "ESC")==0)
            break;
        if (strcmp(tecla, "UP")==0 && Console->LinAtual>0)
            Console->CursorLin(-1);
        if (strcmp(tecla, "DOWN")==0 && (int)Console->LinAtual<total)
            Console->CursorLin(1);
    }
    Console->CursorLin(total - Console->LinAtual);
    Console->Flush();
    Console->Fim();
#endif
    exit(EXIT_FAILURE);
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
// Inicialização
    // Se precisar obter HINSTANCE no Windows:
    //HINSTANCE hInstance = GetModuleHandle(NULL);
    Inicializa(argc<=1 ? argv[0] : argv[1]);

// Coloca o programa em segundo plano
#if !defined DEBUG && !defined __WIN32__
    if (Console==0)
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
#ifdef __WIN32__
    atual = timeGetTime();
#else
    gettimeofday(&tselect,0);
    atual = tselect.tv_sec*1000LL + tselect.tv_usec/1000;
#endif
    while (true)
    {
    // Obtém: espera = tempo decorrido
        espera = atual;
#ifdef __WIN32__
        atual = timeGetTime(); // Tempo atual
#else
        gettimeofday(&tselect,0);
        atual = tselect.tv_sec*1000LL + tselect.tv_usec/1000;
#endif
        espera = atual - espera;// Quanto tempo se passou em milissegundos
        atual -= espera%100;    // Para passar para décimos de segundo
        espera /= 100;          // Arredonda para décimos de segundo
        if (espera>0x800) espera=0;
        if (espera>TESPERA_MAX) espera=TESPERA_MAX;

    // Eventos de IntTempo
        if (espera>0)
            TempoIni += espera;
        TVarIntTempo::ProcEventos(espera);

    // Eventos do console
        TVarTelaTxt::Processa();

    // Chama eventos serv e socket
        TVarServ::ProcEventos(&set_entrada, espera);
        TSocket::ProcEventos(&set_entrada, &set_saida, &set_err);

    // Chama eventos de arqsav
        TVarSav::ProcEventos(espera);

    // Chama eventos de intexec
        TVarIntExec::ProcEventos();

    // Limpa objetos de listaobj e listaitem
        TGrupoX::ProcEventos();

    // Limpa objetos de textotxt
        TTextoGrupo::ProcEventos();

    // Grava logs pendentes
    // Obtém tempo de espera conforme TVarLog
        espera = TVarLog::TempoEspera(espera);

    // Prepara variáveis para select()
    // Acerta tempo de espera conforme TVarServ
        FD_ZERO(&set_entrada);
        FD_ZERO(&set_saida);
        FD_ZERO(&set_err);
        TSocket::Fd_Set(&set_entrada, &set_saida, &set_err);
        int esp = TVarServ::Fd_Set(&set_entrada, &set_saida);
        if (espera > esp)
            espera = esp;

    // Processamento pendente no console
        TVarTelaTxt::ProcFim();
#ifdef __WIN32__
        espera = 2;
#else
        if (Console)
            FD_SET(Console->Stdin(), &set_entrada);
#endif

    // Acerta tempo de espera conforme TVarIntTempo
        esp = TVarIntTempo::TempoEspera();
        if (espera > esp)
            espera = esp;

    // Acerta tempo de espera conforme TVarSavDir
        if (espera>10 && TVarSavDir::ChecaPend())
            espera=10;
        if (espera>TESPERA_MAX) espera=TESPERA_MAX;

#ifdef __WIN32__
        tempo = timeGetTime();
#else
        gettimeofday(&tselect,0);
        tempo = tselect.tv_sec*1000LL + tselect.tv_usec/1000;
#endif
        tempo -= atual;
        espera *= 100;
        if (espera < tempo)
            espera = 0;
        else
            espera -= tempo;

    // Espera
        tselect.tv_sec = espera/1000;
        tselect.tv_usec = espera%1000*1000;
#ifdef __WIN32__
        // Nota:
        // Se não especificar nenhum socket, o Windows retorna SOCKET_ERROR
        // Se acontecer da rede falhar, também pode retornar SOCKET_ERROR
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
/// Inicialização do programa
/// @param arg Nome do arquivo .int, ou "" se não foi especificado
void Inicializa(const char * arg)
{
// Variáveis
    char mens[2048];
    bool erro = false; // Se ocorreu algum erro
    bool telatxt = false; // Se deve abrir janela do console
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

// Inicializa variáveis
    tabASCinic();   // Prepara tabela ASCII
    circle_srandom(time(0)); // Para gerar números aleatórios
    TVarIntTempo::PreparaIni(); // Variáveis inttempo

// Obtém nome do programa: arqnome, arqinicio e arqext
    {
        char nome[0xC000];
        char * pnome = nome;
        const char * endfim = nome + sizeof(nome) - 1;

    // Verifica se nome é nulo
        while (*arg==' ')
            arg++;
        if (*arg==0)
            arg = PACKAGE;

    // Obtém primeiro argumento em nome[]
    // Obtém: pnome = endereço do 0 no final
        pnome = copiastr(nome, arg, sizeof(nome));

    // Muda para o diretório do nome
    // Obtém: pnome = endereço do nome do arquivo sem o diretório
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

    // Copia o nome do arquivo para o início de nome[]
    // Obtém: pnome = endereço do primeiro byte após o 0
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

    // Obtém arqinicio
        arqinicio = arqnome;
        if (*nome)
            arqinicio += 1 + strlen(pnome);

    // Obtém arqext
        int tam = strlen(INT_EXT);
        arqext = arqinicio + strlen(arqinicio);
        if (arqext > arqinicio + tam + 1)
        {
            if (arqext[-tam-1]=='.' && comparaZ(arqext-tam, INT_EXT)==0)
                arqext -= tam + 1;
            else if (comparaZ(arqext-4, ".exe")==0)
                arqext -= 4;
        }
        *arqext = 0;
    }

// Cria classes a partir dos arquivos .int
    bool ini_arq = true;
    TArqMapa * mapa = new TArqMapa(""); // Arquivo intmud.int
    mapa->Existe = true;

    // Abre intmud.int
    strcpy(arqext, "." INT_EXT);
    if (!arq.Abrir(arqnome))
    {
        err_printf("Abrindo arquivo %s: %s\n", arqinicio, strerror(errno));
        err_fim();
    }
    while (true)
    {
    // Lê próxima linha
        int linhanum = arq.Linha(mens, sizeof(mens));
        if (linhanum<0) // Erro
        {
            err_printf("Lendo arquivo %s: %s\n", arqinicio, strerror(errno));
            err_fim();
        }
        if (linhanum==0) // Fim do arquivo
        {
            mapa = mapa->Proximo;
            if (mapa==0)
                break;
            mprintf(arqext, 60, "-%s." INT_EXT, mapa->Arquivo);
            if (!arq.Abrir(arqnome))
            {
                err_printf("Abrindo arquivo %s: %s\n",
                        arqinicio, strerror(errno));
                err_fim();
            }
            continue;
        }

    // Configuração
        if (ini_arq)
        {
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
            //printf("[%s] [%s]\n", mens, valor);

        // Verifica opções
            if (comparaZ(mens, "exec")==0)
                Instr::VarExecIni = atoi(valor);
            if (comparaZ(mens, "telatxt")==0)
                telatxt = atoi(valor);
            if (comparaZ(mens, "log")==0)
                err_tipo = (atoi(valor) != 0);

        // Verifica opção MAPAGRANDE, que indica vários arquivos
            if (comparaZ(mens, "mapagrande")==0 && atoi(valor))
            {
                if (TArqMapa::MapaGrande) // Se já obteve a lista de arquivos
                    continue;

            // Abre diretório
                TArqMapa::MapaGrande = true;
                DIR * dir=opendir(".");
                struct dirent * sdir;
                if (dir==0)
                {
                    err_printf("Procurando arquivos: %s\n", strerror(errno));
                    err_fim();
                }

            // Obtém nomes dos arquivos
                copiastr(arqext, "-");
                int tam = strlen(arqinicio);
                int ext = strlen(INT_EXT) + 1;
                while ( (sdir=readdir(dir))!=0 )
                {
                    char * pont=sdir->d_name;
                    if (strlen(pont)<=4 || memcmp(arqinicio, pont, tam)!=0)
                        continue;
                    for (; *pont; pont++);
                    pont-=ext;
                    if ( pont[0]!='.' || comparaZ(pont+1, INT_EXT)!=0)
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

            // Fecha diretório
                closedir(dir);
                copiastr(arqext, "." INT_EXT);
                continue;
            }
        }

    // Verifica se é definição de classe
        char * pclasse = TClasse::NomeDef(mens);
        if (pclasse==0)
            continue;
        ini_arq = false;

    // Verifica se classe é válida ou já existe
        if (TClasse::NomeValido(pclasse)==false)
        {
            err_printf("%s:%d: Classe inválida: %s\n",
                            arqinicio, linhanum, pclasse);
            erro=true;
            continue;
        }
        if (TClasse::Procura(pclasse))
        {
            err_printf("%s:%d: Classe repetida: %s\n",
                            arqinicio, linhanum, pclasse);
            erro=true;
            continue;
        }

    // Cria classe
        new TClasse(pclasse, mapa);
        mapa->Mudou = false;
    }
    if (TClasse::RBfirst()==0)
    {
        err_printf("Nenhuma classe foi definida\n");
        erro=true;
    }

// Verifica se ocorreu erro no mapa
    if (erro)
        err_fim();

// Para testes - mostra arquivos e classes
#if 0
    printf("Classes:");
    for (TClasse * obj = TClasse::RBfirst(); obj; obj=TClasse::RBnext(obj))
        printf(" classe %s", obj->Nome);
    putchar('\n');
    if (TArqMapa::Inicio)
    {
        printf("Arquivos:");
        for (TArqMapa * obj = TArqMapa::Inicio; obj; obj=obj->Proximo)
        {
            if (*obj->Arquivo==0)
                strcpy(arqext, "." INT_EXT);
            else
                mprintf(arqext, 60, "-%s." INT_EXT, obj->Arquivo);
            printf(" %s", arqinicio);
        }
        putchar('\n');
    }
#endif

// Obtém instruções das classes
// Acerta TClasse::Comandos
    TClasse * classeatual = 0;
    Instr::ChecaLinha checalinha;
    TAddBuffer classecom;
    char comando[2048];
    ini_arq = true;

    // Abre intmud.int
    strcpy(arqext, "." INT_EXT);
    if (!arq.Abrir(arqnome))
    {
        err_printf("Abrindo arquivo %s: %s\n", arqinicio, strerror(errno));
        err_fim();
    }
    mapa = TArqMapa::Inicio;
    while (true)
    {
    // Lê próxima linha
        int linhanum = arq.Linha(mens, sizeof(mens));
        if (linhanum<0) // Erro
        {
            err_printf("Lendo arquivo %s: %s\n", arqinicio, strerror(errno));
            err_fim();
        }
        // Verifica se é nome de classe
        char * pclasse = 0;
        if (linhanum>0)
            pclasse = TClasse::NomeDef(mens);
        if (linhanum==0 || pclasse) // Fim do arquivo ou próxima classe
        {
            ini_arq = false;
            if (classeatual) // Anota instruções da classe
            {
                classecom.Add("\x00\x00", 2); // Marca fim da lista de comandos
                classeatual->Comandos = new char[classecom.Total]; // Anota comandos
                classecom.Anotar(classeatual->Comandos, classecom.Total);
            }
            classecom.Limpar();
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
            mprintf(arqext, 60, "-%s." INT_EXT, mapa->Arquivo);
            if (!arq.Abrir(arqnome))
            {
                err_printf("Abrindo arquivo %s: %s\n",
                        arqinicio, strerror(errno));
                err_fim();
            }
            continue;
        }
    // Verifica linha MAPAGRANDE
        if (ini_arq)
            continue;
    // Verifica classe
        if (pclasse)
        {
            classeatual = TClasse::Procura(pclasse);
            if (classeatual==0)
            {
                err_printf("%s:%d: Classe não encontrada: %s\n",
                            arqinicio, linhanum, pclasse);
                erro=true;
                break;
            }
            continue;
        }
    // Instruções antes da definição da classe
        if (classeatual==0)
        {
            err_printf("%s:%d: Instruções não pertencem a nenhuma classe\n",
                        arqinicio, linhanum);
            erro=true;
            continue;
        }
    // Codifica instrução
        //err_printf("= %s\n", mens);
        if (!Instr::Codif(comando, mens, sizeof(comando)))
        {
            err_printf("%s:%d: %s\n", arqinicio, linhanum, comando);
            erro=true;
            continue;
        }
    // Informações sobre o que codificou
#if 0
        unsigned tam = Num16(comando);
        if (tam>sizeof(comando))
            tam=sizeof(comando);
        for (unsigned x=0; x<tam; x++)
            err_printf(" %d", (unsigned char)comando[x]);
        err_printf("\n");

        if (Instr::Mostra(mens, comando, sizeof(mens)))
            err_printf("+ %s\n", mens);
        else
            err_printf("- %s\n", mens);
        if (Instr::Decod(mens, comando, sizeof(mens)))
            err_printf("++ %s\n", mens);
        else
            err_printf("-- %s\n", mens);
#endif
    // Verifica instrução
        if (comando[2]==Instr::cHerda)
        {
            const char * p = comando+4;
            for (unsigned char x = comando[3]; x; x--)
            {
                TClasse * obj = TClasse::Procura(p);
                assert(obj!=0);
                while (*p++);
                if (obj!=classeatual)
                    continue;
                err_printf("%s:%d: Impossível herdar a própria classe\n",
                            arqinicio, linhanum);
                erro=true;
                continue;
            }
        }
        const char * p = checalinha.Instr(comando);
        if (p)
        {
            err_printf("%s:%d: %s\n", arqinicio, linhanum, p);
            erro=true;
            continue;
        }
        classecom.Add(comando, Num16(comando));
    }

// Verifica se ocorreu erro no mapa
    if (erro)
        err_fim();

// Verifica se todas as classes foram anotadas
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        if (cl->Comandos==0)
        {
            err_printf("Erro de fase; provavelmente algum arquivo mudou\n");
            err_fim();
        }

// Acerta lista de classes derivadas
    TClasse::AcertaDeriv();

// Para testes - mostra classes e instruções
#if 0
    printf("Classes:\n");
    for (TClasse * obj = TClasse::RBfirst(); obj; obj=TClasse::RBnext(obj))
    {
        printf("\nclasse %s\n", obj->Nome);
        if (obj->NumDeriv)
        {
            printf("*** Herdada em:");
            for (unsigned int x=0; x<obj->NumDeriv; x++)
                printf("%s%s", x==0 ? " " : ", ", obj->ListaDeriv[x]->Nome);
            putchar('\n');
        }
        for (const char * p = obj->Comandos; Num16(p); p+=Num16(p))
        {
            char mens[4096];
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

// Acerta variáveis das classes
    for (TClasse * cl = TClasse::RBfirst(); cl; cl = TClasse::RBnext(cl))
        cl->AcertaVar(false);

// Inicializa console
    if (telatxt && Console==0)
        Console = new TConsole;
    if (!TVarTelaTxt::Inic())
    {
        err_printf("Problema iniciando console\n");
        err_fim();
    }
#ifndef __WIN32__
    signal(SIGINT, TerminaSign);
    signal(SIGQUIT, TerminaSign);
#endif

#ifdef __WIN32__
// Inicializa WinSock
    WSADATA info;
    bool xinic=(WSAStartup(MAKELONG(1, 1), &info) != SOCKET_ERROR);
    if (!xinic)
    {
        err_printf("Problema inicializando WinSock\n");
        err_fim();
    }
#else
        // Ignora sinal PIPE
    signal(SIGPIPE, SIG_IGN);
#endif

// Executa funções iniclasse das classes
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
#ifndef __WIN32__
static int termsign = 0;
/// Processa sinal que encerra o programa
void TerminaSign(int sig)
{
// Início
    if (termsign)
        raise(sig);
    termsign = 1;
// Encerra
    TVarTelaTxt::Fim();
// Gera sinal padrão
    signal(sig, SIG_DFL);
    raise(sig);
}
#endif

//------------------------------------------------------------------------------
/// Encerra o programa
void Termina()
{
#ifdef __WIN32__
    WSACleanup();
#endif
    TVarTelaTxt::Fim();
    exit(EXIT_SUCCESS);
}
