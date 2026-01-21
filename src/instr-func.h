#ifndef INSTR_FUNC_H
#define INSTR_FUNC_H

class TVariavel;

/// Funções predefinidas da linguagem
/** Funções predefinidas da linguagem */
namespace InstrFunc {

bool FuncArg0(TVariavel * v); ///< Função arg0
bool FuncArg1(TVariavel * v); ///< Função arg1
bool FuncArg2(TVariavel * v); ///< Função arg2
bool FuncArg3(TVariavel * v); ///< Função arg3
bool FuncArg4(TVariavel * v); ///< Função arg4
bool FuncArg5(TVariavel * v); ///< Função arg5
bool FuncArg6(TVariavel * v); ///< Função arg6
bool FuncArg7(TVariavel * v); ///< Função arg7
bool FuncArg8(TVariavel * v); ///< Função arg8
bool FuncArg9(TVariavel * v); ///< Função arg9
bool FuncArgs(TVariavel * v); ///< Número de argumentos (args)
bool FuncCriar(TVariavel * v); ///< Criar objeto (função criar)
bool FuncApagar(TVariavel * v); ///< Apagar objeto (função apagar)
bool FuncMatPi(TVariavel * v); ///< Função matpi
bool FuncEste(TVariavel * v); ///< Objeto "este"
bool FuncIntPos(TVariavel * v); ///< Função intpos
bool FuncIntAbs(TVariavel * v); ///< Função intabs
bool FuncInt(TVariavel * v); ///< Função int
bool FuncIntDiv(TVariavel * v); ///< Função intdiv
bool FuncMatSin(TVariavel * v); ///< Função matsin
bool FuncMatCos(TVariavel * v); ///< Função matcos
bool FuncMatTan(TVariavel * v); ///< Função mattan
bool FuncMatAsin(TVariavel * v); ///< Função matasin
bool FuncMatAcos(TVariavel * v); ///< Função matacos
bool FuncMatAtan(TVariavel * v); ///< Função matatan
bool FuncMatExp(TVariavel * v); ///< Função matexp
bool FuncMatLog(TVariavel * v); ///< Função matlog
bool FuncMatRaiz(TVariavel * v); ///< Função matraiz
bool FuncMatCima(TVariavel * v); ///< Função matcima
bool FuncMatBaixo(TVariavel * v); ///< Função matbaixo
bool FuncMatRad(TVariavel * v); ///< Função matrad
bool FuncMatDeg(TVariavel * v); ///< Função matdeg
bool FuncMatPow(TVariavel * v); ///< Função mathpow
bool FuncIntBit(TVariavel * v); ///< Função intbit
bool FuncIntBitH(TVariavel * v); ///< Função intbith
bool FuncTxtBit(TVariavel * v); ///< Função txtbit
bool FuncTxtBitH(TVariavel * v); ///< Função txtbith
bool FuncTxtHex(TVariavel * v); ///< Função txthex
bool FuncIntMax(TVariavel * v); ///< Função intmax
bool FuncIntMin(TVariavel * v); ///< Função intmin
bool FuncRand(TVariavel * v); ///< Função rand
bool FuncRef(TVariavel * v); ///< Referência (ref)
bool FuncTxtTipoVar(TVariavel * v); ///< Função txttipovar
bool FuncTxtNum(TVariavel * v); ///< Função txtnum
bool FuncIntSub(TVariavel * v); ///< Função intsub
bool FuncIntSubLin(TVariavel * v); ///< Função intsublin
bool FuncTxt(TVariavel * v); ///< Função txt
bool FuncTxtSub(TVariavel * v); ///< Função txtsub
bool FuncTxtSubLin(TVariavel * v); ///< Função txtsublin
bool FuncTxtFim(TVariavel * v); ///< Função txtfim
bool FuncTxt1(TVariavel * v); ///< Função txt1
bool FuncTxt2(TVariavel * v); ///< Função txt2
bool FuncTxtCor(TVariavel * v); ///< Função txtcor
bool FuncTxtMai(TVariavel * v); ///< Função txtmai
bool FuncTxtMaiIni(TVariavel * v); ///< Função txtmaiini
bool FuncTxtMin(TVariavel * v); ///< Função txtmin
bool FuncTxtMaiMin(TVariavel * v); ///< Função txtmaimin
bool FuncTxtFiltro(TVariavel * v); ///< Função txtfiltro
bool FuncTxtSha1Bin(TVariavel * v); ///< Função txtsha1bin
bool FuncTxtSha1(TVariavel * v); ///< Função txtsha1
bool FuncTxtMd5(TVariavel * v); ///< Função txtms5
bool FuncTxtNome(TVariavel * v); ///< Função txtnome
bool FuncTxtInvis(TVariavel * v); ///< Função txtinvis
bool FuncTxtCod(TVariavel * v); ///< Função txtcod
bool FuncTxtDec(TVariavel * v); ///< Função txtdec
bool FuncTxtVis(TVariavel * v); ///< Função txtvis
bool FuncTxtUrlCod(TVariavel * v); ///< Função txturlcod
bool FuncTxtUrlDec(TVariavel * v); ///< Função txturldec
bool FuncTxtE(TVariavel * v); ///< Função txte
bool FuncTxtS(TVariavel * v); ///< Função txts
bool FuncTxtRev(TVariavel * v); ///< Função txtrev
bool FuncTxtMudaMai(TVariavel * v); ///< Função txtmudamai
bool FuncTxtCopiaMai(TVariavel * v); ///< Função txtcopiamai
bool FuncEsp(TVariavel * v); ///< Função txtesp
bool FuncTxtRepete(TVariavel * v); ///< Função txtrepete
bool FuncIntNome(TVariavel * v); ///< Função intnome
bool FuncIntSenha(TVariavel * v); ///< Função intsenha
bool FuncTxtRemove(TVariavel * v); ///< Função txtremove
bool FuncTxtConv(TVariavel * v); ///< Função txtconv
bool FuncTxtChr(TVariavel * v); ///< Função txtchr
bool FuncIntChr(TVariavel * v); ///< Função intchr
bool FuncIntDist(TVariavel * v); ///< Função intdist
bool FuncIntDistMai(TVariavel * v); ///< Função intdistmai
bool FuncIntDistDif(TVariavel * v); ///< Função intdistdif
bool FuncTxtProc(TVariavel * v); ///< Função txtproc
bool FuncTxtProcMai(TVariavel * v); ///< Função txtprocmai
bool FuncTxtProcDif(TVariavel * v); ///< Função txtprocdif
bool FuncTxtProcLin(TVariavel * v); ///< Função txtproclin
bool FuncTxtProcLinMai(TVariavel * v); ///< Função txtproclinmai
bool FuncTxtProcLinDif(TVariavel * v); ///< Função txtproclindif
bool FuncTxtTroca(TVariavel * v); ///< Função txttroca
bool FuncTxtTrocaMai(TVariavel * v); ///< Função txttrocaMai
bool FuncTxtTrocaDif(TVariavel * v); ///< Função txttrocaDif
bool FuncTxtSepara(TVariavel * v); ///< Função txtsepara
bool FuncObjAntes(TVariavel * v); ///< Função objantes
bool FuncObjDepois(TVariavel * v); ///< Função objdepois
bool FuncTotal(TVariavel * v); ///< Função inttotal
bool FuncVarTroca(TVariavel * v); ///< Função vartroca
bool FuncVarTrocaCod(TVariavel * v); ///< Função vartrocacod

}

#endif
