#include <ptl/signal.h>

#include <unistd.h>

#include <catch2/catch_test_macros.hpp>

#include "common.h"

using namespace ptl;

TEST_CASE( "signal name" , "[signal]") {
    CHECK(signalName(SIGINT) == "INT");
    CHECK(signalName(SIGKILL) == "KILL");
}

static int handledSig = 0;

TEST_CASE( "simple handler", "[signal]") {

    setSignalHandler(SIGUSR1, [](int sig) {
        handledSig = sig;
    });
    handledSig = 0;
    sendSignal(getpid(), SIGUSR1);
    CHECK(handledSig == SIGUSR1);
    setSignalHandler(SIGUSR1, SIG_DFL);
}

TEST_CASE( "action handler", "[signal]") {

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
