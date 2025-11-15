/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

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

#line 161 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_PROC = 3,                       /* "proc"  */
  YYSYMBOL_RETURN = 4,                     /* "return"  */
  YYSYMBOL_START = 5,                      /* "start"  */
  YYSYMBOL_END = 6,                        /* "end"  */
  YYSYMBOL_FOR = 7,                        /* "for"  */
  YYSYMBOL_WHILE = 8,                      /* "while"  */
  YYSYMBOL_IF = 9,                         /* "if"  */
  YYSYMBOL_ELSE = 10,                      /* "else"  */
  YYSYMBOL_EXEC = 11,                      /* "exec"  */
  YYSYMBOL_INTKW = 12,                     /* "int"  */
  YYSYMBOL_BOOLKW = 13,                    /* "bool"  */
  YYSYMBOL_CHARKW = 14,                    /* "char"  */
  YYSYMBOL_FLOATKW = 15,                   /* "float"  */
  YYSYMBOL_TRUE = 16,                      /* "true"  */
  YYSYMBOL_FALSE = 17,                     /* "false"  */
  YYSYMBOL_LE = 18,                        /* "<="  */
  YYSYMBOL_GE = 19,                        /* ">="  */
  YYSYMBOL_EQ = 20,                        /* "=="  */
  YYSYMBOL_NE = 21,                        /* "!="  */
  YYSYMBOL_AND_AND = 22,                   /* "&&"  */
  YYSYMBOL_OR_OR = 23,                     /* "||"  */
  YYSYMBOL_ID = 24,                        /* ID  */
  YYSYMBOL_VALOR = 25,                     /* VALOR  */
  YYSYMBOL_DECIMAL = 26,                   /* DECIMAL  */
  YYSYMBOL_LETRA = 27,                     /* LETRA  */
  YYSYMBOL_28_ = 28,                       /* '('  */
  YYSYMBOL_29_ = 29,                       /* ')'  */
  YYSYMBOL_30_ = 30,                       /* ','  */
  YYSYMBOL_31_ = 31,                       /* ';'  */
  YYSYMBOL_32_ = 32,                       /* '='  */
  YYSYMBOL_33_ = 33,                       /* '['  */
  YYSYMBOL_34_ = 34,                       /* ']'  */
  YYSYMBOL_35_ = 35,                       /* '!'  */
  YYSYMBOL_36_ = 36,                       /* '<'  */
  YYSYMBOL_37_ = 37,                       /* '>'  */
  YYSYMBOL_38_ = 38,                       /* '+'  */
  YYSYMBOL_39_ = 39,                       /* '-'  */
  YYSYMBOL_40_ = 40,                       /* '*'  */
  YYSYMBOL_41_ = 41,                       /* '/'  */
  YYSYMBOL_42_ = 42,                       /* '%'  */
  YYSYMBOL_YYACCEPT = 43,                  /* $accept  */
  YYSYMBOL_Programa = 44,                  /* Programa  */
  YYSYMBOL_Declaracion_funcion = 45,       /* Declaracion_funcion  */
  YYSYMBOL_Parametros_funcion = 46,        /* Parametros_funcion  */
  YYSYMBOL_Parametros_funcion_aux = 47,    /* Parametros_funcion_aux  */
  YYSYMBOL_Retorno_funcion = 48,           /* Retorno_funcion  */
  YYSYMBOL_Bloque = 49,                    /* Bloque  */
  YYSYMBOL_BloqueAux = 50,                 /* BloqueAux  */
  YYSYMBOL_Instruccion = 51,               /* Instruccion  */
  YYSYMBOL_Asignacion = 52,                /* Asignacion  */
  YYSYMBOL_Lista_valores = 53,             /* Lista_valores  */
  YYSYMBOL_Lista_valores_aux = 54,         /* Lista_valores_aux  */
  YYSYMBOL_Declaracion = 55,               /* Declaracion  */
  YYSYMBOL_For = 56,                       /* For  */
  YYSYMBOL_While = 57,                     /* While  */
  YYSYMBOL_If = 58,                        /* If  */
  YYSYMBOL_BloqueElseOpt = 59,             /* BloqueElseOpt  */
  YYSYMBOL_ElseOpt = 60,                   /* ElseOpt  */
  YYSYMBOL_Exec_funcion = 61,              /* Exec_funcion  */
  YYSYMBOL_Argumentos = 62,                /* Argumentos  */
  YYSYMBOL_ArgumentosAux = 63,             /* ArgumentosAux  */
  YYSYMBOL_Tipo_dato = 64,                 /* Tipo_dato  */
  YYSYMBOL_Id = 65,                        /* Id  */
  YYSYMBOL_Valor = 66,                     /* Valor  */
  YYSYMBOL_Expresion = 67,                 /* Expresion  */
  YYSYMBOL_OrExpresion = 68,               /* OrExpresion  */
  YYSYMBOL_OrExpresionAux = 69,            /* OrExpresionAux  */
  YYSYMBOL_AndExpresion = 70,              /* AndExpresion  */
  YYSYMBOL_AndExpresionAux = 71,           /* AndExpresionAux  */
  YYSYMBOL_NotExpresion = 72,              /* NotExpresion  */
  YYSYMBOL_Expresion_relacional = 73,      /* Expresion_relacional  */
  YYSYMBOL_Expresion_relacionalAux = 74,   /* Expresion_relacionalAux  */
  YYSYMBOL_Expresion_suma = 75,            /* Expresion_suma  */
  YYSYMBOL_Expresion_sumaAux = 76,         /* Expresion_sumaAux  */
  YYSYMBOL_Expresion_multiplicacion = 77,  /* Expresion_multiplicacion  */
  YYSYMBOL_TerminoAux = 78,                /* TerminoAux  */
  YYSYMBOL_Termino = 79,                   /* Termino  */
  YYSYMBOL_Factor = 80,                    /* Factor  */
  YYSYMBOL_ArrayIndexOpt = 81              /* ArrayIndexOpt  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  28
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   156

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  39
/* YYNRULES -- Number of rules.  */
#define YYNRULES  84
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  163

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   282


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    35,     2,     2,     2,    42,     2,     2,
      28,    29,    40,    38,    30,    39,     2,    41,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    31,
      36,    32,    37,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    33,     2,    34,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   136,   136,   141,   147,   151,   162,   170,   174,   182,
     186,   190,   194,   200,   204,   205,   206,   207,   208,   209,
     210,   214,   215,   220,   224,   227,   231,   235,   236,   237,
     238,   243,   248,   253,   257,   277,   278,   283,   287,   291,
     294,   298,   303,   304,   305,   306,   309,   312,   313,   314,
     315,   316,   321,   323,   324,   326,   329,   330,   332,   335,
     336,   339,   344,   345,   346,   347,   348,   349,   350,   353,
     357,   360,   363,   367,   372,   375,   378,   381,   385,   386,
     390,   391,   392,   396,   397
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "\"proc\"",
  "\"return\"", "\"start\"", "\"end\"", "\"for\"", "\"while\"", "\"if\"",
  "\"else\"", "\"exec\"", "\"int\"", "\"bool\"", "\"char\"", "\"float\"",
  "\"true\"", "\"false\"", "\"<=\"", "\">=\"", "\"==\"", "\"!=\"",
  "\"&&\"", "\"||\"", "ID", "VALOR", "DECIMAL", "LETRA", "'('", "')'",
  "','", "';'", "'='", "'['", "']'", "'!'", "'<'", "'>'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "$accept", "Programa", "Declaracion_funcion",
  "Parametros_funcion", "Parametros_funcion_aux", "Retorno_funcion",
  "Bloque", "BloqueAux", "Instruccion", "Asignacion", "Lista_valores",
  "Lista_valores_aux", "Declaracion", "For", "While", "If",
  "BloqueElseOpt", "ElseOpt", "Exec_funcion", "Argumentos",
  "ArgumentosAux", "Tipo_dato", "Id", "Valor", "Expresion", "OrExpresion",
  "OrExpresionAux", "AndExpresion", "AndExpresionAux", "NotExpresion",
  "Expresion_relacional", "Expresion_relacionalAux", "Expresion_suma",
  "Expresion_sumaAux", "Expresion_multiplicacion", "TerminoAux", "Termino",
  "Factor", "ArrayIndexOpt", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-111)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       4,   104,    84,    10,     4,     4,  -111,  -111,  -111,  -111,
     -18,    34,   -16,     7,    12,   -18,  -111,    13,    15,    84,
      18,    32,  -111,  -111,  -111,    37,   -18,     5,  -111,  -111,
    -111,    51,  -111,  -111,  -111,  -111,  -111,    34,    34,    87,
      57,  -111,  -111,  -111,    47,    43,  -111,    35,   -13,   -26,
    -111,   104,    34,    34,    92,  -111,  -111,  -111,  -111,  -111,
    -111,    -4,     6,   104,    93,  -111,  -111,    34,  -111,    34,
    -111,    34,  -111,    50,    50,    50,    50,    50,    50,  -111,
      50,    50,  -111,    50,    50,    50,  -111,    90,    94,    95,
      34,    34,   100,    34,  -111,    98,   -18,  -111,    97,    47,
      43,  -111,  -111,  -111,  -111,  -111,  -111,   -13,   -13,   -26,
     -26,   -26,    34,   123,   123,   103,    99,  -111,   101,   102,
     107,   123,   108,  -111,  -111,  -111,  -111,  -111,  -111,  -111,
    -111,   109,  -111,   124,  -111,  -111,    34,  -111,   110,  -111,
      34,  -111,  -111,   104,  -111,   -18,   123,  -111,    99,   106,
     107,   -18,   112,  -111,  -111,    34,  -111,   108,   123,   111,
    -111,  -111,  -111
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       4,     0,    13,     0,     4,     4,    42,    43,    44,    45,
       0,     0,     0,     0,     0,     0,    46,     0,     0,    13,
       0,     0,    16,    17,    18,     0,     0,     0,     1,     3,
       2,     0,    50,    51,    47,    48,    49,     0,     0,     0,
      84,    81,    10,    52,    55,    58,    60,    68,    72,    77,
      79,     0,     0,     0,     0,    20,    11,    12,    14,    15,
      19,    27,     0,     7,     0,    59,    78,     0,    80,     0,
      53,     0,    56,     0,     0,     0,     0,     0,     0,    61,
       0,     0,    69,     0,     0,     0,    73,     0,     0,     0,
      39,     0,     0,    24,    21,     0,     0,    82,     0,    55,
      58,    65,    67,    62,    63,    64,    66,    72,    72,    77,
      77,    77,     0,     0,     0,     0,    41,    28,     0,     0,
      26,     0,     9,    83,    54,    57,    70,    71,    74,    75,
      76,     0,    32,    36,    33,    37,     0,    38,    29,    22,
       0,    23,     5,     0,     6,     0,     0,    34,    41,     0,
      26,     0,     0,    35,    40,    24,    25,     9,     0,     0,
       8,    31,    30
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -111,    42,  -111,  -111,   -24,  -111,  -110,   127,  -111,     2,
      -5,     3,   105,  -111,  -111,  -111,  -111,  -111,  -111,  -111,
       0,     1,    -2,  -111,   -10,  -111,    52,    83,    54,   -33,
    -111,  -111,     9,    -7,    29,   -91,    22,   116,  -111
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     3,     4,    95,   144,    17,     5,    18,    19,    20,
     119,   141,    21,    22,    23,    24,   134,   147,    25,   115,
     137,    26,    40,    41,   120,    43,    70,    44,    72,    45,
      46,    79,    47,    82,    48,    86,    49,    50,    68
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      27,    42,    10,   132,   133,    65,    16,     1,    31,     2,
      28,   142,    51,    54,    83,    84,    85,    27,   128,   129,
     130,    56,    32,    33,    61,    80,    81,    64,    91,    92,
      16,    34,    35,    36,    37,    52,   153,    62,   100,    93,
      53,    38,    88,    89,    55,    39,    29,    30,   161,    58,
      32,    33,    94,    73,    74,    75,    76,    98,    16,    34,
      35,    36,    37,    59,    96,    71,    32,    33,    60,    38,
      69,    77,    78,    39,    16,    34,    35,    36,    37,    63,
     116,   117,   101,   102,   103,   104,   105,   106,    11,    39,
      67,    12,    13,    14,   122,    15,     6,     7,     8,     9,
     126,   127,   131,    32,    33,   109,   110,   111,    16,   107,
     108,    16,    34,    35,    36,    37,     6,     7,     8,     9,
      90,   112,    97,   113,   114,   118,   148,   121,     2,   136,
     150,   123,   135,   160,   146,   138,   139,   140,   143,   155,
     145,   158,   149,    27,   151,   162,    57,   152,   154,   157,
     159,   124,    99,   156,   125,    66,    87
};

static const yytype_uint8 yycheck[] =
{
       2,    11,     1,   113,   114,    38,    24,     3,    10,     5,
       0,   121,    28,    15,    40,    41,    42,    19,   109,   110,
     111,     6,    16,    17,    26,    38,    39,    37,    32,    33,
      24,    25,    26,    27,    28,    28,   146,    32,    71,    33,
      28,    35,    52,    53,    31,    39,     4,     5,   158,    31,
      16,    17,    62,    18,    19,    20,    21,    67,    24,    25,
      26,    27,    28,    31,    63,    22,    16,    17,    31,    35,
      23,    36,    37,    39,    24,    25,    26,    27,    28,    28,
      90,    91,    73,    74,    75,    76,    77,    78,     4,    39,
      33,     7,     8,     9,    96,    11,    12,    13,    14,    15,
     107,   108,   112,    16,    17,    83,    84,    85,    24,    80,
      81,    24,    25,    26,    27,    28,    12,    13,    14,    15,
      28,    31,    29,    29,    29,    25,   136,    29,     5,    30,
     140,    34,    29,   157,    10,    34,    34,    30,    30,    33,
      31,    29,    32,   145,   143,    34,    19,   145,   148,   151,
     155,    99,    69,   150,   100,    39,    51
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,    44,    45,    49,    12,    13,    14,    15,
      64,     4,     7,     8,     9,    11,    24,    48,    50,    51,
      52,    55,    56,    57,    58,    61,    64,    65,     0,    44,
      44,    65,    16,    17,    25,    26,    27,    28,    35,    39,
      65,    66,    67,    68,    70,    72,    73,    75,    77,    79,
      80,    28,    28,    28,    65,    31,     6,    50,    31,    31,
      31,    65,    32,    28,    67,    72,    80,    33,    81,    23,
      69,    22,    71,    18,    19,    20,    21,    36,    37,    74,
      38,    39,    76,    40,    41,    42,    78,    55,    67,    67,
      28,    32,    33,    33,    67,    46,    64,    29,    67,    70,
      72,    75,    75,    75,    75,    75,    75,    77,    77,    79,
      79,    79,    31,    29,    29,    62,    67,    67,    25,    53,
      67,    29,    65,    34,    69,    71,    76,    76,    78,    78,
      78,    67,    49,    49,    59,    29,    30,    63,    34,    34,
      30,    54,    49,    30,    47,    31,    10,    60,    67,    32,
      67,    64,    52,    49,    63,    33,    54,    65,    29,    53,
      47,    49,    34
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    43,    44,    44,    44,    45,    46,    46,    47,    47,
      48,    49,    50,    50,    51,    51,    51,    51,    51,    51,
      51,    52,    52,    53,    53,    54,    54,    55,    55,    55,
      55,    56,    57,    58,    59,    60,    60,    61,    62,    62,
      63,    63,    64,    64,    64,    64,    65,    66,    66,    66,
      66,    66,    67,    68,    69,    69,    70,    71,    71,    72,
      72,    73,    74,    74,    74,    74,    74,    74,    74,    75,
      76,    76,    76,    77,    78,    78,    78,    78,    79,    79,
      80,    80,    80,    81,    81
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     2,     0,     7,     3,     0,     4,     0,
       2,     3,     2,     0,     2,     2,     1,     1,     1,     2,
       2,     3,     5,     2,     0,     3,     0,     2,     4,     5,
       9,     9,     5,     5,     2,     2,     0,     5,     2,     0,
       3,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     3,     0,     2,     3,     0,     2,
       1,     2,     2,     2,     2,     2,     2,     2,     0,     2,
       3,     3,     0,     2,     3,     3,     3,     0,     2,     1,
       2,     1,     3,     3,     0
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* Programa: Bloque Programa  */
#line 136 "parser.y"
                            { AstList* xs = list_new(); list_push(xs, (yyvsp[-1].node)); 
                              if ((yyvsp[0].node) && (yyvsp[0].node)->k==NK_PROGRAM) { 
                                for(int i=0;i<(yyvsp[0].node)->program.items->size;i++) list_push(xs,(yyvsp[0].node)->program.items->items[i]); 
                              } 
                              (yyval.node) = mk_program(xs); }
#line 1460 "parser.tab.c"
    break;

  case 3: /* Programa: Declaracion_funcion Programa  */
#line 142 "parser.y"
                            { AstList* xs = list_new(); list_push(xs, (yyvsp[-1].node));
                              if ((yyvsp[0].node) && (yyvsp[0].node)->k==NK_PROGRAM) {
                                for(int i=0;i<(yyvsp[0].node)->program.items->size;i++) list_push(xs,(yyvsp[0].node)->program.items->items[i]);
                              }
                              (yyval.node) = mk_program(xs); }
#line 1470 "parser.tab.c"
    break;

  case 4: /* Programa: %empty  */
#line 147 "parser.y"
                            { (yyval.node) = mk_program(list_new()); }
#line 1476 "parser.tab.c"
    break;

  case 5: /* Declaracion_funcion: "proc" Tipo_dato Id '(' Parametros_funcion ')' Bloque  */
#line 152 "parser.y"
                            { (yyval.node) = (AstNode*) malloc(sizeof(AstNode));
                              (yyval.node)->k = NK_FUNC;
                              (yyval.node)->func.type = ((TypeKind)(intptr_t)(yyvsp[-5].node)); free((yyvsp[-5].node)); /* ver nota abajo */
                              (yyval.node)->func.name = (yyvsp[-4].id);
                              (yyval.node)->func.params = (yyvsp[-2].list);
                              (yyval.node)->func.body = (yyvsp[0].node); }
#line 1487 "parser.tab.c"
    break;

  case 6: /* Parametros_funcion: Tipo_dato Id Parametros_funcion_aux  */
#line 163 "parser.y"
                            { AstList* xs = list_new();
                              AstNode* p = (AstNode*) malloc(sizeof(AstNode));
                              p->k = NK_PARAM; p->param.type=(TypeKind)(intptr_t)(yyvsp[-2].node); free((yyvsp[-2].node));
                              p->param.name=(yyvsp[-1].id); p->param.isArray=0;
                              list_push(xs,p);
                              if ((yyvsp[0].list)) { for(int i=0;i<(yyvsp[0].list)->size;i++) list_push(xs,(yyvsp[0].list)->items[i]); }
                              (yyval.list) = xs; }
#line 1499 "parser.tab.c"
    break;

  case 7: /* Parametros_funcion: %empty  */
#line 170 "parser.y"
                            { (yyval.list) = list_new(); }
#line 1505 "parser.tab.c"
    break;

  case 8: /* Parametros_funcion_aux: ',' Tipo_dato Id Parametros_funcion_aux  */
#line 175 "parser.y"
                            { AstList* xs = list_new();
                              AstNode* p = (AstNode*) malloc(sizeof(AstNode));
                              p->k = NK_PARAM; p->param.type=(TypeKind)(intptr_t)(yyvsp[-2].node); free((yyvsp[-2].node));
                              p->param.name=(yyvsp[-1].id); p->param.isArray=0;
                              list_push(xs,p);
                              if ((yyvsp[0].list)) { for(int i=0;i<(yyvsp[0].list)->size;i++) list_push(xs,(yyvsp[0].list)->items[i]); }
                              (yyval.list) = xs; }
#line 1517 "parser.tab.c"
    break;

  case 9: /* Parametros_funcion_aux: %empty  */
#line 182 "parser.y"
                            { (yyval.list) = list_new(); }
#line 1523 "parser.tab.c"
    break;

  case 10: /* Retorno_funcion: "return" Expresion  */
#line 186 "parser.y"
                            { (yyval.node) = mk_return((yyvsp[0].node)); }
#line 1529 "parser.tab.c"
    break;

  case 11: /* Bloque: "start" BloqueAux "end"  */
#line 190 "parser.y"
                            { (yyval.node) = mk_block(((yyvsp[-1].node) && (yyvsp[-1].node)->k==NK_STMT_LIST)? (yyvsp[-1].node)->stmt_list.items : list_new()); }
#line 1535 "parser.tab.c"
    break;

  case 12: /* BloqueAux: Instruccion BloqueAux  */
#line 194 "parser.y"
                            { AstList* xs = list_new();
                              if ((yyvsp[-1].node)) list_push(xs,(yyvsp[-1].node));
                              if ((yyvsp[0].node) && (yyvsp[0].node)->k==NK_STMT_LIST) {
                                for(int i=0;i<(yyvsp[0].node)->stmt_list.items->size;i++) list_push(xs,(yyvsp[0].node)->stmt_list.items->items[i]);
                              }
                              (yyval.node) = mk_stmt_list(xs); }
#line 1546 "parser.tab.c"
    break;

  case 13: /* BloqueAux: %empty  */
#line 200 "parser.y"
                            { (yyval.node) = mk_stmt_list(list_new()); }
#line 1552 "parser.tab.c"
    break;

  case 14: /* Instruccion: Asignacion ';'  */
#line 204 "parser.y"
                            { (yyval.node) = (yyvsp[-1].node); }
#line 1558 "parser.tab.c"
    break;

  case 15: /* Instruccion: Declaracion ';'  */
#line 205 "parser.y"
                            { (yyval.node) = (yyvsp[-1].node); }
#line 1564 "parser.tab.c"
    break;

  case 16: /* Instruccion: For  */
#line 206 "parser.y"
                            { (yyval.node) = (yyvsp[0].node); }
#line 1570 "parser.tab.c"
    break;

  case 17: /* Instruccion: While  */
#line 207 "parser.y"
                            { (yyval.node) = (yyvsp[0].node); }
#line 1576 "parser.tab.c"
    break;

  case 18: /* Instruccion: If  */
#line 208 "parser.y"
                            { (yyval.node) = (yyvsp[0].node); }
#line 1582 "parser.tab.c"
    break;

  case 19: /* Instruccion: Exec_funcion ';'  */
#line 209 "parser.y"
                            { (yyval.node) = (yyvsp[-1].node); }
#line 1588 "parser.tab.c"
    break;

  case 20: /* Instruccion: Retorno_funcion ';'  */
#line 210 "parser.y"
                            { (yyval.node) = (yyvsp[-1].node); }
#line 1594 "parser.tab.c"
    break;

  case 21: /* Asignacion: Id '=' Expresion  */
#line 214 "parser.y"
                            { (yyval.node) = mk_assign((yyvsp[-2].id), (yyvsp[0].node), NULL, 0, NULL); }
#line 1600 "parser.tab.c"
    break;

  case 22: /* Asignacion: Id '=' '[' Lista_valores ']'  */
#line 216 "parser.y"
                            { (yyval.node) = mk_assign((yyvsp[-4].id), NULL, (yyvsp[-1].list), 1, NULL); }
#line 1606 "parser.tab.c"
    break;

  case 23: /* Lista_valores: Expresion Lista_valores_aux  */
#line 221 "parser.y"
                            { AstList* xs=list_new(); list_push(xs,(yyvsp[-1].node));
                              if ((yyvsp[0].list)) for(int i=0;i<(yyvsp[0].list)->size;i++) list_push(xs,(yyvsp[0].list)->items[i]);
                              (yyval.list) = xs; }
#line 1614 "parser.tab.c"
    break;

  case 24: /* Lista_valores: %empty  */
#line 224 "parser.y"
                            { (yyval.list) = list_new(); }
#line 1620 "parser.tab.c"
    break;

  case 25: /* Lista_valores_aux: ',' Expresion Lista_valores_aux  */
#line 228 "parser.y"
                            { AstList* xs=list_new(); list_push(xs,(yyvsp[-1].node));
                              if ((yyvsp[0].list)) for(int i=0;i<(yyvsp[0].list)->size;i++) list_push(xs,(yyvsp[0].list)->items[i]);
                              (yyval.list) = xs; }
#line 1628 "parser.tab.c"
    break;

  case 26: /* Lista_valores_aux: %empty  */
#line 231 "parser.y"
                            { (yyval.list) = list_new(); }
#line 1634 "parser.tab.c"
    break;

  case 27: /* Declaracion: Tipo_dato Id  */
#line 235 "parser.y"
                                         { (yyval.node) = mk_vardecl((TypeKind)(intptr_t)(yyvsp[-1].node),(yyvsp[0].id),0,0,NULL,NULL); free((yyvsp[-1].node)); }
#line 1640 "parser.tab.c"
    break;

  case 28: /* Declaracion: Tipo_dato Id '=' Expresion  */
#line 236 "parser.y"
                                         { (yyval.node) = mk_vardecl((TypeKind)(intptr_t)(yyvsp[-3].node),(yyvsp[-2].id),0,0,NULL,(yyvsp[0].node)); free((yyvsp[-3].node)); }
#line 1646 "parser.tab.c"
    break;

  case 29: /* Declaracion: Tipo_dato Id '[' VALOR ']'  */
#line 237 "parser.y"
                                         { (yyval.node) = mk_vardecl((TypeKind)(intptr_t)(yyvsp[-4].node),(yyvsp[-3].id),1,(yyvsp[-1].ival),NULL,NULL); free((yyvsp[-4].node)); }
#line 1652 "parser.tab.c"
    break;

  case 30: /* Declaracion: Tipo_dato Id '[' VALOR ']' '=' '[' Lista_valores ']'  */
#line 239 "parser.y"
                                         { (yyval.node) = mk_vardecl((TypeKind)(intptr_t)(yyvsp[-8].node),(yyvsp[-7].id),1,(yyvsp[-5].ival),(yyvsp[-1].list),NULL); free((yyvsp[-8].node)); }
#line 1658 "parser.tab.c"
    break;

  case 31: /* For: "for" '(' Declaracion ';' Expresion ';' Asignacion ')' Bloque  */
#line 244 "parser.y"
                            { (yyval.node) = mk_for((yyvsp[-6].node),(yyvsp[-4].node),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1664 "parser.tab.c"
    break;

  case 32: /* While: "while" '(' Expresion ')' Bloque  */
#line 249 "parser.y"
                            { (yyval.node) = mk_while((yyvsp[-2].node),(yyvsp[0].node)); }
#line 1670 "parser.tab.c"
    break;

  case 33: /* If: "if" '(' Expresion ')' BloqueElseOpt  */
#line 253 "parser.y"
                                         { (yyval.node) = (yyvsp[0].node); /* BloqueElseOpt ya empaqueta THEN/ELSE */ }
#line 1676 "parser.tab.c"
    break;

  case 34: /* BloqueElseOpt: Bloque ElseOpt  */
#line 257 "parser.y"
                             { /* ElseOpt produce else-bloque o vacío; empaquetamos si/no else */
                               if ((yyvsp[0].node)) { /* con else */
                                 AstNode* n=(AstNode*)malloc(sizeof(AstNode));
                                 n->k = NK_IF;
                                 n->ifs.cond = NULL; /* set más arriba */
                                 /* truco: guardamos el then en n->ifs.thenBlk y el else en n->ifs.elseBlk,
                                    luego el IF padre rellena cond */
                                 n->ifs.thenBlk = (yyvsp[-1].node);
                                 n->ifs.elseBlk = (yyvsp[0].node);
                                 (yyval.node) = n;
                               } else {
                                 AstNode* n=(AstNode*)malloc(sizeof(AstNode));
                                 n->k = NK_IF;
                                 n->ifs.thenBlk = (yyvsp[-1].node);
                                 n->ifs.elseBlk = NULL;
                                 (yyval.node) = n;
                               } }
#line 1698 "parser.tab.c"
    break;

  case 35: /* ElseOpt: "else" Bloque  */
#line 277 "parser.y"
                             { (yyval.node) = (yyvsp[0].node); }
#line 1704 "parser.tab.c"
    break;

  case 36: /* ElseOpt: %empty  */
#line 278 "parser.y"
                             { (yyval.node) = NULL; }
#line 1710 "parser.tab.c"
    break;

  case 37: /* Exec_funcion: "exec" Id '(' Argumentos ')'  */
#line 283 "parser.y"
                                { (yyval.node) = mk_exec((yyvsp[-3].id), (yyvsp[-1].list)); }
#line 1716 "parser.tab.c"
    break;

  case 38: /* Argumentos: Expresion ArgumentosAux  */
#line 288 "parser.y"
                            { AstList* xs=list_new(); list_push(xs,(yyvsp[-1].node));
                              if ((yyvsp[0].list)) for(int i=0;i<(yyvsp[0].list)->size;i++) list_push(xs,(yyvsp[0].list)->items[i]);
                              (yyval.list) = xs; }
#line 1724 "parser.tab.c"
    break;

  case 39: /* Argumentos: %empty  */
#line 291 "parser.y"
                            { (yyval.list) = list_new(); }
#line 1730 "parser.tab.c"
    break;

  case 40: /* ArgumentosAux: ',' Expresion ArgumentosAux  */
#line 295 "parser.y"
                            { AstList* xs=list_new(); list_push(xs,(yyvsp[-1].node));
                              if ((yyvsp[0].list)) for(int i=0;i<(yyvsp[0].list)->size;i++) list_push(xs,(yyvsp[0].list)->items[i]);
                              (yyval.list) = xs; }
#line 1738 "parser.tab.c"
    break;

  case 41: /* ArgumentosAux: %empty  */
#line 298 "parser.y"
                            { (yyval.list) = list_new(); }
#line 1744 "parser.tab.c"
    break;

  case 42: /* Tipo_dato: "int"  */
#line 303 "parser.y"
              { (yyval.node) = (AstNode*)(intptr_t)TY_INT; }
#line 1750 "parser.tab.c"
    break;

  case 43: /* Tipo_dato: "bool"  */
#line 304 "parser.y"
              { (yyval.node) = (AstNode*)(intptr_t)TY_BOOL; }
#line 1756 "parser.tab.c"
    break;

  case 44: /* Tipo_dato: "char"  */
#line 305 "parser.y"
              { (yyval.node) = (AstNode*)(intptr_t)TY_CHAR; }
#line 1762 "parser.tab.c"
    break;

  case 45: /* Tipo_dato: "float"  */
#line 306 "parser.y"
              { (yyval.node) = (AstNode*)(intptr_t)TY_FLOAT; }
#line 1768 "parser.tab.c"
    break;

  case 46: /* Id: ID  */
#line 309 "parser.y"
        { (yyval.id) = (yyvsp[0].id); }
#line 1774 "parser.tab.c"
    break;

  case 47: /* Valor: VALOR  */
#line 312 "parser.y"
              { (yyval.node) = mk_lit_i((yyvsp[0].ival)); }
#line 1780 "parser.tab.c"
    break;

  case 48: /* Valor: DECIMAL  */
#line 313 "parser.y"
              { (yyval.node) = mk_lit_f((yyvsp[0].fval)); }
#line 1786 "parser.tab.c"
    break;

  case 49: /* Valor: LETRA  */
#line 314 "parser.y"
              { (yyval.node) = mk_lit_c((yyvsp[0].cval)); }
#line 1792 "parser.tab.c"
    break;

  case 50: /* Valor: "true"  */
#line 315 "parser.y"
              { (yyval.node) = mk_lit_b(1); }
#line 1798 "parser.tab.c"
    break;

  case 51: /* Valor: "false"  */
#line 316 "parser.y"
              { (yyval.node) = mk_lit_b(0); }
#line 1804 "parser.tab.c"
    break;

  case 52: /* Expresion: OrExpresion  */
#line 321 "parser.y"
                                                      { (yyval.node) = (yyvsp[0].node); }
#line 1810 "parser.tab.c"
    break;

  case 53: /* OrExpresion: AndExpresion OrExpresionAux  */
#line 323 "parser.y"
                                                      { (yyval.node) = (yyvsp[0].node) ? mk_bin(OP_OR,(yyvsp[-1].node),(yyvsp[0].node)) : (yyvsp[-1].node); }
#line 1816 "parser.tab.c"
    break;

  case 54: /* OrExpresionAux: "||" AndExpresion OrExpresionAux  */
#line 325 "parser.y"
                                                { (yyval.node) = (yyvsp[0].node) ? mk_bin(OP_OR,(yyvsp[-1].node),(yyvsp[0].node)) : (yyvsp[-1].node); }
#line 1822 "parser.tab.c"
    break;

  case 55: /* OrExpresionAux: %empty  */
#line 326 "parser.y"
                                                      { (yyval.node) = NULL; }
#line 1828 "parser.tab.c"
    break;

  case 56: /* AndExpresion: NotExpresion AndExpresionAux  */
#line 329 "parser.y"
                                                      { (yyval.node) = (yyvsp[0].node) ? mk_bin(OP_AND,(yyvsp[-1].node),(yyvsp[0].node)) : (yyvsp[-1].node); }
#line 1834 "parser.tab.c"
    break;

  case 57: /* AndExpresionAux: "&&" NotExpresion AndExpresionAux  */
#line 331 "parser.y"
                                                { (yyval.node) = (yyvsp[0].node) ? mk_bin(OP_AND,(yyvsp[-1].node),(yyvsp[0].node)) : (yyvsp[-1].node); }
#line 1840 "parser.tab.c"
    break;

  case 58: /* AndExpresionAux: %empty  */
#line 332 "parser.y"
                                                      { (yyval.node) = NULL; }
#line 1846 "parser.tab.c"
    break;

  case 59: /* NotExpresion: '!' NotExpresion  */
#line 335 "parser.y"
                                                      { (yyval.node) = mk_un(OP_NOT,(yyvsp[0].node)); }
#line 1852 "parser.tab.c"
    break;

  case 60: /* NotExpresion: Expresion_relacional  */
#line 336 "parser.y"
                                                      { (yyval.node) = (yyvsp[0].node); }
#line 1858 "parser.tab.c"
    break;

  case 61: /* Expresion_relacional: Expresion_suma Expresion_relacionalAux  */
#line 340 "parser.y"
                     { (yyval.node) = (yyvsp[0].node) ? mk_bin((OpKind)(intptr_t)(yyvsp[0].node),(yyvsp[-1].node), ((AstNode*)(yyvsp[0].node))->binop.right) : (yyvsp[-1].node); }
#line 1864 "parser.tab.c"
    break;

  case 62: /* Expresion_relacionalAux: "==" Expresion_suma  */
#line 344 "parser.y"
                         { AstNode* n=mk_bin(OP_EQ, NULL, (yyvsp[0].node)); (yyval.node)=(AstNode*)(intptr_t)OP_EQ; n->binop.right=(yyvsp[0].node); (yyval.node)=n; }
#line 1870 "parser.tab.c"
    break;

  case 63: /* Expresion_relacionalAux: "!=" Expresion_suma  */
#line 345 "parser.y"
                         { AstNode* n=mk_bin(OP_NE, NULL, (yyvsp[0].node)); (yyval.node)=n; }
#line 1876 "parser.tab.c"
    break;

  case 64: /* Expresion_relacionalAux: '<' Expresion_suma  */
#line 346 "parser.y"
                         { AstNode* n=mk_bin(OP_LT, NULL, (yyvsp[0].node)); (yyval.node)=n; }
#line 1882 "parser.tab.c"
    break;

  case 65: /* Expresion_relacionalAux: "<=" Expresion_suma  */
#line 347 "parser.y"
                         { AstNode* n=mk_bin(OP_LE, NULL, (yyvsp[0].node)); (yyval.node)=n; }
#line 1888 "parser.tab.c"
    break;

  case 66: /* Expresion_relacionalAux: '>' Expresion_suma  */
#line 348 "parser.y"
                         { AstNode* n=mk_bin(OP_GT, NULL, (yyvsp[0].node)); (yyval.node)=n; }
#line 1894 "parser.tab.c"
    break;

  case 67: /* Expresion_relacionalAux: ">=" Expresion_suma  */
#line 349 "parser.y"
                         { AstNode* n=mk_bin(OP_GE, NULL, (yyvsp[0].node)); (yyval.node)=n; }
#line 1900 "parser.tab.c"
    break;

  case 68: /* Expresion_relacionalAux: %empty  */
#line 350 "parser.y"
                         { (yyval.node) = NULL; }
#line 1906 "parser.tab.c"
    break;

  case 69: /* Expresion_suma: Expresion_multiplicacion Expresion_sumaAux  */
#line 354 "parser.y"
                     { (yyval.node) = (yyvsp[0].node) ? mk_bin(((AstNode*)(yyvsp[0].node))->binop.op,(yyvsp[-1].node),((AstNode*)(yyvsp[0].node))->binop.right) : (yyvsp[-1].node); }
#line 1912 "parser.tab.c"
    break;

  case 70: /* Expresion_sumaAux: '+' Expresion_multiplicacion Expresion_sumaAux  */
#line 358 "parser.y"
                     { AstNode* t = (yyvsp[0].node) ? mk_bin(((AstNode*)(yyvsp[0].node))->binop.op,(yyvsp[-1].node),((AstNode*)(yyvsp[0].node))->binop.right):(yyvsp[-1].node);
                       (yyval.node) = mk_bin(OP_ADD, t, NULL); }
#line 1919 "parser.tab.c"
    break;

  case 71: /* Expresion_sumaAux: '-' Expresion_multiplicacion Expresion_sumaAux  */
#line 361 "parser.y"
                     { AstNode* t = (yyvsp[0].node) ? mk_bin(((AstNode*)(yyvsp[0].node))->binop.op,(yyvsp[-1].node),((AstNode*)(yyvsp[0].node))->binop.right):(yyvsp[-1].node);
                       (yyval.node) = mk_bin(OP_SUB, t, NULL); }
#line 1926 "parser.tab.c"
    break;

  case 72: /* Expresion_sumaAux: %empty  */
#line 363 "parser.y"
                     { (yyval.node) = NULL; }
#line 1932 "parser.tab.c"
    break;

  case 73: /* Expresion_multiplicacion: Termino TerminoAux  */
#line 368 "parser.y"
  { (yyval.node) = (yyvsp[0].node) ? mk_bin(((AstNode*)(yyvsp[0].node))->binop.op,(yyvsp[-1].node),((AstNode*)(yyvsp[0].node))->binop.right) : (yyvsp[-1].node); }
#line 1938 "parser.tab.c"
    break;

  case 74: /* TerminoAux: '*' Termino TerminoAux  */
#line 373 "parser.y"
                     { AstNode* t=(yyvsp[0].node) ? mk_bin(((AstNode*)(yyvsp[0].node))->binop.op,(yyvsp[-1].node),((AstNode*)(yyvsp[0].node))->binop.right):(yyvsp[-1].node);
                       (yyval.node) = mk_bin(OP_MUL, t, NULL); }
#line 1945 "parser.tab.c"
    break;

  case 75: /* TerminoAux: '/' Termino TerminoAux  */
#line 376 "parser.y"
                     { AstNode* t=(yyvsp[0].node) ? mk_bin(((AstNode*)(yyvsp[0].node))->binop.op,(yyvsp[-1].node),((AstNode*)(yyvsp[0].node))->binop.right):(yyvsp[-1].node);
                       (yyval.node) = mk_bin(OP_DIV, t, NULL); }
#line 1952 "parser.tab.c"
    break;

  case 76: /* TerminoAux: '%' Termino TerminoAux  */
#line 379 "parser.y"
                     { AstNode* t=(yyvsp[0].node) ? mk_bin(((AstNode*)(yyvsp[0].node))->binop.op,(yyvsp[-1].node),((AstNode*)(yyvsp[0].node))->binop.right):(yyvsp[-1].node);
                       (yyval.node) = mk_bin(OP_MOD, t, NULL); }
#line 1959 "parser.tab.c"
    break;

  case 77: /* TerminoAux: %empty  */
#line 381 "parser.y"
                     { (yyval.node) = NULL; }
#line 1965 "parser.tab.c"
    break;

  case 78: /* Termino: '-' Factor  */
#line 385 "parser.y"
                      { (yyval.node) = mk_un(OP_NEG, (yyvsp[0].node)); }
#line 1971 "parser.tab.c"
    break;

  case 79: /* Termino: Factor  */
#line 386 "parser.y"
                      { (yyval.node) = (yyvsp[0].node); }
#line 1977 "parser.tab.c"
    break;

  case 80: /* Factor: Id ArrayIndexOpt  */
#line 390 "parser.y"
                      { (yyval.node) = (yyvsp[0].node) ? mk_index((yyvsp[-1].id),(yyvsp[0].node)) : mk_id((yyvsp[-1].id)); }
#line 1983 "parser.tab.c"
    break;

  case 81: /* Factor: Valor  */
#line 391 "parser.y"
                      { (yyval.node) = (yyvsp[0].node); }
#line 1989 "parser.tab.c"
    break;

  case 82: /* Factor: '(' Expresion ')'  */
#line 392 "parser.y"
                      { (yyval.node) = (yyvsp[-1].node); }
#line 1995 "parser.tab.c"
    break;

  case 83: /* ArrayIndexOpt: '[' Expresion ']'  */
#line 396 "parser.y"
                      { (yyval.node) = (yyvsp[-1].node); }
#line 2001 "parser.tab.c"
    break;

  case 84: /* ArrayIndexOpt: %empty  */
#line 397 "parser.y"
                      { (yyval.node) = NULL; }
#line 2007 "parser.tab.c"
    break;


#line 2011 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 400 "parser.y"
  /* ===================== Helpers AST (implementación simple) ===================== */

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
