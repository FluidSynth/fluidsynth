# Try to find the READLINE library
#  MidiShare_FOUND - system has MidiShare
#  MidiShare_INCLUDE_DIR - MidiShare include directory
#  MidiShare_LIBS - Libraries needed to use MidiShare

if ( MidiShare_INCLUDE_DIR AND MidiShare_LIBS )
    set ( MidiShare_FIND_QUIETLY TRUE )
endif ( MidiShare_INCLUDE_DIR AND MidiShare_LIBS )

find_path ( MidiShare_INCLUDE_DIR NAMES MidiShare.h )
find_library ( MidiShare_LIBS NAMES MidiShare )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args( MidiShare DEFAULT_MSG 
                                   MidiShare_INCLUDE_DIR 
                                   MidiShare_LIBS )

mark_as_advanced( MidiShare_INCLUDE_DIR MidiShare_LIBS )
