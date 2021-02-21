#include "utility/std.hpp"
#include "utility/string.hpp"
#include "utility/utf8.hpp"
#include "parser/parse.hpp"


bool SILENT_EXECUTION = false;
bool SILENT_EXCEPTION = false;
bool SILENT_COMPILATION = false;

int executeArgs(const std::vector<kh::String>& args) {
    std::cout << "Test!\n";
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

    int exit_code = executeArgs(args);
    std::cin.get();
    return exit_code;
}
