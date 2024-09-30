lib_LTLIBRARIES=libxdada.la

AM_CPPFLAGS=-I$(top_srcdir)/include
LDFLAGS+=-L$(top_srcdir)/src/container -L$(top_srcdir)/src/formats -L$(top_srcdir)/src/utils
LDADD=-lpsrdada -lcontainer -lformats -lutils

libxdada_la_SOURCES=dadareader.cpp