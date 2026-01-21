#ifndef ARQMAPA_H
#define ARQMAPA_H

#include "misc.h"

//------------------------------------------------------------------------------
class TClasse;
/** Lista de arquivos que compõem o programa interpretado.
    Se não houver arquivo, significa que está em um único arquivo */
class TArqMapa /// Arquivos que compôem o programa interpretado
{
public:
    TArqMapa(const char * arquivo);
    ~TArqMapa();
    static bool NomeValido(const char * nome, bool incluir=false);
        ///< Verifica se nome é um nome válido para arquivo
        /**< @param nome Nome a pesquisar
         *   @param incluir Se está checando nome para a instrução incluir
         *   @return true se o nome é valido */
    static TArqMapa * Procura(const char * nome);
        ///< Procura um arquivo a partir do nome
        /**< @param nome Nome a pesquisar
         *   @return Endereço do objeto, ou 0 se não foi encontrado */
    static void SalvarArq(bool tudo);
            ///< Salva classes em arquivo
            /**< @param tudo true=salvar todas as classes,
                             false=salvar só o que foi modificado */
    static unsigned short ParamLinha; ///< Caracteres por linha
    static unsigned char ParamN;      ///< Como dividir em \\n
    static unsigned char ParamIndent; ///< Quantos espaços para indentação
    static unsigned char ParamClasse; ///< Linhas entre classes
    static unsigned char ParamFunc;   ///< Linhas entre funções
    static unsigned char ParamVar;    ///< Linhas entre variáveis

    bool Mudou;             ///< Se o arquivo foi alterado
    bool Existe;            ///< Se o arquivo já existe
    char Arquivo[INT_NOME_TAM];  /**< Nome do arquivo sem o ".int" no final
                             Se for nulo, é o arquivo principal */

// Objetos TClasse
    TClasse * ClInicio;     ///< Primeira classe do arquivo
    TClasse * ClFim;        ///< Última classe do arquivo

// Objetos TArqMapa
    TArqMapa *Anterior;     ///< Lista ligada; objeto anterior
    TArqMapa *Proximo;      ///< Lista ligada; próximo objeto
    static TArqMapa *Inicio;///< Lista ligada; primeiro objeto
    static TArqMapa *Fim;   ///< Lista ligada; último objeto
};

//------------------------------------------------------------------------------

#endif
