#ifndef ALOCAMEM_H
#define ALOCAMEM_H

#include <stdio.h>
#include <assert.h>

// Mecanismos de alocação de memória de objetos de tamanho fixo

//#define DEBUG_MEM // Verifica se está alocando corretamente

//---------------------------------------------------------------------------
/// Aloca memória exclusivamente pelo sistema
template<class T>
class TAlocaMemSistema
{
public:
    T * malloc() { return new T; }
    T * free(T * obj) { delete obj; return nullptr; }
    void LiberaBloco() {}
};

//---------------------------------------------------------------------------
/// Aloca memória em blocos
/** N = número de objetos por bloco; são mantidos até 2 blocos livres
 *  Quando for necessário mover um objeto para outro lugar, é chamada
 *  a função Mover() do objeto, passando o novo endereço
 *  A função LiberaBloco() apaga um bloco livre, se houver */
template<class T, unsigned short N>
class TAlocaMemSimples
{
private:
    class TBloco
    {
    public:
#ifdef DEBUG_MEM
        TBloco() { printf("Criar(%p)\n", this); fflush(stdout); }
        ~TBloco() { printf("Apagar(%p)\n", this); fflush(stdout); }
#endif

        T Lista[N];       ///< Objetos
        unsigned int Total; ///< Número de objetos usados
        TBloco * Depois;    ///< Próxima variável TBloco
    };

    TBloco * Disp;      ///< Lista de TBloco disponívels (com Total=0)
    TBloco * Usado;     ///< Lista de TBloco usados (com Total!=0)
    int TotalDisp;      ///< Blocos disponíveis em Disp

public:
    /// Construtor
    TAlocaMemSimples()
    {
        Disp = Usado = nullptr;
        TotalDisp = 0;
    }

    /// Destrutor
    ~TAlocaMemSimples()
    {
#ifdef DEBUG_MEM
        assert(Usado == nullptr);
#endif
        while (Disp)
        {
            TBloco * bl = Disp->Depois;
            delete Disp;
            Disp = bl;
        }
    }

    /// Aloca um objeto na memória
    T * malloc()
    {
    // Se tem objeto disponível...
        if (Usado && Usado->Total < N)
            return Usado->Lista + (Usado->Total++);
    // Se não tem objeto TListaX disponível...
        TBloco * obj;
        if (Disp == nullptr)    // Não tem objeto TGrupoX disponível
            obj = new TBloco;
        else            // Tem objeto TGrupoX disponível
        {
            obj = Disp, Disp = Disp->Depois; // Retira da lista Disp
            TotalDisp--;
        }
        obj->Total = 1;
        obj->Depois = Usado; // Coloca na lista Usado
        Usado = obj;
        return obj->Lista;
    }

    /// Libera um objeto alocado
    /** @return Objeto que ocupou o lugar do objeto apagado ou nullptr se nenhum */
    T * free(T * lista)
    {
        T * lfim = Usado->Lista + Usado->Total - 1;
        if (lista != lfim)
            lfim->Mover(lista);
        Usado->Total--;
        if (Usado->Total == 0)
        {
            TBloco * obj = Usado;
            Usado = Usado->Depois; // Retira da lista Usado
            if (TotalDisp < 2)
            {
                obj->Depois = Disp;    // Coloca na lista Disp
                Disp = obj;
                TotalDisp++;
            }
            else
                delete obj;
        }
        return (lista != lfim ? lfim : nullptr);
    }

    void LiberaBloco()
    {
        if (Disp)
        {
            TBloco * obj = Disp;
            Disp = Disp->Depois; // Retira da lista Disp
            delete obj;
            TotalDisp--;
        }
    }
};

//---------------------------------------------------------------------------
/// Aloca memória em blocos
/** É mais lento que TAlocaMemSimples porém chama Mover() menos vezes
 *  N = número de objetos por bloco; são mantidos até 2 blocos livres
 *  TIndice = unsigned char para N < 0xFF
 *  TIndice = unsigned short para N < 0xFFFF
 *  TIndice = unsigned int para outros valores
 *  Quando for necessário mover um objeto para outro lugar, é chamada
 *  a função Mover() do objeto, passando o novo endereço */
template<class T, unsigned short N, class TIndice>
class TAlocaMem2
{
private:
    static const TIndice TDisponivel = (TIndice)-1;

    /// Objeto com informações sobre alocação de memória
    class TObjetoRef
    {
    public:
        T Objeto;
        TIndice Indice; // Número desse objeto
        TIndice Antes; // Número do objeto anterior ou TDisponivel
        TIndice Depois; // Número do próximo objeto ou TDisponivel
    };

    /// Bloco de objetos alocados
    class TBloco
    {
    public:
        /// Lista de objetos
        TObjetoRef Lista[N];

        //// Número de objetos alocados em Lista
        TIndice NumAlocado;

        /// Número de objetos usados em Lista
        TIndice NumTotal;

        /// Número do primeiro objeto alocado ou TDisponivel
        /** Lista duplamente ligada com TObjetoRef::Antes e TObjetoRef::Depois */
        TIndice ObjAlocado;

        /// Número do próximo objeto disponível ou TDisponivel
        /** Lista ligada com TObjetoRef::Depois */
        TIndice ObjDisponivel;

        /// Bloco anterior na lista ligada, usado em TAlocaMemoria
        TBloco * Antes;

        /// Próximo bloco na lista ligada, usado em TAlocaMemoria
        TBloco * Depois;

        /// Se está inserindo objetos nesse bloco, usado em TAlocaMemoria
        //bool Inserindo;

        /// Construtor: Cria bloco com um objeto alocado
        TBloco()
        {
            NumTotal = NumAlocado = 1;
            ObjAlocado = 0;
            ObjDisponivel = TDisponivel;
            Lista[0].Antes = TDisponivel;
            Lista[0].Depois = TDisponivel;
            Lista[0].Indice = 0;
        }

        /// Aloca objeto, sendo que Total < N
        T * malloc()
        {
            if (ObjDisponivel != TDisponivel)
            {
                NumAlocado++;
                TIndice indice = ObjDisponivel;
                ObjDisponivel = Lista[indice].Depois;
                Lista[indice].Antes = TDisponivel;
                Lista[indice].Depois = ObjAlocado;
                if (ObjAlocado != TDisponivel)
                    Lista[ObjAlocado].Antes = indice;
                ObjAlocado = indice;
                return &Lista[indice].Objeto;
            }
            if (NumTotal < N)
            {
                NumAlocado++;
                TIndice indice = NumTotal++;
                Lista[indice].Indice = indice;
                Lista[indice].Antes = TDisponivel;
                Lista[indice].Depois = ObjAlocado;
                if (ObjAlocado != TDisponivel)
                    Lista[ObjAlocado].Antes = indice;
                ObjAlocado = indice;
                return &Lista[indice].Objeto;
            }
            return nullptr;
        }

        /// Libera o objeto alocado mais recente ou retorna nullptr se não houver
        T * free()
        {
            if (ObjAlocado == TDisponivel)
                return nullptr;
            NumAlocado--;
            TIndice indice = ObjAlocado;
            ObjAlocado = Lista[indice].Depois;
            if (ObjAlocado != TDisponivel)
                Lista[ObjAlocado].Antes = TDisponivel;
            Lista[indice].Depois = ObjDisponivel;
            ObjDisponivel = indice;
            return &Lista[indice].Objeto;
        }

        /// Libera objeto alocado
        void free(TObjetoRef * obj)
        {
            NumAlocado--;
#ifdef DEBUG_MEM
            assert(obj->Indice < N);
            assert(obj == &Lista[obj->Indice]);
#endif
            if (obj->Antes != TDisponivel)
                Lista[obj->Antes].Depois = obj->Depois;
            else
                ObjAlocado = obj->Depois;
            if (obj->Depois != TDisponivel)
                Lista[obj->Depois].Antes = obj->Antes;
            obj->Depois = ObjDisponivel;
            ObjDisponivel = obj->Indice;
        }

        /// Verifica se o objeto TBloco está correto
        void Debug()
        {
#ifdef DEBUG_MEM
            for (TIndice cont = 0; cont < NumTotal; cont++)
            {
                assert(Lista[cont].Indice == cont);
                assert(Lista[cont].Antes < NumTotal ||
                       Lista[cont].Antes == TDisponivel);
                assert(Lista[cont].Depois < NumTotal ||
                       Lista[cont].Depois == TDisponivel);
            }
            TIndice antes = TDisponivel;
            TIndice cont1 = 0;
            for (TIndice agora = ObjAlocado; agora != TDisponivel; )
            {
                assert(Lista[agora].Antes == antes);
                TIndice depois = Lista[agora].Depois;
                if (depois != TDisponivel)
                    assert(Lista[depois].Antes == agora);
                antes = agora, agora = depois, cont1++;
            }
            assert(cont1 == NumAlocado);
            TIndice cont2 = 0;
            for (TIndice agora = ObjDisponivel; agora != TDisponivel; )
                agora = Lista[agora].Depois, cont2++;
            //printf("(%d %d %d)", cont1, NumTotal, NumAlocado); fflush(stdout);
            assert(cont2 == NumTotal - NumAlocado);
#endif
        }
    };

    /// Primeiro bloco
    TBloco * Inicio;

    /// Último bloco
    TBloco * Fim;

    /// Número de objetos não usados nos blocos da lista ligada
    unsigned int TotalLivre;

    /// Número de blocos existentes
    unsigned int TotalBlocos;

    /// Retira bloco da lista
    inline void BlRetira(TBloco * bl)
    {
        (bl->Antes ? bl->Antes->Depois : Inicio) = bl->Depois;
        (bl->Depois ? bl->Depois->Antes : Fim) = bl->Antes;
    }

    /// Coloca bloco no início da lista
    inline void BlColocaInicio(TBloco * bl)
    {
        bl->Antes = nullptr;
        bl->Depois = Inicio;
        (Inicio ? Inicio->Antes : Fim) = bl;
        Inicio = bl;
    }

    /// Coloca bloco no fim da lista
    inline void BlColocaFim(TBloco * bl)
    {
        bl->Depois = nullptr;
        bl->Antes = Fim;
        (Fim ? Fim->Depois : Inicio) = bl;
        Fim = bl;
    }

    /// Verifica se está tudo certo
    void Debug()
    {
#ifdef DEBUG_MEM
        unsigned int total = 0;
        assert(Inicio == nullptr || Inicio->Antes == nullptr);
        assert(Fim == nullptr || Fim->Depois == nullptr);
        for (TBloco * obj = Inicio; obj; obj = obj->Depois)
        {
            total += N - obj->NumAlocado;
            assert((obj->Antes ? obj->Antes->Depois : Inicio) == obj);
            assert((obj->Depois ? obj->Depois->Antes : Fim) == obj);
        }
        assert(total == TotalLivre);
#endif
    }

public:
    /// Construtor
    TAlocaMem2()
    {
        TotalLivre = TotalBlocos = 0;
        Inicio = Fim = nullptr;
    }

    /// Destrutor
    ~TAlocaMem2()
    {
        while (Inicio)
        {
            TBloco * depois = Inicio->Depois;
#ifdef DEBUG_MEM
            assert(Inicio->NumAlocado == 0);
            delete Inicio;
#else
            if (Inicio->NumAlocado == 0)
                delete Inicio;
#endif
            Inicio = depois;
            TotalBlocos--;
        }
#ifdef DEBUG_MEM
        assert(TotalBlocos == 0);
#endif
    }

    /// Aloca um objeto na memória
    T * malloc()
    {
        //putchar(' ');
        if (Inicio != nullptr)
        {
            //putchar('A'); fflush(stdout);
            TotalLivre--;
            //BlAcertaInicio();
            T * obj = Inicio->malloc();
            Inicio->Debug();
            if (Inicio->NumAlocado == N)
                BlRetira(Inicio);
            Debug();
            //putchar('B'); fflush(stdout);
            return obj;
        }
        //putchar('C'); fflush(stdout);
        TotalLivre = N - 1;
        TotalBlocos++;
        TBloco * bl = new TBloco();
        bl->Debug();
        bl->Antes = bl->Depois = nullptr;
        //bl->Inserindo = true;
        Inicio = Fim = bl;
        Debug();
        //putchar('D'); fflush(stdout);
        return &bl->Lista[0].Objeto;
    }

    /// Libera um objeto alocado
    /** @return Objeto que ocupou o lugar do objeto apagado ou nullptr se nenhum */
    T * free(T * objeto)
    {
        TObjetoRef * ref = reinterpret_cast<TObjetoRef*>(objeto);
        TBloco * bl = reinterpret_cast<TBloco*>(ref - ref->Indice);
        // Verifica se não precisa apagar bloco
        if (TotalLivre <= N * 2)
        {
            bl->free(ref);
            bl->Debug();
            TotalLivre++;
            if (bl->NumAlocado == N - 1)
                BlColocaInicio(bl);
            else if (bl->NumAlocado < Fim->NumAlocado)
            {
                BlRetira(bl);
                BlColocaFim(bl);
            }
            else if (bl->Depois && bl->NumAlocado < bl->Depois->NumAlocado)
            {
                TBloco * b2 = bl->Depois;
                BlRetira(bl);
                bl->Antes = b2;
                bl->Depois = b2->Depois;
                b2->Depois = bl;
                (bl->Depois ? bl->Depois->Antes : Fim) = bl;
            }
        }
        else if (Fim->NumAlocado == 0) // Apaga bloco
        {
        // Libera objeto
            bl->free(ref);
            bl->Debug();
            if (bl->NumAlocado == N - 1)
                BlColocaInicio(bl);
        // Apaga o último bloco
            TBloco * b2 = Fim;
            (b2->Antes ? b2->Antes->Depois : Inicio) = nullptr;
            Fim = b2->Antes;
            delete b2;
            TotalLivre -= N - 1;
            TotalBlocos--;
        }
        else if (bl == Fim) // Libera objeto do último bloco
        {
            bl->free(ref);
            bl->Debug();
            TotalLivre++;
        }
        else // Move do último bloco para o objeto
        {
            T * obj1 = Fim->free();
            Fim->Debug();
            obj1->Mover(&ref->Objeto);
            TotalLivre++;
            return obj1;
        }
        Debug();
        return nullptr;
    }

    void LiberaBloco() {}
};

//---------------------------------------------------------------------------
// Programa para testar alocamem.h

#if 0
// ulimit -S -c unlimited
// time ./alocamem

//#define DEBUG_MEM

class CL1;
const unsigned int TotalObj = 10*1000;
CL1 ** Objetos = new CL1*[TotalObj];

//----------------
class CL1
{
public:
    unsigned int valor;
    void Mover(CL1 * destino)
    {
        assert(valor < TotalObj);
        assert(Objetos[valor] = this);
        destino->valor = valor;
        Objetos[valor] = destino;
        valor = 0xffffff;
    }
};

//----------------
#include <string.h>
#include <time.h>
static unsigned long seed;
void circle_srandom(unsigned long initial_seed)
{
    seed = initial_seed;
}
unsigned long circle_random(void)
{
    const unsigned long m = (unsigned long)2147483647;
    const unsigned long q = (unsigned long)127773;
    const unsigned long a = (unsigned int)16807;
    const unsigned long r = (unsigned int)2836;
    int hi   = seed/q;
    int lo   = seed%q;
    int test = a*lo - r*hi;
    if (test > 0)
        seed = test;
    else
        seed = test+ m;
    return (seed);
}

//----------------
int main()
{
    //TAlocaMem2<CL1, 250, unsigned char> Teste1; // 160ms
    //TAlocaMemSimples<CL1, 250> Teste1; // 121ms
    TAlocaMemSistema<CL1> Teste1; // 195ms
    for (unsigned int cont = 0; cont < TotalObj; cont++)
        Objetos[cont] = nullptr;
    circle_srandom(time(0));
    for (int cont = 0; cont < 10000000; cont++)
    {
        //putchar('.'); fflush(stdout);
        unsigned int num = circle_random() % TotalObj;
        if (Objetos[num] == nullptr)
        {
            Objetos[num] = Teste1.malloc();
            Objetos[num]->valor = num;
        }
        else
        {
            assert(Objetos[num]->valor == num);
            Teste1.free(Objetos[num]);
            Objetos[num] = nullptr;
        }
    }
    for (unsigned int num = 0; num < TotalObj; num++)
    {
        if (Objetos[num] != nullptr)
        {
            assert(Objetos[num]->valor == num);
            Teste1.free(Objetos[num]);
        }
    }
}
#endif

//---------------------------------------------------------------------------

#endif
