#  install the man pages and include in distribution
man_MANS = fluidsynth.1 
EXTRA_DIST = $(man_MANS) Doxyfile example.c example.sf2 fluidsynth.1 \
  fluidsynth-v10-devdoc.xml html api FluidSynth-LADSPA.pdf

docbook_docs = fluidsynth-v10-devdoc.xml

DOCBOOK_STYLESHEET ?= http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl

# Run "make update-docs" to update developer doc and doxygen reference
update-docs: html/index.html doxygen

# The new XML DocBook way:
html/index.html: $(docbook_docs)
	-rm -rf html
	$(mkinstalldirs) html
	xsltproc -o html/ --catalogs $(DOCBOOK_STYLESHEET) $<

doc-clean:
	-rm -rf html

maintainer-clean-local: doc-clean

doxygen: Doxyfile
	doxygen Doxyfile

# Update docs for distribution
dist-hook: update-docs
