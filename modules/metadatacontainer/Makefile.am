AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libmetadatacontainer.la

AM_CPPFLAGS = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include \
  -isystem$(ROOTSYS)/include

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib

pkginclude_HEADERS = MetadataContainer.h

ROOTDICTS = MetadataContainer_Dict.cc

pcmdir = $(libdir)

nobase_dist_pcm_DATA = MetadataContainer_Dict_rdict.pcm

libmetadatacontainer_la_SOURCES = MetadataContainer.cc

libmetadatacontainer_la_LIBADD = \
  -lmetadata \
  -lfun4all \
  -lSubsysReco

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS =  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD = libmetadatacontainer.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

%_Dict.cc: %.h %LinkDef.h
	rootcint -f $@ @CINTDEFS@ $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $^

#just to get the dependency
%_Dict_rdict.pcm: %_Dict.cc ;

clean-local:
	rm -f *Dict.* *.pcm $(BUILT_SOURCES)
