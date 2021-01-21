/**
 * @file
 * @brief Definition of recursive parser
 */
#ifndef COMPILER_RECURSIVE_PARSER_H
#define COMPILER_RECURSIVE_PARSER_H

#include <cstring>
#include <map>
#include <memory>
#include "ast.h"
#include "tokenizer.h"

std::shared_ptr<StatementsNode> buildASTRecursively(char* expression);

#endif // COMPILER_RECURSIVE_PARSER_H
