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

      # -- XSLT-based Zensical targets (alternative to Python-based targets) --
      set ( XSLT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zensical/xslt )

      # Convert fluidsettings.xml to Markdown using XSLT
      add_custom_target ( fluidsettings2md_xslt
          COMMAND ${CMAKE_COMMAND} -E make_directory
              ${CMAKE_CURRENT_SOURCE_DIR}/zensical/docs/settings
          COMMAND bash ${XSLT_DIR}/run-xslt.sh
              fluidsettings
              ${CMAKE_CURRENT_SOURCE_DIR}/fluidsettings.xml
              ${CMAKE_CURRENT_SOURCE_DIR}/zensical/docs/settings
              ${XSLT_DIR}/fluidsettings2md.xsl
          COMMENT "Converting fluidsettings.xml to Markdown via XSLT"
      )

      # Convert Doxygen XML output to API Markdown using XSLT
      add_custom_target ( doxy2md_xslt
          COMMAND ${CMAKE_COMMAND} -E make_directory
              ${CMAKE_CURRENT_SOURCE_DIR}/zensical/docs/api
          COMMAND bash ${XSLT_DIR}/run-xslt.sh
              doxy
              ${CMAKE_CURRENT_BINARY_DIR}/api/xml
              ${CMAKE_CURRENT_SOURCE_DIR}/zensical/docs/api
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
  # Build targets (Python pipeline):
  #   fluidsettings2md  -- convert fluidsettings.xml to settings Markdown
  #   doxy2md           -- convert Doxygen XML output to API reference Markdown
  #   usage2md          -- convert usage .txt files to usage Markdown
  #   changelog2md      -- convert recent_changes.txt to changelog.md
  #   reverbators2md    -- convert reverbators.md (strips Doxygen tags, keeps HTML)
  #   ladspa2md         -- copy ladspa.md (already clean Markdown)
  #   zensical          -- run all of the above, then build the Zensical site
  #
  # XSLT pipeline (alternative, requires xsltproc):
  #   fluidsettings2md_xslt  -- XSLT alternative to fluidsettings2md
  #   doxy2md_xslt           -- XSLT alternative to doxy2md
  #   zensical_xslt          -- run all of the above (XSLT + Python shared), then build
  # ---------------------------------------------------------------------------
  find_package ( Python3 COMPONENTS Interpreter )
  find_program ( ZENSICAL_EXECUTABLE NAMES zensical )

  set ( ZENSICAL_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/zensical )
  set ( ZENSICAL_DOCS_DIR ${ZENSICAL_DIR}/docs )
  set ( ZENSICAL_SCRIPTS  ${ZENSICAL_DIR}/scripts )
  set ( ZENSICAL_SITE_DIR ${CMAKE_CURRENT_BINARY_DIR}/zensical_site )

  if ( Python3_FOUND )

    # -- Convert fluidsettings.xml to Markdown --------------------------------
    add_custom_target ( fluidsettings2md
      ${Python3_EXECUTABLE}
          ${ZENSICAL_SCRIPTS}/fluidsettings2md.py
          ${CMAKE_CURRENT_SOURCE_DIR}/fluidsettings.xml
          ${ZENSICAL_DOCS_DIR}/settings
      COMMENT "Converting fluidsettings.xml to Markdown"
    )

    # -- Convert Doxygen XML output to API reference Markdown -----------------
    add_custom_target ( doxy2md
      ${Python3_EXECUTABLE}
          ${ZENSICAL_SCRIPTS}/doxy2md.py
          ${CMAKE_CURRENT_BINARY_DIR}/api/xml
          ${ZENSICAL_DOCS_DIR}/api
      DEPENDS doxygen
      COMMENT "Converting Doxygen XML to API reference Markdown"
    )

    # -- Convert usage .txt files to Markdown ----------------------------------
    add_custom_target ( usage2md
      COMMAND ${CMAKE_COMMAND} -E make_directory ${ZENSICAL_DOCS_DIR}/usage
      COMMAND ${Python3_EXECUTABLE}
          ${ZENSICAL_SCRIPTS}/txt2md.py
          --batch
          ${CMAKE_CURRENT_SOURCE_DIR}/usage
          ${ZENSICAL_DOCS_DIR}/usage
      COMMENT "Converting usage .txt files to Markdown"
    )

    # -- Convert recent_changes.txt to changelog.md ---------------------------
    add_custom_target ( changelog2md
      ${Python3_EXECUTABLE}
          ${ZENSICAL_SCRIPTS}/txt2md.py
          ${CMAKE_CURRENT_SOURCE_DIR}/recent_changes.txt
          ${ZENSICAL_DOCS_DIR}/changelog.md
      COMMENT "Converting recent_changes.txt to changelog.md"
    )

    # -- Convert reverbators.md (Doxygen .md with \htmlonly) to Markdown ------
    add_custom_target ( reverbators2md
      ${Python3_EXECUTABLE}
          ${ZENSICAL_SCRIPTS}/txt2md.py
          ${CMAKE_CURRENT_SOURCE_DIR}/reverbators.md
          ${ZENSICAL_DOCS_DIR}/reverbators.md
      COMMENT "Converting reverbators.md to Zensical Markdown"
    )

    # -- Copy ladspa.md (already clean Markdown, no Doxygen tags) -------------
    add_custom_target ( ladspa2md
      ${CMAKE_COMMAND} -E copy
          ${CMAKE_CURRENT_SOURCE_DIR}/ladspa.md
          ${ZENSICAL_DOCS_DIR}/ladspa.md
      COMMENT "Copying ladspa.md to docs directory"
    )

    # -- Build the complete Zensical site (Python pipeline) -------------------
    if ( ZENSICAL_EXECUTABLE )
      add_custom_target ( zensical
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ZENSICAL_SITE_DIR}
        COMMAND ${ZENSICAL_EXECUTABLE} build
            --config-file ${ZENSICAL_DIR}/mkdocs.yml
        WORKING_DIRECTORY ${ZENSICAL_DIR}
        DEPENDS fluidsettings2md doxy2md usage2md changelog2md reverbators2md ladspa2md
        COMMENT "Building Zensical documentation site"
      )
    else ()
      message ( STATUS "zensical executable not found -- 'zensical' CMake target unavailable.\n"
                       "Install it with: pip install -r doc/zensical/requirements.txt" )
    endif ()

    # -- Build the complete Zensical site (XSLT pipeline) ----------------------
    # The XSLT pipeline uses xsltproc for fluidsettings and doxy conversions,
    # but still uses Python for usage/changelog/reverbators/ladspa conversions.
    if ( LIBXSLT_XSLTPROC_EXECUTABLE AND ZENSICAL_EXECUTABLE )
      add_custom_target ( zensical_xslt
          COMMAND ${ZENSICAL_EXECUTABLE} build
              --config-file ${CMAKE_CURRENT_SOURCE_DIR}/zensical/mkdocs.yml
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/zensical
          DEPENDS fluidsettings2md_xslt doxy2md_xslt
                  usage2md changelog2md reverbators2md ladspa2md
          COMMENT "Building Zensical documentation site (XSLT pipeline)"
      )
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
