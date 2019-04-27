set(SOURCE_FILES    scanner.c
                    parser.c
                    display.c
                    routines.c
                    strings.c
                    interpreter.c
                    values.c
                    io.c
                    native.c
                    object.c
                    foreign_interface.c)

add_library(lalang SHARED ${SOURCE_FILES})
target_link_libraries(lalang dl m)

add_executable(alang   main.c)
target_link_libraries(alang lalang)

add_library(alang_math SHARED native/nmath.c)
target_link_libraries(alang_math lalang)

add_library(nfibo SHARED algos/nativefibonacci.c)
target_link_libraries(nfibo lalang)

add_library(vantest SHARED algos/varargnative.c)
target_link_libraries(vantest lalang)
