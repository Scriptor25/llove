#include <llove/builder.hpp>
#include <llove/parameter.hpp>

std::ostream &llove::Parameter::Print(std::ostream &stream) const
{
    return Info.Print(stream, true, Name);
}

bool llove::Parameter::TypeInfo(Builder &builder, std::vector<llvm::Constant *> &dst) const
{
    dst.push_back(builder.GetStr(Name));
    return Info.TypeInfo(builder, dst);
}

void llove::Parameter::Reflect(Context &context, Parameter &parameter) const
{
    parameter.Name = Name;
    Info.Reflect(context, parameter.Info);
}
