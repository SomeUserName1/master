# Add commands to compile latex using xetex and biber
set(XETEX_COMMAND "xelatex -shell-escape -interaction=nonstopmode 
    ${TEX_MAIN_FILE}")

set(BIBER_COMMAND "biber ${BIBER_FILE}")

set(BUILD_TEX_CUSTOM_COMMAND 
    "COMMAND ${XELATEX_COMMAND}
     COMMAND ${BIBER_COMMAND}
     COMMAND ${XELATEX_COMMAND}
     COMMAND ${XELATEX_COMMAND}")

# Add documentation subdirectories
add_subdirectory(neo4jstorage)
add_subdirectory(thesis)

