%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  /* ===== Global variable to store parsed AST ===== */
  AstNode* program_root = NULL;

  /* ===== AST ===== */
  typedef enum {
    NK_PROGRAM, NK_FUNC, NK_BLOCK,
    NK_VAR_DECL, NK_ASSIGN, NK_FOR, NK_WHILE, NK_IF, NK_RETURN, NK_EXEC,
    NK_STMT_LIST, NK_PARAM, NK_PARAM_LIST,
    NK_ID, NK_INDEX, NK_CALL,
    NK_BOOL, NK_INT, NK_FLOAT, NK_CHAR,
    NK_BINOP, NK_UNOP
  } NodeKind;

  typedef enum {
    TY_INT, TY_BOOL, TY_CHAR, TY_FLOAT
  } TypeKind;

  typedef enum {
    OP_OR, OP_AND, OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_NOT, OP_NEG
  } OpKind;

  typedef struct AstNode AstNode;
  typedef struct AstList AstList;

  struct AstList { AstNode **items; int size, cap; };
  struct AstNode {
    NodeKind k;
    int line, col;        /* opcional: ubicaciones */
    union {
      /* Programa / listas */
      struct { AstList *items; } program;
      struct { AstList *items; } stmt_list;
      /* Función */
      struct { TypeKind type; char *name; AstList *params; AstNode *body; } func;
      struct { TypeKind type; char *name; int isArray; long arrSize; AstList *initList; AstNode *initExpr; } vardecl;
      struct { char *name; AstList *args; } exec;
      /* Control */
      struct { AstNode *cond, *thenBlk, *elseBlk; } ifs;
      struct { AstNode *init, *cond, *update, *body; } fors;
      struct { AstNode *cond, *body; } whiles;
      struct { AstList *stmts; } block;
      struct { AstNode *expr; } ret;
      struct { char *name; AstNode *rhs; AstList *rhsList; int rhsIsList; AstNode *indexExpr; int isIndexed; } assign;
      /* Expresiones */
      struct { char *name; } id;
      struct { char *name; AstNode *index; } index;
      struct { char *name; AstList *args; } call;
      struct { OpKind op; AstNode *left, *right; } binop;
      struct { OpKind op; AstNode *expr; } unop;
      struct { long ival; } lit_i;
      struct { double fval; } lit_f;
      struct { int bval; } lit_b;
      struct { char cval; } lit_c;
      /* Parámetros */
      struct { TypeKind type; char *name; int isArray; } param;
    };
  };

  /* helpers (abajo implementadas rápidas) */
  static AstList* list_new(void);
  static void     list_push(AstList*, AstNode*);
  static AstNode* mk_program(AstList*);
  static AstNode* mk_block(AstList*);
  static AstNode* mk_stmt_list(AstList*);
  static AstNode* mk_vardecl(TypeKind,char*,int,long,AstList*,AstNode*);
  static AstNode* mk_assign(char*, AstNode*, AstList*, int rhsIsList, AstNode *indexOpt);
  static AstNode* mk_if(AstNode*,AstNode*,AstNode*);
  static AstNode* mk_while(AstNode*,AstNode*);
  static AstNode* mk_for(AstNode*,AstNode*,AstNode*,AstNode*);
  static AstNode* mk_return(AstNode*);
  static AstNode* mk_exec(char*, AstList*);
  static AstNode* mk_id(char*);
  static AstNode* mk_index(char*, AstNode*);
  static AstNode* mk_call(char*, AstList*);
  static AstNode* mk_bin(OpKind, AstNode*, AstNode*);
  static AstNode* mk_un(OpKind, AstNode*);
  static AstNode* mk_lit_i(long);
  static AstNode* mk_lit_f(double);
  static AstNode* mk_lit_b(int);
  static AstNode* mk_lit_c(char);
  static TypeKind tk_from_kw(int kw);

  int yylex(void);
  void yyerror(const char* s) { fprintf(stderr, "parse error: %s\n", s); }
%}

%define api.value.type {union}
%locations

/* ===== Tokens con alias de string para que puedas escribir "start", "if", ... ===== */
%token PROC  "proc"
%token RETURN "return"
%token START "start"
%token END   "end"
%token FOR   "for"
%token WHILE "while"
%token IF    "if"
%token ELSE  "else"
%token EXEC  "exec"
%token INTKW "int"
%token BOOLKW "bool"
%token CHARKW "char"
%token FLOATKW "float"
%token TRUE  "true"
%token FALSE "false"

/* Operadores y signos: usa literales de char cuando son 1 char */
%token LE "<=" GE ">=" EQ "==" NE "!=" AND_AND "&&" OR_OR "||"
/* Los de un solo char podemos usarlos como literales: '+', '-', '*', '/', '%', '=', '(', ')', '[', ']', ',', ';' */

%token <id> ID
%token <ival> VALOR  /* entero */
%token <fval> DECIMAL /* float */
%token <cval> LETRA   /* char literal */

%type  <node> Programa Declaracion_funcion Retorno_funcion Bloque BloqueAux Instruccion
%type  <node> Asignacion Declaracion For While If Exec_funcion
%type  <node> Expresion OrExpresion OrExpresionAux AndExpresion AndExpresionAux
%type  <node> NotExpresion Expresion_relacional Expresion_relacionalAux
%type  <node> Expresion_suma Expresion_sumaAux Expresion_multiplicacion
%type  <node> Termino TerminoAux Factor ArrayIndexOpt
%type  <list> Lista_valores Lista_valores_aux Argumentos ArgumentosAux
%type  <list> Parametros_funcion Parametros_funcion_aux
%type  <node> Valor
%type  <node> Tipo_dato
%type  <id>   Id
%type  <node> BloqueElseOpt ElseOpt

%%  /* ===================== GRAMÁTICA + ACCIONES ===================== */

Programa
  : Bloque Programa         { AstList* xs = list_new(); list_push(xs, $1); 
                              if ($2 && $2->k==NK_PROGRAM) { 
                                for(int i=0;i<$2->program.items->size;i++) list_push(xs,$2->program.items->items[i]); 
                              } 
                              $$ = mk_program(xs); program_root = $$; }
  | Declaracion_funcion Programa
                            { AstList* xs = list_new(); list_push(xs, $1);
                              if ($2 && $2->k==NK_PROGRAM) {
                                for(int i=0;i<$2->program.items->size;i++) list_push(xs,$2->program.items->items[i]);
                              }
                              $$ = mk_program(xs); program_root = $$; }
  | /* empty */             { $$ = mk_program(list_new()); program_root = $$; }
  ;

Declaracion_funcion
  : PROC Tipo_dato Id '(' Parametros_funcion ')' Bloque
                            { $$ = (AstNode*) malloc(sizeof(AstNode));
                              $$->k = NK_FUNC;
                              $$->func.type = ((TypeKind)(intptr_t)$2); free($2); /* ver nota abajo */
                              $$->func.name = $3;
                              $$->func.params = $5;
                              $$->func.body = $7; }
  ;

/* Parametros: (Tipo Id) (',' Tipo Id)* | ε */
Parametros_funcion
  : Tipo_dato Id Parametros_funcion_aux
                            { AstList* xs = list_new();
                              AstNode* p = (AstNode*) malloc(sizeof(AstNode));
                              p->k = NK_PARAM; p->param.type=(TypeKind)(intptr_t)$1; free($1);
                              p->param.name=$2; p->param.isArray=0;
                              list_push(xs,p);
                              if ($3) { for(int i=0;i<$3->size;i++) list_push(xs,$3->items[i]); }
                              $$ = xs; }
  | /* empty */             { $$ = list_new(); }
  ;

Parametros_funcion_aux
  : ',' Tipo_dato Id Parametros_funcion_aux
                            { AstList* xs = list_new();
                              AstNode* p = (AstNode*) malloc(sizeof(AstNode));
                              p->k = NK_PARAM; p->param.type=(TypeKind)(intptr_t)$2; free($2);
                              p->param.name=$3; p->param.isArray=0;
                              list_push(xs,p);
                              if ($4) { for(int i=0;i<$4->size;i++) list_push(xs,$4->items[i]); }
                              $$ = xs; }
  | /* empty */             { $$ = list_new(); }
  ;

Retorno_funcion
  : RETURN Expresion        { $$ = mk_return($2); }
  ;

Bloque
  : START BloqueAux END     { $$ = mk_block(($2 && $2->k==NK_STMT_LIST)? $2->stmt_list.items : list_new()); }
  ;

BloqueAux
  : Instruccion BloqueAux   { AstList* xs = list_new();
                              if ($1) list_push(xs,$1);
                              if ($2 && $2->k==NK_STMT_LIST) {
                                for(int i=0;i<$2->stmt_list.items->size;i++) list_push(xs,$2->stmt_list.items->items[i]);
                              }
                              $$ = mk_stmt_list(xs); }
  | /* empty */             { $$ = mk_stmt_list(list_new()); }
  ;

Instruccion
  : Asignacion ';'          { $$ = $1; }
  | Declaracion ';'         { $$ = $1; }
  | For                     { $$ = $1; }
  | While                   { $$ = $1; }
  | If                      { $$ = $1; }
  | Exec_funcion ';'        { $$ = $1; }
  | Retorno_funcion ';'     { $$ = $1; }
  ;

Asignacion
  : Id '=' Expresion        { $$ = mk_assign($1, $3, NULL, 0, NULL); }
  | Id '=' '[' Lista_valores ']' 
                            { $$ = mk_assign($1, NULL, $4, 1, NULL); }
  ;

Lista_valores
  : Expresion Lista_valores_aux
                            { AstList* xs=list_new(); list_push(xs,$1);
                              if ($2) for(int i=0;i<$2->size;i++) list_push(xs,$2->items[i]);
                              $$ = xs; }
  | /* empty */             { $$ = list_new(); }
  ;
Lista_valores_aux
  : ',' Expresion Lista_valores_aux
                            { AstList* xs=list_new(); list_push(xs,$2);
                              if ($3) for(int i=0;i<$3->size;i++) list_push(xs,$3->items[i]);
                              $$ = xs; }
  | /* empty */             { $$ = list_new(); }
  ;

Declaracion
  : Tipo_dato Id                         { $$ = mk_vardecl((TypeKind)(intptr_t)$1,$2,0,0,NULL,NULL); free($1); }
  | Tipo_dato Id '=' Expresion           { $$ = mk_vardecl((TypeKind)(intptr_t)$1,$2,0,0,NULL,$4); free($1); }
  | Tipo_dato Id '[' VALOR ']'           { $$ = mk_vardecl((TypeKind)(intptr_t)$1,$2,1,$4,NULL,NULL); free($1); }
  | Tipo_dato Id '[' VALOR ']' '=' '[' Lista_valores ']'
                                         { $$ = mk_vardecl((TypeKind)(intptr_t)$1,$2,1,$4,$8,NULL); free($1); }
  ;

For
  : FOR '(' Declaracion ';' Expresion ';' Asignacion ')' Bloque
                            { $$ = mk_for($3,$5,$7,$9); }
  ;

While
  : WHILE '(' Expresion ')' Bloque
                            { $$ = mk_while($3,$5); }
  ;

If
  : IF '(' Expresion ')' BloqueElseOpt   { $$ = $5; /* BloqueElseOpt ya empaqueta THEN/ELSE */ }
  ;

BloqueElseOpt
  : Bloque ElseOpt           { /* ElseOpt produce else-bloque o vacío; empaquetamos si/no else */
                               if ($2) { /* con else */
                                 AstNode* n=(AstNode*)malloc(sizeof(AstNode));
                                 n->k = NK_IF;
                                 n->ifs.cond = NULL; /* set más arriba */
                                 /* truco: guardamos el then en n->ifs.thenBlk y el else en n->ifs.elseBlk,
                                    luego el IF padre rellena cond */
                                 n->ifs.thenBlk = $1;
                                 n->ifs.elseBlk = $2;
                                 $$ = n;
                               } else {
                                 AstNode* n=(AstNode*)malloc(sizeof(AstNode));
                                 n->k = NK_IF;
                                 n->ifs.thenBlk = $1;
                                 n->ifs.elseBlk = NULL;
                                 $$ = n;
                               } }
  ;

ElseOpt
  : ELSE Bloque              { $$ = $2; }
  | /* empty */              { $$ = NULL; }
  ;

/* ---------- Llamadas tipo exec ---------- */
Exec_funcion
  : EXEC Id '(' Argumentos ')'  { $$ = mk_exec($2, $4); }
  ;

Argumentos
  : Expresion ArgumentosAux
                            { AstList* xs=list_new(); list_push(xs,$1);
                              if ($2) for(int i=0;i<$2->size;i++) list_push(xs,$2->items[i]);
                              $$ = xs; }
  | /* empty */             { $$ = list_new(); }
  ;
ArgumentosAux
  : ',' Expresion ArgumentosAux
                            { AstList* xs=list_new(); list_push(xs,$2);
                              if ($3) for(int i=0;i<$3->size;i++) list_push(xs,$3->items[i]);
                              $$ = xs; }
  | /* empty */             { $$ = list_new(); }
  ;

/* ---------- Tipos y Atomos ---------- */
Tipo_dato
  : INTKW     { $$ = (AstNode*)(intptr_t)TY_INT; }
  | BOOLKW    { $$ = (AstNode*)(intptr_t)TY_BOOL; }
  | CHARKW    { $$ = (AstNode*)(intptr_t)TY_CHAR; }
  | FLOATKW   { $$ = (AstNode*)(intptr_t)TY_FLOAT; }
  ;

Id : ID { $$ = $1; } ;

Valor
  : VALOR     { $$ = mk_lit_i($1); }
  | DECIMAL   { $$ = mk_lit_f($1); }
  | LETRA     { $$ = mk_lit_c($1); }
  | TRUE      { $$ = mk_lit_b(1); }
  | FALSE     { $$ = mk_lit_b(0); }
  ;

/* ================= EXPRESIONES con precedencia ================= */

Expresion            : OrExpresion                    { $$ = $1; } ;

OrExpresion          : AndExpresion OrExpresionAux    { $$ = $2 ? mk_bin(OP_OR,$1,$2) : $1; } ;
OrExpresionAux       : OR_OR AndExpresion OrExpresionAux
                                                { $$ = $3 ? mk_bin(OP_OR,$2,$3) : $2; }
                     | /* empty */                    { $$ = NULL; }
                     ;

AndExpresion         : NotExpresion AndExpresionAux   { $$ = $2 ? mk_bin(OP_AND,$1,$2) : $1; } ;
AndExpresionAux      : AND_AND NotExpresion AndExpresionAux
                                                { $$ = $3 ? mk_bin(OP_AND,$2,$3) : $2; }
                     | /* empty */                    { $$ = NULL; }
                     ;

NotExpresion         : '!' NotExpresion               { $$ = mk_un(OP_NOT,$2); }
                     | Expresion_relacional           { $$ = $1; }
                     ;

Expresion_relacional : Expresion_suma Expresion_relacionalAux
                     { $$ = $2 ? mk_bin((OpKind)(intptr_t)$2,$1, ((AstNode*)$2)->binop.right) : $1; }
                     ;

Expresion_relacionalAux
  : EQ  Expresion_suma   { AstNode* n=mk_bin(OP_EQ, NULL, $2); $$=(AstNode*)(intptr_t)OP_EQ; n->binop.right=$2; $$=n; }
  | NE  Expresion_suma   { AstNode* n=mk_bin(OP_NE, NULL, $2); $$=n; }
  | '<' Expresion_suma   { AstNode* n=mk_bin(OP_LT, NULL, $2); $$=n; }
  | LE  Expresion_suma   { AstNode* n=mk_bin(OP_LE, NULL, $2); $$=n; }
  | '>' Expresion_suma   { AstNode* n=mk_bin(OP_GT, NULL, $2); $$=n; }
  | GE  Expresion_suma   { AstNode* n=mk_bin(OP_GE, NULL, $2); $$=n; }
  | /* empty */          { $$ = NULL; }
  ;

Expresion_suma       : Expresion_multiplicacion Expresion_sumaAux
                     { $$ = $2 ? mk_bin(((AstNode*)$2)->binop.op,$1,((AstNode*)$2)->binop.right) : $1; } ;

Expresion_sumaAux
  : '+' Expresion_multiplicacion Expresion_sumaAux
                     { AstNode* t = $3 ? mk_bin(((AstNode*)$3)->binop.op,$2,((AstNode*)$3)->binop.right):$2;
                       $$ = mk_bin(OP_ADD, t, NULL); }
  | '-' Expresion_multiplicacion Expresion_sumaAux
                     { AstNode* t = $3 ? mk_bin(((AstNode*)$3)->binop.op,$2,((AstNode*)$3)->binop.right):$2;
                       $$ = mk_bin(OP_SUB, t, NULL); }
  | /* empty */      { $$ = NULL; }
  ;

Expresion_multiplicacion
  : Termino TerminoAux
  { $$ = $2 ? mk_bin(((AstNode*)$2)->binop.op,$1,((AstNode*)$2)->binop.right) : $1; }
  ;

TerminoAux
  : '*' Termino TerminoAux
                     { AstNode* t=$3 ? mk_bin(((AstNode*)$3)->binop.op,$2,((AstNode*)$3)->binop.right):$2;
                       $$ = mk_bin(OP_MUL, t, NULL); }
  | '/' Termino TerminoAux
                     { AstNode* t=$3 ? mk_bin(((AstNode*)$3)->binop.op,$2,((AstNode*)$3)->binop.right):$2;
                       $$ = mk_bin(OP_DIV, t, NULL); }
  | '%' Termino TerminoAux
                     { AstNode* t=$3 ? mk_bin(((AstNode*)$3)->binop.op,$2,((AstNode*)$3)->binop.right):$2;
                       $$ = mk_bin(OP_MOD, t, NULL); }
  | /* empty */      { $$ = NULL; }
  ;

Termino
  : '-' Factor        { $$ = mk_un(OP_NEG, $2); }
  | Factor            { $$ = $1; }
  ;

Factor
  : Id ArrayIndexOpt  { $$ = $2 ? mk_index($1,$2) : mk_id($1); }
  | Valor             { $$ = $1; }
  | '(' Expresion ')' { $$ = $2; }
  ;

ArrayIndexOpt
  : '[' Expresion ']' { $$ = $2; }
  | /* empty */       { $$ = NULL; }
  ;

%%  /* ===================== Helpers AST (implementación simple) ===================== */

static AstList* list_new(void){ AstList* L=calloc(1,sizeof(*L)); L->cap=4; L->items=malloc(sizeof(AstNode*)*L->cap); return L; }
static void list_push(AstList* L, AstNode* n){ if(L->size==L->cap){ L->cap*=2; L->items=realloc(L->items,sizeof(AstNode*)*L->cap);} L->items[L->size++]=n; }

static AstNode* node_new(NodeKind k){ AstNode* n=calloc(1,sizeof(*n)); n->k=k; return n; }
static AstNode* mk_program(AstList* xs){ AstNode* n=node_new(NK_PROGRAM); n->program.items=xs; return n; }
static AstNode* mk_stmt_list(AstList* xs){ AstNode* n=node_new(NK_STMT_LIST); n->stmt_list.items=xs; return n; }
static AstNode* mk_block(AstList* xs){ AstNode* n=node_new(NK_BLOCK); n->block.stmts=xs; return n; }
static AstNode* mk_vardecl(TypeKind t,char*name,int isArr,long sz,AstList*initList,AstNode*initExpr){
  AstNode* n=node_new(NK_VAR_DECL); n->vardecl.type=t; n->vardecl.name=name; n->vardecl.isArray=isArr; n->vardecl.arrSize=sz;
  n->vardecl.initList=initList; n->vardecl.initExpr=initExpr; return n; }
static AstNode* mk_assign(char*name, AstNode* rhs, AstList* list, int rhsIsList, AstNode* idx){
  AstNode* n=node_new(NK_ASSIGN); n->assign.name=name; n->assign.rhs=rhs; n->assign.rhsList=list; n->assign.rhsIsList=rhsIsList; n->assign.indexExpr=idx; n->assign.isIndexed=(idx!=NULL); return n; }
static AstNode* mk_if(AstNode* c, AstNode* th, AstNode* el){ AstNode* n=node_new(NK_IF); n->ifs.cond=c; n->ifs.thenBlk=th; n->ifs.elseBlk=el; return n; }
static AstNode* mk_while(AstNode* c, AstNode* b){ AstNode* n=node_new(NK_WHILE); n->whiles.cond=c; n->whiles.body=b; return n; }
static AstNode* mk_for(AstNode* i, AstNode* c, AstNode* u, AstNode* b){ AstNode* n=node_new(NK_FOR); n->fors.init=i; n->fors.cond=c; n->fors.update=u; n->fors.body=b; return n; }
static AstNode* mk_return(AstNode* e){ AstNode* n=node_new(NK_RETURN); n->ret.expr=e; return n; }
static AstNode* mk_exec(char* name, AstList* args){ AstNode* n=node_new(NK_EXEC); n->exec.name=name; n->exec.args=args; return n; }
static AstNode* mk_id(char* s){ AstNode* n=node_new(NK_ID); n->id.name=s; return n; }
static AstNode* mk_index(char* s, AstNode* e){ AstNode* n=node_new(NK_INDEX); n->index.name=s; n->index.index=e; return n; }
static AstNode* mk_call(char* s, AstList* a){ AstNode* n=node_new(NK_CALL); n->call.name=s; n->call.args=a; return n; }
static AstNode* mk_bin(OpKind op, AstNode* l, AstNode* r){ AstNode* n=node_new(NK_BINOP); n->binop.op=op; n->binop.left=l; n->binop.right=r; return n; }
static AstNode* mk_un(OpKind op, AstNode* e){ AstNode* n=node_new(NK_UNOP); n->unop.op=op; n->unop.expr=e; return n; }
static AstNode* mk_lit_i(long v){ AstNode* n=node_new(NK_INT); n->lit_i.ival=v; return n; }
static AstNode* mk_lit_f(double v){ AstNode* n=node_new(NK_FLOAT); n->lit_f.fval=v; return n; }
static AstNode* mk_lit_b(int v){ AstNode* n=node_new(NK_BOOL); n->lit_b.bval=v; return n; }
static AstNode* mk_lit_c(char v){ AstNode* n=node_new(NK_CHAR); n->lit_c.cval=v; return n; }

int main(){ return yyparse(); }
