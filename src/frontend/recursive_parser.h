/**
 * @file
 * @brief Definition of calculator that uses recursive parser
 */
#ifndef RECURSIVE_PARSER_CALCULATOR_H
#define RECURSIVE_PARSER_CALCULATOR_H

#include <cstring>
#include <map>
#include <memory>
#include "ast.h"
#include "tokenizer.h"

std::shared_ptr<StatementsNode> buildASTRecursively(char* expression);

#endif // RECURSIVE_PARSER_CALCULATOR_H
