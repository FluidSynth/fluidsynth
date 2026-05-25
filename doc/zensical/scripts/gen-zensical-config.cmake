# CMake script to generate a runtime zensical.toml with all API pages listed in the nav.
#
# Reads TEMPLATE (zensical.toml.in), substitutes @API_NAV_ENTRIES@ with the
# list of .md files found under WIKI_DIR/api/, and writes the result to OUTPUT.
#
# Usage:
#   cmake -D WIKI_DIR=<path/to/doc/wiki>
#         -D OUTPUT=<path/to/zensical_runtime.toml>
#         -D TEMPLATE=<path/to/zensical.toml.in>
#         -P gen-zensical-config.cmake

# Priority ordering for well-known top-level API pages (everything else is sorted alphabetically).
# "examples.md" is intentionally excluded here; it becomes the index of the Examples subsection.
set(_priority_files "index.md" "recent-changes.md" "deprecated.md")

# Collect top-level API pages (exclude examples.md — it goes into the subsection below)
file(GLOB _api_top RELATIVE "${WIKI_DIR}/api" "${WIKI_DIR}/api/*.md")
list(SORT _api_top)
list(REMOVE_ITEM _api_top "examples.md")

# Build sorted list: priority pages first, then remaining alphabetically
set(_sorted_top "")
foreach(_pf IN LISTS _priority_files)
    list(FIND _api_top "${_pf}" _idx)
    if(NOT _idx EQUAL -1)
        list(APPEND _sorted_top "${_pf}")
    endif()
endforeach()
foreach(_n IN LISTS _api_top)
    list(FIND _priority_files "${_n}" _idx)
    if(_idx EQUAL -1)
        list(APPEND _sorted_top "${_n}")
    endif()
endforeach()

# Collect api/examples/ sub-pages (individual example files)
file(GLOB _api_examples RELATIVE "${WIKI_DIR}/api/examples" "${WIKI_DIR}/api/examples/*.md")
list(SORT _api_examples)

# Build the TOML nav entries string (one quoted entry per line, 16 spaces indent)
set(API_NAV_ENTRIES "")
foreach(_n IN LISTS _sorted_top)
    string(APPEND API_NAV_ENTRIES "                \"api/${_n}\",\n")
endforeach()

if(_api_examples)
    foreach(_n IN LISTS _api_examples)
        string(APPEND EXA_NAV_ENTRIES "                    \"api/examples/${_n}\",\n")
    endforeach()
endif()

if(NOT API_NAV_ENTRIES)
    message(FATAL "API_NAV_ENTRIES was empty, possibly doxygen XML to markdown conversion failed.")
endif()

if(NOT EXA_NAV_ENTRIES)
    message(FATAL "No examples have been discovered, though there should be some...")
endif()

# Substitute @API_NAV_ENTRIES@ in the template and write the runtime config
configure_file("${TEMPLATE}" "${OUTPUT}" @ONLY)
message(STATUS "Runtime zensical config written to ${OUTPUT}")
