#
# Topology for Apollo Lake with direct attach digital microphones array
#

# Capture configuration
# DAI0 none
# DAI1 2ch 32b mic3-4

# Include topology builder
include(`utils.m4')
include(`dai.m4')
include(`pipeline.m4')

# Include TLV library
include(`common/tlv.m4')

# Include Token library
include(`sof/tokens.m4')

# Include Apollolake DSP configuration
include(`platform/intel/bxt.m4')
include(`platform/intel/dmic.m4')

#
# Define the pipelines
#
# PCM7 <----- DMIC7 (DMIC16k)
#

dnl PIPELINE_PCM_ADD(pipeline,
dnl     pipe id, pcm, max channels, format,
dnl     frames, deadline, priority, core)

# Passthrough capture pipeline 7 on PCM 7 using max channels 2.
# Set 1000us deadline on core 0 with priority 0
PIPELINE_PCM_ADD(sof/pipe-volume-capture-16khz.m4,
	7, 7, 2, s32le,
	1000, 0, 0,
	16000, 16000, 16000)

#
# DAIs configuration
#

dnl DAI_ADD(pipeline,
dnl     pipe id, dai type, dai_index, dai_be,
dnl     buffer, periods, format,
dnl     frames, deadline, priority, core)

# capture DAI is DMIC 1 using 2 periods
# Buffers use s32le format, 1000us deadline on core 0 with priority 0
DAI_ADD(sof/pipe-dai-capture.m4,
	7, DMIC, 1, NoCodec-7,
	PIPELINE_SINK_7, 2, s32le,
	1000, 0, 0)


dnl PCM_DUPLEX_ADD(name, pcm_id, playback, capture)
dnl PCM_CAPTURE_ADD(name, pipeline, capture)
PCM_CAPTURE_ADD(DMIC16k, 7, PIPELINE_PCM_7)

#
# BE configurations - overrides config in ACPI if present
#

dnl DAI_CONFIG(type, dai_index, link_id, name, ssp_config/dmic_config)

DAI_CONFIG(DMIC, 1, 7, NoCodec-7,
	   dnl DMIC_CONFIG(driver_version, clk_min, clk_mac, duty_min, duty_max,
	   dnl		   sample_rate, fifo word length, type, dai_index,
	   dnl             pdm controller config)
	   DMIC_CONFIG(1, 500000, 4800000, 40, 60, 16000,
		dnl DMIC_WORD_LENGTH(frame_format)
		DMIC_WORD_LENGTH(s32le), DMIC, 1,
		dnl PDM_CONFIG(type, dai_index, num pdm active, pdm tuples list)
		dnl STEREO_PDM0 is a pre-defined pdm config for stereo capture
		PDM_CONFIG(DMIC, 1, STEREO_PDM1)))
