/**
 * @file
 * @brief Source file with testlib implementation
 */
#include "testlib.h"

Test::Test(const TestPtr& function_ptr, const char* fileName, unsigned int line) {
    assert(function_ptr != nullptr);
    assert(fileName != nullptr);
    assert(line != 0);

    _function_ptr = function_ptr;
    _fileName = fileName;
    _line = line;
}

/**
 * Runs this test.
 */
void Test::run() const {
    _function_ptr();
}

/**
 * Name of the file this test was created in.
 * @return name of the file.
 */
const char* Test::fileName() const {
    return _fileName;
}

/**
 * Line of the file this test was created on.
 * @return line number of the file.
 */
unsigned int Test::line() const {
    return _line;
}

//----------------------------------------------------------------------------------------------------------------------

TestRunner::TestRunner() = default;

TestRunner::~TestRunner() {
    for (Test* test : allTests) {
        delete test;
    }
}

/**
 * Registers new test in this runner.
 * @param[in] testPtr  pointer to a test function
 * @param[in] fileName name of the file test declared in
 * @param[in] line     line number of the file test declared on
 * @return pointer to a created Test object.
 *
 */
Test* TestRunner::addTest(TestPtr testPtr, const char* fileName, unsigned int line) {
    Test* test = new Test(testPtr, fileName, line);
    allTests.push_back(test);
    return test;
}

/**
 * Removes all tests from the container.
 */
void TestRunner::clear() {
    allTests.clear();
}

/**
 * Runs all tests that exist in this runner. Use in the main() method to run every written test.
 * @return true, if all tests succeeded, false otherwise.
 */
bool TestRunner::runAllTests() {
    unsigned int passedTestsNumber = 0;
    unsigned int failedTestsNumber = 0;

    for (Test* test : allTests) {
        if (runTest(test)) {
            ++passedTestsNumber;
        } else {
            ++failedTestsNumber;
        }
    }

    if (passedTestsNumber > 0) {
        std::cerr << TESTLIB_ANSI_COLOR_GREEN;
        std::cerr << '\n' << passedTestsNumber << (passedTestsNumber == 1 ? " TEST" : " TESTS") << " PASSED\n";
    }
    if (failedTestsNumber > 0) {
        std::cerr << TESTLIB_ANSI_COLOR_RED;
        std::cerr << '\n' << failedTestsNumber << (failedTestsNumber == 1 ? " TEST" : " TESTS") << " FAILED\n";
    }
    std::cerr << TESTLIB_ANSI_COLOR_RESET;

    return failedTestsNumber == 0;
}

/**
 * Run the given test.
 * @param[in] test pointer to a test to run
 * @return true, if the test succeeded, false otherwise.
 */
bool TestRunner::runTest(Test* test) {
    assert(test != nullptr);

    _currentTest = test;

    test->run();

    if (_currentTest != nullptr) {
        _currentTest = nullptr;

        std::cerr << TESTLIB_ANSI_COLOR_GREEN;
        std::cerr << "[TEST PASSED] " << test->fileName() << ':' << test->line() << '\n';
        std::cerr << TESTLIB_ANSI_COLOR_RESET;

        return true;
    } else {
        return false;
    }
}

/**
 * Fails current test. Use in assertions to show that the current test is failed.
 */
void TestRunner::failCurrentTest() {
    _currentTest = nullptr;
}

/**
 * Pointer to a current test executed by this runner.
 * @return pointer to a current running test
 */
const Test* TestRunner::currentTest() const {
    return _currentTest;
}

/**
 * Returns a singleton instance of the runner.
 * @return singleton runner.
 */
TestRunner* TestRunner::getInstance() {
    static TestRunner runner;
    return &runner;
}
