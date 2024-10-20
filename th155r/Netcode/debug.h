#pragma once

#ifndef DEBUG_H
#define DEBUG_H 1

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include "kite_api.h"

bool CompileScriptBuffer(HSQUIRRELVM v, const char *Src, const char *to);
HSQOBJECT SQGetObjectByName(HSQUIRRELVM v, const SQChar *name);

#endif