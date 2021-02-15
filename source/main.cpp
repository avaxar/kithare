#include "utility/std.hpp"
#include "utility/string.hpp"
#include "utility/utf8.hpp"
#include "lexer/lex.hpp"


bool SILENT_EXECUTION = false;
bool SILENT_COMPILATION = false;

int executeArgs(const std::vector<kh::String>& args) {
    if (args.size() < 3) {
        std::wcerr << 
            "Kithare: More arguments are required!\n"
            "  (./)kithare -c <source file> [-s SILENT_EXECUTION] [-s SILENCE_COMPILATION] [-o <output bytecode path>]\n"
            "  (./)kithare -r <bytecode file> ... {Optional arguments}\n";
        return 1;
    }

    if (!(args.size() % 2)) {
        std::wcerr << L"Kithare: More arguments are required!\n"
            << L"  ... " << kh::fromStringW(args[args.size() - 1]) << L" < Missing argument\n";
        return 1;
    }

    return 0;
}

#undef main
#undef wmain
#ifdef _WIN32
int wmain(const int argc, wchar_t* argv[])
#else
int main(const int argc, char* argv[])
#endif
{
    srand((unsigned int)time(NULL));
    std::setlocale(LC_ALL, "en_US.utf8");

    std::vector<kh::String> args;

    #ifdef _WIN32
    std::ios_base::sync_with_stdio(false);
    std::locale utf8(std::locale(), new std::codecvt_utf8_utf16<wchar_t>);
    std::wcout.imbue(utf8);

    for (int arg = 0; arg < argc; arg++)
        args.emplace_back((uint32*)argv[arg]);
    #else
    for (int arg = 0; arg < argc; arg++)
        args.push_back(kh::decodeUtf8(argv[arg]));
    #endif

    kh::String example_code = kh::toString(
        L"int main() {                                      \n"
        L"    List[<int>] list = someFunction();            \n"
        L"    int a_number = list[0] * (list[1] + list[2]); \n"
        L"    print(a_number);                              \n"
        L"}                                                 \n"
    );

    std::vector<kh::Token> tokens = kh::lex(example_code, kh::toString(L"<string>"));
    std::cout << "Amount of tokens: " << tokens.size() << '\n';

    for (auto& token : tokens)
        std::wcout << token << "\n";

    //int exit_code = executeArgs(args);
    while (true);
    //return exit_code;
}
