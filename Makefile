LIBRARY_NAME		:= libprojectM
SUBPROJECT_NAME		:= projectM

SOURCES							:= source source/Renderer \
									source/NativePresetFactory \
									source/MilkdropPresetFactory \
									source/omptl source/Renderer/SOIL

$(LIBRARY_NAME)_FILES				:= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c) $(wildcard $(dir)/*.cpp))
$(LIBRARY_NAME)_FRAMEWORKS			:= OpenGLES CoreFoundation
$(LIBRARY_NAME)_LDFLAGS				:=
$(LIBRARY_NAME)_CFLAGS				:= $(foreach dir,$(SOURCES),-I$(dir)) -Iinclude -I../../iglu/include -DMACOS -DUSE_GLES1 -D__APPLE__ -DCMAKE_INSTALL_PREFIX="\"/usr\"" -DUSE_THREADS -DSYNC_PRESET_SWITCHES -DUSE_FBO -DFBO_HACK2 -DFBO_HACK2

$(SUBPROJECT_NAME)_FILES := $($(LIBRARY_NAME)_FILES)
$(SUBPROJECT_NAME)_FRAMEWORKS := $($(LIBRARY_NAME)_FRAMEWORKS)
$(SUBPROJECT_NAME)_LDFLAGS := $($(LIBRARY_NAME)_LDFLAGS)
$(SUBPROJECT_NAME)_CFLAGS := $($(LIBRARY_NAME)_CFLAGS)

include ../../common/common.mk
