#pragma once

#ifndef DEBUG_H
#define DEBUG_H 1

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include "kite_api.h"

void sq_throwexception(HSQUIRRELVM v, const char* src);
bool CompileScriptBuffer(HSQUIRRELVM v, const char *Src, const char *to);
void show_tree(HSQUIRRELVM v, SQObject Root);
HSQOBJECT SQGetObjectByName(HSQUIRRELVM v, const SQChar *name);

#endif