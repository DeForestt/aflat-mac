#ifndef GEN
#define GEN

#include "LinkedList.hpp"
#include "LinkedListS.hpp"
#include "ASM.hpp"
#include "AST.hpp"
#include "CodeGenerator/ScopeManager.hpp"
namespace gen{

    class Expr{
        public:
        asmc::OpType op = asmc::Hard;
        std::string access;
        asmc::Size size;
    };
    
    class Symbol{
        public:
        std::string symbol;
        ast::Type type;
        bool mask;
        int byteMod;
    };

    class Type{
        public:
        std::string Ident;
        
        // The public Symbol table will hold all public symbols
        // This is to be used externally
        links::LinkedList<Symbol> publicSymbols;

        // The private Symbol table will hold all public and private symbols
        // This is used internally
        links::LinkedList<Symbol> SymbolTable;
        static bool compair(Type * t, std::string ident);
        virtual int poly(){return 0;};
        int size;
    };

    class Class : public Type{
        public:
            // The public Name table will hold all public functions
            // This is to be used externally
            links::SLinkedList<ast::Function, std::string> publicNameTable;
            // The private Name table will hold all public and private functions
            // This is used internally
            links::SLinkedList<ast::Function, std::string> nameTable;
            static bool compair(Type * t, std::string ident);
    };

    class CodeGenerator{
        private:

        #pragma region State Variables
            gen::Class * scope;
            ast::Type returnType;
            int scopePop = 0;
            int lablecount = 0; 
            int intArgsCounter;
            int selectReg = 0;
            bool globalScope = true;
            bool inFunction = false;
        #pragma endregion

        #pragma region Item Lists
            links::SLinkedList<gen::Type *, std::string> typeList;
            links::SLinkedList<asmc::Register, std::string> registers;
            links::LinkedList<Symbol> SymbolTable;
            links::LinkedList<Symbol> GlobalSymbolTable;
            links::SLinkedList<ast::Function, std::string> nameTable;
            links::SLinkedList<ast::Type, std::string> TypeList;
        #pragma endregion

        int getBytes(asmc::Size size);
        
        asmc::Register intArgs[6] = { asmc::Register("rdi", "edi", "di", "dil"),
                                    asmc::Register("rsi", "esi", "si", "sil"),
                                    asmc::Register("rdx", "edx", "dx", "dl"),
                                    asmc::Register("rcx", "ecx", "cx", "cl"),
                                    asmc::Register("r8", "r8d", "r8w", "r8b"),
                                    asmc::Register("r9", "r9d", "r9w", "r9b")};

        void GenArgs(ast::Statment * STMT, asmc::File &OutputFile);
        ast::Function GenCall(ast::Call * call, asmc::File &OutputFile);
        Expr GenExpr(ast::Expr * expr, asmc::File &OutputFile);
        void prepareCompound(ast::Compound compound, asmc::File &OutputFile, bool isDiv = false);

        public:
        asmc::File GenSTMT(ast::Statment * stmt);
        links::LinkedList<gen::Symbol> GenTable(ast::Statment * STMT, links::LinkedList<gen::Symbol> &table);
        CodeGenerator();
    };
}

#endif