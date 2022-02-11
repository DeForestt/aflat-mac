#include "CodeGenerator/CodeGenerator.hpp"
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

int gen::CodeGenerator::getBytes(asmc::Size size){
    switch(size){ 
            case asmc::QWord:
                return 8;
                break;
            case asmc::Word:
                return 2;
                break;
            case asmc::Byte:
                return 1;
                break;
            case asmc::DWord:
                return 4;
                break;
            default:
                throw err::Exception("Unknown Size");
        }
}

gen::CodeGenerator::CodeGenerator(){
    this->registers << asmc::Register("rax", "eax", "ax", "al");
    this->registers << asmc::Register("rcx", "ecx", "cx", "cl");
    this->registers << asmc::Register("rdx", "edx", "dx", "dl");
    this->registers << asmc::Register("rbx", "ebx", "bx", "bl");
    this->registers << asmc::Register("rsi", "esi", "si", "sil");
    this->registers << asmc::Register("rdi", "edi", "di", "dil");   
    this->registers << asmc::Register("rsp", "esp", "sp", "spl");
    this->registers << asmc::Register("rbp", "ebp", "bp", "bpl");
    this->registers << asmc::Register("xmm0", "xmm0", "xmm0", "xmm0");
    this->registers << asmc::Register("xmm1", "xmm1", "xmm1", "xmm1");
    this->registers << asmc::Register("xmm2", "xmm2", "xmm2", "xmm2");
    this->registers << asmc::Register("xmm3", "xmm3", "xmm3", "xmm3");
    this->registers.foo = asmc::Register::compair;
    this->nameTable.foo = compairFunc;
    this->globalScope = true;
    this->typeList.foo = gen::Type::compair;
    this->scope = nullptr;
}

void gen::CodeGenerator::prepareCompound(AST::Compound compound, asmc::File &OutputFile, bool isDiv = false){
    asmc::Mov * mov1 = new asmc::Mov();
    asmc::Mov * mov2 = new asmc::Mov();
    std::string r1 = "%edx", r2 = "%eax";
    // if expr1 op is Float set to the float registers

    gen::Expr expr2 = this->GenExpr(compound.expr2, OutputFile);
    
    if (expr2.op == asmc::Float){
        r1 = "%xmm1";
        r2 = "%xmm0";
    }

    mov1->op = expr2.op;
    mov1->to = this->registers[r1]->get(expr2.size);
    mov1->from = expr2.access;
    if (!isDiv) OutputFile.text << mov1;

    gen::Expr expr1 = this->GenExpr(compound.expr1, OutputFile);
    mov2->op = expr1.op;
    mov2->to = this->registers[r2]->get(expr1.size);
    mov2->from = expr1.access;
    OutputFile.text << mov2;

    mov1->size = asmc::AUTO;
    mov2->size = asmc::AUTO;
    
}

gen::Expr gen::CodeGenerator::GenExpr(AST::Expr * expr, asmc::File &OutputFile){
    gen::Expr output;
    output.op = asmc::Hard;
    if(dynamic_cast<AST::IntLiteral *>(expr) != nullptr){
        AST::IntLiteral * intlit = dynamic_cast<AST::IntLiteral *>(expr);
        output.access = '$' + std::to_string(intlit->val);
        output.size = asmc::DWord;
    }
    else if(dynamic_cast<AST::LongLiteral *>(expr) != nullptr){
        AST::LongLiteral * intlit = dynamic_cast<AST::LongLiteral *>(expr);
        output.access = '$' + std::to_string(intlit->val);
        output.size = asmc::QWord;
    }
    else if(dynamic_cast<AST::CharLiteral *>(expr) != nullptr){
        AST::CharLiteral * intlit = dynamic_cast<AST::CharLiteral *>(expr);
        output.access = '$' + std::to_string(intlit->value);
        output.size = asmc::Byte;
    }
    else if(dynamic_cast<AST::CallExpr *>(expr) != nullptr){
        AST::CallExpr * exprCall = dynamic_cast<AST::CallExpr *>(expr);
        AST::Call * call = exprCall->call;
        output.size = this->GenCall(call, OutputFile).type.size;
        output.access = this->registers["%rax"]->get(output.size);

    }
    else if (dynamic_cast<AST::Var *>(expr) != nullptr){
        AST::Var var = *dynamic_cast<AST::Var *>(expr);
        bool global = false;
        bool handled = false;
        gen::Symbol * sym;
        sym = this->SymbolTable.search<std::string>(searchSymbol, var.Ident);
        if(sym == nullptr) {
            sym = this->GlobalSymbolTable.search<std::string>(searchSymbol, var.Ident);
            global = true;
            if(sym == nullptr){
                Type ** t = this->typeList[var.Ident];
                if (t != nullptr){
                    Type * type = *t;
                    output.size = asmc::DWord;
                    output.access = '$' + std::to_string(type->SymbolTable.head->data.byteMod);
                    handled = true;
                }else if(var.Ident == "int"){
                    output.size = asmc::DWord;
                    output.access = "$4";
                    handled = true;
                }else if(var.Ident == "char"){
                    output.size = asmc::DWord;
                    output.access = "$1";
                    handled = true;
                }else if(var.Ident == "adr"){
                    output.size = asmc::DWord;
                    output.access = "$8";
                    handled = true;
                }else if(var.Ident == "byte"){
                    output.size = asmc::DWord;
                    output.access = "$1";
                    handled = true;
                }else if(var.Ident == "float"){
                    output.size = asmc::DWord;
                    output.access = "$4";
                    handled = true;
                }else if(var.Ident == "NULL"){
                    output.size = asmc::QWord;
                    output.access = "$0";
                    handled = true;
                }else if(this->nameTable[var.Ident] != nullptr){
                    output.size = asmc::QWord;
                    output.access = '$' + this->nameTable[var.Ident]->ident.ident;
                    handled = true;
                }else throw err::Exception("variable not found " + var.Ident);
            }
        }

        if (!handled){
            if (sym != nullptr){
                output.size = sym->type.size;
                output.op = sym->type.opType;
                if (global) output.access = sym->symbol;
                else output.access = '-' + std::to_string(sym->byteMod) + "(%rbp)";

                var.modList.invert();
                int tbyte = 0;
                
                AST::Type last = sym->type;

                while(var.modList.head != nullptr){
                    if(this->typeList[last.typeName] == nullptr) throw err::Exception("type not found " + last.typeName);
                    gen::Type type = **this->typeList[last.typeName];
                    gen::Symbol * modSym;
                    std::string sto = var.modList.peek();
                    if(this->scope == *this->typeList[last.typeName]){
                        // if we are scoped to the type seache the symbol in the type symbol table
                         modSym = type.SymbolTable.search<std::string>(searchSymbol, var.modList.pop());
                    }else{
                        // if we are not scoped to the type search the symbol in the public symbol table
                        modSym = type.publicSymbols.search<std::string>(searchSymbol, var.modList.pop());
                    };
                    if (modSym == nullptr) throw err::Exception("variable not found " + last.typeName + "." + sto);
                    last = modSym->type;
                    tbyte = modSym->byteMod;
                    asmc::Mov * mov = new asmc::Mov();
                    mov->op = modSym->type.opType;
                    mov->size = asmc::QWord;
                    mov->to = this->registers["%edx"]->get(asmc::QWord);
                    mov->from = output.access;
                    OutputFile.text << mov;
                    output.access = std::to_string(tbyte - this->getBytes(last.size)) + '(' + mov->to + ')';
                    output.op = modSym->type.opType;
                    output.size = last.size;
                }
            }else{
                Type ** t = this->typeList[var.Ident];
                if (t != nullptr){
                    Type * type = *t;
                    output.size = asmc::DWord;
                    output.access = '$' + std::to_string(type->SymbolTable.head->data.byteMod);
                    handled = true;
                }else if(this->nameTable[var.Ident] != nullptr){
                    output.size = asmc::DWord;
                    output.access = '$' + this->nameTable[var.Ident]->ident.ident;
                    handled = true;
                }  else throw err::Exception("variable not found " + var.Ident);
            }
        }
    }
    else if (dynamic_cast<AST::Refrence *>(expr) != nullptr){
        links::LinkedList<gen::Symbol>  * Table;
        AST::Refrence ref = *dynamic_cast<AST::Refrence *>(expr);
        if(this->scope == nullptr || ref.internal) Table = &this->SymbolTable;
        //else Table = &this->scope->SymbolTable;
        
        gen::Symbol sym = *Table->search<std::string>(searchSymbol, ref.Ident);
        asmc::Lea * lea = new asmc::Lea();
        lea->to = this->registers["%rax"]->get(asmc::QWord);
        lea->from = '-' + std::to_string(sym.byteMod) + "(%rbp)";
        //ASMC::Mov * mov = new ASMC::Mov();
        OutputFile.text << lea;
        output.access = registers["%rax"]->get(asmc::QWord);
        output.size = asmc::QWord;
        output.op = asmc::OpType::Hard;
    }
    else if (dynamic_cast<AST::StringLiteral *>(expr) != nullptr){
        AST::StringLiteral str = *dynamic_cast<AST::StringLiteral *>(expr);
        asmc::StringLiteral * strlit = new asmc::StringLiteral();
        asmc::Lable * lable = new asmc::Lable();
        if(this->scope == nullptr)lable->lable = ".str" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        else lable->lable = ".str" + scope->Ident +'.'+ scope->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;
        strlit->value = str.val;
        OutputFile.data << lable;
        OutputFile.data << strlit;
        output.access = "$" + lable->lable;
        output.size = asmc::QWord;
    }
    else if (dynamic_cast<AST::FloatLiteral *>(expr) != nullptr){
        AST::FloatLiteral * floatlit = dynamic_cast<AST::FloatLiteral *>(expr);
        asmc::FloatLiteral * fltlit = new asmc::FloatLiteral();
        asmc::Lable * lable = new asmc::Lable();
        if(this->scope == nullptr)lable->lable = ".float" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        else lable->lable = ".float" +scope->Ident +'.'+ scope->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;
        fltlit->value = floatlit->val;

        //move value to the xmm0 register
        asmc::Mov * mov = new asmc::Mov();
        mov->size = asmc::DWord;
        mov->op = asmc::Float;
        mov->to = this->registers["%xmm0"]->get(asmc::DWord);
        mov->from = lable->lable;

        output.op = asmc::Float;
        OutputFile.text << mov;
        OutputFile.data << lable;
        OutputFile.data << fltlit;
        output.access = this->registers["%xmm0"]->get(asmc::DWord);
        output.size = asmc::DWord;
    }
    else if(dynamic_cast<AST::DeRefence *>(expr)){

        AST::DeRefence deRef = *dynamic_cast<AST::DeRefence *>(expr);
        gen::Symbol sym = *this->SymbolTable.search<std::string>(searchSymbol, deRef.Ident);

        asmc::Mov * mov = new asmc::Mov();
        asmc::Mov * mov2 = new asmc::Mov();

        mov->size = asmc::QWord;
        mov->from = '-' + std::to_string(sym.byteMod) + "(%rbp)";
        mov->to = this->registers["%rax"]->get(asmc::QWord);

        mov2->from = "(" + this->registers["%rax"]->get(asmc::QWord) + ")";
        mov2->size = deRef.type.size;
        mov2->to = this->registers["%rax"]->get(deRef.type.size);

        output.access = mov2->to;
        output.size = mov2->size;

        OutputFile.text << mov;
        OutputFile.text << mov2;

        
    }
    else if (dynamic_cast<AST::Compound *>(expr) != nullptr){
        AST::Compound comp = *dynamic_cast<AST::Compound *>(expr);
        asmc::File Dummby = asmc::File();
        output.op = asmc::Hard;
        switch (comp.op)
        {
            case AST::Plus:{
                asmc::Add * add = new asmc::Add();
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);
                this->prepareCompound(comp, OutputFile);
                
                add->opType = expr1.op;
                

                std::string to1 = this->registers["%rdx"]->get(expr1.size);
                std::string to2 = this->registers["%rax"]->get(expr1.size);
                output.access = "%eax";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm0";
                    output.op = asmc::Float;
                }


                add->op2 = to2;
                add->op1 = to1;
                OutputFile.text << add;
                
                output.size = asmc::DWord;
                break;
            }
            case AST::Minus:{
                asmc::Sub * sub = new asmc::Sub();
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);

                this->prepareCompound(comp, OutputFile);
                sub->opType = expr1.op;

                std::string to1 = this->registers["%rdx"]->get(expr1.size);
                std::string to2 = this->registers["%rax"]->get(expr1.size);
                output.access = "%eax";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm0";
                    output.op = asmc::Float;
                }
            
                sub->op2 = to2;
                sub->op1 = to1;

                OutputFile.text << sub;

                output.size = asmc::DWord;
                break;
            }
            case AST::AndBit:{
                asmc::And * andBit = new asmc::And();
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);

                this->prepareCompound(comp, OutputFile);

                std::string to1 = this->registers["%rdx"]->get(expr1.size);
                std::string to2 = this->registers["%rax"]->get(expr1.size);
                output.access = "%eax";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm0";
                    output.op = asmc::Float;
                }
            
                andBit->op2 = to2;
                andBit->op1 = to1;

                OutputFile.text << andBit;

                output.size = asmc::DWord;
                break;
            }
            case AST::OrBit:{
                asmc::Or * orBit = new asmc::Or();
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);

                this->prepareCompound(comp, OutputFile);

                std::string to1 = this->registers["%rdx"]->get(expr1.size);
                std::string to2 = this->registers["%rax"]->get(expr1.size);
                output.access = "%eax";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm0";
                    output.op = asmc::Float;
                }
            
                orBit->op2 = to2;
                orBit->op1 = to1;

                OutputFile.text << orBit;

                output.size = asmc::DWord;
                break;
            }
            case AST::Less:{
                asmc::Sal * andBit = new asmc::Sal();
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);

                this->prepareCompound(comp, OutputFile);

                std::string to1 = this->registers["%rdx"]->get(expr1.size);
                std::string to2 = this->registers["%cl"]->get(expr1.size);
                output.access = "%eax";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm0";
                    output.op = asmc::Float;
                }
            
                andBit->op2 = to2;
                andBit->op1 = "%cl";

                                //Move the value from edx to ecx
                asmc::Mov * mov = new asmc::Mov();
                mov->to = to1;
                mov->from = this->registers["%rdx"]->get(expr1.size);;
                mov->size = expr1.size;

                OutputFile.text << mov;

                OutputFile.text << andBit;

                output.size = asmc::DWord;
                break;
            }
            case AST::Great:{
                asmc::Sar * andBit = new asmc::Sar();
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);

                this->prepareCompound(comp, OutputFile);

                std::string to1 = this->registers["%cl"]->get(expr1.size);
                std::string to2 = this->registers["%rax"]->get(expr1.size);
                output.access = "%eax";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm0";
                    output.op = asmc::Float;
                }

                //Move the value from edx to ecx
                asmc::Mov * mov = new asmc::Mov();
                mov->to = to1;
                mov->from = this->registers["%rdx"]->get(expr1.size);;
                mov->size = expr1.size;

                OutputFile.text << mov;
            
                andBit->op2 = to2;
                andBit->op1 = "%cl";

                OutputFile.text << andBit;

                output.size = asmc::DWord;
                break;
            }
            case AST::Mul:{
                asmc::Mul * mul = new asmc::Mul();
                
                this->selectReg = 0;
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                this->selectReg = 1;
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);
                
                this->prepareCompound(comp, OutputFile);
                mul->opType = expr1.op;

                std::string to1 = this->registers["%rdx"]->get(expr1.size);
                std::string to2 = this->registers["%rax"]->get(expr1.size);
                output.access = "%eax";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm0";
                    output.op = asmc::Float;
                }
                
                mul->op2 = to2;
                mul->op1 = to1;

                OutputFile.text << mul;
                
                output.size = asmc::DWord;
                break;
            }
            case AST::Div:{

                asmc::Div * div = new asmc::Div();

                this->selectReg = 0;
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                this->selectReg = 1;
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);

                div->op1 = expr2.access;
                div->opType = expr1.op;

                std::string to1 = this->registers["%rdx"]->get(expr1.size);
                std::string to2 = this->registers["%rax"]->get(expr1.size);
                output.access = "%eax";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm0";
                    div->op1 = to1;
                    div->op2 = to2;
                    this->prepareCompound(comp, OutputFile);
                    output.op = asmc::Float;
                }else this->prepareCompound(comp, OutputFile, true);
                
                

                OutputFile.text << div;
                output.size = asmc::DWord;
                break;
            }
            case AST::Mod:{
                asmc::Div * div = new asmc::Div();
                
                this->selectReg = 0;
                gen::Expr expr1 = this->GenExpr(comp.expr1, Dummby);
                this->selectReg = 1;
                gen::Expr expr2 = this->GenExpr(comp.expr2, Dummby);

                div->op1 = expr2.access;

                std::string to1 = this->registers["%rdx"]->get(expr1.size);
                std::string to2 = this->registers["%rax"]->get(expr1.size);
                output.access = "%edx";

                if(expr1.op == asmc::Float){
                    to1 = this->registers["%xmm1"]->get(asmc::DWord);
                    to2 = this->registers["%xmm0"]->get(asmc::DWord);
                    output.access = "%xmm1";
                    output.op = asmc::Float;
                    div->op1 = to1;
                    div->op2 = to2;
                    this->prepareCompound(comp, OutputFile);
                }else this->prepareCompound(comp, OutputFile, true);

                OutputFile.text << div;
                output.size = asmc::DWord;
                break;
            }
            default:{
                throw err::Exception("Unhandled oporator");
                break;
            }
        }   
    }  
    else if (dynamic_cast<AST::Lambda *>(expr) != nullptr){
        AST::Lambda lambda = *dynamic_cast<AST::Lambda *>(expr);
        AST::Function * func = lambda.function;
        func->ident.ident = "lambda_" + std::to_string(lablecount);
        lablecount++;
        func->scopeName = "global";
        func->isLambda = true;

        if(OutputFile.lambdas == nullptr) OutputFile.lambdas = new asmc::File;
        OutputFile.hasLambda = true;
        OutputFile.lambdas->operator<<(this->GenSTMT(func));

        output.access = "$" + func->ident.ident;
        output.size = asmc::QWord;
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

        if(table.search<std::string>(searchSymbol, arg->Ident) != nullptr) throw err::Exception("redefined variable:" + arg->Ident);

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

void gen::CodeGenerator::GenArgs(AST::Statment * STMT, asmc::File &OutputFile){
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
        asmc::Size size;
        int offset = 0;
        gen::Symbol symbol;
        asmc::Mov * mov = new asmc::Mov();

        links::LinkedList<gen::Symbol>  * Table = &this->SymbolTable;

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
    }
}

AST::Function gen::CodeGenerator::GenCall(AST::Call * call, asmc::File &OutputFile){
    std::string mod = "";

        AST::Function * func;

        links::LinkedList<gen::Symbol>  * Table;
        Table = &this->SymbolTable;

        this->intArgsCounter = 0;
        if(call->modList.head == nullptr){
            func = this->nameTable[call->ident];
            if (func == nullptr){
                gen::Symbol * smbl = Table->search<std::string>(searchSymbol, call->ident);
                if ( smbl != nullptr){
                                AST::Function nfunc;
                                AST::Var * var = new AST::Var();
                                var->Ident = smbl->symbol;
                                var->modList = links::LinkedList<std::string>();

                                gen::Expr exp1 = this->GenExpr(var, OutputFile);

                                asmc::Mov * mov = new asmc::Mov();
                                mov->size = exp1.size;
                                mov->from = exp1.access;
                                mov->to = this->registers["%rcx"]->get(exp1.size);
                                OutputFile.text << mov;

                                func = new AST::Function();
                                func->ident.ident = '*' + this->registers["%rcx"]->get(exp1.size);
                                func->type = smbl->type;
                                func->type.size = asmc::DWord;
                };
            };
        }
        else{
            gen::Symbol * sym = this->SymbolTable.search<std::string>(searchSymbol, call->ident);
            if (sym == nullptr) throw err::Exception("cannot find type: " + call->ident);
            
            AST::Type last = sym->type;
            std::string my = sym->symbol;
            // get the type of the original function
            gen::Type * type = *this->typeList[last.typeName];

            while(call->modList.head != nullptr){
                sym = type->SymbolTable.search<std::string>(searchSymbol, call->modList.peek());
                if (sym == nullptr) {
                    if(this->typeList[last.typeName] == nullptr) throw err::Exception("type not found " + last.typeName);
                    type = *this->typeList[last.typeName];
                    gen::Class * cl = dynamic_cast<gen::Class *>(type);
                    if(cl != nullptr) {
                        func = cl->nameTable[call->modList.pop()];
                        if (func == nullptr){
                            // search the class symbol table for a pointer
                            sym = cl->SymbolTable.search<std::string>(searchSymbol, call->modList.peek());
                            if (sym != nullptr){
                                AST::Function nfunc;
                                AST::Var * var = new AST::Var();
                                var->Ident = sym->symbol;
                                var->modList = links::LinkedList<std::string>();

                                gen::Expr exp1 = this->GenExpr(var, OutputFile);

                                asmc::Mov * mov = new asmc::Mov();
                                mov->size = exp1.size;
                                mov->from = exp1.access;
                                mov->to = this->registers["%rcx"]->get(exp1.size);
                                OutputFile.text << mov;

                                func = new AST::Function();
                                func->ident.ident = '*' + this->registers["%rcx"]->get(exp1.size);
                                func->type = sym->type;
                                func->type.size = asmc::DWord;
                            };
                        };
                        AST::Refrence * ref = new AST::Refrence();
                        ref->Ident = my;
                        ref->internal = true;
                        mod = "pub_" + cl->Ident + "_";
                        gen::Expr exp =  this->GenExpr(ref, OutputFile);
                        asmc::Mov * mov = new asmc::Mov();
                        asmc::Mov * mov2 = new asmc::Mov();

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
             gen::Expr exp;
            //check if args.pop is a callExpr
            if(dynamic_cast<AST::CallExpr *>(call->Args.peek()) != nullptr){
                AST::CallExpr * callExpr = dynamic_cast<AST::CallExpr *>(call->Args.peek());
                // get the number of args
                int argc = callExpr->call->Args.count;
                // push all of the used int args to the stack
                for(int i = 0; i < argc; i++){
                    asmc::Push * push = new asmc::Push();
                    push->op = this->intArgs[intArgsCounter].get(asmc::QWord);
                    OutputFile.text << push;
                }
                // genorate the expr
                exp = this->GenExpr(callExpr, OutputFile);
                // pop all of the used int args from the stack
                for(int i = 0; i < argc; i++){
                    asmc::Pop * pop = new asmc::Pop();
                    pop->op = this->intArgs[intArgsCounter].get(asmc::QWord);
                    OutputFile.text << pop;
                };
                // pop the args list
                call->Args.pop();
            } else exp =  this->GenExpr(call->Args.pop(), OutputFile);
            asmc::Mov * mov = new asmc::Mov();
            asmc::Mov * mov2 = new asmc::Mov();
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

        asmc::Call * calls = new asmc::Call;
        calls->function = mod + func->ident.ident;
        OutputFile.text << calls;
        intArgsCounter = 0;
        return *func;
};

asmc::File gen::CodeGenerator::GenSTMT(AST::Statment * STMT){

    asmc::File OutputFile = asmc::File();

    if(dynamic_cast<AST::Sequence *>(STMT) != nullptr){
        AST::Sequence * sequence = dynamic_cast<AST::Sequence *>(STMT);
        OutputFile << this->GenSTMT(sequence->Statment1);
        OutputFile << this->GenSTMT(sequence->Statment2);
    }
    else if(dynamic_cast<AST::Function *>(STMT)){
        /*
            ident:
                push rbp
                mov  rsp, rbp
                this->GenSTMT()
        */

        AST::Function * func = dynamic_cast<AST::Function *>(STMT);
        int saveIntArgs = intArgsCounter;
        bool isLambda = func->isLambda;
        if (!isLambda){
            this->SymbolTable.clear();
            this->SymbolTable.head = nullptr;
        }

        if(this->scope == nullptr) this->nameTable << *func;
        else{
            // add the function to the class name table
            this->scope->nameTable << * func;
            // if the function is public add it to the public name table
            if(func->scope == AST::Public) this->scope->publicNameTable << * func;
        }
        
        if(func->statment != nullptr){
            this->inFunction = true;
            gen::Class * saveScope = this->scope;
            bool saveGlobal = this->globalScope;
            this->globalScope = false;

            asmc::Lable * lable = new asmc::Lable;
            if(this->scope == nullptr || func->isLambda) lable->lable = func->ident.ident;
            else lable->lable = "pub_" + scope->Ident + "_" + func->ident.ident;
            if(func->scopeName != "global"){
                lable->lable = "pub_" + func->scopeName + "_" + func->ident.ident;
                gen::Type * tscope = *this->typeList[func->scopeName];
                if(tscope == nullptr) throw err::Exception("Failed to locate function Scope");
                if(dynamic_cast<gen::Class *>(tscope) == nullptr) throw err::Exception("Can only scope to  a class");
                this->scope = dynamic_cast<gen::Class *>(tscope);
            }

            asmc::Push * push = new asmc::Push();
            push->op = "%rbp";
            asmc::Mov * mov = new asmc::Mov();
            mov->size = asmc::QWord;
            mov->from =  "%rsp";
            mov->to = "%rbp";

            OutputFile.text.push(lable);
            OutputFile.text.push(push);
            OutputFile.text.push(mov);

            int AlignmentLoc = OutputFile.text.count;
            this->intArgsCounter = 0;
            this->returnType = func->type;
            asmc::LinkTask * link = new asmc::LinkTask();
            link->command = "global";
            link->operand = lable->lable;

            if(this->scope != nullptr && !func->isLambda){
                // add the opject to the arguments of the function
                int offset = this->getBytes(asmc::QWord);
                int size = asmc::QWord;
                gen::Symbol symbol;
                asmc::Mov * movy = new asmc::Mov();
                movy->from = this->intArgs[intArgsCounter].get(asmc::QWord);

                if(this->SymbolTable.search<std::string>(searchSymbol, "my") != nullptr) throw err::Exception("redefined veriable: my");

                symbol.symbol = "my";
                
                                
                auto ty = AST::Type();
                ty.typeName = scope->Ident;
                ty.size = asmc::QWord;
                symbol.type = ty;
             

                if (this->scope->SymbolTable.head == nullptr){
                    symbol.byteMod = offset;
                }else{
                    symbol.byteMod = scope->SymbolTable.peek().byteMod + offset;
                }
                this->SymbolTable.push(symbol);

                movy->size = asmc::QWord;
                movy->to = "-" + std::to_string(symbol.byteMod) + + "(%rbp)";
                OutputFile.text << movy;
                this->intArgsCounter++;
            };

            this->GenArgs(func->args, OutputFile);
            OutputFile.linker.push(link);

            asmc::File file = this->GenSTMT(func->statment);
            OutputFile << file;
            
            // align the stack
            int align = 16;
            if(this->SymbolTable.count  > 0){
                align = ((this->SymbolTable.peek().byteMod + 15) / 16) * 16;
            }

            asmc::Subq * sub = new asmc::Subq;
            sub->op1 = "$" + std::to_string(align);
            sub->op2 = this->registers["%rsp"]->get(asmc::QWord);
            OutputFile.text.insert(sub, AlignmentLoc + 1);
            
            this->scope = saveScope;
            this->globalScope = saveGlobal;
            this->inFunction = false;
        }
        delete(func);

        if(!isLambda){
            this->SymbolTable.clear();
            this->SymbolTable.head = nullptr;
        }
        this->intArgsCounter = saveIntArgs;
    }
    else if (dynamic_cast<AST::Declare *>(STMT) != nullptr){
        /*
            movl $0x0, -[SymbolT + size](rdp)
            **also needs to be added to symbol table**
        */
        
        AST::Declare * dec =  dynamic_cast<AST::Declare *>(STMT);
        int offset = this->getBytes(dec->type.size);
        links::LinkedList<gen::Symbol>  * Table;
        
        if (!this->globalScope){
            // if the there  is no scope or in a function use the main SymbolTable
            if(this->scope == nullptr || this->inFunction) Table = &this->SymbolTable;
            else{
                // add the symbol to the class symbol table
                Table = &this->scope->SymbolTable;
            }

            if(Table->search<std::string>(searchSymbol, dec->Ident) != nullptr) throw err::Exception("redefined veriable: " + dec->Ident);

            gen::Symbol Symbol;
            if (Table->head == nullptr){
                Symbol.byteMod = offset;
            }else{
                Symbol.byteMod = Table->head->data.byteMod + offset;
            }
            Symbol.type = dec->type;
            Symbol.symbol = dec->Ident;
            Table->push(Symbol);
            // if the symbol is public add it to the public symbol table
            if(dec->scope == AST::Public && this->scope != nullptr) this->scope->publicSymbols.push(Symbol);
        } else{
            Table = &this->GlobalSymbolTable;
            asmc::LinkTask * var = new asmc::LinkTask();
            asmc::Lable * lable = new asmc::Lable();
            if(Table->search<std::string>(searchSymbol, dec->Ident) != nullptr) throw err::Exception("redefined global veriable: " + dec->Ident);
            
            lable->lable = dec->Ident;
            if (dec->type.size = asmc::QWord){
                var->command = "quad";
            };
            gen::Symbol Symbol;

            Symbol.type = dec->type;
            Symbol.symbol = dec->Ident;
            OutputFile.bss << lable;
            OutputFile.bss << var;
            Table->push(Symbol);
        }
        
    }
    else if(dynamic_cast<AST::DecArr *>(STMT) != nullptr){
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

    }
    else if (dynamic_cast<AST::DecAssign *>(STMT) != nullptr){
        /*
            movl $0x0, -[SymbolT + size](rdp)
            **also needs to be added to symbol table**
        */
        AST::DecAssign * decAssign =  dynamic_cast<AST::DecAssign *>(STMT);
        AST::Declare * dec = decAssign->declare;
        int offset = this->getBytes(dec->type.size);
        
        links::LinkedList<gen::Symbol>  * Table;
        Table = &this->SymbolTable;

        if(!this->globalScope){
        
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

            asmc::Mov * mov = new asmc::Mov();
            gen::Expr expr = this->GenExpr(decAssign->expr, OutputFile);
            mov->op = expr.op;
            mov->size = expr.size;
            mov->from = expr.access;
            mov->to = "-" + std::to_string(symbol.byteMod) + "(%rbp)";
            OutputFile.text << mov;
        }
        else{
            gen::Symbol Symbol;

            Symbol.type = dec->type;
            Symbol.symbol = dec->Ident;
            Table = &this->GlobalSymbolTable;

            asmc::LinkTask * var = new asmc::LinkTask();
            asmc::Lable * lable = new asmc::Lable();

            lable->lable = dec->Ident;
            if (dec->type.size = asmc::QWord){
                var->command = "quad";
            }
            gen::Expr exp = this->GenExpr(decAssign->expr, OutputFile);
            var->operand = exp.access.erase(0, 1);
            Symbol.type.opType = exp.op;
            OutputFile.data << lable;
            OutputFile.data << var;
            if(Table->search<std::string>(searchSymbol, dec->Ident) != nullptr) throw err::Exception("redefined veriable:" + dec->Ident);
            Table->push(Symbol);
        };

    }
    else if (dynamic_cast<AST::Return *>(STMT) != nullptr){
        /*
            mov [this.GenExpr(ret.value)]
            pop rbp
            ret
        */

        AST::Return * ret = dynamic_cast<AST::Return *>(STMT);

        asmc::Mov * mov = new asmc::Mov();
        mov->size = this->returnType.size;
        mov->from = this->GenExpr(ret->expr, OutputFile).access;
        mov->to = this->registers["%rax"]->get(this->returnType.size);
        OutputFile.text << mov;
        
        asmc::Return * re = new asmc::Return();
        OutputFile.text << re;

    }
    else if (dynamic_cast<AST::Assign *>(STMT) != nullptr){
        AST::Assign * assign = dynamic_cast<AST::Assign *>(STMT);
        bool global = false;

        links::LinkedList<gen::Symbol>  * Table = &this->SymbolTable;
        
        
        Symbol * symbol = Table->search<std::string>(searchSymbol, assign->Ident);
        if(symbol == nullptr) {
            Table = &this->GlobalSymbolTable;
            symbol = Table->search<std::string>(searchSymbol, assign->Ident);
            if(symbol == nullptr) throw err::Exception("unknown name: " + assign->Ident);
            global = true;
        };
        asmc::Mov * mov = new asmc::Mov();
        asmc::Mov * mov2 = new asmc::Mov();
        gen::Expr expr = this->GenExpr(assign->expr, OutputFile);
        mov->op = expr.op;
        mov2->op = expr.op;
        mov->size = expr.size;
        mov2->size = expr.size;
        mov2->from = expr.access;

        if(expr.op == asmc::Float) mov2->to = this->registers["%xmm0"]->get(expr.size);
        else mov2->to = this->registers["%rbx"]->get(expr.size);
        mov->from = mov2->to;

        assign->modList.invert();
        int tbyte = 0;
        
        AST::Type last = symbol->type;
        asmc::Size size;
        std::string output;
        if(global) output = symbol->symbol; 
        else output = "-" + std::to_string(symbol->byteMod) + "(%rbp)";

        while(assign->modList.head != nullptr){
            if(this->typeList[last.typeName] == nullptr) throw err::Exception("type not found " + last.typeName);
            gen::Type type = **this->typeList[last.typeName];
             gen::Symbol * modSym;
             // store the next dot mod in case of an error
             std::string sto = assign->modList.peek();
                    if(this->scope == *this->typeList[last.typeName]){
                        // if we are scoped to the type seache the symbol in the type symbol table
                         modSym = type.SymbolTable.search<std::string>(searchSymbol, assign->modList.pop());
                    }else{
                        // if we are not scoped to the type search the symbol in the public symbol table
                        modSym = type.publicSymbols.search<std::string>(searchSymbol, assign->modList.pop());
                    };
                    if(modSym == nullptr) throw err::Exception("unknown name: " + last.typeName + "." + sto);
            last = modSym->type;
            tbyte = modSym->byteMod;

            asmc::Mov * mov7 = new asmc::Mov();
            mov7->size = asmc::QWord;
            mov7->to = this->registers["%edx"]->get(asmc::QWord);
            mov7->from =  output;
            OutputFile.text << mov7;
            output = std::to_string(tbyte - this->getBytes(last.size)) + '(' + mov7->to + ')';
            size = last.size;
        }


        if(assign->refrence == true){
            asmc::Mov * m1 = new asmc::Mov;
            m1->from = output;
            m1->size = asmc::QWord;
            m1->to = this->registers["%eax"]->get(asmc::QWord);
            mov->to = "(" + this->registers["%eax"]->get(asmc::QWord) + ")";
            OutputFile.text << m1;
        }else mov->to = output;

        OutputFile.text << mov2;
        OutputFile.text << mov;

    
    }
    else if (dynamic_cast<AST::Call *>(STMT) != nullptr){
        AST::Call * call = dynamic_cast<AST::Call *>(STMT);
        this->GenCall(call, OutputFile);
    }
    else if (dynamic_cast<AST::Push *>(STMT) != nullptr){
        AST::Push * push = dynamic_cast<AST::Push *>(STMT);
        asmc::Mov * count = new asmc::Mov;

        count->size = asmc::QWord;
        count->to = this->registers["%rdx"]->get(count->size);
        count->from = "$1";
        asmc::Mov * pointer = new asmc::Mov;
        pointer->size = asmc::QWord;
        pointer->to = this->registers["%rsi"]->get(pointer->size);
        pointer->from = this->GenExpr(push->expr, OutputFile).access;

        asmc::Mov * callnum = new asmc::Mov;
        callnum->size = asmc::QWord;
        callnum->to = this->registers["%rax"]->get(callnum->size);
        callnum->from = "$1";

        asmc::Mov * rdi = new asmc::Mov;
        rdi->size = asmc::QWord;
        rdi->from = "$1";
        rdi->to = this->registers["%rdi"]->get(rdi->size);

        OutputFile.text << rdi;
        OutputFile.text << pointer;
        OutputFile.text << count;
        OutputFile.text << callnum;
        
        OutputFile.text << new asmc::SysCall;
    }
    else if (dynamic_cast<AST::Pull *>(STMT) != nullptr){
        AST::Pull * pull = dynamic_cast<AST::Pull *>(STMT);
        asmc::Mov * count = new asmc::Mov;
        count->size = asmc::QWord;
        count->to = this->registers["%rdx"]->get(count->size);
        count->from = "$1";
        asmc::Mov * pointer = new asmc::Mov;
        pointer->size = asmc::QWord;
        pointer->to = this->registers["%rsi"]->get(pointer->size);
        pointer->from = this->GenExpr(pull->expr, OutputFile).access;
        asmc::Mov * callnum = new asmc::Mov;
        callnum->size = asmc::QWord;
        callnum->to = this->registers["%rax"]->get(callnum->size);
        callnum->from = "$0";
        asmc::Mov * rdi = new asmc::Mov;
        rdi->size = asmc::QWord;
        rdi->from = "$1";
        rdi->to = this->registers["%rdi"]->get(rdi->size);

        OutputFile.text << rdi;
        OutputFile.text << pointer;
        OutputFile.text << count;
        OutputFile.text << callnum;
        
        OutputFile.text << new asmc::SysCall;
    }
    else if (dynamic_cast<AST::If *>(STMT) != nullptr){
        AST::If ifStmt = *dynamic_cast<AST::If *>(STMT);

        asmc::Lable * lable1 = new asmc::Lable();
        lable1->lable = ".L"+ this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount ++;
        asmc::Lable * lableElse = new asmc::Lable();
        lableElse->lable = ".L" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount ++;
        asmc::Lable * endElse = new asmc::Lable();
        endElse->lable = ".L" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount ++;

        if(ifStmt.elseStatment != nullptr){
            asmc::Jmp * jmpStart = new asmc::Jmp();
            jmpStart->to = endElse->lable;
            OutputFile.text << jmpStart;

            // place the Else Lable
            OutputFile.text << lableElse;
            OutputFile << this->GenSTMT(ifStmt.elseStatment);
            // jmp to lable 1
            asmc::Jmp * jmp = new asmc::Jmp();
            jmp->to = lable1->lable;
            OutputFile.text << jmp;
            OutputFile.text << endElse;
        }

        gen::Expr expr1 =this->GenExpr(ifStmt.Condition->expr1, OutputFile);
        gen::Expr expr2 =this->GenExpr(ifStmt.Condition->expr2, OutputFile);
    
        asmc::Mov * mov1 = new asmc::Mov();
        asmc::Mov * mov2 = new asmc::Mov();

        mov1->size = expr1.size;
        mov2->size = expr2.size;

        mov1->from = expr1.access;
        mov2->from = expr2.access;

        mov1->to = this->registers["%eax"]->get(mov1->size);
        mov2->to = this->registers["%rcx"]->get(mov2->size);

        mov2->size = mov2->size;

        asmc::Cmp * cmp = new asmc::Cmp();
        asmc::Jne * jne = new asmc::Jne();

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
            if (ifStmt.elseStatment != nullptr) {jne->to = lableElse->lable;} else jne->to = lable1->lable;
            
            OutputFile.text << jne;
            OutputFile << this->GenSTMT(ifStmt.statment);
            if (ifStmt.elseStatment != nullptr){
                asmc::Je * je = new asmc::Je();
                je->to = lableElse->lable;
                OutputFile.text << je;
            }
            OutputFile.text << lable1;
            break;
        }
        case AST::NotEqu :
        {
            asmc::Je * je = new asmc::Je();

            if (ifStmt.elseStatment != nullptr) {je->to = lableElse->lable;} else je->to = lable1->lable;

            OutputFile.text << je;
            OutputFile << this->GenSTMT(ifStmt.statment);
            OutputFile.text << lable1;
            break;
        }
        case AST::Great :
        {
            asmc::Jle * jl = new asmc::Jle();

             if (ifStmt.elseStatment != nullptr) {jl->to = lableElse->lable;} else jl->to = lable1->lable;


            OutputFile.text << jl;
            OutputFile << this->GenSTMT(ifStmt.statment);
            OutputFile.text << lable1;
            break;
        }
        case AST::Less :
        {
            asmc::Jge * jg = new asmc::Jge();

            if (ifStmt.elseStatment != nullptr){jg->to = lableElse->lable;} else jg->to = lable1->lable;


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

        asmc::Lable * lable1 = new asmc::Lable();
        lable1->lable =".L" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;

        asmc::Lable * lable2 = new asmc::Lable();
        lable2->lable = ".L" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;

        asmc::Jmp * jmp = new asmc::Jmp();
        jmp->to = lable2->lable;
        OutputFile.text << jmp;

        OutputFile.text << lable1;

        OutputFile << this->GenSTMT(loop->stmt);

        OutputFile.text << lable2;

        gen::Expr expr1 =this->GenExpr(loop->condition->expr1, OutputFile);
        gen::Expr expr2 =this->GenExpr(loop->condition->expr2, OutputFile);
    
        asmc::Mov * mov1 = new asmc::Mov();
        asmc::Mov * mov2 = new asmc::Mov();

        mov1->size = expr1.size;
        mov2->size = expr2.size;

        mov1->from = expr1.access;
        mov2->from = expr2.access;

        mov1->to = this->registers["%eax"]->get(mov1->size);
        mov2->to = this->registers["%ecx"]->get(mov2->size);

        asmc::Cmp * cmp = new asmc::Cmp();
        asmc::Jne * jne = new asmc::Jne();

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
            asmc::Je * je = new asmc::Je();
            je->to = lable1->lable;
            OutputFile.text << je;
            break;
        }
        case AST::NotEqu :
        {
            asmc::Jne * jne = new asmc::Jne();

            jne->to = lable1->lable;
            OutputFile.text << jne;
            break;
        }
        case AST::Great :
        {
            asmc::Jg * jg = new asmc::Jg();

            jg->to = lable1->lable;

            OutputFile.text << jg;
            break;
        }
        case AST::Less :
        {
            asmc::Jl * jl = new asmc::Jl();

            jl->to = lable1->lable;
            OutputFile.text << jl;
            break;
        }
        }
        
    }
    else if (dynamic_cast<AST::For *>(STMT) != nullptr){
        AST::For * loop = dynamic_cast<AST::For *>(STMT);

        asmc::Lable * lable1 = new asmc::Lable();
        lable1->lable =".L" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;

        asmc::Lable * lable2 = new asmc::Lable();
        lable2->lable = ".L" + this->nameTable.head->data.ident.ident + std::to_string(this->lablecount);
        this->lablecount++;
        OutputFile << this->GenSTMT(loop->declare);
        asmc::Jmp * jmp = new asmc::Jmp();
        jmp->to = lable2->lable;
        OutputFile.text << jmp;

        OutputFile.text << lable1;

        OutputFile << this->GenSTMT(loop->Run);
        OutputFile << this->GenSTMT(loop->increment);

        OutputFile.text << lable2;

        gen::Expr expr1 =this->GenExpr(loop->condition->expr1, OutputFile);
        gen::Expr expr2 =this->GenExpr(loop->condition->expr2, OutputFile);
    
        asmc::Mov * mov1 = new asmc::Mov();
        asmc::Mov * mov2 = new asmc::Mov();

        mov1->size = expr1.size;
        mov2->size = expr2.size;

        mov1->from = expr1.access;
        mov2->from = expr2.access;

        mov1->to = this->registers["%eax"]->get(mov1->size);
        mov2->to = this->registers["%ecx"]->get(mov2->size);

        asmc::Cmp * cmp = new asmc::Cmp();
        asmc::Jne * jne = new asmc::Jne();

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
            asmc::Je * je = new asmc::Je();
            je->to = lable1->lable;
            OutputFile.text << je;
            break;
        }
        case AST::NotEqu :
        {
            asmc::Jne * jne = new asmc::Jne();

            jne->to = lable1->lable;
            OutputFile.text << jne;
            break;
        }
        case AST::Great :
        {
            asmc::Jg * jg = new asmc::Jg();

            jg->to = lable1->lable;

            OutputFile.text << jg;
            break;
        }
        case AST::Less :
        {
            asmc::Jl * jl = new asmc::Jl();

            jl->to = lable1->lable;
            OutputFile.text << jl;
            break;
        }
        }
    }
    else if(dynamic_cast<AST::UDeffType *>(STMT) != nullptr){
        AST::UDeffType * udef = dynamic_cast<AST::UDeffType *>(STMT);
        gen::Type * type = new gen::Type();
        bool saveScope = this->globalScope;
        this->globalScope = false;
        type->Ident = udef->ident.ident;
        type->SymbolTable  = this->GenTable(udef->statment, type->SymbolTable);
        this->typeList.push(type);
        this->globalScope = saveScope;
    }
    else if(dynamic_cast<AST::Class *>(STMT) != nullptr){
        AST::Class * deff = dynamic_cast<AST::Class *>(STMT);
        gen::Class * type = new gen::Class();
        bool saveScope = this->globalScope;
        this->globalScope = false;
        type->Ident = deff->ident.ident;
        type->nameTable.foo = compairFunc;
        this->scope = type;
        type->SymbolTable;
        this->typeList.push(type);
        asmc::File file = this->GenSTMT(deff->statment);
        OutputFile << file;
        this->globalScope = saveScope;
        this->scope = nullptr;
    }
    else{
        OutputFile.text.push(new asmc::Instruction());
    }

    return OutputFile;
}