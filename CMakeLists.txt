set(SOURCE_FILES    main.c
                    display.c
                    scanner.c
                    parser.c
                    interpreter.c
                    environment.c
                    allocator.c
                    io.c)

add_executable(alconv   ${SOURCE_FILES})
target_link_libraries(alconv m)
