idf_component_register(SRCS
                         "src/ps4.c"
                         "src/ps4_gap.c"
                         "src/ps4_spp.c"
                         "src/ps4_parser.c"
                       INCLUDE_DIRS
                         "src/."
                       REQUIRES bt)