// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright(c) 2015 Intel Corporation. All rights reserved.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "rimage.h"
#include "file_format.h"
#include "manifest.h"

static const struct adsp *machine[] = {
	&machine_byt,
	&machine_cht,
	&machine_bsw,
	&machine_hsw,
	&machine_bdw,
	&machine_apl,
	&machine_cnl,
	&machine_icl,
	&machine_jsl,
	&machine_tgl,
	&machine_sue,
	&machine_kbl,
	&machine_skl,
	&machine_imx8,
	&machine_imx8x,
	&machine_imx8m,
};

static struct fw_version parse_version(const char *ver_str)
{
	struct fw_version ver;
	int comps[4] = { 0 };
	int i = 0;
	char *tmp = strtok(ver_str, ".");

	while (i < 4 && tmp) {
		comps[i] = atoi(tmp);
		tmp = strtok(NULL, ".");
		++i;
	}

	ver.major_version = comps[0];
	ver.minor_version = comps[1];
	ver.hotfix_version = comps[2];
	ver.build_version = comps[3];

	return ver;
}

static void set_fw_image_version(struct image *image, struct fw_version ver)
{
	struct sof_man_fw_desc *desc = NULL;
	struct css_header_v1_8 *css = NULL;

	if (image->adsp->man_v1_5)
		desc = &image->adsp->man_v1_5->desc;

	if (image->adsp->man_v1_5_sue)
		desc = &image->adsp->man_v1_5_sue->desc;

	if (image->adsp->man_v1_8) {
		desc = &image->adsp->man_v1_8->desc;
		css = &image->adsp->man_v1_8->css;
	}

	if (image->adsp->man_v2_5) {
		desc = &image->adsp->man_v2_5->desc;
		css = &image->adsp->man_v2_5->css;
	}

	if (desc) {
		desc->header.major_version = ver.major_version;
		desc->header.minor_version = ver.minor_version;
		desc->header.hotfix_version = ver.hotfix_version;
		desc->header.build_version = ver.build_version;
	}

	if (css)
		css->version = ver;
}

static void usage(char *name)
{
	fprintf(stdout, "%s:\t -m machine -o outfile -k [key] ELF files\n",
		name);
	fprintf(stdout, "\t -v enable verbose output\n");
	fprintf(stdout, "\t -r enable relocatable ELF files\n");
	fprintf(stdout, "\t -s MEU signing offset\n");
	fprintf(stdout, "\t -p log dictionary outfile\n");
	fprintf(stdout, "\t -i set IMR type\n");
	fprintf(stdout, "\t -x set xcc module offset\n");
	fprintf(stdout, "\t -n fw version in format major.minor.micro.build\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	struct image image;
	const char *mach = NULL;
	int opt, ret, i, elf_argc = 0;
	int imr_type = MAN_DEFAULT_IMR_TYPE;
	struct fw_version fw_ver = { 0 }; // default version is 0.0.0.0

	memset(&image, 0, sizeof(image));

	image.xcc_mod_offset = DEFAULT_XCC_MOD_OFFSET;

	while ((opt = getopt(argc, argv, "ho:p:m:vba:s:k:l:ri:x:n:")) != -1) {
		switch (opt) {
		case 'o':
			image.out_file = optarg;
			break;
		case 'p':
			image.ldc_out_file = optarg;
			break;
		case 'm':
			mach = optarg;
			break;
		case 'v':
			image.verbose = 1;
			break;
		case 's':
			image.meu_offset = atoi(optarg);
			break;
		case 'a':
			image.abi = atoi(optarg);
			break;
		case 'k':
			image.key_name = optarg;
			break;
		case 'r':
			image.reloc = 1;
			break;
		case 'i':
			imr_type = atoi(optarg);
			break;
		case 'x':
			image.xcc_mod_offset = atoi(optarg);
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 'n':
			fw_ver = parse_version(optarg);
			break;
		default:
			break;
		}
	}

	elf_argc = optind;

	/* make sure we have an outfile and machine */
	if (!image.out_file || !mach)
		usage(argv[0]);

	if (!image.ldc_out_file)
		image.ldc_out_file = "out.ldc";

	/* find machine */
	for (i = 0; i < ARRAY_SIZE(machine); i++) {
		if (!strcmp(mach, machine[i]->name)) {
			image.adsp = machine[i];
			goto found;
		}
	}
	fprintf(stderr, "error: machine %s not found\n", mach);
	fprintf(stderr, "error: available machines ");
	for (i = 0; i < ARRAY_SIZE(machine); i++)
		fprintf(stderr, "%s, ", machine[i]->name);
	fprintf(stderr, "\n");

	return -EINVAL;

found:

	set_fw_image_version(&image, fw_ver);

	/* set IMR Type in found machine definition */
	if (image.adsp->man_v1_8)
		image.adsp->man_v1_8->adsp_file_ext.imr_type = imr_type;

	if (image.adsp->man_v2_5)
		image.adsp->man_v2_5->adsp_file_ext.imr_type = imr_type;

	/* parse input ELF files */
	image.num_modules = argc - elf_argc;
	for (i = elf_argc; i < argc; i++) {
		fprintf(stdout, "\nModule Reading %s\n", argv[i]);
		ret = elf_parse_module(&image, i - elf_argc, argv[i]);
		if (ret < 0)
			goto out;
	}

	/* validate all modules */
	ret = elf_validate_modules(&image);
	if (ret < 0)
		goto out;

	/* open outfile for writing */
	unlink(image.out_file);
	image.out_fd = fopen(image.out_file, "wb");
	if (!image.out_fd) {
		fprintf(stderr, "error: unable to open %s for writing %d\n",
			image.out_file, errno);
		ret = -EINVAL;
		goto out;
	}

	/* process and write output */
	if (image.meu_offset)
		ret = image.adsp->write_firmware_meu(&image);
	else
		ret = image.adsp->write_firmware(&image);

	unlink(image.ldc_out_file);
	image.ldc_out_fd = fopen(image.ldc_out_file, "wb");
	if (!image.ldc_out_fd) {
		fprintf(stderr, "error: unable to open %s for writing %d\n",
			image.ldc_out_file, errno);
		ret = -EINVAL;
		goto out;
	}
	ret = write_logs_dictionary(&image);
out:
	/* close files */
	if (image.out_fd)
		fclose(image.out_fd);

	if (image.ldc_out_fd)
		fclose(image.ldc_out_fd);

	return ret;
}
