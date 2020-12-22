# Taget for the report on the Neo4J model & implementation
set(TEX_MAIN_FILE 00-main.tex)
SET(BIBER_FILE 00-main.bcf)
# add compile commands for the targe# add compile commands for the targett

add_custom_command(OUTPUT thesis.pdf 
                   COMMAND "${BUILD_TEX_CUSTOM_COMMAND}")
