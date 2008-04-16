thisdir = class/System.Silverlight
include ../../build/rules.make

LIBRARY = System.Windows.Browser.dll

LIB_MCS_FLAGS = -r:System -r:Mono.Moonlight -noconfig -unsafe

ifeq (net_2_1, $(PROFILE))
LIB_MCS_FLAGS += -r:System.Net
endif

include ../../build/library.make
