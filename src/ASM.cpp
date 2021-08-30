#include "ASM.hpp"

std::string ASMC::Instruction::toString(){
    return("");
}

std::string ASMC::Lable::toString(){
    return this->lable + ":\n";
}

std::string ASMC::LinkTask::toString(){
    return  "." + this->command + "\t" + this->operand + "\n";
}

std::string ASMC::Call::toString(){
    return "\tcall\t" + this->function + "\n";
}

std::string ASMC::Mov::toString(){
    return "\tmov\t" + this->from + ", " + this->to + "\n";
}

std::string ASMC::Movq::toString(){
    return "\tmovl\t" + this->from + ", " + this->to + "\n";
}

std::string ASMC::Add::toString(){
    return "\tadd\t" + this->op1 + ", " + this->op2 + "\n";
}

std::string ASMC::Push::toString(){
    return "\tpush\t" + this->op + "\n";
}

std::string ASMC::Pop::toString(){
    return "\tpop\t" + this->op + "\n";
}

std::string ASMC::Return::toString(){
    return "\tret\n";
}

void ASMC::File::operator<<(ASMC::File file){
    this->linker.stitch(file.linker);
    this->text.istitch(file.text);
}