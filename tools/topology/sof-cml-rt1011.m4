#
# Topology for Cometlake with rt1011 spk on SSP1
#
#

# Include SOF CML RT5682 Topology
# This includes topology for RT5682, DMIC and 3 HDMI Pass through pipeline

include(`utils.m4')
include(`dai.m4')
include(`pipeline.m4')
include(`ssp.m4')

# Include TLV library
 include(`common/tlv.m4')
#
# # Include Token library
 include(`sof/tokens.m4')
#
# # Include Cannonlake or Icelake DSP configuration
 ifelse(PLATFORM, `icl', include(`platform/intel/icl.m4'),
         ifelse(PLATFORM, `whl', include(`platform/intel/cnl.m4'),
                         ifelse(PLATFORM, `cml', include(`platform/intel/cnl.m4'), `')))
#                         include(`platform/intel/dmic.m4')


DEBUG_START
#
# Define the Speaker pipeline
#
# PCM5  ----> volume (pipe 7) ----> SSP1 (speaker - rt1011, BE link 5)
#

dnl PIPELINE_PCM_ADD(pipeline,
dnl     pipe id, pcm, max channels, format,
dnl     frames, deadline, priority, core)

# Low Latency playback pipeline 7 on PCM 5 using max 2 channels of s32le.
# Schedule 48 frames per 1000us deadline on core 0 with priority 0
PIPELINE_PCM_ADD(sof/pipe-src-volume-playback.m4,
	1, 5, 2, s32le,
	48, 1000, 0, 0)

#
# Speaker DAIs configuration
#

dnl DAI_ADD(pipeline,
dnl     pipe id, dai type, dai_index, dai_be,
dnl     buffer, periods, format,
dnl     frames, deadline, priority, core)

# playback DAI is SSP1 using 2 periods
# Buffers use s16le format, with 48 frame per 1000us on core 0 with priority 0
DAI_ADD(sof/pipe-dai-playback.m4,
	1, SSP, 1, SSP1-Codec,
	PIPELINE_SOURCE_1, 2, s16le,
	48, 1000, 0, 0)

# PCM Low Latency, id 0
dnl PCM_PLAYBACK_ADD(name, pcm_id, playback)
PCM_PLAYBACK_ADD(Speakers, 5, PIPELINE_PCM_1)

#
# BE configurations for Speakers - overrides config in ACPI if present
#

#SSP 1 (ID: 5)
DAI_CONFIG(SSP, 1, 5, SSP1-Codec,
	SSP_CONFIG(DSP_B, SSP_CLOCK(mclk, 24000000, codec_mclk_in),
		SSP_CLOCK(bclk, 3000000, codec_slave),
		SSP_CLOCK(fsync, 46875, codec_slave),
		SSP_TDM(2, 16, 3, 3),
		SSP_CONFIG_DATA(SSP, 1, 16)))

DEBUG_END
