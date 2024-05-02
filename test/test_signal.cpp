// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/signal.h>

#include "common.h"

#if !defined(__CYGWIN__) && __has_include(<features.h>)
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
    #include <features.h>
    #ifndef __USE_GNU
        #define __MUSL__
    #endif
    #undef _GNU_SOURCE 
#else
    #include <features.h>
    #ifndef __USE_GNU
        #define __MUSL__
    #endif
#endif
#endif

using namespace ptl;

TEST_SUITE("signal") {

TEST_CASE( "signal name" ) {
    #if !defined(_WIN32) && !defined(__MUSL__) && !defined(__sun)
        CHECK(signalName(SIGINT) == "INT");
        CHECK(signalName(SIGKILL) == "KILL");
    #else
        CHECK(signalName(SIGINT) == std::to_string(SIGINT));
        CHECK(signalName(SIGTERM) == std::to_string(SIGTERM));
    #endif
}

static int handledSig = 0;

TEST_CASE( "simple handler" ) {
    #ifndef _WIN32
        constexpr int signo = SIGUSR1;
    #else
        constexpr int signo = SIGINT;
    #endif

    setSignalHandler(signo, [](int sig) {
        handledSig = sig;
    });
    handledSig = 0;
    #ifndef _WIN32
    sendSignal(getpid(), signo);
    CHECK(handledSig == signo);
    #endif
    
    //... on SVID systems the signal handler is deinstalled after signal delivery. 
    //On BSD systems the handler must be explicitly deinstalled.
    #ifdef __sun
    setSignalHandler(signo, [](int sig) {
        handledSig = sig;
    });
    #endif

    raiseSignal(signo);
    CHECK(handledSig == signo);
    
    setSignalHandler(signo, SIG_DFL);
}

#ifndef _WIN32
TEST_CASE( "action handler" ) {

    auto handler = [](int sig, siginfo_t *, void *) {
        handledSig = sig;
    };
    
    setSignalAction(SIGUSR1, SignalAction(handler));
    handledSig = 0;
    sendSignal(getpid(), SIGUSR1);
    CHECK(handledSig == SIGUSR1);
    {
        SignalAction ret;
        getSignalAction(SIGUSR1, ret);
        CHECK(ret.sa_sigaction == handler);
    }
    {
        SignalAction ret;
        setSignalAction(SIGUSR1, SignalAction(SIG_DFL), ret);
        CHECK(ret.sa_sigaction == handler);
    }
}
#endif

}
