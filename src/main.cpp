#include <fstream>
#include <iostream>
#include <istream>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

int main(const int argc, const char *const *argv)
{
    if (argc != 2)
        return 1;

    std::ifstream stream(argv[1]);
    if (!stream.is_open())
        return 1;

    llove::Context types;
    llove::Parser parser(types, stream);
    llove::Builder builder(types);

    while (parser.Ok())
        if (auto ptr = parser.Parse())
        {
            std::cerr << ptr << std::endl;
            ptr->Gen(builder);
        }

    builder.Gen("out.o");

    stream.close();
    return 0;
}
