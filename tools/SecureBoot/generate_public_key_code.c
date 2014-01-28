#include "rsa_params.h"

#include <stdio.h>
#include <stdlib.h>

static const char header_name[] = "rsa_public_key.h";
static const char header_prefix[] =
	"/*\n"
	" * Automatically generated from generate_public_key_code.c\n"
	" *\n"
	" * DO NOT EDIT.\n"
	" */\n"
	"\n"
	"#ifndef _RSA_PUBLIC_KEY_H\n"
	"#define _RSA_PUBLIC_KEY_H\n"
	"\n"
	"#include <common.h>\n"
	"\n";

static const char header_suffix[] = "\n#endif\n";

static const int col_width = 6;

/*
 * Formats and outputs a uint32_t array as C code to a file.
 *
 * Returns 0 on success, non-zero if unable to write any of the parts.
 */
static int _output_int32_t_arr(FILE *fp, char *arr_name, uint32_t *arr,
		unsigned int len) {
	int i;

	if (fprintf(fp, "uint32_t %s[] = {\n\t", arr_name) <= 0) {
		return 1;
	}

	/* Print all but the last element. */
	for (i = 0; i < len - 1; ++i) {
		if ((i % col_width) == 0 && i > 0) {
			if (fprintf(fp, ",\n\t") != 3) return 1;
		}

		if ((i % col_width) > 0) {
			if (fprintf(fp, ", ") != 2) return 1;
		}

		if (fprintf(fp, "0x%.8x", arr[i]) != 10) return 1;
	}

	/* Print the last element */
	if ((i % col_width) == 0 && i > 0) {
		if (fprintf(fp, ",\n\t") != 3) return 1;
		if (fprintf(fp, "0x%.8x", arr[i]) != 10) return 1;
	} else {
		if (fprintf(fp, ", 0x%.8x", arr[i]) != 12) return 1;
	}

	if (fprintf(fp, "\n};\n") != 4) return 1;

	return 0;
}

static int _generate_verify_data_code(uint32_t *modulus, uint32_t *rr,
		uint32_t n0inv, unsigned int len) {
	int num_elements, i, num_chars;
	FILE *header_file;
	int ret = 0;

	header_file = fopen(header_name, "w");
	if (!header_file) {
		fprintf(stderr, "Unable to open file %s\n", header_name);
		return 1;
	}

	if (fprintf(header_file, header_prefix) != (sizeof(header_prefix) - 1)) {
		fprintf(stderr, "Unable to write prefix to header file.\n");
		ret = 1;
		goto end;
	}

	if (fprintf(header_file, "uint len = %uu;\n", len) <= 0) {
		fprintf(stderr, "Unable to write len to header file.\n");
		ret = 1;
		goto end;
	}

	if (fprintf(header_file, "uint32_t n0inv = %uu;\n", n0inv) <= 0) {
		fprintf(stderr, "Unable to write n0inv to header file.\n");
		ret = 1;
		goto end;
	}

	// Convert len from bits to multiple of uint32_t.
	num_elements = len / (8 * sizeof(uint32_t));

	if (_output_int32_t_arr(header_file, "modulus", modulus, num_elements)) {
		fprintf(stderr, "Unable to write modulus to header file.\n");
		ret = 1;
		goto end;
	}

	if (_output_int32_t_arr(header_file, "rr", rr, num_elements)) {
		fprintf(stderr, "Unable to write rr to header file.\n");
		ret = 1;
		goto end;
	}

	if (fprintf(header_file, header_suffix) != (sizeof(header_suffix) - 1)) {
		fprintf(stderr, "Unable to write suffix to header file.\n");
		ret = 1;
		goto end;
	}

end:
	close(header_file);

	return ret;
}

int main(int argc, char *argv[]) {
	uint32_t *modulus, *r_squared;
	uint32_t n0inv;
	int len, ret;

	if (argc < 2) {
		printf("Usage: %s pub_key_file\n", argv[0]);
		printf("  The resultant code is written to rsa_public_key.h\n\n");
		return 1;
	}

	modulus = NULL;
	r_squared = NULL;

	ret = rsa_get_verify_data(argv[1], &modulus, &r_squared, &n0inv, &len);
	if (ret) {
		fprintf(stderr, "Error in getting parameters\n");
		goto end;
	}

	ret = _generate_verify_data_code(modulus, r_squared, n0inv, len);
	if (ret) {
		fprintf(stderr, "Error generating verify data code\n");
		goto end;
	}

end:
	if (modulus != NULL)
		free(modulus);
	if (r_squared != NULL)
		free(r_squared);

	return ret;
}
