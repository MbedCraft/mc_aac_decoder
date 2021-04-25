idf_component_register( SRCS
                            "src/mc_aac.c"
                            "src/libhelix_HAACDECODER/aac_decoder.cpp"
                        INCLUDE_DIRS "inc"
                        PRIV_REQUIRES mc_assert)
