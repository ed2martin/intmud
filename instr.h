#ifndef INSTR_H
#define INSTR_H

class TClasse;
class TObjeto;
class TVariavel;

#define OTIMIZAR_VAR // Otimização de funções predefinidas e variáveis locais
#define VAR_NOME_TAM 80 // Tamanho máximo dos nomes das variáveis + 1
#define BUF_MENS 16384 // Tamanho do buffer de texto usado nas funções
#define BUF_CODIF 8192 // Tamanho do buffer com uma instrução codificada

/// Codificar, decodificar e executar instruções
/** Codificar, decodificar e executar instruções */
namespace Instr {

//----------------------------------------------------------------------------
// Funções
bool Codif(char * destino, const char * origem, int tamanho);
bool Decod(char * destino, const char * origem, int tamanho);
bool Mostra(char * destino, const char * origem, int tamanho);

int  Prioridade(int operador);
bool ChecaHerda(const char * instr, const char * nomeclasse);
const char * ProximaInstr(const char * instr, const char * texto, int tamanho);

bool CriarVarInt(int valor);
bool CriarVarInt(TVariavel * v, int valor);
bool CriarVarDouble(double valor);
bool CriarVarDouble(TVariavel * v, double valor);
bool CriarVarTexto(const char * mens, int tammens = -1);
bool CriarVarTxtFixo(const char * texto);
bool CriarVarTxtFixo(TVariavel * v, const char * texto);
bool CriarVarObj(TObjeto * obj);
bool CriarVar(const char * defvar);
void ApagarVar(TVariavel * v);
void ApagarRet(TVariavel * v);
void VarFuncPrepara(TVariavel * varini);
bool VarFuncFim();
bool ComparaInstr(const char * instr1, const char * instr2);
const char * ProcuraExpr(const char * expr, int valor);
const char * NomeInstr(const char * instr);
const char * NomeComando(int valor);
const char * NomeExpr(int valor);

bool ExecIni(TClasse * classe, const char * func);
bool ExecIni(TObjeto * este, const char * func);
void ExecArg(const char * txt);
void ExecArg(int valor);
void ExecArgCriar(const char * def);
bool ExecX();
void ExecFim();

//----------------------------------------------------------------------------
// Variáveis predefinidas
/// TVariavel::defvar para Instr::cNulo
extern const char InstrNulo[];
/// TVariavel::defvar para Instr::cSocket
extern const char InstrSocket[];
/// TVariavel::defvar para Instr::cTxtFixo
extern const char InstrTxtFixo[];
/// TVariavel::defvar para Instr::cVarNome
extern const char InstrVarNome[];
/// TVariavel::defvar para Instr::cVarInicio
extern const char InstrVarInicio[];
/// TVariavel::defvar para Instr::cVarIniFunc
extern const char InstrVarIniFunc[];
/// TVariavel::defvar para Instr::cVarClasse
extern const char InstrVarClasse[];
/// TVariavel::defvar para Instr::cVarObjeto
extern const char InstrVarObjeto[];
/// TVariavel::defvar para Instr::cVarInt
extern const char InstrVarInt[];
/// TVariavel::defvar para Instr::cVarDouble
extern const char InstrVarDouble[];
/// TVariavel::defvar para Instr::cListaItem
extern const char InstrVarListaItem[];
/// TVariavel::defvar para Instr::cTextoPos
extern const char InstrVarTextoPos[];
/// TVariavel::defvar para Instr::cTextoVarSub
extern const char InstrVarTextoVarSub[];
/// TVariavel::defvar para Instr::cTextoObjSub
extern const char InstrVarTextoObjSub[];
/// TVariavel::func para Instr::cDebug
extern const char InstrDebugFunc[];
/// TVariavel::defvar para operadores ++ e --
extern const char InstrAddSub[];
/// Quantas instruções pode executar antes que o controle retorne ao programa
extern int VarExec;
/// Valor inicial de Instr::VarExec quando Instr::ExecIni é executado
extern int VarExecIni;

//----------------------------------------------------------------------------
/** Verifica se instruções de uma classe (codificadas por Instr::Codif)
    estão na ordem correta */
class ChecaLinha /// Verifica ordem das instruções de uma classe
{
public:
    ChecaLinha();
    void Inicio();
            /**< Indica que está no início da lista de instruções */
    const char * Instr(const char * instr);
            /**< Checa próxima instrução da lista
                @param instr Instrução codificada
                @return 0 se instrução está no lugar correto,
                        outro valor=mensagem de erro */
    const char * Fim();
            ///< Chamado após checar todas as instruções
            /**< Verifica se há algum bloco pendente */
    static int ChecaErro;
            ///< Checagem de erros em blocos de instruções
            /**< - 0=não verifica se cada bloco está terminando corretamente
             *   - 1=verifica blocos exceto fimse no final das funções
             *   - 2=verifica blocos inclusive fimse no final das funções */
private:
    char esperando;
            /**< O que está esperando:
                - 0=início do arquivo; pode receber "herda"
                - 1=função ou variável (pertencente à classe)
                - 2=instruções de uma função
                - 3=após definição de função + definição de constante */
    unsigned char buf[1024];
            ///< Blocos; cada byte:
            /**< - 0=bloco se/senao/fimse
             *   - 1=bloco enquanto/efim ou epara/efim
             *   - 2=bloco casovar/casose/casofim */
    unsigned int pbuf;
            ///< Quantos bytes de buf[] estão sendo usados
};

//----------------------------------------------------------------------------
/** Pilha de funções - usado ao executar funções */
class ExecFunc /// Pilha de funções
{
public:
    TObjeto * este;     ///< Objeto ao qual a função pertence
    const char * nome;  ///< Nome da função; codificado como instrução
    const char * linha; ///< Instrução codificada sendo executada
                        ///< Mesmo formato de TClasse::Comandos
    const char * expr;  ///< Aonde parou na expressão numérica
                        ///< =0 se não estiver processando expressão numérica
    TVariavel * inivar; ///< Primeiro argumento da função
    TVariavel * fimvar; ///< Primeira variável após variáveis locais da função
    char  numarg;       ///< Número de argumentos arg0 a arg9
    char  tipo;         ///< Como é o retorno da função:
            /**< - 0 = função normal
             *   - 1 = ler varfunc
             *   - 2 = mudar varfunc
             *   - 3 = instrução criar()
             *   - 4 = instrução debug.cmd (transforma retorno em texto) */
    unsigned char indent;///< Para apagar variáveis que saíram do escopo
    char * funcdebug;   ///< Função executada a cada instrução, 0 se nenhuma
    TObjeto * objdebug; ///< Objeto relacionado a funcdebug
};

//----------------------------------------------------------------------------
/** Processa funções predefinidas
    @note A lista de funções está definida em instr-exec.cpp
*/
class TListaFunc /// Funções internas
{
public:
        /// Nome da função predefinida
    const char * Nome;

        /// Função que processa a função predefinida
        /** Ao executar a função, a pilha de variáveis está assim:
        - cVarNome
        - cVarInicio  (v aponta para essa variável)
        - Argumentos da função interna (VarAtual aponta para a última)
        .
        Se retornar true, deve:
        - Apagar variáveis de v em diante
        - Criar uma variável com o resultado
        .
        @param v Endereço da variável cVarInicio
        @retval true Se processou normalmente
        @retval false Se erro; o resultado será o tipo NULO */
    bool (*Func)(TVariavel * v);
};

/// Lista de funções predefinidas
/// Deve obrigatoriamente estar em ordem alfabética
extern const TListaFunc ListaFunc[];

/// Obtém o índice de uma função em ListaFunc a partir do nome
/** @param nome Nome da função
 *  @return -1 se não encontrou; caso contrário é o índice em ListaFunc */
int InfoFunc(const char * nome);

//----------------------------------------------------------------------------
    /// Pilha de dados (64K)
extern char * const DadosPilha;
extern char * const DadosFim;
    /// Pilha de dados - início da área de dados disponível
extern char * DadosTopo;

    /// Pilha de variáveis
extern TVariavel * const VarPilha;
extern TVariavel * const VarFim;
    /// Pilha de variáveis - última variável da pilha
extern TVariavel * VarAtual;

    /// Pilha de funções
extern ExecFunc * const FuncPilha;
extern ExecFunc * const FuncFim;
    /// Pilha de funções - última função da pilha
extern ExecFunc * FuncAtual;

//----------------------------------------------------------------------------
}

#endif
