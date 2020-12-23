/**
 * @file
 */
#include "testlib.h"

int main() {
    return TestRunner::getInstance()->runAllTests() ? 0 : -1;
}
