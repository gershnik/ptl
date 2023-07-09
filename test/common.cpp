// Copyright (c) 2023, Eugene Gershnik
// SPDX-License-Identifier: BSD-3-Clause

#include "common.h"

#include <ptl/spawn.h>

using namespace ptl;

#ifndef _WIN32

std::string shell(const StringRefArray & args) {
    auto [readPipe, writePipe] = Pipe::create();
    SpawnFileActions fa;
    fa.addDuplicateTo(writePipe, stdout);
    auto child = spawn(args, SpawnSettings().fileActions(fa).usePath());
    writePipe.close();
    auto ret = rtrim(readAll(readPipe));
    child.wait();
    return ret;
}

#endif
