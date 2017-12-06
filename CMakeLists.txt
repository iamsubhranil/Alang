set(SOURCE_FILES    main.c
                    scanner.c
                    parser.c
                    display.c
                    routines.c
                    strings.c
                    interpreter.c
                    values.c
                    io.c
                    native.c
                    foreign_interface.c)

add_executable(alang   ${SOURCE_FILES})
target_link_libraries(alang m
                            dl)

add_library(lalang SHARED native/nmath.c)
target_link_libraries(lalang m)
