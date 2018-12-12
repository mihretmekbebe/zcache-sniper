# This file is auto-generated, changes made to it will be lost. Please edit makebuildscripts.py instead.

if [ -z "${SNIPER_ROOT}" ] ; then SNIPER_ROOT=$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..") ; fi

GRAPHITE_CC="/usr/bin/gcc-4.8"
GRAPHITE_CFLAGS="-mno-sse4 -mno-sse4.1 -mno-sse4.2 -mno-sse4a -mno-avx -mno-avx2 -I${SNIPER_ROOT}/include "
GRAPHITE_CXX="/usr/bin/g++-4.8"
GRAPHITE_CXXFLAGS="-mno-sse4 -mno-sse4.1 -mno-sse4.2 -mno-sse4a -mno-avx -mno-avx2 -I${SNIPER_ROOT}/include "
GRAPHITE_LD="/usr/bin/g++-4.8"
GRAPHITE_LDFLAGS="-static -L${SNIPER_ROOT}/lib -pthread "
GRAPHITE_LD_LIBRARY_PATH=""
GRAPHITE_UPCCFLAGS="-I${SNIPER_ROOT}/include  -link-with='/usr/bin/g++-4.8 -static -L${SNIPER_ROOT}/lib -pthread'"
PIN_HOME="/home/bragadeesh153/Desktop/sniper/sniper-6.1/pin_kit"
SNIPER_CC="/usr/bin/gcc-4.8"
SNIPER_CFLAGS="-mno-sse4 -mno-sse4.1 -mno-sse4.2 -mno-sse4a -mno-avx -mno-avx2 -I${SNIPER_ROOT}/include "
SNIPER_CXX="/usr/bin/g++-4.8"
SNIPER_CXXFLAGS="-mno-sse4 -mno-sse4.1 -mno-sse4.2 -mno-sse4a -mno-avx -mno-avx2 -I${SNIPER_ROOT}/include "
SNIPER_LD="/usr/bin/g++-4.8"
SNIPER_LDFLAGS="-static -L${SNIPER_ROOT}/lib -pthread "
SNIPER_LD_LIBRARY_PATH=""
SNIPER_UPCCFLAGS="-I${SNIPER_ROOT}/include  -link-with='/usr/bin/g++-4.8 -static -L${SNIPER_ROOT}/lib -pthread'"
