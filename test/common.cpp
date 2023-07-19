// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include "common.h"

#include <ptl/spawn.h>

using namespace ptl;

#ifndef _WIN32

std::string shell(const StringRefArray & args) {
    auto [readPipe, writePipe] = Pipe::create();

    #if !defined(__ANDROID__) || (defined(__ANDROID__) && __ANDROID_API__ >= 28)
        SpawnFileActions fa;
        fa.addClose(readPipe);
        fa.addDuplicateTo(writePipe, stdout);
        auto child = spawn(args, SpawnSettings().fileActions(fa).usePath());
    #else 
        auto child = forkProcess();
        if (!child) {
            readPipe.close();
            duplicateTo(writePipe, stdout);
            execp(args);
        }
    #endif
    writePipe.close();
    auto ret = rtrim(readAll(readPipe));
    child.wait();
    return ret;
}

#endif
