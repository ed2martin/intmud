#ifndef CLASSE_H
#define CLASSE_H

//----------------------------------------------------------------------------
// Classes dos arquivos .map
class TClasse
{
public:
    TClasse(const char * nome);
    ~TClasse();
    static bool NomeClasse(char * nome);
    static TClasse * Procura(const char * nome);

    char * Comandos;    // Lista de comandos da classe
    char Nome[32];      // Nome da classe; n�o deve ser mudado

// Classes derivadas dessa (heran�a)
// Nota: antes de apagar uma classe, deve-se apagar as classes derivadas
    TClasse ** ListaDeriv; // Matriz (� NULL se NumDeriv==0)
    int NumDeriv;       // N�mero de elementos da matriz

// �rvore organizada por TClasse::Nome
public:
    void RBinsert(void);
    void RBremove(void);
    static TClasse * RBfirst(void);
    static TClasse * RBlast(void);
    static TClasse * RBnext(TClasse *);
    static TClasse * RBprevious(TClasse *);
private:
    static TClasse * RBroot;  // Objeto raiz
    TClasse *RBleft,*RBright,*RBparent; // Objetos filhos e objeto pai
    void RBleft_rotate(void); // 8 fun��es prontas em rbt.cpp
    void RBright_rotate(void);
    static int RBcomp(TClasse * x, TClasse * y); // Compara objetos
    unsigned char RBcolour;
};

//----------------------------------------------------------------------------

#endif
