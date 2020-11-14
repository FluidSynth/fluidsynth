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
  configure_file ( Doxyfile.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
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
  endif ( LIBXSLT_XSLTPROC_EXECUTABLE )

endif ( DOXYGEN_FOUND )


if ( UNIX )
  install ( FILES fluidsynth.1
    DESTINATION ${MAN_INSTALL_DIR} )
endif ( UNIX )

add_subdirectory ( examples )
