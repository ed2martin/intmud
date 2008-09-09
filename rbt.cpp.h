// A documentação que encontrei sobre "Red Black Tree" foi:
//
// Introduction To Algorithms; Cormen, Thomas H. / Leiserson, Charles E . /
//  Rivest, Ronald L . The MIT Press; 07/1990;
//
//     A red-black tree is a binary search tree with one extra bit of storage
// per node: its color, which can be either RED or BLACK. By constraining the
// way nodes can be colored on any path from the root to a leaf, red-black
// trees ensure that no such path is more than twice as long as any other, so
// that the tree is approximately balanced. Each node of the tree now
// contains the fields color, key, left, right, and p. If a child or the
// parent of a node does not exist, the corresponding pointer field of the
// node contains the value NIL. We shall regard these NIL'S as being pointers
// to external nodes (leaves) of the binary search tree and the normal,
// key-bearing nodes as being internal nodes of the tree. A binary search
// tree is a red-black tree if it satisfies the following red-black
// properties:
//
//        1. Every node is either red or black.
//        2. Every leaf (NIL) is black. (dh: NIL leaf nodes are not shown on
//           the animation, assume every terminal node has two NIL leaves)
//        3. If a node is red, then both its children are black.
//        4. Every simple path from a node to a descendant leaf contains the
//           same number of black nodes.
//
// Based on the description of Red-Black Tree algorithms found in
//      Berman and Paul.
//      Sequential and Parallel Algorithms.
//      Brooks/Cole PWS Publishing Co,
//      1997 (ISBN:0-534-94674-7).
//
// Red Black Tree's author: probably "Jason"
//
// Adaptations in the Red Black Tree (in this file) made by Edward Martin

//------------------------------------------------------------------------------
// Exemplo de programa usando a RBT
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

class TObjeto
{
public:
    TObjeto(const char * str);
    ~TObjeto();
    TObjeto * procura(const char * str);
    char nome[20];

// Para árvore
    TObjeto *RBleft, *RBright, *RBparent;
    char RBcolour;
    static TObjeto * RBroot;
    void RBleft_rotate(void);
    void RBright_rotate(void);
    void RBinsert(void);
    void RBremove(void);
    static TObjeto * RBfirst(void);
    static TObjeto * RBlast(void);
    static TObjeto * RBnext(TObjeto *);
    static TObjeto * RBprevious(TObjeto *);
    static int RBcomp(TObjeto * x, TObjeto * y);

// Para lista encadeada
    static TObjeto *inicio;
    TObjeto * proximo;
};

TObjeto * TObjeto::RBroot=0;
TObjeto * TObjeto::inicio=0;

// Construtor - copia string para o objeto e coloca objeto na árvore
TObjeto::TObjeto(const char * str)
{
    for (int a=0; a<sizeof(nome); a++)
        if ((nome[a]=str[a]) ==0)
            break;
    nome[19]=0;
    RBinsert();
    proximo=inicio;
    inicio=this;
}

// Destrutor - retira objeto da árvore
TObjeto::~TObjeto()
{
    RBremove();
}

// Compara objetos, retorna: 0 se iguais, <0 se x<y, >0 se x>y
int TObjeto::RBcomp(TObjeto * x, TObjeto * y)
{
    int a=strcmp(x->nome, y->nome);
    //printf("[CMP] [%s] %c [%s]\n", x->nome,
    //            a==0 ? '=' : a<0 ? '<' : '>', y->nome);
    return a;
}

// Mostra cada objeto da árvore, incluindo objetos filhos
void Mostra(TObjeto * obj, int nivel)
{
    if (obj->RBleft)
        Mostra(obj->RBleft, nivel+2);
    for (int a=0; a<nivel; a++)
        putchar(' ');
    putchar(obj->RBcolour&1 ? 'p' : 'v');
    for (int a=nivel; a<40; a++)
        putchar('-');
    printf("[%s]\n", obj->nome);
    if (obj->RBright)
        Mostra(obj->RBright, nivel+2);
}

// Mostra como está a árvore
void MostraArvore(void)
{
    if (TObjeto::RBroot)
        Mostra(TObjeto::RBroot,0);
    else
        puts("Árvore vazia");
    putchar('\n');
}

// Checa cada objeto da árvore
// Chamar Checa sem parâmetros para checar a árvore inteira
void Checa(TObjeto * obj=0)
{
    if (obj==0)
    {
        if (TObjeto::RBroot==0)
            return;
        if (TObjeto::RBroot->RBparent!=0)
        {
            puts("Árvore incorreta");
            exit(0);
        }
        if ((TObjeto::RBroot->RBcolour&1)==0)
        {
            puts("Não é rbt");
            exit(0);
        }
        Checa(TObjeto::RBroot);
        return;
    }
    if (obj->RBleft)
    {
        if (obj->RBleft->RBparent!=obj)
        {
            puts("Árvore incorreta");
            exit(0);
        }
        if ((obj->RBleft->RBcolour&1)==0 && (obj->RBcolour&1)==0)
        {
            puts("Não é rbt");
            exit(0);
        }
        Checa(obj->RBleft);
    }
    if (obj->RBright)
    {
        if (obj->RBright->RBparent!=obj)
        {
            puts("Árvore incorreta");
            exit(0);
        }
        if ((obj->RBright->RBcolour&1)==0 && (obj->RBcolour&1)==0)
        {
            puts("Não é rbt");
            exit(0);
        }
        Checa(obj->RBright);
    }
}


//puts("-------Inicio-------"); MostraArvore(); puts("--------Fim---------");

int main(int argc, char *argv[])
{
    FILE * arq;
    struct timeval tinicio,tfim;
    char string[20],indice;
    int totalregistros=0;
    int totalapagado=0;
    int ch;

    if (argc!=2)
    {
        printf("Teste de árvore:\n%s  <arquivo_de_texto>\n",argv[0]);
        return 0;
    }

// Mostrar tempo inicial
    if (gettimeofday(&tinicio,0))
    {
        perror("Não consegui ler hora");
        return 1;
    }
    printf("Tempo = %d:%d:%d s %d us\n", tinicio.tv_sec/3600, tinicio.tv_sec/60%60,
        tinicio.tv_sec%60, tinicio.tv_usec);

// Ordenar strings e colocar na árvore
    arq=fopen(argv[1],"r");
    if (arq==0)
    {
        perror("Abrindo arquivo");
        return 1;
    }
    indice=0;
    while (true)
    {
        ch=fgetc(arq);
        if (ch<0)
            break;
        if ((unsigned char)ch<=' ' && indice==0)        // Ignora espaços iniciais
            continue;
        if ((unsigned char)ch>' ' && indice<19)       // Anota na string
        {
            string[indice++]=ch;
            continue;
        }
        string[indice]=0;
        indice=0;
        if ((new TObjeto(string))==0)
            continue;
        totalregistros++;
        //Checa();
        //MostraArvore(); fflush(stdout);
    }
    fclose(arq);

    MostraArvore();
    printf("Checando árvore "); fflush(stdout);
    Checa();
    puts("OK"); fflush(stdout);

// Apagar as strings da árvore
 //   while (TObjeto::proot->pright)
 //   {
 //       delete TObjeto::proot->pright;
 //       Checa();
 //       puts("aaaa"); MostraArvore();
 //       totalapagado++;
 //   }
 //   while (TObjeto::proot->pleft)
 //   {
 //       delete TObjeto::proot->pleft;
 //       Checa();
 //       puts("aaaa"); MostraArvore(); fflush(stdout);
 //       totalapagado++;
 //   }
 //   while (TObjeto::RBroot)
 //   {
 // //      printf("Apagando [%s]\n",TObjeto::proot->nome); fflush(stdout);
 //       delete TObjeto::RBroot;
 //       //puts("bbbb"); MostraArvore(); fflush(stdout);
 //       totalapagado++;
 //Checa();
 //   }

    TObjeto * x;
    while (TObjeto::inicio)
    {
        x=TObjeto::inicio->proximo;
        delete TObjeto::inicio;
        TObjeto::inicio=x;
        totalapagado++;
    }

// Mostrar tempo final
    if (gettimeofday(&tfim,0))
    {
        perror("Não consegui ler hora");
        return 1;
    }
    printf("Tempo = %d:%d:%d s %d us\n", tfim.tv_sec/3600, tfim.tv_sec/60%60,
        tfim.tv_sec%60, tfim.tv_usec);
    printf("Total = %d s %d us\n",
        tfim.tv_sec-tinicio.tv_sec-(tfim.tv_usec<tinicio.tv_usec),
        tfim.tv_usec-tinicio.tv_usec+(tfim.tv_usec<tinicio.tv_usec)*1000000);
    printf("\nRegistros criados: %d\n",totalregistros);
    printf("Registros apagados: %d\n\n",totalapagado);
}


// Deve ser definido
#define CLASS           TObjeto           // Nome da classe

// Devem ser definidos se a classe tiver mais de uma árvore
        // Variáveis
//#define RBroot          proot             // (static) Aponta para primeiro
                                          // objeto da árvore
//#define RBleft          pleft             // Objeto filho à esquerda
//#define RBright         pright            // Objeto filho à direita
//#define RBparent        pparent           // Objeto pai
//#define RBcolour        pcolour           // Cor do objeto
//#define RBmask          1                 // Bit de RBcolour que contém a cor
                                          // Os demais bits não são alterados pela RBT
        // Funções não estáticas
//#define RBleft_rotate   pleft_rotate      // Função - usada internamente
//#define RBright_rotate  pright_rotate     // Função - usada internamente
//#define RBinsert        pinsert           // Função - insere objeto na árvore
//#define RBremove        pdelete           // Função - retira objeto da árvore
        // Funções estáticas
//#define RBfirst         pprimeiro         // Função - retorna o primeiro objeto
//#define RBlast          pultimo           // Função - retorna o último objeto
//#define RBnext          pproximo          // Função - retorna o próximo objeto
//#define RBprevious      panterior         // Função - retorna o objeto anterior
//#define RBcomp          pcomp             // Função - compara objetos
*/

//------------------------------------------------------------------------------
/*  Exemplo de como definir uma classe com uma RBT
class TObjeto
{
public:
    TObjeto();  // Construtor e destrutor - definidos pelo usuário
    ~TObjeto();

// Para árvore
    TObjeto *RBleft, *RBright, *RBparent;       // Objetos filhos e pai
    char RBcolour;                              // Cor do objeto
    static TObjeto * RBroot;                    // Primeiro objeto da árvore
    void RBleft_rotate(void);                   // Usado internamente
    void RBright_rotate(void);                  // Usado internamente
    void RBinsert(void);                        // Insere objeto na árvore
    void RBremove(void);                        // Retira objeto da árvore
    static TObjeto * RBfirst(void);             // Retorna o primeiro objeto
    static TObjeto * RBlast(void);              // Retorna o último objeto
    static TObjeto * RBnext(TObjeto *);         // Retorna o próximo objeto
    static TObjeto * RBprevious(TObjeto *);     // Retorna o objeto anterior
    static int RBcomp(TObjeto * x, TObjeto * y);// Função definida pelo usuário
                                                // compara objetos
};
*/
//------------------------------------------------------------------------------
/*
Função:   int RBcomp(CLASS * x, CLASS * y)
Retorna:  0  se x==y
          >0 se x>y
          <0 se x<y
Assim a árvore estará organizada em ordem crescente
*/

//------------------------------------------------------------------------------
#ifndef RBmask
#define RBmask 1
#endif

// Bit de cor:     0=vermelho     1=preto
// Tornar preto                cor |= RBmask; // black
// Tornar vermelho             cor &= ~RBmask; // red
// if (cor==preto)             if (cor&RBmask)
// if (cor==vermelho)          if (~cor&RBmask)

//------------------------------------------------------------------------------
// Rotações
// Condição: x e y não podem ser folhas (devem ser objetos existentes)
//
//                      y                                x          //
//                     / \     <- RBleft_rotate(x)      / \         //
//  A x B y C         x   C                            A   y        //
//                   / \       RBright_rotate(y) ->       / \       //
//                  A   B                                B   C      //

//------------------------------------------------------------------------------
// Rotação para esquerda
void CLASS::RBleft_rotate(void)
{
    CLASS * x = this;
    CLASS * y;
    y = x->RBright;
    // Turn y's left sub-tree into x's right sub-tree
    x->RBright = y->RBleft;
    if ( y->RBleft != 0 )
        y->RBleft->RBparent = x;
    // y's new parent was x's parent
    y->RBparent = x->RBparent;
    // Set the parent to point to y instead of x
    // First see whether we're at the root
    if ( x->RBparent == 0 )
        RBroot = y;
    else
        if ( x == x->RBparent->RBleft )
            // x was on the left of its parent
            x->RBparent->RBleft = y;
        else
            // x must have been on the right
            x->RBparent->RBright = y;
    // Finally, put x on y's left
    y->RBleft = x;
    x->RBparent = y;
}

//------------------------------------------------------------------------------
// Rotação para direita
void CLASS::RBright_rotate(void)
{
    CLASS * x = this;
    CLASS * y;  
    y = x->RBleft;
    x->RBleft = y->RBright;
    if ( y->RBright != 0 )
        y->RBright->RBparent = x;
    y->RBparent = x->RBparent;
    if ( x->RBparent == 0 )
        RBroot = y;
    else
        if ( x == x->RBparent->RBright )
            x->RBparent->RBright = y;
        else
            x->RBparent->RBleft = y;
    y->RBright = x;
    x->RBparent = y;
}

//------------------------------------------------------------------------------
// Retorna o primeiro objeto da árvore
CLASS * CLASS::RBfirst(void)
{
    CLASS * x=RBroot;
    if (x==0)
        return 0;
    while (x->RBleft)
        x=x->RBleft;
    return x;
}

//------------------------------------------------------------------------------
// Retorna o último objeto da árvore
CLASS * CLASS::RBlast(void)
{
    CLASS * x=RBroot;
    if (x==0)
        return 0;
    while (x->RBright)
        x=x->RBright;
    return x;
}

//------------------------------------------------------------------------------
// Retorna o próximo objeto
CLASS * CLASS::RBnext(CLASS * x)
{
    if (x==0)
        return 0;
    if (x->RBright)
    {
        for (x=x->RBright; x->RBleft; x=x->RBleft);
        return x;
    }
    while (x->RBparent)
    {
        if (x->RBparent->RBleft==x)
            return x->RBparent;
        x=x->RBparent;
    }
    return 0;
}

//------------------------------------------------------------------------------
// Retorna o objeto anterior
CLASS * CLASS::RBprevious(CLASS * x)
{
    if (x==0)
        return 0;
    if (x->RBleft)
    {
        for (x=x->RBleft; x->RBright; x=x->RBright);
        return x;
    }
    while (x->RBparent)
    {
        if (x->RBparent->RBright==x)
            return x->RBparent;
        x=x->RBparent;
    }
    return 0;
}

//------------------------------------------------------------------------------
// Inserir na árvore objeto já existente
void CLASS::RBinsert(void)
{
    CLASS * x = this;
    // Insert in the tree in the usual way
    CLASS * y = RBroot;
    x->RBleft = 0;
    x->RBright = 0;
    if (y == 0)
    {
        RBroot = x;
        x->RBcolour |= RBmask; // black
        x->RBparent = 0;
        return;
    }
    while (true)
        if ( RBcomp(x,y) < 0 )
        {
            if (y->RBleft == 0)
            {
                y->RBleft = x;
                break;
            }
            y = y->RBleft;
        }
        else
        {
            if (y->RBright == 0)
            {
                y->RBright = x;
                break;
            }
            y = y->RBright;
        }
    x->RBparent = y;
    // Now restore the red-black property
    x->RBcolour &= ~RBmask; // red
    while ( x != RBroot && (~x->RBparent->RBcolour & RBmask)) // if _red_
    {
        if ( x->RBparent == x->RBparent->RBparent->RBleft )
        {
            // If x's parent is a left, y is x's right 'uncle'
            y = x->RBparent->RBparent->RBright;
            if ( y != 0 && (~y->RBcolour & RBmask )) // if _red_
            {
                // case 1 - change the colours
                x->RBparent->RBcolour |= RBmask; // black
                y->RBcolour |= RBmask; // black
                x->RBparent->RBparent->RBcolour &= ~RBmask; // red
                // Move x up the tree
                x = x->RBparent->RBparent;
            }
            else
            {
                // y is a black node
                if ( x == x->RBparent->RBright )
                {
                    // and x is to the right
                    // case 2 - move x up and rotate
                    x = x->RBparent;
                    x->RBleft_rotate();
                }
                // case 3
                x->RBparent->RBcolour |= RBmask; // black
                x->RBparent->RBparent->RBcolour &= ~RBmask; // red
                x->RBparent->RBparent->RBright_rotate();
            }
        }
        else
        {
            // repeat the "if" part with right and lef exchanged

            // If x's parent is a right, y is x's left 'uncle'
            y = x->RBparent->RBparent->RBleft;
            if ( y != 0 && (~y->RBcolour & RBmask )) // if _red_
            {
                // case 1 - change the colours
                x->RBparent->RBcolour |= RBmask; // black
                y->RBcolour |= RBmask; // black
                x->RBparent->RBparent->RBcolour &= ~RBmask; // red
                // Move x up the tree
                x = x->RBparent->RBparent;
            }
            else
            {
                // y is a black node
                if ( x == x->RBparent->RBleft )
                {
                    // and x is to the right
                    // case 2 - move x up and rotate
                    x = x->RBparent;
                    x->RBright_rotate();
                }
                // case 3
                x->RBparent->RBcolour |= RBmask; // black
                x->RBparent->RBparent->RBcolour &= ~RBmask; // red
                x->RBparent->RBparent->RBleft_rotate();
            }
        }
    }
    // Colour the root black
    RBroot->RBcolour |= RBmask; // black
}

//------------------------------------------------------------------------------
// Retirar objeto já existente da árvore
void CLASS::RBremove(void)
{
    CLASS * x = this;
    CLASS * i;
// Se x tem dois filhos: Procura o objeto sucessor
    CLASS * y = x;
    if (x->RBleft && x->RBright)
    {
        y = y->RBright;
        while (y->RBleft)
            y = y->RBleft;
        // Troca com objeto que será excluído
        CLASS * pont;
                // Troca pais de x e y
        if (x->RBparent)
            if (x->RBparent->RBleft == x)
                x->RBparent->RBleft = y;
            else
                x->RBparent->RBright = y;
        else
            RBroot = y;
        if (y->RBparent)
            if (y->RBparent->RBleft == y)
                y->RBparent->RBleft = x;
            else
                y->RBparent->RBright = x;
        else
            RBroot = x;
        pont = x->RBparent;
        x->RBparent = y->RBparent;
        y->RBparent = pont;
                // Troca filhos de x e y
        pont = x->RBleft;
        x->RBleft = y->RBleft;
        y->RBleft = pont;
        pont = x->RBright;
        x->RBright = y->RBright;
        y->RBright = pont;
        if (x->RBleft)  x->RBleft->RBparent  = x;
        if (x->RBright) x->RBright->RBparent = x;
        if (y->RBleft)  y->RBleft->RBparent  = y;
        if (y->RBright) y->RBright->RBparent = y;
                // Troca cores
        x->RBcolour ^= (y->RBcolour & RBmask);  // Troca a cor de x
        y->RBcolour ^= (x->RBcolour & RBmask);  // com a cor de y
        x->RBcolour ^= (y->RBcolour & RBmask);
    }

// Retira objeto (x) da árvore, faz y=objeto filho
    i=y=0;                      // Obtém endereço do filho (0=não tem)
    if (x->RBleft)  y = x->RBleft;
    if (x->RBright) y = x->RBright;
    if (x->RBparent==0)           // Aponta pai para o filho
        RBroot = y;               // Obtém irmão do objeto que está sendo retirado
    else
        if (x->RBparent->RBleft == x)
        {
            x->RBparent->RBleft = y;
            i = x->RBparent->RBright;
        }
        else
        {
            x->RBparent->RBright = y;
            i = x->RBparent->RBleft;
        }
    if (y)                      // Aponta filho para o pai
        y->RBparent = x->RBparent;

// Se x é vermelho ou tem filho, sem problema
    if (~x->RBcolour & RBmask)       // x é vermelho: a árvore está certa
        return;
    if (y)                      // x tem 1 filho (vermelho): o filho vira preto
    {
        y->RBcolour |= RBmask; // black
        return;
    }

// Acerta a árvore
    while (x->RBparent && (x->RBcolour&RBmask)) // if _black_
    {
        if (i==0)                       // Obtém irmão de x
        {
            if (x->RBparent->RBleft == x)
                i = x->RBparent->RBright;
            else
                i = x->RBparent->RBleft;
        }
        if (x->RBparent->RBright == i)      // Apagou à esquerda
        {
            // Caso 1: Irmão é vermelho
            if (~i->RBcolour & RBmask) // if _red_
            {
                i->RBcolour |= RBmask;      // Irmão de x é preto
                i->RBparent->RBcolour &= ~RBmask;// Pai de x é vermelho
                i->RBparent->RBleft_rotate();// Rotaciona em direçào a x
                i = x->RBparent->RBright;   // Obtém novo irmão de x
            }

            // Agora irmão é preto

            // Caso 2: Pelo menos um dos filhos do irmão é vermelho
            if ( (i->RBleft  && (~i->RBleft->RBcolour&RBmask)) || // if _red_
                 (i->RBright && (~i->RBright->RBcolour&RBmask)) ) // if _red_
            {

                // Caso 2a: O filho do irmão mais distante de x é preto ou não existe
                if (i->RBright==0 || (i->RBright->RBcolour&RBmask)) // if _black_
                {
                    i->RBcolour &= ~RBmask;   // Irmão de x é vermelho
                    i->RBleft->RBcolour |= RBmask;// Filho do irmão mais perto de x é preto
                    i->RBright_rotate();   // Rotaciona na direção contrária de x
                    i = x->RBparent->RBright;// Obtém novo irmão de x
                }

                // Caso 2b:
                // Agora filho do irmão mais distante de x é vermelho
                i->RBright->RBcolour |= RBmask; // Filho do irmão mais distante de x é preto
                i->RBcolour ^= (i->RBcolour ^ i->RBparent->RBcolour) & RBmask;
                                        // Irmão tem a mesma cor do pai:
                                        // Cor em i->RBcolour = Cor em i->RBparent->RBcolour
                i->RBparent->RBcolour |= RBmask;  // Pai é preto
                i->RBparent->RBleft_rotate(); // Rotaciona em direção a x
                return;
            }
        }
        else   // Repetir o if acima, trocando left com right
        {
            if (~i->RBcolour&RBmask) // if _red_
            {
                i->RBcolour |= RBmask; // black
                i->RBparent->RBcolour &= ~RBmask;// Pai de x é vermelho
                i->RBparent->RBright_rotate();
                i = x->RBparent->RBleft;
            }
            if ( (i->RBleft  && (~i->RBleft->RBcolour&RBmask)) || // if _red_
                 (i->RBright && (~i->RBright->RBcolour&RBmask)) ) // if _red_
            {
                if (i->RBleft==0 || (i->RBleft->RBcolour&RBmask)) // if _black_
                {
                    i->RBcolour &= ~RBmask; // red
                    i->RBright->RBcolour |= RBmask; // black
                    i->RBleft_rotate();
                    i = x->RBparent->RBleft;
                }
                i->RBleft->RBcolour |= RBmask; // black
                i->RBcolour ^= (i->RBcolour ^ i->RBparent->RBcolour) & RBmask;
                                        // i->RBcolour=i->RBparent->RBcolour
                i->RBparent->RBcolour |= RBmask; // black
                i->RBparent->RBright_rotate();
                return;
            }
        }
        // Caso 3: o irmão preto de X não tem filho vermelho
        i->RBcolour &= ~RBmask; // O irmão de x é vermelho
        x=x->RBparent;    // Move para cima na árvore
        i=0;            // i=0 para calcular o irmão novamente
    }
    x->RBcolour |= RBmask;  // x é preto
}

//------------------------------------------------------------------------------
/*
RBRemove(value)
{
	// Pretend BST returns the place in the tree where a CLASS was actually
	// deleted.
    // x may be NULL (black), or it may be the IOS of the deleted value
	// This is an important detail glossed over by Jason!
	x = BST_remove(value);

	if(deleted CLASS was red)
		return; // STOP
	else
	{
		// go from here up the tree until we see a red CLASS
		while(x != root and x is black)
		{
			// Since the black height of this side has decreased, we must
			// increase the black height of the current side.

			// case 1 is that X's sibling is red.
			if(x's sibling is red)
			{
				color x's sibling black
				color x's parent red
				rotate toward x

				// This operation has NOT changed the black heights at all.
				// It simply makes the next operation (which does) possible.
				// x remains the same
			}

			// now the sibling is black. From here, we can actually reduce the
			// black height on the opposite side by a recoloring

			if(one of x's sibling's children is red) // case 2
			{
				if(x's sibling's child furthest from x is black) // case 2a
				{
					color x's sibling red
					color child of sibling closest to x black
					single rotation on x's sibling away from x.
					// case 2a is always followed by 2b, because
					// 2a only gets us into the position where 2b can do its
					// work (See pg 36 of jason's notes).
					// If it helps, you can consider 2a + 2b a "double
					// rotation".
				}
				// case 2b
				// now x's sibling's child furthest from x is red.
				// This is the only operation that actually fixes
				// the black height
				color child of x's sibling furthest from x black
				color x's sibling to whatever color x's parent is.
				color x's parent black
				single rotation on x's parent towards x

				// the black height is now repaired. BREAK from the loop
			}
			else // case 3: x's black sibling has no red children
			{
				// we must move up the tree, as the balancing problem
				// is up there
				color x's sibling red
				x = x->parent
			}

		}
		color x black
	}
} */
