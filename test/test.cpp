// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#define DOCTEST_CONFIG_IMPLEMENT

#include <doctest.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#define umask _umask
#endif

#include <sys/stat.h>

#include <ptl/signal.h>

using namespace ptl;

int main(int argc, char ** argv)
{
    umask(0);

    #if defined (_WIN32)
        SetConsoleOutputCP(CP_UTF8);
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    #else
        SignalAction act(SIG_DFL, SA_RESTART);
        setSignalAction(SIGCHLD, act);
    #endif

    return doctest::Context(argc, argv).run();
}
