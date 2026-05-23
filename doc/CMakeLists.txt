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
      add_custom_target ( fluidsettings2md_xslt
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
      add_custom_target ( doxy2md_xslt
          COMMAND ${CMAKE_COMMAND} -E make_directory
              ${CMAKE_CURRENT_SOURCE_DIR}/wiki/api
          COMMAND bash ${XSLT_DIR}/run-xslt.sh
              doxy
              ${CMAKE_CURRENT_BINARY_DIR}/api/xml
              ${CMAKE_CURRENT_SOURCE_DIR}/wiki/api
              ${XSLT_DIR}/doxy2md.xsl
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
  # Build targets (XSLT + Python pipeline):
  #   fluidsettings2md_xslt -- convert fluidsettings.xml to settings Markdown (XSLT)
  #   doxy2md_xslt          -- convert Doxygen XML output to API reference Markdown (XSLT)
  #   usage2md              -- convert usage .txt files to usage Markdown (Python)
  #   reverbators2md        -- convert reverbators.md (Python, strips Doxygen tags)
  #   zensical_xslt         -- run all of the above, then build the unified wiki+API site
  # ---------------------------------------------------------------------------
  find_package ( Python3 COMPONENTS Interpreter )
  find_program ( ZENSICAL_EXECUTABLE NAMES zensical )

  set ( WIKI_DOCS_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/wiki )
  set ( ZENSICAL_SCRIPTS  ${CMAKE_CURRENT_SOURCE_DIR}/zensical/scripts )

  if ( Python3_FOUND )

    # -- Convert usage .txt files to Markdown ----------------------------------
    add_custom_target ( usage2md
      COMMAND ${CMAKE_COMMAND} -E make_directory ${WIKI_DOCS_DIR}/usage
      COMMAND ${Python3_EXECUTABLE}
          ${ZENSICAL_SCRIPTS}/txt2md.py
          --batch
          ${CMAKE_CURRENT_SOURCE_DIR}/usage
          ${WIKI_DOCS_DIR}/usage
      COMMENT "Converting usage .txt files to Markdown"
    )

    # -- Convert reverbators.md (Doxygen .md with \htmlonly) to Markdown ------
    add_custom_target ( reverbators2md
      COMMAND ${Python3_EXECUTABLE}
          ${ZENSICAL_SCRIPTS}/txt2md.py
          --page-prefix usage/
          ${CMAKE_CURRENT_SOURCE_DIR}/reverbators.md
          ${WIKI_DOCS_DIR}/reverbators.md
      COMMENT "Converting reverbators.md to Zensical Markdown"
    )

    # -- Build the unified wiki + API documentation site (XSLT pipeline) ------
    # Uses zensical.toml from the repository root which points docs_dir to
    # doc/wiki/.  Generated API/settings/usage pages are written there by the
    # XSLT and Python targets above before zensical is invoked.
    if ( LIBXSLT_XSLTPROC_EXECUTABLE AND ZENSICAL_EXECUTABLE )
      add_custom_target ( zensical_xslt
          COMMAND ${ZENSICAL_EXECUTABLE} build --clean
          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          DEPENDS fluidsettings2md_xslt doxy2md_xslt
                  usage2md reverbators2md
          COMMENT "Building unified wiki and API documentation site (XSLT pipeline)"
      )
    elseif ( NOT LIBXSLT_XSLTPROC_EXECUTABLE )
      message ( STATUS "xsltproc not found -- 'zensical_xslt' CMake target unavailable.\n"
                       "Install it with: sudo apt-get install xsltproc" )
    else ()
      message ( STATUS "zensical executable not found -- 'zensical_xslt' CMake target unavailable.\n"
                       "Install it with: pip install -r doc/zensical/requirements.txt" )
    endif ()

  else ()
    message ( STATUS "Python3 not found -- Zensical doc targets are unavailable." )
  endif ()

endif ( DOXYGEN_FOUND )


if ( UNIX )
  install ( FILES fluidsynth.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
    COMPONENT ${PACKAGE}_program
  )
endif ( UNIX )

add_subdirectory ( examples )
