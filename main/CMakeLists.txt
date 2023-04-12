set(srcs "main.c"
        "nus_service.c"
        )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
