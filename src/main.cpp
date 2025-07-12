#include <fstream>
#include <istream>
#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>

int main(int argc, const char **argv)
{
    std::ifstream stream("example/example.lov");
    if (!stream.is_open())
        return 1;

    llove::Context context;
    llove::Parser parser(context, stream);
    while (parser.Ok())
        parser.Parse();

    stream.close();
    return 0;
}
