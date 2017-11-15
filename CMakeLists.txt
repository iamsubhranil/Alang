set(SOURCE_FILES    main.c
                    scanner.c
                    parser.c
                    interpreter.c
                    environment.c
                    allocator.c
                    io.c
                    preprocessor.c
                    native.c
                    foreign_interface.c)

add_executable(alang   ${SOURCE_FILES})
target_link_libraries(alang m
                            dl)
