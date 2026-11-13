#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Insufficient Arguments.\n");
        return e_failure;
    }
    
    OperationType opt = checks_operation_type(argv[1][1]);


    if(opt == e_encode)
    {
        EncodeInfo encInfo;
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Arguments Validation is done.\n");

            if(do_encoding(&encInfo)==e_success)
            {
                printf("encoding done successfully\n");
            }
            else
            {
                printf("encoding failed\n");
                return e_failure;
            }
        }
        else{
            printf("Arguments Validation is failed.\n");
            return e_failure;
        }
    }
    else if(opt == e_decode)
    {
        DecodeInfo decInfo;
        if(read_and_validate_decode_args(argv,&decInfo)==e_success)
        {
            printf("Arguments Validation is done\n");

            if(do_decoding(&decInfo)==e_success)
            {
                printf("decoding done successfully\n");
            }
            else
            {
                printf("decoding failed\n");
                return e_failure;
            }
        }
        else
        {
            printf("Arguments Validation is failed\n");
            return e_failure;
        }
    }
    else
    {
        printf("pass '-e' for encoding or '-d' for decoding\n");
        return e_failure;
    }
    return 0;
}

OperationType check_operation_type(char opt)
{
    /*
        if opt is 'e'
        return e_encode

        if opt is 'd'
        return e_decode

        return e_unsupported
    */

    if(opt == 'e')
    {
        return e_encode;
    }
    else if (opt == 'd')
    {
        return e_decode;
    }
    else{
        return e_unsupported;
    }

}
