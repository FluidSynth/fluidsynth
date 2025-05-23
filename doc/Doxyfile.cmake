# Doxyfile 1.6

#---------------------------------------------------------------------------
# Project related configuration options
#---------------------------------------------------------------------------
DOXYFILE_ENCODING = UTF-8
PROJECT_NAME = libfluidsynth
PROJECT_NUMBER = @VERSION@
OUTPUT_DIRECTORY = api
CREATE_SUBDIRS = NO
OUTPUT_LANGUAGE = English
BRIEF_MEMBER_DESC = YES
REPEAT_BRIEF = YES
ABBREVIATE_BRIEF = "Functions for" "Functions to" "The $name class" "The $name widget" "The $name file" is provides specifies contains represents a an the
ALWAYS_DETAILED_SEC = NO
INLINE_INHERITED_MEMB = NO
FULL_PATH_NAMES = NO
STRIP_FROM_PATH = @CMAKE_SOURCE_DIR@/
STRIP_FROM_INC_PATH = @CMAKE_SOURCE_DIR@/include/
SHORT_NAMES = NO
JAVADOC_AUTOBRIEF = YES
QT_AUTOBRIEF = NO
MULTILINE_CPP_IS_BRIEF = NO
INHERIT_DOCS = YES
SEPARATE_MEMBER_PAGES = NO
TAB_SIZE = 8
ALIASES += startlifecycle{1}="\name Lifecycle Functions for \1\_linebr@{"
ALIASES += endlifecycle="@}"
ALIASES += setting{1}="\ref settings_\1"
OPTIMIZE_OUTPUT_FOR_C = YES
OPTIMIZE_OUTPUT_JAVA = NO
OPTIMIZE_FOR_FORTRAN = NO
OPTIMIZE_OUTPUT_VHDL = NO
BUILTIN_STL_SUPPORT = NO
CPP_CLI_SUPPORT = NO
SIP_SUPPORT = NO
IDL_PROPERTY_SUPPORT = YES
DISTRIBUTE_GROUP_DOC = NO
SUBGROUPING = YES
TYPEDEF_HIDES_STRUCT = NO
#---------------------------------------------------------------------------
# Build related configuration options
#---------------------------------------------------------------------------
EXTRACT_ALL = NO
EXTRACT_PRIVATE = NO
EXTRACT_STATIC = NO
EXTRACT_LOCAL_CLASSES = NO
EXTRACT_LOCAL_METHODS = NO
EXTRACT_ANON_NSPACES = NO
HIDE_UNDOC_MEMBERS = YES
HIDE_UNDOC_CLASSES = YES
HIDE_FRIEND_COMPOUNDS = NO
HIDE_IN_BODY_DOCS = NO
INTERNAL_DOCS = NO
CASE_SENSE_NAMES = YES
HIDE_SCOPE_NAMES = NO
SHOW_INCLUDE_FILES = NO
INLINE_INFO = YES
SORT_MEMBER_DOCS = YES
SORT_BRIEF_DOCS = YES
SORT_GROUP_NAMES = YES
SORT_BY_SCOPE_NAME = NO
GENERATE_TODOLIST = NO
GENERATE_TESTLIST = NO
GENERATE_BUGLIST = NO
GENERATE_DEPRECATEDLIST = YES
ENABLED_SECTIONS = 
MAX_INITIALIZER_LINES = 30
SHOW_USED_FILES = YES
SHOW_FILES = YES
SHOW_NAMESPACES = YES
FILE_VERSION_FILTER = 
#---------------------------------------------------------------------------
# configuration options related to warning and progress messages
#---------------------------------------------------------------------------
QUIET = NO
WARNINGS = YES
WARN_IF_UNDOCUMENTED = YES
WARN_IF_DOC_ERROR = YES
WARN_NO_PARAMDOC = YES
WARN_FORMAT = "$file:$line: $text"
WARN_LOGFILE = 
#---------------------------------------------------------------------------
# configuration options related to the input files
#---------------------------------------------------------------------------
INPUT = \
@CMAKE_SOURCE_DIR@/doc/fluidsynth-v20-devdoc.txt \
@CMAKE_SOURCE_DIR@/doc/recent_changes.txt \
@CMAKE_SOURCE_DIR@/doc/usage \
@CMAKE_SOURCE_DIR@/include \
@CMAKE_SOURCE_DIR@/include/fluidsynth \
@CMAKE_SOURCE_DIR@/src \
@CMAKE_BINARY_DIR@/include/fluidsynth \
@CMAKE_BINARY_DIR@/doc/fluidsettings.txt

INPUT_ENCODING = UTF-8
FILE_PATTERNS = *.c *.h *.txt
RECURSIVE = YES
EXCLUDE = 
EXCLUDE_SYMLINKS = NO
EXCLUDE_PATTERNS = fluid_*.h
EXCLUDE_SYMBOLS = 
EXAMPLE_PATH = @CMAKE_SOURCE_DIR@/doc/examples
EXAMPLE_PATTERNS = *.c
EXAMPLE_RECURSIVE = NO
IMAGE_PATH = @CMAKE_SOURCE_DIR@/doc/images
INPUT_FILTER = 
FILTER_PATTERNS = 
FILTER_SOURCE_FILES = NO
#---------------------------------------------------------------------------
# configuration options related to source browsing
#---------------------------------------------------------------------------
SOURCE_BROWSER = NO
INLINE_SOURCES = NO
STRIP_CODE_COMMENTS = YES
REFERENCED_BY_RELATION = NO
REFERENCES_RELATION = NO
REFERENCES_LINK_SOURCE = YES
USE_HTAGS = NO
VERBATIM_HEADERS = NO
#---------------------------------------------------------------------------
# configuration options related to the alphabetical class index
#---------------------------------------------------------------------------
ALPHABETICAL_INDEX = YES
COLS_IN_ALPHA_INDEX = 5
IGNORE_PREFIX = 
#---------------------------------------------------------------------------
# configuration options related to the HTML output
#---------------------------------------------------------------------------
GENERATE_HTML = YES
HTML_OUTPUT = html
HTML_FILE_EXTENSION = .html
HTML_EXTRA_FILES = \
@CMAKE_SOURCE_DIR@/doc/fluidsettings.xml \
@CMAKE_SOURCE_DIR@/doc/fluidsettings.xsl

# FluidSynth specific layout and style customizations.
# Comment the following four lines to return to the
# default doxygen styling and layout
LAYOUT_FILE = @CMAKE_CURRENT_BINARY_DIR@/layout.xml
HTML_HEADER =
HTML_FOOTER = @CMAKE_SOURCE_DIR@/doc/doxygen/footer.html
HTML_EXTRA_STYLESHEET = @CMAKE_SOURCE_DIR@/doc/doxygen/custom.css
@DOC_COLORSTYLE_CONFIG@
# end FluidSynth styling and layout

GENERATE_HTMLHELP = NO
GENERATE_DOCSET = NO
DOCSET_FEEDNAME = "Doxygen generated docs"
DOCSET_BUNDLE_ID = org.doxygen.Project
HTML_DYNAMIC_SECTIONS = YES
CHM_FILE = 
HHC_LOCATION = 
GENERATE_CHI = NO
CHM_INDEX_ENCODING = 
BINARY_TOC = NO
TOC_EXPAND = NO
DISABLE_INDEX = NO
ENUM_VALUES_PER_LINE = 1
GENERATE_TREEVIEW = YES
TREEVIEW_WIDTH = 350
FORMULA_FONTSIZE = 10
#---------------------------------------------------------------------------
# configuration options related to the LaTeX output
#---------------------------------------------------------------------------
GENERATE_LATEX = NO
LATEX_OUTPUT = latex
LATEX_CMD_NAME = latex
MAKEINDEX_CMD_NAME = makeindex
COMPACT_LATEX = NO
PAPER_TYPE = a4wide
EXTRA_PACKAGES = 
LATEX_HEADER = 
PDF_HYPERLINKS = YES
USE_PDFLATEX = YES
LATEX_BATCHMODE = NO
LATEX_HIDE_INDICES = NO
#---------------------------------------------------------------------------
# configuration options related to the RTF output
#---------------------------------------------------------------------------
GENERATE_RTF = NO
RTF_OUTPUT = rtf
COMPACT_RTF = NO
RTF_HYPERLINKS = NO
RTF_STYLESHEET_FILE = 
RTF_EXTENSIONS_FILE = 
#---------------------------------------------------------------------------
# configuration options related to the man page output
#---------------------------------------------------------------------------
GENERATE_MAN = NO
MAN_OUTPUT = man
MAN_EXTENSION = .3
MAN_LINKS = NO
#---------------------------------------------------------------------------
# configuration options related to the XML output
#---------------------------------------------------------------------------
GENERATE_XML = NO
XML_OUTPUT = xml
XML_PROGRAMLISTING = YES
#---------------------------------------------------------------------------
# configuration options for the AutoGen Definitions output
#---------------------------------------------------------------------------
GENERATE_AUTOGEN_DEF = NO
#---------------------------------------------------------------------------
# configuration options related to the Perl module output
#---------------------------------------------------------------------------
GENERATE_PERLMOD = NO
PERLMOD_LATEX = NO
PERLMOD_PRETTY = YES
PERLMOD_MAKEVAR_PREFIX = 
#---------------------------------------------------------------------------
# Configuration options related to the preprocessor   
#---------------------------------------------------------------------------
ENABLE_PREPROCESSING = YES
MACRO_EXPANSION = YES
EXPAND_ONLY_PREDEF = YES
SEARCH_INCLUDES = YES
INCLUDE_PATH = 
INCLUDE_FILE_PATTERNS = 
PREDEFINED = __DOXYGEN__ \
             FLUIDSYNTH_API \
             FLUID_DEPRECATED
EXPAND_AS_DEFINED = 
SKIP_FUNCTION_MACROS = YES
#---------------------------------------------------------------------------
# Configuration::additions related to external references   
#---------------------------------------------------------------------------
TAGFILES = 
GENERATE_TAGFILE = 
ALLEXTERNALS = NO
EXTERNAL_GROUPS = YES
PERL_PATH = /usr/bin/perl
#---------------------------------------------------------------------------
# Configuration options related to the dot tool   
#---------------------------------------------------------------------------
CLASS_DIAGRAMS = YES
MSCGEN_PATH = 
HIDE_UNDOC_RELATIONS = YES
HAVE_DOT = NO
DOT_FONTNAME =
DOT_FONTPATH = 
CLASS_GRAPH = YES
COLLABORATION_GRAPH = YES
GROUP_GRAPHS = YES
UML_LOOK = NO
TEMPLATE_RELATIONS = YES
INCLUDE_GRAPH = YES
INCLUDED_BY_GRAPH = YES
CALL_GRAPH = NO
CALLER_GRAPH = NO
GRAPHICAL_HIERARCHY = YES
DIRECTORY_GRAPH = YES
DOT_IMAGE_FORMAT = png
DOT_PATH = 
DOTFILE_DIRS = 
DOT_GRAPH_MAX_NODES = 50
MAX_DOT_GRAPH_DEPTH = 1000
DOT_TRANSPARENT = YES
DOT_MULTI_TARGETS = NO
GENERATE_LEGEND = YES
DOT_CLEANUP = YES
#---------------------------------------------------------------------------
# Configuration::additions related to the search engine   
#---------------------------------------------------------------------------
SEARCHENGINE = YES
