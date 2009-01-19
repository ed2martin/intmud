#ifndef PROCURA_H
#define PROCURA_H

//----------------------------------------------------------------------------
/** Procura a ocorrência de um texto em outro; pode realizar substituções */
class TProcurar /// Procura texto
{
public:
    TProcurar();
    bool Padrao(const char * padrao, bool exato);
    int  Proc(const char * texto, int tamanho);

    char * dest;        ///< Aonde colocar o texto modificado, ou 0
    int  tamdest;       ///< Tamanho do buffer em dest
    char troca[4096];   ///< Texto que deverá substituir o padrão

//private:
    int  tabela[0x100]; ///< Tabela de deslocamento
    char padrao[4096];  ///< Texto procurado
    int  tampadrao;     ///< Tamanho do padrão
    bool exato;         ///< Se diferencia minúsculas de maiúsculas
    void AddTroca(const char * texto);
    void AddTroca();
    const char * lidoatual; ///< Até aonde já anotou o texto lido
    const char * lidofim;   ///< Marca o fim do texto em lidoatual
    char * destatual;   ///< Aonde anotar o texto
    char * destfim;     ///< dest + tamdest - 1
};

#endif
