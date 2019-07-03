enable_testing()
add_executable(test_averager tests/AveragerTest.cpp)
add_test(TestAverager test_averager)

# The `test` target doesn't build the test exes even though they would be missing.
# Use a custom target `check` instead.
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS test_averager)
