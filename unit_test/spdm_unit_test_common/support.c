/**
 *  Copyright Notice:
 *  Copyright 2021 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "spdm_unit_test.h"

void libspdm_dump_hex_str(const uint8_t *buffer, uintn buffer_size)
{
    uintn index;

    for (index = 0; index < buffer_size; index++) {
        printf("%02x", buffer[index]);
    }
}

void libspdm_dump_data(const uint8_t *buffer, uintn buffer_size)
{
    uintn index;

    for (index = 0; index < buffer_size; index++) {
        printf("%02x ", buffer[index]);
    }
}

void libspdm_dump_hex(const uint8_t *data, uintn size)
{
    uintn index;
    uintn count;
    uintn left;

#define COLUME_SIZE (16 * 2)

    count = size / COLUME_SIZE;
    left = size % COLUME_SIZE;
    for (index = 0; index < count; index++) {
        printf("%04x: ", (uint32_t)(index * COLUME_SIZE));
        libspdm_dump_data(data + index * COLUME_SIZE, COLUME_SIZE);
        printf("\n");
    }

    if (left != 0) {
        printf("%04x: ", (uint32_t)(index * COLUME_SIZE));
        libspdm_dump_data(data + index * COLUME_SIZE, left);
        printf("\n");
    }
}

bool libspdm_read_input_file(const char *file_name, void **file_data,
                             uintn *file_size)
{
    FILE *fp_in;
    uintn temp_result;

    if ((fp_in = fopen(file_name, "rb")) == NULL) {
        printf("Unable to open file %s\n", file_name);
        *file_data = NULL;
        return false;
    }

    fseek(fp_in, 0, SEEK_END);
    *file_size = ftell(fp_in);

    *file_data = (void *)malloc(*file_size);
    if (NULL == *file_data) {
        printf("No sufficient memory to allocate %s\n", file_name);
        fclose(fp_in);
        return false;
    }

    fseek(fp_in, 0, SEEK_SET);
    temp_result = fread(*file_data, 1, *file_size, fp_in);
    if (temp_result != *file_size) {
        printf("Read input file error %s", file_name);
        free((void *)*file_data);
        fclose(fp_in);
        return false;
    }

    fclose(fp_in);

    return true;
}

bool libspdm_write_output_file(const char *file_name, const void *file_data,
                               uintn file_size)
{
    FILE *fp_out;

    if ((fp_out = fopen(file_name, "w+b")) == NULL) {
        printf("Unable to open file %s\n", file_name);
        return false;
    }

    if ((fwrite(file_data, 1, file_size, fp_out)) != file_size) {
        printf("Write output file error %s\n", file_name);
        fclose(fp_out);
        return false;
    }

    fclose(fp_out);

    return true;
}
