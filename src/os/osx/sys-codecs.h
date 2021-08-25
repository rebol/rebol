#pragma once

#include "sys-utils.h"
#include "reb-codec.h"

int DecodeImageFromFile(const char *uri, unsigned int frame, REBCDI *codi);
int EncodeImageToFile(const char *uri, REBCDI *codi);
