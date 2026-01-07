
#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"

OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e")==0 || strcmp(argv[1], "-E")==0 )
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d")==0 || strcmp(argv[1], "-D")==0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    } 
}

int main(int argc,char*argv[])
{

    
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    if(argc>3 && check_operation_type(argv)== e_encode)
    {
        
        if(do_encoding(&encInfo,argc,argv)==e_success)
        {
            printf("-------Encoding completed-------\n");
        }
        else
        {
            printf("------Encoding failed------\n");
            return e_failure;
        }
        
    }
    else if(argc>2 && check_operation_type(argv)== e_decode)
    {
        
        
        if(do_decoding(&decInfo,argc,argv)!=e_success)
        {
            printf("------Decoding failed------\n");
            return e_failure;
        }
        
    }
    else
    {
        printf("ERROR: Insufficient arguments\n");
        printf("Usage: \n ./a.out -e <bmp_file> <secret_file> <output_file>\n");
        printf("./a.out -d <stego_image.bmp> [<decoded_file.txt>]\n");
        
    }
}
