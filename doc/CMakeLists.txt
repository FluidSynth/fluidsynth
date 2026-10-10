# FluidSynth - A Software Synthesize
#
# Copyright (C) 2003-2010 Peter Hanappe and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA

# CMake based build system. Pedro Lopez-Cabanillas <plcl@users.sf.net>

find_package ( Doxygen )
if ( DOXYGEN_FOUND )
  if ( DOXYGEN_VERSION VERSION_GREATER_EQUAL "1.9.8" )
    set ( DOC_API_GROUP_TYPE "topics" )
  else ( DOXYGEN_VERSION VERSION_GREATER_EQUAL "1.9.8" )
    set ( DOC_API_GROUP_TYPE "modules" )
  endif ( DOXYGEN_VERSION VERSION_GREATER_EQUAL "1.9.8" )

  if ( DOXYGEN_VERSION VERSION_GREATER_EQUAL "1.9.5" )
    set ( DOC_COLORSTYLE_CONFIG "HTML_COLORSTYLE = LIGHT" )
  endif ( DOXYGEN_VERSION VERSION_GREATER_EQUAL "1.9.5" )

  configure_file ( Doxyfile.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
  configure_file ( doxygen/layout.xml.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/layout.xml )
  add_custom_target ( doxygen
    ${DOXYGEN} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  find_package ( LibXslt )
  if ( LIBXSLT_XSLTPROC_EXECUTABLE )
      add_custom_target ( doxygen_settings
          ${LIBXSLT_XSLTPROC_EXECUTABLE}
              --output ${CMAKE_CURRENT_BINARY_DIR}/fluidsettings.txt
              ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/fluidsettings.xsl
              ${CMAKE_CURRENT_SOURCE_DIR}/fluidsettings.xml
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
      add_dependencies(doxygen doxygen_settings)

      # -- XSLT-based Zensical targets --
      set ( XSLT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zensical/xslt )

      # Convert fluidsettings.xml to Markdown using XSLT
      # Output goes directly into the wiki docs_dir so zensical.toml picks it up
      add_custom_target ( fluidsettings2md
          COMMAND ${CMAKE_COMMAND} -E make_directory
              ${CMAKE_CURRENT_SOURCE_DIR}/wiki/settings
          COMMAND bash ${XSLT_DIR}/run-xslt.sh
              fluidsettings
              ${CMAKE_CURRENT_SOURCE_DIR}/fluidsettings.xml
              ${CMAKE_CURRENT_SOURCE_DIR}/wiki/settings
              ${XSLT_DIR}/fluidsettings2md.xsl
          COMMENT "Converting fluidsettings.xml to Markdown via XSLT"
      )

      # Convert Doxygen XML output to API Markdown using XSLT
      # Output goes directly into the wiki docs_dir so zensical.toml picks it up
      add_custom_target ( doxy2md
          COMMAND ${CMAKE_COMMAND} -E make_directory
              ${CMAKE_CURRENT_SOURCE_DIR}/wiki/api
          COMMAND bash ${XSLT_DIR}/run-xslt.sh
              doxy
              ${CMAKE_CURRENT_BINARY_DIR}/api/xml
              ${CMAKE_CURRENT_SOURCE_DIR}/wiki/api
              ${XSLT_DIR}/doxy2md.xsl
              ${CMAKE_CURRENT_SOURCE_DIR}/fluidsettings.xml
          DEPENDS doxygen
          COMMENT "Converting Doxygen XML to API reference Markdown via XSLT"
      )

  endif ( LIBXSLT_XSLTPROC_EXECUTABLE )

  # ---------------------------------------------------------------------------
  # Zensical documentation site
  #
  # The site requires Python 3 plus the packages listed in
  # doc/zensical/requirements.txt.  Install them once with:
  #   pip install -r doc/zensical/requirements.txt
  #
  # Build targets (XSLT pipeline):
  #   fluidsettings2md -- convert fluidsettings.xml to settings Markdown (XSLT)
  #   doxy2md          -- convert Doxygen XML group output to API reference Markdown (XSLT)
  #   usage2md              -- convert Doxygen page XML to usage guide Markdown (XSLT)
  #   recent_changes2md     -- convert RecentChanges page XML to api/recent-changes.md (XSLT)
  #   zensical         -- run all of the above, then build the unified wiki+API site
  # ---------------------------------------------------------------------------
  find_program ( ZENSICAL_EXECUTABLE NAMES zensical )

  set ( WIKI_DOCS_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/wiki )
  set ( ZENSICAL_SCRIPTS  ${CMAKE_CURRENT_SOURCE_DIR}/zensical/scripts )

  if ( LIBXSLT_XSLTPROC_EXECUTABLE )

    # -- Convert usage \page XML to usage guide Markdown (XSLT) ---------------
    # Processes all usage guide pages from Doxygen page XML (built by doxygen target).
    # Skips RecentChanges (handled by recent_changes2md below).
    # The api_prefix "../api/" is passed so that cross-references from usage/
    # pages to API symbols resolve to ../api/<group>.md#<symbol>.
    # NOTE: CMake strips empty-string arguments from COMMAND lists. Use the
    #       sentinel "_nofilter_" (handled by run-xslt.sh) to mean "no filter".
    add_custom_target ( usage2md
      COMMAND ${CMAKE_COMMAND} -E make_directory ${WIKI_DOCS_DIR}/usage
      COMMAND bash ${XSLT_DIR}/run-xslt.sh
          pages
          ${CMAKE_CURRENT_BINARY_DIR}/api/xml
          ${WIKI_DOCS_DIR}/usage
          ${XSLT_DIR}/doxy2md.xsl
          "../api/"
          "_nofilter_"
          ${CMAKE_CURRENT_SOURCE_DIR}/fluidsettings.xml
      DEPENDS doxygen
      COMMENT "Converting Doxygen page XML to usage guide Markdown via XSLT"
    )

    # -- Convert RecentChanges page XML to api/recent-changes.md (XSLT) -------
    # Uses the same pages mode but filters to only the RecentChanges page.
    # API prefix is "./" (already in the api/ directory).
    add_custom_target ( recent_changes2md
      COMMAND ${CMAKE_COMMAND} -E make_directory ${WIKI_DOCS_DIR}/api
      COMMAND bash ${XSLT_DIR}/run-xslt.sh
          pages
          ${CMAKE_CURRENT_BINARY_DIR}/api/xml
          ${WIKI_DOCS_DIR}/api
          ${XSLT_DIR}/doxy2md.xsl
          "./"
          "RecentChanges"
          ${CMAKE_CURRENT_SOURCE_DIR}/fluidsettings.xml
      DEPENDS doxygen
      COMMENT "Converting RecentChanges page XML to Markdown via XSLT"
    )

    # -- Generate API examples page --------------------------------------------
    add_custom_target ( examples2md
      COMMAND bash ${XSLT_DIR}/run-xslt.sh
          examples
          ${CMAKE_CURRENT_SOURCE_DIR}/examples
          ${WIKI_DOCS_DIR}/api
      COMMENT "Generating API examples page"
    )

  endif ( LIBXSLT_XSLTPROC_EXECUTABLE )

  # -- Build the unified wiki + API documentation site (XSLT pipeline) ------
  # All content is generated into doc/wiki/ by the targets above.
  # A CMake script (gen-zensical-config.cmake) reads zensical.toml.in, fills in
  # @API_NAV_ENTRIES@ with the list of generated API pages (so the sidebar and
  # breadcrumbs work for every page), and writes a runtime config to the PROJECT
  # SOURCE ROOT (not the build dir).
  # Writing it there ensures that docs_dir = "doc/wiki" in the config resolves
  # correctly relative to the config file – zensical requires docs_dir to be
  # a relative path.  The file is gitignored via the root .gitignore.
  if ( LIBXSLT_XSLTPROC_EXECUTABLE AND ZENSICAL_EXECUTABLE )
    set ( ZENSICAL_RUNTIME_CONFIG ${CMAKE_SOURCE_DIR}/zensical_runtime.toml )
    if ( DOXYGEN_VERSION VERSION_LESS "1.15.0" )
      message ( WARNING "Your doxygen version ${DOXYGEN_VERSION} is old. This logic has only been tested to work correctly with doxygen 1.15 and newer." )
    endif ()

    add_custom_target ( zensical
        COMMAND ${CMAKE_COMMAND}
            -D WIKI_DIR=${WIKI_DOCS_DIR}
            -D OUTPUT=${ZENSICAL_RUNTIME_CONFIG}
            -D TEMPLATE=${CMAKE_SOURCE_DIR}/zensical.toml.in
            -P ${ZENSICAL_SCRIPTS}/gen-zensical-config.cmake
        COMMAND ${ZENSICAL_EXECUTABLE} build --clean
            -f ${ZENSICAL_RUNTIME_CONFIG}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS fluidsettings2md doxy2md
                usage2md recent_changes2md examples2md
        COMMENT "Building unified wiki and API documentation site (XSLT pipeline)"
    )
  elseif ( NOT LIBXSLT_XSLTPROC_EXECUTABLE )
    message ( STATUS "xsltproc not found -- 'zensical' CMake target unavailable.\n"
                     "Install it with: sudo apt-get install xsltproc" )
  else ()
    message ( STATUS "zensical executable not found -- 'zensical' CMake target unavailable.\n"
                     "Install it with: pip install -r doc/zensical/requirements.txt" )
  endif ()

endif ( DOXYGEN_FOUND )


if ( UNIX )
  install ( FILES fluidsynth.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
    COMPONENT ${PACKAGE}_program
  )
endif ( UNIX )

add_subdirectory ( examples )
