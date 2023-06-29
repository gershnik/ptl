#include <catch2/catch_session.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/stat.h>

#include <ptl/signal.h>

using namespace ptl;

int main(int argc, char ** argv)
{
    #if defined (_WIN32)
        SetConsoleOutputCP(CP_UTF8);
    #endif
    umask(0);

    SignalAction act(SIG_DFL, SA_RESTART);
    setSignalAction(SIGCHLD, act);

    return Catch::Session().run( argc, argv );
}
