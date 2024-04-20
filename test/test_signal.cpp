// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include <ptl/signal.h>

#include "common.h"

using namespace ptl;

TEST_SUITE("signal") {

TEST_CASE( "signal name" ) {
    #ifndef _WIN32
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
