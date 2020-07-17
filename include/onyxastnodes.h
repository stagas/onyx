#ifndef ONYXASTNODES_H
#define ONYXASTNODES_H

#include "onyxlex.h"
#include "onyxtypes.h"

typedef struct AstUnaryOp AstUnaryOp;
typedef struct AstBinOp AstBinaryOp;
typedef struct AstAssign AstAssign;
typedef struct AstNumLit AstNumLit;
typedef struct AstLocal AstLocal;
typedef struct AstReturn AstReturn;
typedef struct AstCall AstCall;
typedef struct AstIntrinsicCall AstIntrinsicCall;
typedef struct AstArgument AstArgument;

typedef struct AstBlock AstBlock;
typedef struct AstIf AstIf;
typedef struct AstWhile AstWhile;
typedef struct AstLocalGroup AstLocalGroup;

typedef struct AstBasicType AstBasicType;
typedef struct AstPointerType AstPointerType;
typedef struct AstFunctionType AstFunctionType;

typedef struct AstBinding AstBinding;
typedef struct AstFunction AstFunction;
typedef struct AstForeign AstForeign;
typedef struct AstGlobal AstGlobal;
typedef struct AstUse AstUse;

typedef enum AstKind {
    Ast_Kind_Error,
    Ast_Kind_Program,
    Ast_Kind_Use,

    Ast_Kind_Binding,
    Ast_Kind_Function,
    Ast_Kind_Foreign,
    Ast_Kind_Block,
    Ast_Kind_Local_Group,
    Ast_Kind_Local,
    Ast_Kind_Global,
    Ast_Kind_Symbol,

    Ast_Kind_Unary_Op,
    Ast_Kind_Binary_Op,

    Ast_Kind_Type_Start,
    Ast_Kind_Type,
    Ast_Kind_Basic_Type,
    Ast_Kind_Pointer_Type,
    Ast_Kind_Function_Type,
    Ast_Kind_Type_End,

    Ast_Kind_Literal,
    Ast_Kind_Param,
    Ast_Kind_Argument,
    Ast_Kind_Call,
    Ast_Kind_Intrinsic_Call,
    Ast_Kind_Assignment,
    Ast_Kind_Return,

    Ast_Kind_If,
    Ast_Kind_While,
    Ast_Kind_Break,
    Ast_Kind_Continue,

    Ast_Kind_Count
} AstKind;

// NOTE: Some of these flags will overlap since there are
// only 32-bits of flags to play with
typedef enum AstFlags {
    // Top-level flags
    Ast_Flag_Exported        = BH_BIT(0),
    Ast_Flag_Lval            = BH_BIT(1),
    Ast_Flag_Const           = BH_BIT(2),
    Ast_Flag_Comptime        = BH_BIT(3),

    // Function flags
    Ast_Flag_Inline          = BH_BIT(8),
    Ast_Flag_Intrinsic       = BH_BIT(9),
    Ast_Flag_Foreign         = BH_BIT(10),
} AstFlags;

typedef enum UnaryOp {
    Unary_Op_Negate,
    Unary_Op_Not,
    Unary_Op_Cast,
} UnaryOp;

typedef enum BinaryOp {
    Binary_Op_Add           = 0,
    Binary_Op_Minus         = 1,
    Binary_Op_Multiply      = 2,
    Binary_Op_Divide        = 3,
    Binary_Op_Modulus       = 4,

    Binary_Op_Equal         = 5,
    Binary_Op_Not_Equal     = 6,
    Binary_Op_Less          = 7,
    Binary_Op_Less_Equal    = 8,
    Binary_Op_Greater       = 9,
    Binary_Op_Greater_Equal = 10,
} BinaryOp;


// Base Nodes

// NOTE: AstNode and AstTyped need to be EXACTLY the same for all
// arguments existing in AstNode. I do this to avoid a nested
// "inheiritance" where you would have to say node.base.base.next
// for example

#define AstNode_members {     \
    AstKind kind;             \
    u32 flags;                \
    OnyxToken *token;         \
    AstNode *next;            \
};
#define AstNode_base struct AstNode_members;
struct AstNode AstNode_members;

// NOTE: 'type_node' is filled out by the parser.                                               \
// For a type such as '^^i32', the tree would look something like
//
//      Typed Thing -> AstPointerType -> AstPointerType -> AstNode (symbol node)
//
// The symbol node will be filled out during symbol resolution.
// It will end up pointing to an AstBasicType that corresponds to
// the underlying type.
//
// 'type' is filled out afterwards. If it is NULL, the Type* is built
// using the type_node. This can then be used to typecheck this node.
#define AstTyped_members     { \
    AstKind kind;              \
    u32 flags;                 \
    OnyxToken *token;          \
    AstNode *next;             \
    AstType *type_node;        \
    Type *type;                \
}
#define AstTyped_base struct AstTyped_members;
struct AstTyped AstTyped_members;

// Expression Nodes
struct AstBinOp         { AstTyped_base; BinaryOp operation; AstTyped *left, *right; };
struct AstUnaryOp       { AstTyped_base; UnaryOp operation; AstTyped *expr; };
struct AstAssign        { AstNode_base;  AstTyped* lval; AstTyped* expr; };
struct AstNumLit        { AstTyped_base; union { i32 i; i64 l; f32 f; f64 d; } value; };
struct AstLocal         { AstTyped_base; AstLocal *prev_local; };
struct AstReturn        { AstNode_base;  AstTyped* expr; };
struct AstCall          { AstTyped_base; AstArgument *arguments; AstNode *callee; };
struct AstArgument      { AstTyped_base; AstTyped *value; };

// Structure Nodes
struct AstLocalGroup    { AstNode_base;  AstLocalGroup *prev_group; AstLocal *last_local; };
struct AstBlock         { AstNode_base;  AstNode *body; AstLocalGroup *locals; };
struct AstWhile         { AstNode_base;  AstTyped *cond; AstBlock *body; };
struct AstIf {
    AstNode_base;
    AstTyped *cond;

    union {
        AstIf *as_if;
        AstBlock* as_block;
    } true_block, false_block;
};

// Type Nodes
// NOTE: This node is very similar to an AstNode, just
// without the 'next' member. This is because types
// can't be in expressions so a 'next' thing
// doesn't make sense.
#define AstType_members { AstKind kind; u32 flags; char* name; }
#define AstType_base struct AstType_members;
struct AstType AstType_members;

struct AstBasicType     { AstType_base; Type* type; };
struct AstPointerType   { AstType_base; AstType* elem; };
struct AstFunctionType  { AstType_base; AstType* return_type; u64 param_count; AstType* params[]; };

// Top level nodes
struct AstBinding       { AstTyped_base; AstNode* node; };
struct AstForeign       { AstNode_base;  OnyxToken *mod_token, *name_token; AstNode *import; };
struct AstUse           { AstNode_base;  OnyxToken *filename; };
struct AstGlobal        {
    AstTyped_base;

    union {
        // NOTE: Used when a global is exported with a specific name
        OnyxToken* exported_name;

        // NOTE: Used when the global is declared as foreign
        struct {
            OnyxToken* foreign_module;
            OnyxToken* foreign_name;
        };
    };
};
struct AstFunction      {
    AstTyped_base;

    AstBlock *body;
    AstLocal *params;

    union {
        // NOTE: Used when a function is exported with a specific name
        OnyxToken* exported_name;
        OnyxToken* intrinsic_name;

        // NOTE: Used when the function is declared as foreign
        struct {
            OnyxToken* foreign_module;
            OnyxToken* foreign_name;
        };
    };
};

typedef enum OnyxIntrinsic {
    ONYX_INTRINSIC_UNDEFINED,

    ONYX_INTRINSIC_MEMORY_SIZE, ONYX_INTRINSIC_MEMORY_GROW,

    ONYX_INTRINSIC_I32_CLZ,   ONYX_INTRINSIC_I32_CTZ, ONYX_INTRINSIC_I32_POPCNT,
    ONYX_INTRINSIC_I32_AND,   ONYX_INTRINSIC_I32_OR,  ONYX_INTRINSIC_I32_XOR,
    ONYX_INTRINSIC_I32_SHL,   ONYX_INTRINSIC_I32_SLR, ONYX_INTRINSIC_I32_SAR,
    ONYX_INTRINSIC_I32_ROTL,  ONYX_INTRINSIC_I32_ROTR,

    ONYX_INTRINSIC_I64_CLZ,   ONYX_INTRINSIC_I64_CTZ, ONYX_INTRINSIC_I64_POPCNT,
    ONYX_INTRINSIC_I64_AND,   ONYX_INTRINSIC_I64_OR,  ONYX_INTRINSIC_I64_XOR,
    ONYX_INTRINSIC_I64_SHL,   ONYX_INTRINSIC_I64_SLR, ONYX_INTRINSIC_I64_SAR,
    ONYX_INTRINSIC_I64_ROTL,  ONYX_INTRINSIC_I64_ROTR,

    ONYX_INTRINSIC_F32_ABS,   ONYX_INTRINSIC_F32_SQRT,
    ONYX_INTRINSIC_F32_CEIL,  ONYX_INTRINSIC_F32_FLOOR,
    ONYX_INTRINSIC_F32_TRUNC, ONYX_INTRINSIC_F32_NEAREST,
    ONYX_INTRINSIC_F32_MIN,   ONYX_INTRINSIC_F32_MAX,
    ONYX_INTRINSIC_F32_COPYSIGN,

    ONYX_INTRINSIC_F64_ABS,   ONYX_INTRINSIC_F64_SQRT,
    ONYX_INTRINSIC_F64_CEIL,  ONYX_INTRINSIC_F64_FLOOR,
    ONYX_INTRINSIC_F64_TRUNC, ONYX_INTRINSIC_F64_NEAREST,
    ONYX_INTRINSIC_F64_MIN,   ONYX_INTRINSIC_F64_MAX,
    ONYX_INTRINSIC_F64_COPYSIGN,
} OnyxIntrinsic;

// NOTE: This needs to have 'arguments' in the
// same position as AstNodeCall
struct AstIntrinsicCall { AstTyped_base; AstArgument *arguments; OnyxIntrinsic intrinsic; };

// NOTE: Simple data structure for storing what comes out of the parser
typedef struct ParserOutput {
    bh_arr(AstBinding *)  top_level_bindings;
    bh_arr(AstNode *)     nodes_to_process;

    bh_arr(AstFunction *) functions;
    bh_arr(AstGlobal *)   globals;
} ParserOutput;





// NOTE: Basic internal types constructed in the parser
extern AstBasicType basic_type_void;
extern AstBasicType basic_type_bool;
extern AstBasicType basic_type_i8;
extern AstBasicType basic_type_u8;
extern AstBasicType basic_type_i16;
extern AstBasicType basic_type_u16;
extern AstBasicType basic_type_i32;
extern AstBasicType basic_type_u32;
extern AstBasicType basic_type_i64;
extern AstBasicType basic_type_u64;
extern AstBasicType basic_type_f32;
extern AstBasicType basic_type_f64;
extern AstBasicType basic_type_rawptr;

#endif // #ifndef ONYXASTNODES_H
