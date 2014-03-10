/*****************************************************************************
 * asa: portable digital subtitle renderer
 *****************************************************************************
 * Copyright (C) 2007  David Lamparter
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "acconf.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include <csri/csri.h>
#include <csri/logging.h>
#include <csri/openerr.h>
#include <csri/stream.h>

#include "render.h"

csri_rend *r;

static int do_usage(FILE *fd)
{
	fprintf(fd, "usage: csri [COMMON-OPTIONS] COMMAND [COMMAND-OPTIONS]\n"
		"\n"
		"common options: [-r renderer [-s specific]]\n"
		"\t-r\tselect a renderer, by name\n"
		"\t-s\tselect a renderer version, by specific name\n"
		"\n"
		"commands:\n"
		"\tlist\tshow installed renderers\n"
		"\tinfo\tshow detailed renderer information\n"
		"\trender\trender subtitle output\n"
#ifdef HAVE_LIBPNG
		"\t\t-i FILE\t\tread background from PNG file\n"
		"\t\t-o PREFIX\twrite output to PREFIX_nnnn.png\n"
#endif
		"\t\t-A\t\tkeep alpha\n"
		"\t\t-t [start][:[end][:[step]]]\tspecify timestamps to be rendered\n"
		"\t\tSUBFILE\t\tsubtitle file to load\n"
		"\n");
	return 2;
}

static int do_list(int argc, char **argv)
{
	unsigned num = 0;

	if (argc)
		return do_usage(stderr);

	while (r) {
		struct csri_info *info = csri_renderer_info(r);
		if (!info)
			continue;
		printf("%s:%s %s, %s, %s\n", info->name, info->specific,
			info->longname, info->author, info->copyright);
		r = csri_renderer_next(r);
		num++;
	}
	fprintf(stderr, "%u renderers found\n", num);
	return num > 0 ? 0 : 1;
}

static csri_ext_id known_exts[] = {
	CSRI_EXT_OPENERR,
	CSRI_EXT_LOGGING,
	CSRI_EXT_STREAM,
	CSRI_EXT_STREAM_ASS,
	CSRI_EXT_STREAM_TEXT,
	CSRI_EXT_STREAM_DISCARD,
	NULL
};

static const char *dummy_script = "[Script Info]\r\n"
	"ScriptType: v4.00\r\n"
	"[V4 Styles]\r\n"
	"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, "
		"TertiaryColour, BackColour, Bold, Italic, BorderStyle, "
		"Outline, Shadow, Alignment, MarginL, MarginR, MarginV, "
		"AlphaLevel, Encoding\r\n"
	"Style: Default,Arial,20,&HFFFFFF&,&H00FFFF&,&H000000&,&H000000&,"
		"0,0,1,2,2,2,10,10,10,0,0\r\n"
	"[Events]\r\n"
	"Format: Marked, Start, End, Style, Name, "
		"MarginL, MarginR, MarginV, Effect, Text\r\n"
	"Dialogue: Marked=0,0:00:01.00,0:00:02.00,Default,,0000,0000,0000,,"
		"test\r\n";

static const char *dummy_stream = "1,0,Default,,0000,0000,0000,,stream\r\n";

#define e(x) { #x, CSRI_F_ ## x }
#define ree(x) e(RGB ## x), e(x ## RGB), e(BGR ## x), e(x ## BGR)
static struct csri_fmtlistent {
	const char *label;
	enum csri_pixfmt fmt;
} csri_fmts[] = {
	ree(A), ree(_),
	e(RGB), e(BGR),
	e(AYUV), e(YUVA), e(YVUA), e(YUY2), e(YV12A), e(YV12),
	{ NULL, 0 }
};

static void listfmts()
{
	csri_inst *i;
	struct csri_fmtlistent *fmt;
	struct csri_fmt f;

	printf("\ntrying to get list of supported colorspaces:\n");
	fflush(stdout);
	i = csri_open_mem(r, dummy_script, strlen(dummy_script), NULL);

	f.width = f.height = 256;
	for (fmt = csri_fmts; fmt->label; fmt++) {
		f.pixfmt = fmt->fmt;
		if (!csri_request_fmt(i, &f))
			printf("\t[%04x] %s\n", fmt->fmt, fmt->label);
	}
	csri_close(i);
}

static int do_info(int argc, char **argv)
{
	struct csri_info *info;
	csri_ext_id *id;
	if (argc)
		return do_usage(stderr);

	info = csri_renderer_info(r);
	if (!info)
		return 1;
	printf("%s:%s\n\t%s\n\t%s\n\t%s\n", info->name, info->specific,
		info->longname, info->author, info->copyright);
	printf("supported CSRI extensions:\n");
	for (id = known_exts; *id; id++) {
		void *rext = csri_query_ext(r, *id);
		void *lext = csri_query_ext(NULL, *id);
		if (lext || rext) {
			printf("\t%s ", *id);
			if (!lext)
				printf("\n");
			else if (!rext)
				printf("[library only]\n");
			else if (rext == lext)
				printf("[emulated by library]\n");
			else
				printf("\n");
		}
	}
	listfmts();
	return 0;
}

static void logfunc(void *appdata, enum csri_logging_severity sev,
	const char *message)
{
	char severity[32];
	switch (sev) {
	case CSRI_LOG_DEBUG:	strcpy(severity, "[debug]"); break;
	case CSRI_LOG_INFO:	strcpy(severity, "[info]"); break;
	case CSRI_LOG_NOTICE:	strcpy(severity, "[notice]"); break;
	case CSRI_LOG_WARNING:	strcpy(severity, "[warning]"); break;
	case CSRI_LOG_ERROR:	strcpy(severity, "[error]"); break;
	default:	snprintf(severity, 32, "[%d?]", (int)sev);
	}
	fprintf(stderr, "%-10s %s\n", severity, message);
}

static int real_render(double *times, const char *infile, const char *outfile,
	const char *script, enum csri_pixfmt pfmt)
{
	struct csri_frame *bg, *a;
	csri_inst *inst;
	struct csri_fmt fmt;
	double now;
	int idx;
	uint32_t width = 640, height = 480;

	bg = infile ? png_load(infile, &width, &height, pfmt)
		: frame_alloc(width, height, pfmt);
	a = frame_alloc(width, height, pfmt);
	if (!bg || !a) {
		fprintf(stderr, "failed to allocate frame\n");
		if (!bg)
			fprintf(stderr, "\t- problem with background.\n");
		return 2;
	}
	inst = csri_open_file(r, script, NULL);
	if (!inst) {
		fprintf(stderr, "failed to open script \"%s\"\n", script);
		return 2;
	}
	fmt.pixfmt = pfmt;
	fmt.width = width;
	fmt.height = height;
	if (csri_request_fmt(inst, &fmt)) {
		fprintf(stderr, "format not supported by renderer\n");
		return 2;
	}

	idx = 0;
	for (now = times[0]; now <= times[1]; now += times[2]) {
		frame_copy(a, bg, width, height);
		csri_render(inst, a, now);
		if (outfile) {
			char buffer[256];
			snprintf(buffer, sizeof(buffer),
				 "%s_%04d.png", outfile, idx);
			printf("%s\n", buffer);
			png_store(a, buffer, width, height);
		}
		idx++;
	}
	csri_close(inst);
	inst = NULL;
	frame_free(bg);
	frame_free(a);
	return 0;
}

static int do_render(int argc, char **argv)
{
	double times[3] = {0.0, 0.0, 1.0};
	const char *outfile = NULL, *infile = NULL;
	struct csri_fmtlistent *fmte;
	int keepalpha = 0;
	enum csri_pixfmt fmt = ~0U;
	argv--, argc++;
	while (1) {
		int c, i;
		const char *short_options = "t:o:i:F:A";
		char *arg, *end, *err;
		struct option long_options[] = {
			{"time", 1, 0, 't'},
			{"output", 1, 0, 'o'},
			{"input", 1, 0, 'i'},
			{"format", 0, 0, 'F'},
			{"alpha", 0, 0, 'A'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 't':
			arg = optarg;
			for (i = 0; i < 3; i++) {
				end = strchr(arg, ':');
				if (end)
					*end = '\0';
				if (*arg) {
					times[i] = strtod(arg, &err);
					if (*err) {
						fprintf(stderr,
							"invalid time: %s\n",
							arg);
						return do_usage(stderr);
					}
				}
				if (!end)
					break;
				arg = end + 1;
			}
			break;
		case 'i':
			infile = optarg;
			break;
		case 'o':
			outfile = optarg;
			break;
		case 'A':
			if (fmt != ~0U)
				return do_usage(stderr);
			keepalpha = 1;
			break;
		case 'F':
			if (keepalpha || fmt != ~0U)
				return do_usage(stderr);
			for (fmte = csri_fmts; fmte->label; fmte++)
				if (!strcmp(fmte->label, optarg))
					break;
			if (!fmte->label)
				return do_usage(stderr);
			fmt = fmte->fmt;
			break;
		default:
			return do_usage(stderr);
		};
	}
	if (fmt == ~0U)
		fmt = keepalpha ? CSRI_F_RGBA : CSRI_F_RGB_;
	if (!isfinite(times[0])) {
		fprintf(stderr, "invalid start time\n");
		return do_usage(stderr);
	}
	if (!isfinite(times[1]) || times[1] < times[0]) {
		fprintf(stderr, "invalid end time\n");
		return do_usage(stderr);
	}
	if (!isnormal(times[2]) || times[2] < 0.0) {
		fprintf(stderr, "invalid end time\n");
		return do_usage(stderr);
	}
	if (argc - optind != 1) {
		fprintf(stderr, "script name missing\n");
		return do_usage(stderr);
	}
	return real_render(times, infile, outfile, argv[optind], fmt);
}

static int do_streamtest(int argc, char **argv)
{
	const char *outfile = NULL;
	struct csri_fmtlistent *fmte;
	enum csri_pixfmt pfmt = ~0U;
	struct csri_frame *bg, *a;
	csri_inst *inst;
	struct csri_fmt fmt;
	uint32_t width = 640, height = 480;
	struct csri_stream_ext *sext;

	argv--, argc++;
	while (1) {
		int c;
		const char *short_options = "o:F:";
		struct option long_options[] = {
			{"output", 1, 0, 'o'},
			{"format", 0, 0, 'F'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c == -1)
			break;
		switch (c) {
		case 'o':
			outfile = optarg;
			break;
		case 'F':
			if (pfmt != ~0U)
				return do_usage(stderr);
			for (fmte = csri_fmts; fmte->label; fmte++)
				if (!strcmp(fmte->label, optarg))
					break;
			if (!fmte->label)
				return do_usage(stderr);
			pfmt = fmte->fmt;
			break;
		default:
			return do_usage(stderr);
		};
	}
	if (pfmt == ~0U)
		pfmt = CSRI_F_RGB_;

	sext = (struct csri_stream_ext *)csri_query_ext(r,
		CSRI_EXT_STREAM_ASS);
	if (!sext) {
		fprintf(stderr, "renderer does not support ASS streaming\n");
		return 2;
	}

	bg = frame_alloc(width, height, pfmt);
	a = frame_alloc(width, height, pfmt);
	if (!bg || !a) {
		fprintf(stderr, "failed to allocate frame\n");
		return 2;
	}
	inst = sext->init_stream(r, dummy_script, strlen(dummy_script), NULL);
	if (!inst) {
		fprintf(stderr, "failed to initialize stream\n");
		return 2;
	}
	fmt.pixfmt = pfmt;
	fmt.width = width;
	fmt.height = height;
	if (csri_request_fmt(inst, &fmt)) {
		fprintf(stderr, "format not supported by renderer\n");
		return 2;
	}

	frame_copy(a, bg, width, height);
	csri_render(inst, a, 1.75);
	if (outfile) {
		char buffer[256];
		snprintf(buffer, sizeof(buffer),
			 "%s_nstream.png", outfile);
		printf("%s\n", buffer);
		png_store(a, buffer, width, height);
	}

	frame_copy(a, bg, width, height);
	sext->push_packet(inst, dummy_stream, strlen(dummy_stream),
		1.5, 2.0);
	csri_render(inst, a, 1.75);
	if (outfile) {
		char buffer[256];
		snprintf(buffer, sizeof(buffer),
			 "%s_stream.png", outfile);
		printf("%s\n", buffer);
		png_store(a, buffer, width, height);
	}

	csri_close(inst);
	inst = NULL;
	frame_free(bg);
	frame_free(a);
	return 0;
}

int main(int argc, char **argv)
{
	struct csri_logging_ext *logext;

	if (argc < 2)
		return do_usage(stderr);

	logext = (struct csri_logging_ext *)csri_query_ext(NULL,
		CSRI_EXT_LOGGING);
	if (logext && logext->set_logcallback)
		logext->set_logcallback(logfunc, NULL);
	else
		fprintf(stderr, "warning: unable to set log callback\n");

	r = csri_renderer_default();
	argc--, argv++;
	if (!strcmp(argv[0], "list"))
		return do_list(argc - 1, argv + 1);
	if (!strcmp(argv[0], "-r")) {
		const char *name = NULL, *spec = NULL;
		if (argc < 2)
			return do_usage(stderr);
		name = argv[1];
		argc -= 2, argv += 2;
		if (!strcmp(argv[0], "-s")) {
			if (argc < 2)
				return do_usage(stderr);
			spec = argv[1];
		}
		r = csri_renderer_byname(name, spec);
		if (!r) {
			fprintf(stderr, "renderer %s:%s not found.\n",
				name, spec ? spec : "*");
			return 2;
		}
	}
	if (!strcmp(argv[0], "info"))
		return do_info(argc - 1, argv + 1);
	if (!strcmp(argv[0], "render"))
		return do_render(argc - 1, argv + 1);
	if (!strcmp(argv[0], "streamtest"))
		return do_streamtest(argc - 1, argv + 1);
	return do_usage(stderr);
}
