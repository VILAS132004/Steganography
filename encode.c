#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "common.h"
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    
    if(argv[2] != NULL)
    {
        int len = strlen(argv[2]);
        if(len > 4 && argv[2][len-4] == '.' && argv[2][len-3] == 'b' && argv[2][len-2] == 'm' && argv[2][len-1] == 'p' )
        {
            encInfo->src_image_fname = argv[2];
            encInfo->fptr_src_image = fopen(argv[2], "r");
            if (encInfo->fptr_src_image == NULL)
            {
                return e_failure;
            }
        }
        else
        {
            return e_failure;
        }
    }
    else
    {
        return e_failure;
    }

    if (argv[3] != NULL)
    {
        encInfo->secret_fname = argv[3];
        encInfo->fptr_secret = fopen(argv[3], "r");
        if (encInfo->fptr_secret == NULL)
        {
            return e_failure;
        }
    }
    else
    {
        return e_failure;
    }

    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "default.bmp";
    }

    int len_stego = strlen(encInfo->stego_image_fname);
    if(len_stego > 4 && encInfo->stego_image_fname[len_stego-4] == '.' && encInfo->stego_image_fname[len_stego-3] == 'b' && encInfo->stego_image_fname[len_stego-2] == 'm' && encInfo->stego_image_fname[len_stego-1] == 'p')
    {
        encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
        if(encInfo->fptr_stego_image == NULL)
        {
            return e_failure;
        }
    }
    else
    {
        return e_failure;
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
// 01
    if(check_capacity(encInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: check capacity failed, insufficient image size\n");
        return e_failure;
    }
//02
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to copy BMP header\n");
        return e_failure;
    }
// 03
    if(encode_magic_string(MAGIC_STRING,encInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to encode magic string\n");
        return e_failure;
    }
// 04
    char *dot=strchr(encInfo->secret_fname,'.');
    if(dot==NULL)
    {
        fprintf(stderr,"ERROR: secret file extension not found\n");
        return e_failure;
    }

    strcpy(encInfo->extn_secret_file,dot);
    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to encode secret file extension size\n");
        return e_failure;
    }

    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to encode secret file extension\n");
        return e_failure;
    }
/// 05
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to encode secret file size\n");
        return e_failure;
    }
// 06
    if(encode_secret_file_data(encInfo)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to encode secret file data\n");
        return e_failure;
    }
// 07
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure)
    {
        fprintf(stderr,"ERROR: failed to copy remaining image data\n");
        return e_failure;
    }
    return e_success;

}   


Status check_capacity(EncodeInfo *encInfo)
{

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    uint source_img_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    long req_size = 54 + (strlen(MAGIC_STRING)+ 4 + 4 + encInfo->size_secret_file) * 8;

    if(source_img_capacity > req_size)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }


   
}

uint get_file_size(FILE *fptr)
{

    fseek(fptr,0,SEEK_END);
    long size = ftell(fptr);
    rewind(fptr);
    return size;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);
    char buff[54];
    if(fread(buff,1,54,fptr_src_image)!=54)
    {
        return e_failure;
    }
    if(fwrite(buff,1,54,fptr_dest_image)!=54)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{

    char buff[8];
    for(int i=0;i<strlen(magic_string);i++)
    {
        fread(buff,1,8,encInfo->fptr_src_image);

        encode_byte_to_lsb(magic_string[i],buff);
        
        fwrite(buff,1,8,encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{

    for(int i=0;i<8;i++)
    {
        char bit = (data>>(7-i))&1;
        image_buffer[i] = (image_buffer[i]& ~1)|bit;
    }
    return e_success;
}

Status encode_secret_file_extn_size(uint size, EncodeInfo *encInfo)
{
    char buff[32];
    fread(buff,1,32,encInfo->fptr_src_image);
    encode_size_to_lsb(size,buff);
    fwrite(buff,1,32,encInfo->fptr_stego_image);
    return e_success;
}

Status encode_size_to_lsb(uint size, char *image_buffer)
{

    for(int i=0;i<32;i++)
    {
        char bit = (size>>(31-i))&1;
        image_buffer[i] = (image_buffer[i]& ~1)| bit;
    }
    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buff[8];
    for(int i=0;i<strlen(file_extn);i++)
    {
        fread(buff,1,8,encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],buff);
        fwrite(buff,1,8,encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{

    char buff[32];
    fread(buff,1,32,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,buff);
    fwrite(buff,1,32,encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{

    char buff[8];
    char data;

    rewind(encInfo->fptr_secret);
    for(long i=0;i<encInfo->size_secret_file;i++)
    {
        fread(buff,1,8,encInfo->fptr_src_image);
        fread(&data,1,1,encInfo->fptr_secret);
        encode_byte_to_lsb(data,buff);
        fwrite(buff,1,8,encInfo->fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while(fread(&ch,1,1,fptr_src)==1)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;
}