#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(argv[2]==NULL)
    {
        fprintf(stderr, "ERROR : stego img arg not found\n");
        return e_failure;
    }

    // validating whether it is a .bmp file or not 

    char *extnn = strrchr(argv[2],'.');
    if(extnn==NULL||strcmp(extnn,".bmp")!=0)
    {
        fprintf(stderr,"ERROR : stego img must be .bmp file\n");
        return e_failure;
    }
    decInfo->stego_image_fname=argv[2];

    // checking for the optional filenmae 
    if(argv[3]!=NULL)
    {
        strcpy(decInfo->output_fname,argv[3]);
    }
    else{
        strcpy(decInfo->output_fname, "decoded_secret.txt");
        printf("output file not mentioned done using default file\n");
    }
    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    // open stego img in read binary mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if(decInfo->fptr_stego_image==NULL)
    {
        fprintf(stderr,"ERROR: unable to open stego img file\n");
        return e_failure;
    }

    //  open o/p file in write mode
    decInfo->fptr_output = fopen(decInfo->output_fname,"w");
    if(decInfo->fptr_output==NULL)
    {
        fprintf(stderr,"ERROR: unable to create output file\n");
        return e_failure;
    }

    return e_success;
}

Status skip_bmp_header(FILE *fptr_stego_image)
{
    if(fseek(fptr_stego_image,54,SEEK_SET)!=0)
    {
        fprintf(stderr,"ERROR: failed to skip the bmp header\n");
        return e_failure;
    }
    
    return e_success;
}

Status decode_byte_from_lsb(char *data,char *image_buffer)
{
    *data = 0;
    for(int i=0;i<8;i++)
    {
        *data = (*data<<1)|(image_buffer[i]&1);
    }
    return e_success;
}
Status decode_size_from_lsb(long *size,char *image_buffer)
{
    *size = 0;
    for(int i=0;i<32;i++)
    {
        *size = (*size<<1)|(image_buffer[i]&1);
    }
    return e_success;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;
    for(int i=0;i<2;i++)
    {
        if(fread(image_buffer,1,8,decInfo->fptr_stego_image)!=8)
        {
            return e_failure;
        }
        
        decode_byte_from_lsb(&ch,image_buffer);
        if(ch!=magic_string[i])
        {
            return e_failure;
        }
       
    }
     return e_success;
}

Status decode_secret_file_extn_size(int *size,DecodeInfo *decInfo)
{
    char image_buffer[32];
    long decoded_size = 0;
    if(fread(image_buffer,1,32,decInfo->fptr_stego_image)!=32)
    {
        return e_failure;
    }
    decode_size_from_lsb(&decoded_size,image_buffer);
    *size=(int)decoded_size;

    return e_success;
}

Status decode_secret_file_extn(char *file_extn,int size,DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;
    for(int i=0;i<size;i++)
    {
        if(fread(image_buffer,1,8,decInfo->fptr_stego_image)!=8)
        {
            return e_failure;
        }

        decode_byte_from_lsb(&ch,image_buffer);
        file_extn[i]=ch;
    }
    file_extn[size]='\0';
    strcpy(decInfo->extn_decoded_file,file_extn);

    return e_success;
}

Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo)
{
    char image_buffer[32];
    if(fread(image_buffer,1,32,decInfo->fptr_stego_image)!=32)
    {
        return e_failure;
    }

    decode_size_from_lsb(file_size,image_buffer);
    decInfo->size_secret_file = *file_size;

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;
    for(long i=0;i<decInfo->size_secret_file;i++)
    {
        if(fread(image_buffer,1,8,decInfo->fptr_stego_image)!=8)
        {
            return e_failure;
        }
        decode_byte_from_lsb(&ch,image_buffer);
        fputc(ch,decInfo->fptr_output);
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to open decode files\n");
        return e_failure;
    }

    if(skip_bmp_header(decInfo->fptr_stego_image)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to skip bmp header\n");
        return e_failure;
    }

    if(decode_magic_string(MAGIC_STRING,decInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: magic string is mismatching\n");
        return e_failure;
    }

    int extn_size=0;
    if(decode_secret_file_extn_size(&extn_size,decInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to decode the extension size\n");
        return e_failure;
    }

    char file_extn[32];
    if(decode_secret_file_extn(file_extn,extn_size,decInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to decode file extension\n");
        return e_failure;
    }

    long file_size=0;
    if(decode_secret_file_size(&file_size,decInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to decode file size\n");
        return e_failure;
    }

    if(decode_secret_file_data(decInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to decode file data\n");
        return e_failure;
    }

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    printf("decoding finished successfully\n");
    return e_success;
}   
