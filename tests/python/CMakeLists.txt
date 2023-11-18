find_program(PYTHON_EXE python)
message("-- Found python at location " ${PYTHON_EXE})
set(test_name test_py_both)
add_test(${test_name} ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py ${CMAKE_CURRENT_SOURCE_DIR})