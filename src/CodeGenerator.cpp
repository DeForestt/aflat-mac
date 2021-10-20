#include "CodeGenerator.hpp"
#include "Exceptions.hpp"

bool searchSymbol(gen::Symbol sym, std::string str){
    if (sym.symbol == str) return true; else return false;
}

bool compairFunc(AST::Function F, std::string input){
    if (input == F.ident.ident){
        return true;
    }
        return false;
}

bool gen::Type::compair(gen::Type *  t, std::string ident){
    if (ident == t->Ident) return true;
    return false;
}

int gen::CodeGenerator::getBytes(ASMC::Size size){
    switch(size){
            case ASMC::QWord:
                return 8;
                break;
            case ASMC::Word:
                return 2;
                break;
            case ASMC::Byte:
                return 1;
                break;
            case ASMC::DWord:
                return 4;
                break;
            default:
                throw err::Exception("Unknown Size");
        }
}

gen::CodeGenerator::CodeGenerator(){
    this->registers << ASMC::Register("rax", "eax", "ax", "al");
    this->registers << ASMC::Register("rcx", "ecx", "cx", "cl");
    this->registers << ASMC::Register("rdx", "edx", "dx", "dl");
    this->registers << ASMC::Register("rbx", "ebx", "bx", "bl");
    this->registers << ASMC::Register("rsi", "esi", "si", "sil");
    this->registers << ASMC::Register("rdi", "edi", "di", "dil");   
    this->registers << ASMC::Register("rsp", "esp", "sp", "spl");
    this->registers << ASMC::Register("rbp", "ebp", "bp", "bpl");
    this->registers.foo = ASMC::Register::compair;
    this->nameTable.foo = compairFunc;
    this->typeList.foo = gen::Type::compair;
    this->scope = nullptr;
}

gen::Expr gen::CodeGenerator::GenExpr(AST::Expr * expr, ASMC::File &OutputFile){
    gen::Expr output;
    if(dynamic_cast<AST::IntLiteral *>(expr) != nullptr){
        AST::IntLiteral * intlit = dynamic_cast<AST::IntLiteral *>(expr);
        output.access = '$' + std::to_string(intlit->val);
        output.size = ASMC::DWord;
    } else if(dynamic_cast<AST::LongLiteral *>(expr) != nullptr){
        AST::LongLiteral * intlit = dynamic_cast<AST::LongLiteral *>(expr);
        output.access = '$' + std::to_string(intlit->val);
        output.size = ASMC::QWord;
    }
    else if(dynamic_cast<AST::CharLiteral *>(expr) != nullptr){
        AST::CharLiteral * intlit = dynamic_cast<AST::CharLiteral *>(expr);
        output.access = '$' + std::to_string(intlit->value);
        output.size = ASMC::Byte;
    }else if(dynamic_cast<AST::CallExpr *>(expr) != nullptr){
        AST::CallExpr * exprCall = dynamic_cast<AST::CallExpr *>(expr);
        AST::Call * call = exprCall->call;
        std::string errhold = call ->ident;
        std::string mod = "";

        AST::Function * func;

        if(call->modList.head == nullptr) func = this->nameTable[call->ident];
        else{

            gen::Symbol * sym = this->SymbolTable.search<std::string>(searchSymbol, call->ident);
            if (sym == nullptr) throw err::Exception("cannot find type: " + call->ident);
            
            AST::Type last = sym->type;
            std::string my = sym->symbol;

            while(call->modList.head != nullptr){
                if (this->scope == nullptr)sym = this->SymbolTable.search<std::string>(searchSymbol, call->modList.peek());
                else sym = this->scope->SymbolTable.search<std::string>(searchSymbol, call->modList.peek());

                if (sym == nullptr) {
                    if(this->typeList[last.typeName] == nullptr) throw err::Exception("type not found");
                    gen::Type * type = *this->typeList[last.typeName];
                    if (dynamic_cast<gen::Class *>(type) != nullptr){
                        gen::Class * cl = dynamic_cast<gen::Class *>(type);
                        func = cl->nameTable[call->modList.pop()];
                        mod = "pub_" + type->Ident + "_";
                        AST::Refrence * ref = new AST::Refrence();
                        ref->Ident = my;

                        gen::Expr exp =  this->GenExpr(ref, OutputFile);
                        ASMC::Mov * mov = new ASMC::Mov();
                        ASMC::Mov * mov2 = new ASMC::Mov();

                        mov->size = exp.size;
                        mov2->size = exp.size;
                        
                        mov->from = '(' + exp.access + ')';
                        mov->to = this->registers["%eax"]->get(exp.size);
                        mov2->from = this->registers["%eax"]->get(exp.size);
                        mov2->to = this->intArgs[intArgsCounter].get(exp.size);

                        intArgsCounter++;
                        OutputFile.text << mov;
                        OutputFile.text << mov2;
                        break;
                    }
                    call->modList.pop();
                    last = sym->type;
                };
            };
        };

        if (func == nullptr) throw err::Exception("Cannot Find Function: " + call->ident);
        
        this->intArgsCounter = 0;
        call->Args.invert();
        while (call->Args.count > 0)
        {
            gen::Expr exp =  this->GenExpr(call->Args.pop(), OutputFile);
            ASMC::Mov * mov = new ASMC::Mov();
            ASMC::Mov * mov2 = new ASMC::Mov();

            mov->size = exp.size;
            mov2->size = exp.size;
            
            mov->from = exp.access;
            mov->to = this->registers["%eax"]->get(exp.size);
            mov2->from = this->registers["%eax"]->get(exp.size);
            mov2->to = this->intArgs[intArgsCounter].get(exp.size);

            intArgsCounter++;
            OutputFile.text << mov;
            OutputFile.text << mov2;
        }

        ASMC::Call * calls = new ASMC::Call;
        calls->function = func->ident.ident;
        OutputFile.text << calls;
        output.access = this->registers["%rax"]->get(func->type.size);
        output.size = func->type.size;

    }else if (dynamic_cast<AST::Var *>(expr) != nullptr){
        AST::Var var = *dynamic_cast<AST::Var *>(expr);
        gen::Symbol * sym;
        if(this->scope == nullptr) sym = this->SymbolTable.search<std::string>(searchSymbol, var.Ident);
        else sym = scope->SymbolTable.search<std::string>(searchSymbol, var.Ident);
        if (sym == nullptr) throw err::Exception("cannot find: " + var.Ident);

        output.size = sym->type.size;
        output.access = '-' + std::to_string(sym->byteMod) + "(%rbp)";

        var.modList.invert();
        int tbyte = 0;
        
        AST::Type last = sym->type;

        while(var.modList.head != nullptr){
            if(this->typeList[last.typeName] == nullptr) throw err::Exception("type not found");
            gen::Type type = **this->typeList[last.typeName];
            gen::Symbol * modSym = type.SymbolTable.search<std::string>(searchSymbol, var.modList.pop());
            last = modSym->type;
            tbyte = modSym->byteMod;
            ASMC::Mov * mov = new ASMC::Mov();
            mov->size = ASMC::QWord;
            mov->to = this->registers["%edx"]->get(ASMC::QWord);
            mov->from = output.access;
            OutputFile.text << mov;
            output.access = std::to_string(tbyte - this->getBytes(last.size)) + '(' + mov->to + ')';
            output.size = last.size;
        }

    }else if (dynamic_cast<AST::Refrence *>(expr) != nullptr)
    {
        links::LinkedList<gen::Symbol>  * Table;
        if(this->scope == nullptr) Table = &this->SymbolTable;
        else Table = &this->scope->SymbolTable;
        AST::Refrence ref = *dynamic_cast<AST::Refrence *>(expr);
        gen::Symbol sym = *Table->search<std::string>(searchSymbol, ref.Ident);
        ASMC::Lea * lea = new ASMC::Lea();
        lea->to = this->registers["%rax"]->get(ASMC::QWord);
        lea->from = '-' + std::to_string(sym.byteMod) + "(%rbp)";
        //ASMC::Mov * mov = new ASMC::Mov();
        OutputFile.text << lea;
        output.access = registers["%rax"]->get(ASMC::QWord);
        output.size = ASMC::QWord;
    }
    else if (dynamic_cast<AST::StringLiteral *>(expr) != nullptr){
        AST::StringLiteral str = *dynamic_cast<AST::StringLiteral *>(expr);
        ASMC::StringLiteral * strlit = new ASMC::StringLiteral();
        ASMC::Lable * lable = new ASMC::Lable();
        if(this->scope == nullptr)lable->lable = ".str" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        else lable->lable = ".str" + scope->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;
        strlit->value = str.val;
        OutputFile.data << lable;
        OutputFile.data << strlit;
        output.access = "$" + lable->lable;
        output.size = ASMC::QWord;
    }
    else if(dynamic_cast<AST::DeRefence *>(expr)){

        AST::DeRefence deRef = *dynamic_cast<AST::DeRefence *>(expr);
        gen::Symbol sym = *this->SymbolTable.search<std::string>(searchSymbol, deRef.Ident);

        ASMC::Mov * mov = new ASMC::Mov();
        ASMC::Mov * mov2 = new ASMC::Mov();

        mov->size = ASMC::QWord;
        mov->from = '-' + std::to_string(sym.byteMod) + "(%rbp)";
        mov->to = this->registers["%rax"]->get(ASMC::QWord);

        mov2->from = "(" + this->registers["%rax"]->get(ASMC::QWord) + ")";
        mov2->size = deRef.type.size;
        mov2->to = this->registers["%rax"]->get(deRef.type.size);

        output.access = mov2->to;
        output.size = mov2->size;

        OutputFile.text << mov;
        OutputFile.text << mov2;

        
    }else if (dynamic_cast<AST::Compound *>(expr) != nullptr)
    {
        AST::Compound comp = *dynamic_cast<AST::Compound *>(expr);
        switch (comp.op)
        {
            case AST::Plus:{
                ASMC::Mov * mov1 = new ASMC::Mov();
                ASMC::Mov * mov2 = new ASMC::Mov();
                ASMC::Add * add = new ASMC::Add();
                gen::Expr expr1 = this->GenExpr(comp.expr1, OutputFile);
                gen::Expr expr2 = this->GenExpr(comp.expr2, OutputFile);
                mov1->size = ASMC::AUTO;
                mov2->size = ASMC::AUTO;

                mov1->to = this->registers["%edx"]->get(expr1.size);
                mov2->to = this->registers["%eax"]->get(expr1.size);
                
                mov2->to = "%eax";
                mov1->from = expr1.access;
                mov2->from = expr2.access;
                add->op2 = "%eax";
                add->op1 = "%edx";
                OutputFile.text << mov1;
                OutputFile.text << mov2;
                OutputFile.text << add;
                output.access = "%eax";
                output.size = ASMC::DWord;
                break;
            }
            case AST::Minus:{
                ASMC::Mov * mov1 = new ASMC::Mov();
                ASMC::Mov * mov2 = new ASMC::Mov();
                ASMC::Sub * sub = new ASMC::Sub();
                gen::Expr expr1 = this->GenExpr(comp.expr1, OutputFile);
                gen::Expr expr2 = this->GenExpr(comp.expr2, OutputFile);
                mov1->size = expr1.size;
                mov2->size = expr1.size;
                
                mov1->to = this->registers["%edx"]->get(expr1.size);
                mov2->to = this->registers["%eax"]->get(expr1.size);
                
                mov2->to = "%eax";
                mov1->from = expr2.access;
                mov2->from = expr1.access;
                sub->op2 = "%eax";
                sub->op1 = "%edx";
                OutputFile.text << mov1;
                OutputFile.text << mov2;
                OutputFile.text << sub;
                output.access = "%eax";
                output.size = ASMC::DWord;
                break;
            }
            case AST::Mul:{
                ASMC::Mov * mov1 = new ASMC::Mov();
                ASMC::Mov * mov2 = new ASMC::Mov();
                ASMC::Mul * mul = new ASMC::Mul();
                gen::Expr expr1 = this->GenExpr(comp.expr1, OutputFile);
                gen::Expr expr2 = this->GenExpr(comp.expr2, OutputFile);
                
                mov1->size = expr1.size;
                mov2->size = expr1.size;
                
                mov1->to = this->registers["%edx"]->get(expr1.size);
                mov2->to = this->registers["%eax"]->get(expr1.size);
                
                mov2->to = "%eax";
                mov1->from = expr1.access;
                mov2->from = expr2.access;
                mul->op2 = "%eax";
                mul->op1 = "%edx";
                OutputFile.text << mov1;
                OutputFile.text << mov2;
                OutputFile.text << mul;
                output.access = "%eax";
                output.size = ASMC::DWord;
                break;
            }
            case AST::Div:{
                ASMC::Mov * mov1 = new ASMC::Mov();
                ASMC::Mov * mov2 = new ASMC::Mov();
                ASMC::Div * div = new ASMC::Div();
                gen::Expr expr1 = this->GenExpr(comp.expr1, OutputFile);
                gen::Expr expr2 = this->GenExpr(comp.expr2, OutputFile);
                mov1->size = expr1.size;
                mov2->size = expr1.size;
                
                mov1->to = this->registers["%edx"]->get(expr1.size);
                mov2->to = this->registers["%eax"]->get(expr1.size);
                
                mov2->to = "%eax";
                //mov1->from = expr1.access;
                mov2->from = expr1.access;
                div->op1 = expr2.access;
                //OutputFile.text << mov1;
                OutputFile.text << mov2;
                OutputFile.text << div;
                output.access = "%eax";
                output.size = ASMC::DWord;
                break;
            }
            case AST::Mod:{
                ASMC::Mov * mov1 = new ASMC::Mov();
                ASMC::Mov * mov2 = new ASMC::Mov();
                ASMC::Div * div = new ASMC::Div();
                gen::Expr expr1 = this->GenExpr(comp.expr1, OutputFile);
                gen::Expr expr2 = this->GenExpr(comp.expr2, OutputFile);
                
                mov1->size = expr1.size;
                mov2->size = expr1.size;
                
                mov1->to = this->registers["%edx"]->get(expr1.size);
                mov2->to = this->registers["%eax"]->get(expr1.size);
                
                mov2->to = "%eax";
                //mov1->from = expr1.access;
                mov2->from = expr1.access;
                div->op1 = expr2.access;
                //OutputFile.text << mov1;
                OutputFile.text << mov2;
                OutputFile.text << div;
                output.access = "%edx";
                output.size = ASMC::DWord;
                break;
            }
            default:{
                throw err::Exception("Unhandled oporator");
                break;
            }
        }   
    }  
    else{
        throw err::Exception("cannot gen expr");
    }
    return output;
};

links::LinkedList<gen::Symbol> gen::CodeGenerator::GenTable(AST::Statment * STMT, links::LinkedList<gen::Symbol> &table){
    if(dynamic_cast<AST::Sequence *>(STMT) != nullptr){
        AST::Sequence * sequence = dynamic_cast<AST::Sequence *>(STMT);
        this->GenTable(sequence->Statment1, table);
        this->GenTable(sequence->Statment2, table);
    }else if (dynamic_cast<AST::Declare *>(STMT) != nullptr)
    {
        /*
            movl $0x0, -[SymbolT + size](rdp)
            **also needs to be added to symbol table**
        */

        AST::Declare * arg =  dynamic_cast<AST::Declare *>(STMT);
        gen::Symbol symbol;
        
        int offset = this->getBytes(arg->type.size);

        if(table.search<std::string>(searchSymbol, arg->Ident) != nullptr) throw err::Exception("redefined veriable:" + arg->Ident);

        symbol.symbol = arg->Ident;
        if (table.head == nullptr){
            symbol.byteMod = offset;
        }else{
            symbol.byteMod = table.peek().byteMod + offset;
        }
        symbol.type = arg->type;
        table << symbol;
    }else if(dynamic_cast<AST::DecArr *>(STMT) != nullptr){
         /*
            movl $0x0, -[SymbolT + size](rdp)
            **also needs to be added to symbol table**
        */
        AST::DecArr * dec =  dynamic_cast<AST::DecArr *>(STMT);
        int offset = 0;
        offset = this->getBytes(dec->type.size);

        offset = offset * dec->count;

        if(this->SymbolTable.search<std::string>(searchSymbol, dec->ident) != nullptr) throw err::Exception("redefined veriable:" + dec->ident);

        gen::Symbol Symbol;
        if (this->SymbolTable.head == nullptr){
            Symbol.byteMod = offset;
        }else{
            Symbol.byteMod = table.head->data.byteMod + offset;
        }
        Symbol.type = dec->type;
        Symbol.symbol = dec->ident;
        table << Symbol;

    }

    return table;
}

void gen::CodeGenerator::GenArgs(AST::Statment * STMT, ASMC::File &OutputFile){
     if(dynamic_cast<AST::Sequence *>(STMT) != nullptr){
        AST::Sequence * sequence = dynamic_cast<AST::Sequence *>(STMT);
        this->GenArgs(sequence->Statment1, OutputFile);
        this->GenArgs(sequence->Statment2, OutputFile);
    }else if (dynamic_cast<AST::Declare *>(STMT) != nullptr)
    {
        /*
            movl $0x0, -[SymbolT + size](rdp)
            **also needs to be added to symbol table**
        */

        if (intArgsCounter > 6) throw err::Exception("AFlat compiler cannot handle more than 6 int arguments.");
        AST::Declare * arg =  dynamic_cast<AST::Declare *>(STMT);
        ASMC::Size size;
        int offset = 0;
        gen::Symbol symbol;
        ASMC::Mov * mov = new ASMC::Mov();

        links::LinkedList<gen::Symbol>  * Table;
        if(this->scope == nullptr) Table = &this->SymbolTable;
        else Table = &this->scope->SymbolTable;

        offset = this->getBytes(arg->type.size);
        size = arg->type.size;
        mov->from = this->intArgs[intArgsCounter].get(arg->type.size);

        if(Table->search<std::string>(searchSymbol, arg->Ident) != nullptr) throw err::Exception("redefined veriable:" + arg->Ident);

        symbol.symbol = arg->Ident;

        if (Table->head == nullptr){
            symbol.byteMod = offset;
        }else{
            symbol.byteMod = Table->peek().byteMod + offset;
        }

        symbol.type = arg->type;
        Table->push(symbol);

        mov->size = size;
        mov->to = "-" + std::to_string(symbol.byteMod) + + "(%rbp)";
        OutputFile.text << mov;
        intArgsCounter++;
        this->scopePop++;
    }
}

ASMC::File gen::CodeGenerator::GenSTMT(AST::Statment * STMT){

    ASMC::File OutputFile;

    if(dynamic_cast<AST::Sequence *>(STMT) != nullptr){
        AST::Sequence * sequence = dynamic_cast<AST::Sequence *>(STMT);
        OutputFile << this->GenSTMT(sequence->Statment1);
        OutputFile << this->GenSTMT(sequence->Statment2);;

    }else if(dynamic_cast<AST::Function *>(STMT)){
        /*
            ident:
                push rbp
                mov  rsp, rbp
                this->GenSTMT()
                
        */

        if(this->scope == nullptr) this->SymbolTable.clear();

        AST::Function * func = dynamic_cast<AST::Function *>(STMT);
        if(this->scope == nullptr) this->nameTable << *func;
        else this->scope->nameTable << * func;
        
        if(func->statment != nullptr){
            ASMC::Lable * lable = new ASMC::Lable;
            if(this->scope == nullptr) lable->lable = func->ident.ident;
            else lable->lable = "pub_" + scope->Ident + "_" + func->ident.ident;

            ASMC::Push * push = new ASMC::Push();
            push->op = "%rbp";
            ASMC::Mov * mov = new ASMC::Mov();
            mov->size = ASMC::QWord;
            mov->from =  "%rsp";
            mov->to = "%rbp";

            OutputFile.text.push(lable);
            OutputFile.text.push(push);
            OutputFile.text.push(mov);

            int AlignmentLoc = OutputFile.text.count;
            this->intArgsCounter = 0;
            this->returnType = func->type;
            ASMC::LinkTask * link = new ASMC::LinkTask();
            link->command = "global";
            link->operand = func->ident.ident;

            if(this->scope != nullptr){
                this->scopePop = 0;
                int offset = this->getBytes(ASMC::QWord);
                int size = ASMC::QWord;
                gen::Symbol symbol;
                ASMC::Mov * movy = new ASMC::Mov();
                movy->from = this->intArgs[intArgsCounter].get(ASMC::QWord);

                if(scope->SymbolTable.search<std::string>(searchSymbol, "my") != nullptr) throw err::Exception("redefined veriable: my");

                symbol.symbol = "my";

                if (scope->SymbolTable.head == nullptr){
                    symbol.byteMod = offset;
                }else{
                    symbol.byteMod = scope->SymbolTable.peek().byteMod + offset;
                }
                
                auto ty = AST::Type();
                ty.typeName = scope->Ident;
                ty.size = ASMC::QWord;
                symbol.type = ty;
                scope->SymbolTable.push(symbol);

                movy->size = ASMC::QWord;
                movy->to = "-" + std::to_string(symbol.byteMod) + + "(%rbp)";
                OutputFile.text << movy;
                this->intArgsCounter++;
                this->scopePop++;
            };

            this->GenArgs(func->args, OutputFile);
            OutputFile.linker.push(link);

            ASMC::File file = this->GenSTMT(func->statment);
            OutputFile << file;
            
            int alligne = 16;
            if (this->scope == nullptr){
                if(this->SymbolTable.count  > 0){
                    alligne = ((this->SymbolTable.peek().byteMod + 15) / 16) * 16;
                }
            }
            else{
                if(this->scopePop > 0){
                    alligne = ((scope->SymbolTable.peek().byteMod + 15) / 16) * 16;
                }
            }

            ASMC::Subq * sub = new ASMC::Subq;
            sub->op1 = "$" + std::to_string(alligne);
            sub->op2 = this->registers["%rsp"]->get(ASMC::QWord);
            OutputFile.text.insert(sub, AlignmentLoc + 1);
            if(this->scope != nullptr ) for(int i = 0; i  < this->scopePop; i++) this->scope->SymbolTable.pop();
            this->scopePop = 0;

        }
        delete(func);
    }else if (dynamic_cast<AST::Declare *>(STMT) != nullptr)
    {
        /*
            movl $0x0, -[SymbolT + size](rdp)
            **also needs to be added to symbol table**
        */
        AST::Declare * dec =  dynamic_cast<AST::Declare *>(STMT);
        int offset = this->getBytes(dec->type.size);
        links::LinkedList<gen::Symbol>  * Table;
        if(this->scope == nullptr) Table = &this->SymbolTable;
        else Table = &this->scope->SymbolTable;

        if(Table->search<std::string>(searchSymbol, dec->Ident) != nullptr) throw err::Exception("redefined veriable:" + dec->Ident);

        gen::Symbol Symbol;
        if (Table->head == nullptr){
            Symbol.byteMod = offset;
        }else{
            Symbol.byteMod = Table->head->data.byteMod + offset;
        }
        Symbol.type = dec->type;
        Symbol.symbol = dec->Ident;
        Table->push(Symbol);
        this->scopePop += 1;
        
    }else if(dynamic_cast<AST::DecArr *>(STMT) != nullptr){
         /*
            movl $0x0, -[SymbolT + size](rdp)
            **also needs to be added to symbol table**
        */
        AST::DecArr * dec =  dynamic_cast<AST::DecArr *>(STMT);
        int offset = this->getBytes(dec->type.size);

        offset = offset * dec->count;

        links::LinkedList<gen::Symbol>  * Table;
        if(this->scope == nullptr) Table = &this->SymbolTable;
        else Table = &this->scope->SymbolTable;

        if(Table->search<std::string>(searchSymbol, dec->ident) != nullptr) throw err::Exception("redefined veriable:" + dec->ident);

        gen::Symbol Symbol;
        if (Table->head == nullptr){
            Symbol.byteMod = offset;
        }else{
            Symbol.byteMod = Table->head->data.byteMod + offset;
        }
        Symbol.type = dec->type;
        Symbol.symbol = dec->ident;
        Table->push(Symbol);
        this->scopePop += 1;

    }else if (dynamic_cast<AST::DecAssign *>(STMT) != nullptr)
    {
        /*
            movl $0x0, -[SymbolT + size](rdp)
            **also needs to be added to symbol table**
        */
        AST::DecAssign * decAssign =  dynamic_cast<AST::DecAssign *>(STMT);
        AST::Declare * dec = decAssign->declare;
        int offset = this->getBytes(dec->type.size);
        
        links::LinkedList<gen::Symbol>  * Table;
        if(this->scope == nullptr) Table = &this->SymbolTable;
        else Table = &this->scope->SymbolTable;
        
        if(Table->search<std::string>(searchSymbol, dec->Ident) != nullptr) throw err::Exception("redefined veriable:" + dec->Ident);

        gen::Symbol symbol;
        if (Table->head == nullptr){
            symbol.byteMod = offset;
        }else{
            symbol.byteMod = Table->head->data.byteMod + offset;
        }
        symbol.type = dec->type;
        symbol.symbol = dec->Ident;
        Table->push(symbol);

        ASMC::Mov * mov = new ASMC::Mov();
        gen::Expr expr = this->GenExpr(decAssign->expr, OutputFile);
        mov->size = expr.size;
        mov->from = expr.access;
        mov->to = "-" + std::to_string(symbol.byteMod) + "(%rbp)";
        OutputFile.text << mov;
        this->scopePop += 1;

    }else if (dynamic_cast<AST::Return *>(STMT) != nullptr)
    {
        /*
            mov [this.GenExpr(ret.value)]
            pop rbp
            ret
        */

        AST::Return * ret = dynamic_cast<AST::Return *>(STMT);

        ASMC::Mov * mov = new ASMC::Mov();
        mov->size = this->returnType.size;
        mov->from = this->GenExpr(ret->expr, OutputFile).access;
        mov->to = this->registers["%rax"]->get(this->returnType.size);
        OutputFile.text << mov;
        
        ASMC::Return * re = new ASMC::Return();
        OutputFile.text << re;

    }else if (dynamic_cast<AST::Assign *>(STMT) != nullptr)
    {
        AST::Assign * assign = dynamic_cast<AST::Assign *>(STMT);

        links::LinkedList<gen::Symbol>  * Table;
        if(this->scope == nullptr) Table = &this->SymbolTable;
        else Table = &this->scope->SymbolTable;
        
        Symbol * symbol = Table->search<std::string>(searchSymbol, assign->Ident);
        if(symbol == nullptr) throw err::Exception("unknown name: " + assign->Ident);
        ASMC::Mov * mov = new ASMC::Mov();
        ASMC::Mov * mov2 = new ASMC::Mov();
        gen::Expr expr = this->GenExpr(assign->expr, OutputFile);

        mov->size = expr.size;
        mov2->size = expr.size;
        mov2->from = expr.access;
        mov2->to = this->registers["%rbx"]->get(expr.size);
        mov->from = mov2->to;

        assign->modList.invert();
        int tbyte = 0;
        
        AST::Type last = symbol->type;
        ASMC::Size size;
        std::string output = "-" + std::to_string(symbol->byteMod) + "(%rbp)";

        while(assign->modList.head != nullptr){
            if(this->typeList[last.typeName] == nullptr) throw err::Exception("type not found");
            gen::Type type = **this->typeList[last.typeName];
            gen::Symbol * modSym = type.SymbolTable.search<std::string>(searchSymbol, assign->modList.pop());
            last = modSym->type;
            tbyte = modSym->byteMod;

            ASMC::Mov * mov7 = new ASMC::Mov();
            mov7->size = ASMC::QWord;
            mov7->to = this->registers["%edx"]->get(ASMC::QWord);
            mov7->from =  output;
            OutputFile.text << mov7;
            output = std::to_string(tbyte - this->getBytes(last.size)) + '(' + mov7->to + ')';
            size = last.size;
        }


        if(assign->refrence == true){
            ASMC::Mov * m1 = new ASMC::Mov;
            m1->from = output;
            m1->size = ASMC::QWord;
            m1->to = this->registers["%eax"]->get(ASMC::QWord);
            mov->to = "(" + this->registers["%eax"]->get(ASMC::QWord) + ")";
            OutputFile.text << m1;
        }else mov->to = output;

        OutputFile.text << mov2;
        OutputFile.text << mov;

    }else if (dynamic_cast<AST::Call *>(STMT) != nullptr)
    {
        AST::Call * call = dynamic_cast<AST::Call *>(STMT);
        std::string errhold = call ->ident;
        std::string mod = "";

        AST::Function * func;

        this->intArgsCounter = 0;
        if(call->modList.head == nullptr) func = this->nameTable[call->ident];
        else{

            gen::Symbol * sym = this->SymbolTable.search<std::string>(searchSymbol, call->ident);
            if (sym == nullptr) throw err::Exception("cannot find type: " + call->ident);
            
            AST::Type last = sym->type;
            std::string my = sym->symbol;

            while(call->modList.head != nullptr){
                sym = this->SymbolTable.search<std::string>(searchSymbol, call->modList.peek());
                if (sym == nullptr) {
                    if(this->typeList[last.typeName] == nullptr) throw err::Exception("type not found");
                    gen::Type * type = *this->typeList[last.typeName];
                    gen::Class * cl = dynamic_cast<gen::Class *>(type);
                    if(cl != nullptr) {
                        func = cl->nameTable[call->modList.pop()];
                        AST::Refrence * ref = new AST::Refrence();
                        ref->Ident = my;
                        mod = "pub_" + cl->Ident + "_";
                        gen::Expr exp =  this->GenExpr(ref, OutputFile);
                        ASMC::Mov * mov = new ASMC::Mov();
                        ASMC::Mov * mov2 = new ASMC::Mov();

                        mov->size = exp.size;
                        mov2->size = exp.size;
                        
                        mov->from = '(' + exp.access + ')';
                        mov->to = this->registers["%eax"]->get(exp.size);
                        mov2->from = this->registers["%eax"]->get(exp.size);
                        mov2->to = this->intArgs[intArgsCounter].get(exp.size);

                        intArgsCounter++;
                        OutputFile.text << mov;
                        OutputFile.text << mov2;
                        break;
                    }
                }
                call->modList.pop();
                last = sym->type;
            };
        };

        if (func == nullptr) throw err::Exception("Cannot Find Function: " + call->ident);
        
        
        call->Args.invert();
        while (call->Args.count > 0)
        {
            gen::Expr exp =  this->GenExpr(call->Args.pop(), OutputFile);
            ASMC::Mov * mov = new ASMC::Mov();
            ASMC::Mov * mov2 = new ASMC::Mov();

            mov->size = exp.size;
            mov2->size = exp.size;
            
            mov->from = exp.access;
            mov->to = this->registers["%eax"]->get(exp.size);
            mov2->from = this->registers["%eax"]->get(exp.size);
            mov2->to = this->intArgs[intArgsCounter].get(exp.size);

            intArgsCounter++;
            OutputFile.text << mov;
            OutputFile.text << mov2;
        }

        ASMC::Call * calls = new ASMC::Call;
        calls->function = mod + func->ident.ident;
        OutputFile.text << calls;

    }
    else if (dynamic_cast<AST::Push *>(STMT) != nullptr)
    {
        AST::Push * push = dynamic_cast<AST::Push *>(STMT);
        ASMC::Mov * count = new ASMC::Mov;

        count->size = ASMC::QWord;
        count->to = this->registers["%rdx"]->get(count->size);
        count->from = "$1";
        ASMC::Mov * pointer = new ASMC::Mov;
        pointer->size = ASMC::QWord;
        pointer->to = this->registers["%rsi"]->get(pointer->size);
        pointer->from = this->GenExpr(push->expr, OutputFile).access;

        ASMC::Mov * callnum = new ASMC::Mov;
        callnum->size = ASMC::QWord;
        callnum->to = this->registers["%rax"]->get(callnum->size);
        callnum->from = "$1";

        ASMC::Mov * rdi = new ASMC::Mov;
        rdi->size = ASMC::QWord;
        rdi->from = "$1";
        rdi->to = this->registers["%rdi"]->get(rdi->size);

        OutputFile.text << rdi;
        OutputFile.text << pointer;
        OutputFile.text << count;
        OutputFile.text << callnum;
        
        OutputFile.text << new ASMC::SysCall;
    }else if (dynamic_cast<AST::Pull *>(STMT) != nullptr)
    {
        AST::Pull * pull = dynamic_cast<AST::Pull *>(STMT);
        ASMC::Mov * count = new ASMC::Mov;
        count->size = ASMC::QWord;
        count->to = this->registers["%rdx"]->get(count->size);
        count->from = "$1";
        ASMC::Mov * pointer = new ASMC::Mov;
        pointer->size = ASMC::QWord;
        pointer->to = this->registers["%rsi"]->get(pointer->size);
        pointer->from = this->GenExpr(pull->expr, OutputFile).access;
        ASMC::Mov * callnum = new ASMC::Mov;
        callnum->size = ASMC::QWord;
        callnum->to = this->registers["%rax"]->get(callnum->size);
        callnum->from = "$0";
        ASMC::Mov * rdi = new ASMC::Mov;
        rdi->size = ASMC::QWord;
        rdi->from = "$1";
        rdi->to = this->registers["%rdi"]->get(rdi->size);

        OutputFile.text << rdi;
        OutputFile.text << pointer;
        OutputFile.text << count;
        OutputFile.text << callnum;
        
        OutputFile.text << new ASMC::SysCall;
    }
    else if (dynamic_cast<AST::If *>(STMT) != nullptr)
    {
        AST::If ifStmt = *dynamic_cast<AST::If *>(STMT);

        ASMC::Lable * lable1 = new ASMC::Lable();
        lable1->lable = ".L"+ this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount ++;

        gen::Expr expr1 =this->GenExpr(ifStmt.Condition->expr1, OutputFile);
        gen::Expr expr2 =this->GenExpr(ifStmt.Condition->expr2, OutputFile);
    
        ASMC::Mov * mov1 = new ASMC::Mov();
        ASMC::Mov * mov2 = new ASMC::Mov();

        mov1->size = expr1.size;
        mov2->size = expr2.size;

        mov1->from = expr1.access;
        mov2->from = expr2.access;

        mov1->to = this->registers["%eax"]->get(mov1->size);
        mov2->to = this->registers["%rcx"]->get(mov2->size);

        mov2->size = mov2->size;

        ASMC::Cmp * cmp = new ASMC::Cmp();
        ASMC::Jne * jne = new ASMC::Jne();

        cmp->from = mov2->to;
        cmp->to = mov1->to;
        cmp->size = expr1.size;
        
        OutputFile.text << mov1;
        OutputFile.text << mov2;
        OutputFile.text << cmp;

        switch (ifStmt.Condition->op)
        {
        case AST::Equ:
        {
            jne->to = lable1->lable;
            
            OutputFile.text << jne;
            OutputFile << this->GenSTMT(ifStmt.statment);
            OutputFile.text << lable1;
            break;
        }
        case AST::NotEqu :
        {
            ASMC::Je * je = new ASMC::Je();

            je->to = lable1->lable;

            OutputFile.text << je;
            OutputFile << this->GenSTMT(ifStmt.statment);
            OutputFile.text << lable1;
            break;
        }
        case AST::Great :
        {
            ASMC::Jle * jl = new ASMC::Jle();

            jl->to = lable1->lable;

            OutputFile.text << jl;
            OutputFile << this->GenSTMT(ifStmt.statment);
            OutputFile.text << lable1;
            break;
        }
        case AST::Less :
        {
            ASMC::Jge * jg = new ASMC::Jge();

            jg->to = lable1->lable;


            OutputFile.text << jg;
            OutputFile << this->GenSTMT(ifStmt.statment);
            OutputFile.text << lable1;
            break;
        }
        
        default:
            break;
        }
    }
    else if(dynamic_cast<AST::While *>(STMT) != nullptr){
        AST::While * loop = dynamic_cast<AST::While *>(STMT);

        ASMC::Lable * lable1 = new ASMC::Lable();
        lable1->lable =".L" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;

        ASMC::Lable * lable2 = new ASMC::Lable();
        lable2->lable = ".L" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;

        ASMC::Jmp * jmp = new ASMC::Jmp();
        jmp->to = lable2->lable;
        OutputFile.text << jmp;

        OutputFile.text << lable1;

        OutputFile << this->GenSTMT(loop->stmt);

        OutputFile.text << lable2;

        gen::Expr expr1 =this->GenExpr(loop->condition->expr1, OutputFile);
        gen::Expr expr2 =this->GenExpr(loop->condition->expr2, OutputFile);
    
        ASMC::Mov * mov1 = new ASMC::Mov();
        ASMC::Mov * mov2 = new ASMC::Mov();

        mov1->size = expr1.size;
        mov2->size = expr2.size;

        mov1->from = expr1.access;
        mov2->from = expr2.access;

        mov1->to = this->registers["%eax"]->get(mov1->size);
        mov2->to = this->registers["%ecx"]->get(mov2->size);

        ASMC::Cmp * cmp = new ASMC::Cmp();
        ASMC::Jne * jne = new ASMC::Jne();

        cmp->from = mov2->to;
        cmp->to = mov1->to;
        cmp->size = expr1.size;

        
        OutputFile.text << mov1;
        OutputFile.text << mov2;
        OutputFile.text << cmp;

        switch (loop->condition->op)
        {
        case AST::Equ:
        {
            ASMC::Je * je = new ASMC::Je();
            je->to = lable1->lable;
            OutputFile.text << je;
            break;
        }
        case AST::NotEqu :
        {
            ASMC::Jne * jne = new ASMC::Jne();

            jne->to = lable1->lable;
            OutputFile.text << jne;
            break;
        }
        case AST::Great :
        {
            ASMC::Jg * jg = new ASMC::Jg();

            jg->to = lable1->lable;

            OutputFile.text << jg;
            break;
        }
        case AST::Less :
        {
            ASMC::Jl * jl = new ASMC::Jl();

            jl->to = lable1->lable;
            OutputFile.text << jl;
            break;
        }
        }
        
    }
    else if(dynamic_cast<AST::UDeffType *>(STMT) != nullptr){
        AST::UDeffType * udef = dynamic_cast<AST::UDeffType *>(STMT);
        gen::Type * type = new gen::Type();
        type->Ident = udef->ident.ident;
        type->SymbolTable  = this->GenTable(udef->statment, type->SymbolTable);
        this->typeList.push(type);
    }
    else if(dynamic_cast<AST::Class *>(STMT) != nullptr){
        AST::Class * deff = dynamic_cast<AST::Class *>(STMT);
        gen::Class * type = new gen::Class();
        type->Ident = deff->ident.ident;
        type->nameTable.foo = compairFunc;
        this->scope = type;
        type->SymbolTable;
        this->typeList.push(type);
        ASMC::File file = this->GenSTMT(deff->statment);
        OutputFile << file;
        
        this->scope = nullptr;
    }
    else{
        OutputFile.text.push(new ASMC::Instruction());
    }

    return OutputFile;
}