LOCAL_PATH	:= $(call my-dir)
include $(CLEAR_VARS)

ROOT_DIR						:=	.
SOURCE							:=	 source/ConfigFile.cpp \
source/KeyHandler.cpp \
source/MilkdropPresetFactory/BuiltinFuncs.cpp \
source/MilkdropPresetFactory/BuiltinParams.cpp \
source/MilkdropPresetFactory/CustomShape.cpp \
source/MilkdropPresetFactory/CustomWave.cpp \
source/MilkdropPresetFactory/Eval.cpp \
source/MilkdropPresetFactory/Expr.cpp \
source/MilkdropPresetFactory/Func.cpp \
source/MilkdropPresetFactory/IdlePreset.cpp \
source/MilkdropPresetFactory/InitCond.cpp \
source/MilkdropPresetFactory/MilkdropPreset.cpp \
source/MilkdropPresetFactory/MilkdropPresetFactory.cpp \
source/MilkdropPresetFactory/Param.cpp \
source/MilkdropPresetFactory/Parser.cpp \
source/MilkdropPresetFactory/PerFrameEqn.cpp \
source/MilkdropPresetFactory/PerPixelEqn.cpp \
source/MilkdropPresetFactory/PerPointEqn.cpp \
source/MilkdropPresetFactory/PresetFrameIO.cpp \
source/NativePresetFactory/NativePresetFactory.cpp \
source/PCM.cpp \
source/PipelineMerger.cpp \
source/Preset.cpp \
source/PresetChooser.cpp \
source/PresetFactory.cpp \
source/PresetFactoryManager.cpp \
source/PresetLoader.cpp \
source/Renderer/BeatDetect.cpp \
source/Renderer/FBO.cpp \
source/Renderer/Filters.cpp \
source/Renderer/MilkdropWaveform.cpp \
source/Renderer/PerPixelMesh.cpp \
source/Renderer/PerlinNoise.cpp \
source/Renderer/Pipeline.cpp \
source/Renderer/PipelineContext.cpp \
source/Renderer/RenderItemDistanceMetric.cpp \
source/Renderer/RenderItemMatcher.cpp \
source/Renderer/Renderable.cpp \
source/Renderer/Renderer.cpp \
source/Renderer/Shader.cpp \
source/Renderer/ShaderEngine.cpp \
source/Renderer/TextureManager.cpp \
source/Renderer/UserTexture.cpp \
source/Renderer/VideoEcho.cpp \
source/Renderer/Waveform.cpp \
source/TimeKeeper.cpp \
source/fftsg.cpp \
source/projectM.cpp \
source/timer.cpp \
source/wipemalloc.cpp \
source/ZipStreamBuffer.cpp \
source/Renderer/SOIL/SOIL.c \
source/Renderer/SOIL/stb_image_aug.c \
source/Renderer/SOIL/image_DXT.c \
source/Renderer/SOIL/image_helper.c \
support/unzip.cpp \
support/ioapi.cpp \
support/math/math_acosf.c \
support/math/math_asinf.c \
support/math/math_atan2f.c \
support/math/math_atanf.c \
support/math/math_ceilf.c \
support/math/math_cosf.c \
support/math/math_coshf.c \
support/math/math_expf.c \
support/math/math_fabsf.c \
support/math/math_floorf.c \
support/math/math_fmodf.c \
support/math/math_frexpf.c \
support/math/math_invsqrtf.c \
support/math/math_ldexpf.c \
support/math/math_log10f.c \
support/math/math_logf.c \
support/math/math_mat2.c \
support/math/math_mat3.c \
support/math/math_mat4.c \
support/math/math_modf.c \
support/math/math_powf.c \
support/math/math_runfast.c \
support/math/math_sincosf.c \
support/math/math_sinf.c \
support/math/math_sinfv.c \
support/math/math_sinhf.c \
support/math/math_sqrtf.c \
support/math/math_sqrtfv.c \
support/math/math_tanf.c \
support/math/math_tanhf.c \
support/math/math_vec2.c \
support/math/math_vec3.c \
support/math/math_vec4.c

INCLUDE							:=	source \
source/MilkdropPresetFactory \
source/NativePresetFactory \
source/Renderer \
source/Renderer/SOIL \
source/omptl \
support \
support/math

LOCAL_MODULE					:=	projectM
LOCAL_MODULE_FILENAME			:=	libprojectM
LOCAL_SRC_FILES					:=	$(SOURCE)
LOCAL_C_INCLUDES				:=	$(foreach inc,$(INCLUDE),$(LOCAL_PATH)/$(inc)) \
	frameworks/base/opengl/include \
	frameworks/base/opengl/include/GLES \
	frameworks/base/include
LOCAL_LDLIBS					:=	-lGLESv1_CM -llog -lz
LOCAL_CFLAGS					:=	-DPROJECTM_INSIDE -DLINUX -DUSE_GLES1 -DCMAKE_INSTALL_PREFIX="\"/usr\"" -DUSE_THREADS -DSYNC_PRESET_SWITCHES -fexceptions -frtti -DUSE_FBO -DANDROID_FBO_HACK -DUSE_FILE32API #-mfpu=neon -pg
LOCAL_CFLAGS					+=	-ffast-math -O3 -funroll-loops #-mfpu=neon
LOCAL_ARM_MODE					:=	arm

#LOCAL_STATIC_LIBRARIES := andprof

include $(BUILD_SHARED_LIBRARY)
