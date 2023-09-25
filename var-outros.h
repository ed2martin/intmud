#ifndef VAR_OUTROS_H
#define VAR_OUTROS_H

class TVarInfo;

//----------------------------------------------------------------------------
const TVarInfo * VarOutrosConstNulo();
const TVarInfo * VarOutrosConstTxt();
const TVarInfo * VarOutrosConstNum();
const TVarInfo * VarOutrosConstExpr();
const TVarInfo * VarOutrosConstVar();
const TVarInfo * VarOutrosFunc();
const TVarInfo * VarOutrosVarFunc();

const TVarInfo * VarOutrosTxtFixo();
const TVarInfo * VarOutrosVarNome();
const TVarInfo * VarOutrosVarInicio();
const TVarInfo * VarOutrosVarIniFunc();
const TVarInfo * VarOutrosVarClasse();
const TVarInfo * VarOutrosVarObjeto();
const TVarInfo * VarOutrosVarInt();

//----------------------------------------------------------------------------

#endif
