#include<stdio.h>
#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char output_fname[256];
    FILE *fptr_output;
    char extn_decoded_file[32];
    long size_secret_file;
}DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status open_decode_files(DecodeInfo *decInfo);

Status skip_bmp_header(FILE *fptr_stego_image);

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

Status decode_byte_from_lsb(char *data, char *image_buffer);

Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo);

Status decode_size_from_lsb(long *size, char *image_buffer);

Status decode_secret_file_extn(char *file_extn,int size,DecodeInfo *decInfo);

Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo);

Status decode_secret_file_data(DecodeInfo *decInfo);

#endif

