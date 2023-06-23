#include <catch2/catch_session.hpp>

#ifdef _WIN32
#include <windows.h>
#endif


int main(int argc, char ** argv)
{
    #if defined (_WIN32)
        SetConsoleOutputCP(CP_UTF8);
    #endif

    return Catch::Session().run( argc, argv );
}
