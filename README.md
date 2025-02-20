# Xlibs

Xlibs is the libraries for PulsarX/TransientX/BasebandX

## Requirements
- sofa library (https://www.iausofa.org/2020_0721_C/sofa_c-20200721.tar.gz) or erfa (https://github.com/liberfa/erfa)

## Installation:
1) ./bootstrap
2) ./configure --prefix=/install\_path LDFLAGS="-L/path_to_sofa" CPPFLAGS="-I/path_to_sofa"
3) make and make install