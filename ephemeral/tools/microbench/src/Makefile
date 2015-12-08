.PHONY: GMAKE clean lean ossup aixtgt lnxtgt soltgt target libmb sampmod forkmalloc forkmmap netmirror mmapexe futexlat

BLDTGT=failtarget
OSSUP=false

# NOTE: Before you put an option in here, read the following:
#       Please use #ifdef PORT_SunOS, #ifdef PORT_Linux, #ifdef PORT_AIX in 
#       your code to include any standards defines. For example:
#
#       #ifdef PORT_SunOS
#       #define _XOPEN_SOURCE 600
#       #define __EXTENSIONS__
#       #endif
#
#       If you have compiler, linker, or ar options, then append them in the
#       local Makefile. This is a top level makefile and should not have
#       project (module) specific options in it as it will be used for ALL
#       modules, not just yours.

ifeq "$(shell uname -s)" "Linux"
   CC=gcc
   CC_OPTS=-Wall -DPORT_$(shell uname -s)
   LD_OPTS=
   OSSUP=true
   LANG=C
   BLDTGT=lnxtgt
endif

ifeq "$(shell uname -s)" "SunOS"
   CC=/bb/util/common/SS12_3-20131030/SUNWspro/bin/c99
   CC_OPTS=-DPORT_$(shell uname -s)
   LD_OPTS=
   OSSUP=true
   LANG=C
   BLDTGT=soltgt
endif

ifeq "$(shell uname -s)" "AIX"
   CC=/bb/util/version11-042012/usr/vac/bin/xlc_r
   LD=/bb/util/version11-042012/usr/vac/bin/xlc_r
   CC_OPTS=-DPORT_$(shell uname -s)
   LD_OPTS=
   OSSUP=true
   BLDTGT=aixtgt
endif

export CC
export LD
export CC_OPTS
export LD_OPTS


# How the target business works:
#
# 1. Write your module (including Makefile). This should be copied (largely)
#    from one of the existing Makefiles. (The point is to make it much easier
#    but not a requirement.) The basic requirements are that your Makefile
#    append to the exported variables just above this comment block, unless
#    they wish to completely override them. From this Makefile the module
#    Makefile will inherit the compiler path as well as enabling warnings.
#    Most everything else is required for the module Makefile to set. The
#    settings in your module Makefile should be os-dependent just like the 
#    "code block" higher in this Makefile.
# 2. Write a module target in this file. It should have one command, and that
#    is: "@$(MAKE) -s -C <your_module_dir>". Copy from examples below.
# 3. Create a lean and clean command in the clean/lean targets below.
# 4. Put your target in either the "all" or your os-specific targets just
#    below this comment block. The all target will build on all platforms,
#    otherwise it will build only on your target platform. The strong
#    recommendation is to build on all platforms.
# 5. Put your target in .PHONY above. 
#
# Additional notes:
# - ossup is really not necessary, as "failtarget" will not be found when
#   there is no match on the OS type. I kept it for no good reason.
# - If you put your target on the os-specific target, then is should go
#   AFTER the all target as the common library is built first in all.
# - If you are annoyed with large builds while developing your module, then
#   consider trying the following:
#    1. Do a top level "make clean".
#       This will clean out any object or library files not intended for your
#       current build platform.
#    2. Do a top level "make".
#       This will build the library (and all the modules).
#    3  Do a top level "make <your_target>"
#       This will allow you to build just your target without the need to
#       build the entire framework.
# - You do not need to specify the platform in your build target. The Makefile
#   is capable of determining the platform appropriate build for you.
# - Do a "manual Jenkins build" for all platforms prior to code check-in and
#   insure that your app at least builds clean on all three platforms.
#   Nobody wants to debug your code, in order to write theirs. If you do not
#   want to build on all platforms (for now), then put your build only in
#   your desired platform. You will likely be challenged on not building on
#   all platforms, but at least you will not break the build.

target: $(BLDTGT)

aixtgt: ossup all

lnxtgt: ossup all futexlat

soltgt: ossup all

all: libmb forkmalloc forkmmap netmirror sampmod mmapexe

libmb: 
	@$(MAKE) -s -C lib

ossup:
	@printf "Testing OS support..."
	@$(OSSUP)
	@printf "Good\n"
	@printf "Starting the top level build.\n"

forkmalloc:
	@$(MAKE) -s -C forkmalloc

forkmmap:
	@$(MAKE) -s -C forkmmap

netmirror:
	@$(MAKE) -s -C netmirror

sampmod:
	@$(MAKE) -s -C sampmod

mmapexe:
	@$(MAKE) -s -C mmapcp

futexlat:
	@$(MAKE) -s -C futexlat

lean:
	@$(MAKE) -s -C lib lean
	@$(MAKE) -s -C sampmod lean
	@$(MAKE) -s -C forkmalloc lean
	@$(MAKE) -s -C forkmmap lean
	@$(MAKE) -s -C netmirror lean
	@$(MAKE) -s -C mmapcp lean
	@$(MAKE) -s -C futexlat lean

clean:
	@$(MAKE) -s -C lib clean
	@$(MAKE) -s -C sampmod clean
	@$(MAKE) -s -C forkmalloc clean
	@$(MAKE) -s -C forkmmap clean
	@$(MAKE) -s -C netmirror clean
	@$(MAKE) -s -C mmapcp clean
	@$(MAKE) -s -C futexlat clean

GMAKE:
	@printf "ERROR: This is a gmake file, not a make file. Use gmake." >&2
	@false
