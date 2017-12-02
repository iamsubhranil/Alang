set(SOURCE_FILES    main.c
                    scanner.c
                    parser.c
                    display.c
                    #                    allocator2.c
                    #                    preprocessor.c
                    routines.c
                    strings.c
                    interpreter.c
                    values.c
                    env.c
                    io.c
                    heap.c
                    callframe.c
                    native.c
                    foreign_interface.c)

add_executable(alang   ${SOURCE_FILES})
target_link_libraries(alang m
                            dl)

add_library(lalang SHARED native/nmath.c native/essentials.c)
target_link_libraries(lalang m)
