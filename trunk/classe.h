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

    void AcertaComandos(); // Acerta dados de Comandos
    void AcertaVar();   // Acarta ListaVar e NumVar

    char * Comandos;    // Lista de comandos da classe
    char Nome[32];      // Nome da classe; não deve ser mudado

// Classes derivadas dessa (herança)
// Nota: antes de apagar uma classe, deve-se apagar as classes derivadas
    TClasse ** ListaDeriv; // Matriz (é NULL se NumDeriv==0)
    int NumDeriv;          // Número de elementos da matriz

// Lista de variáveis da classe, inclusive as herdadas
    char ** InstrVar;      // Instruções que definem as variáveis
    int  * IndiceVar;      // Bits 23-0 = índice na memória
                           // Bits 31-24 = para controle
                           // Bit 24: 0 = índice em TObjeto::Vars
                           //         1 = índice em TClasse::Vars
                           // Bit 25 =1 se variável definida na classe
    unsigned int NumVar;   // Número de variáveis

// Árvore organizada por TClasse::Nome
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
    void RBleft_rotate(void); // 8 funções prontas em rbt.cpp
    void RBright_rotate(void);
    static int RBcomp(TClasse * x, TClasse * y); // Compara objetos
    unsigned char RBcolour;
};

//----------------------------------------------------------------------------

#endif
