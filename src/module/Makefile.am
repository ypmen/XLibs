lib_LTLIBRARIES=libxmodule.la

AM_CPPFLAGS=-I$(top_srcdir)/include
LDFLAGS+=-L$(top_srcdir)/src/container -L$(top_srcdir)/src/formats -L$(top_srcdir)/src/utils
LDADD=-lcontainer -lformats -lutils

libxmodule_la_SOURCES=flip.cpp patch.cpp preprocess.cpp preprocesslite.cpp downsample.cpp equalize.cpp baseline.cpp rfi.cpp stat.cpp stat2.cpp rescale.cpp defaraday.cpp dedispersion.cpp subdedispersion.cpp dedispersionX.cpp pipeline.cpp psrfitsreader.cpp psrfitswriter.cpp filterbankreader.cpp filterbankwriter.cpp
