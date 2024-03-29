include ../global_config
CC = gcc
CFLAGS += -I../libzmpeg3 -I../quicktime

ifeq ($(OBJDIR), i686)
CFLAGS += -DX86_CPU
NASMOBJ = \
	$(OBJDIR)/bdist1_mmx.o \
	$(OBJDIR)/bdist2_mmx.o \
	$(OBJDIR)/dist2_mmx.o \
	$(OBJDIR)/fdct_mmx.o \
	$(OBJDIR)/idct_mmx.o \
	$(OBJDIR)/mblock_sad_mmx.o \
	$(OBJDIR)/mblock_sad_mmxe.o \
	$(OBJDIR)/mblockq_sad_mmxe.o \
	$(OBJDIR)/predcomp_mmx.o \
	$(OBJDIR)/predcomp_mmxe.o \
	$(OBJDIR)/predict_mmx.o \
	$(OBJDIR)/quant_mmx.o \

ASMOBJ = \
	$(OBJDIR)/mblock_sub44_sads.o \
	$(OBJDIR)/quantize_x86.o

#	$(OBJDIR)/mblock_sub44_sads.o \

endif

OBJ = \
	$(OBJDIR)/mpeg2enc.o \
	$(OBJDIR)/conform.o \
	$(OBJDIR)/cpu_accel.o \
	$(OBJDIR)/putseq.o \
	$(OBJDIR)/putpic.o \
	$(OBJDIR)/puthdr.o \
	$(OBJDIR)/putmpg.o \
	$(OBJDIR)/putvlc.o \
	$(OBJDIR)/putbits.o \
	$(OBJDIR)/motion.o \
	$(OBJDIR)/predict.o \
	$(OBJDIR)/readpic.o \
	$(OBJDIR)/transfrm.o \
	$(OBJDIR)/fdctdata.o \
	$(OBJDIR)/fdctref.o \
	$(OBJDIR)/idct.o \
	$(OBJDIR)/idctdata.o \
	$(OBJDIR)/quantize.o \
	$(OBJDIR)/ratectl.o \
	$(OBJDIR)/stats.o \
	$(OBJDIR)/writepic.o

FFMPEG_DIR := $(shell echo ../thirdparty/ffmpeg*)
FF_LIBS := $(FFMPEG_DIR)/libswscale/libswscale.a
FF_LIBS += $(FFMPEG_DIR)/libavcodec/libavcodec.a
FF_LIBS += $(FFMPEG_DIR)/libavfilter/libavfilter.a
FF_LIBS += $(FFMPEG_DIR)/libavformat/libavformat.a
FF_LIBS += $(FFMPEG_DIR)/libavutil/libavutil.a
FF_LIBS += $(FFMPEG_DIR)/libpostproc/libpostproc.a
FF_LIBS += $(FFMPEG_DIR)/libswresample/libswresample.a

TWOLAME_PATH := $(shell echo ../thirdparty/twolame*)
LAME_PATH := $(shell echo ../thirdparty/lame*)
FDK_PATH := $(shell echo ../thirdparty/fdk*)
OGG_PATH := $(shell echo ../thirdparty/libogg*)
VORBIS_PATH := $(shell echo ../thirdparty/libvorbis*)
THEORA_PATH := $(shell echo ../thirdparty/libtheora*)
THEORA_PATH := $(shell echo ../thirdparty/libtheora*)
OPENJ2K_PATH := $(shell echo ../thirdparty/openjpeg*)
VPX_PATH := $(shell echo ../thirdparty/libvpx*)
X264_PATH := $(shell echo ../thirdparty/x264*)
X265_PATH := $(shell echo ../thirdparty/x265*)

TWOLAME_LIB := $(TWOLAME_PATH)/libtwolame/.libs/libtwolame.a
LAME_LIB := $(LAME_PATH)/libmp3lame/.libs/libmp3lame.a
FDK_LIB := $(FDK_PATH)/.libs/libfdk-aac.a
OGG_LIB := $(OGG_PATH)/src/.libs/libogg.a
VORBIS_LIB := $(VORBIS_PATH)/lib/.libs/libvorbis.a
THEORA_LIB := $(THEORA_PATH)/lib/.libs/libtheora.a
VPX_LIB := $(VPX_PATH)/libvpx.a
X264_LIB := $(X264_PATH)/libx264.a
X265_LIB := $(X265_PATH)/libx265.a

#broken
OPENJ2K_LIB := $(OPENJ2K_PATH)/bin/libopenjpip.a \
	$(OPENJ2K_PATH)/bin/libopenjp3d.a \
	$(OPENJ2K_PATH)/bin/libopenjp2.a \

STATIC_LIBS := \
	$(FF_LIBS) \
	$(TWOLAME_LIB) \
	$(LAME_LIB) \
	$(FDK_LIB) \
	$(OGG_LIB) \
	$(VORBIS_LIB) \
	$(THEORA_LIB) \
	$(VPX_LIB) \
	$(X264_LIB) \
	$(X265_LIB) \

LIBS = \
	../quicktime/$(OBJDIR)/libquicktime.a \
	../libzmpeg3/$(OBJDIR)/libzmpeg3.a \
	$(STATIC_LIBS) \
	-lpthread \
	-lpng \
	-ljpeg \
	-lX11 \
	-lz \
	-llzma \
	-lbz2 \
	-lm \
	-ldl

HVEG2LIB = $(OBJDIR)/hveg2enc.a
HVEG2ENC = ../bin/hveg2enc.plugin

$(shell echo $(CFLAGS) > $(OBJDIR)/c_flags)
$(shell echo $(HVEG2LIB)  $(OBJ) $(NASMOBJ) $(ASMOBJ) > $(OBJDIR)/objs)

$(HVEG2ENC):	$(OBJDIR)/hveg2enc.o $(HVEG2LIB)
	$(CXX) `cat $(OBJDIR)/c_flags` -o $@ $^ $(LIBS)

$(OBJDIR)/hveg2enc.o:	mpeg2enc.c
	$(CC) -c `cat $(OBJDIR)/c_flags` -Dmpeg2enc=main -o $@ $^

OUTPUT = $(HVEG2LIB) $(HVEG2ENC)

all: $(OUTPUT)

clean:
	rm -f $(HVEG2ENC)
	rm -rf $(OBJDIR)

install:	$(OUTPUT)
	cp -a $(HVEG2LIB) ../bin/.

$(HVEG2LIB): $(OBJ) $(NASMOBJ) $(ASMOBJ)
	ar rcs `cat $(OBJDIR)/objs`

$(NASMOBJ):
	nasm -f elf $(subst $(OBJDIR)/,, $*.s) -o $*.o

$(OBJ) $(ASMOBJ):
	$(CC) -c `cat $(OBJDIR)/c_flags` $(subst $(OBJDIR)/,, $*.c) -o $*.o

$(OBJDIR)/mpeg2enc.o: 			    mpeg2enc.c
$(OBJDIR)/conform.o: 			    conform.c
$(OBJDIR)/cpu_accel.o:  		    cpu_accel.c
$(OBJDIR)/putseq.o: 			    putseq.c
$(OBJDIR)/putpic.o: 			    putpic.c
$(OBJDIR)/puthdr.o: 			    puthdr.c
$(OBJDIR)/putmpg.o: 			    putmpg.c
$(OBJDIR)/putvlc.o: 			    putvlc.c
$(OBJDIR)/putbits.o: 			    putbits.c
$(OBJDIR)/motion.o: 			    motion.c
$(OBJDIR)/predict.o: 			    predict.c
$(OBJDIR)/readpic.o: 			    readpic.c
$(OBJDIR)/transfrm.o: 			    transfrm.c
$(OBJDIR)/fdctdata.o: 			    fdctdata.c
$(OBJDIR)/fdctref.o: 			    fdctref.c
$(OBJDIR)/idct.o: 			    idct.c
$(OBJDIR)/idctdata.o: 			    idctdata.c
$(OBJDIR)/quantize.o: 			    quantize.c
$(OBJDIR)/ratectl.o: 			    ratectl.c
$(OBJDIR)/stats.o: 			    stats.c
$(OBJDIR)/writepic.o: 			    writepic.c


$(OBJDIR)/bdist1_mmx.o: 		    bdist1_mmx.s
$(OBJDIR)/idct_mmx.o: 			    idct_mmx.s
$(OBJDIR)/mblockq_sad_mmxe.o: 		    mblockq_sad_mmxe.s
$(OBJDIR)/predcomp_mmxe.o: 		    predcomp_mmxe.s
$(OBJDIR)/bdist2_mmx.o: 		    bdist2_mmx.s
$(OBJDIR)/mblock_sad_mmx.o: 	 	    mblock_sad_mmx.s
$(OBJDIR)/predict_mmx.o: 		    predict_mmx.s
$(OBJDIR)/dist2_mmx.o:	    		    dist2_mmx.s
$(OBJDIR)/mblock_sad_mmxe.o:		    mblock_sad_mmxe.s
$(OBJDIR)/predcomp_mmx.o:		    predcomp_mmx.s
$(OBJDIR)/quant_mmx.o:	    		    quant_mmx.s
$(OBJDIR)/fdct_mmx.o:			    fdct_mmx.s

#$(OBJDIR)/mblock_sub44_sads.o: 	 	    mblock_sub44_sads.c
$(OBJDIR)/quantize_x86.o:		    quantize_x86.c

