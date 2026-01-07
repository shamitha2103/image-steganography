#ifndef DECODE_H
#define DECODE_H

#include"types.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4
typedef struct _DecodeInfo
{
    /* Secret file info */
    char *secret_fname;
    FILE *fptr_secret;

    /* Decoded Image info */
    char *decoded_image_fname;
    FILE *fptr_decoded_image;
    FILE *fptr_stego_file;
    /*stego file*/
    char *stego_image_fname;

    /* Image info */
    uint image_capacity;
} DecodeInfo;

/* Decoding function */

//read_and_validate_decode_args
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo);

//open_files
Status open_file(DecodeInfo *decInfo);

//decode length of magic string,extension and secret file size
Status decode_integers(FILE *fptr_secret, uint *integers);

Status decode_len(FILE *fptr_secret, int *len);
//decode magic string length and string
Status decode_magic_string(FILE *fptr_decoded_image, char *magic_string,int len);

//decode extensiom length and string
Status decode_secret_file_extn(FILE *fptr_stego_file, char *extn);

//decode secret file size
Status decode_secret_file_size(FILE *fptr_secret, int *file_size);

Status do_decoding(DecodeInfo *decInfo,int argc,char*argv[]);

//decode secret file data
Status decode_secret_file_data(DecodeInfo *decInfo, int file_size);

#endif