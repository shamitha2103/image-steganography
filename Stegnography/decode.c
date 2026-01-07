#include<stdio.h>
#include "decode.h"
#include "types.h"
#include<string.h>
#include<stdlib.h>

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    // Check for number of input arguments
    if (argc < 3)
    {
        printf("ERROR: Insufficient arguments\n");
        printf("Usage: \n ./a.out -e <bmp_file> <secret_file> <output_file>\n");
        printf("./a.out -d <stego_image.bmp> [<decoded_file.txt>]\n");
        return e_failure;

    }
    // Validate the arguments
    if(strstr(argv[2],".bmp"))
    {
        decInfo->stego_image_fname=argv[2];

        //check if output file name giving is optional

        if(argc>3)
        {
            decInfo->decoded_image_fname=argv[3];
        }
        else
        {
            printf("INFO: Output file name not provided, using default name\n");
            decInfo->decoded_image_fname = "decoded_secret.txt";
        }
        return e_success;
    
    }
    else
    {
        printf("INFO : %s is not a bmp file\n",argv[2]);
        return e_failure;
    }
}

Status open_file(DecodeInfo *decInfo)
{
    // Open the stego file in read mode
    decInfo->fptr_stego_file=fopen(decInfo->stego_image_fname,"rb");
    // Do Error handling
    if(decInfo->fptr_stego_file==NULL)
    {
        perror("fopen");
        fprintf(stderr,"ERROR: Unable to open file %s\n",decInfo->stego_image_fname);
        return e_failure;
    }
    printf("INFO : Open file succesfully\n");
    // Open the decoded file in write mode
    if(decInfo->decoded_image_fname!=NULL)
    {
        decInfo->fptr_decoded_image=fopen(decInfo->decoded_image_fname,"wb");
        // Do Error handling
        if(decInfo->fptr_decoded_image==NULL)
        {
            perror("fopen");
            fprintf(stderr,"ERROR: Unable to open file %s\n",decInfo->decoded_image_fname);
            return e_failure;
        }
        printf("INFO : Opened output file successfully\n");
    }
    else
    {
        perror("fopen");
        fprintf(stderr,"INFO : No output file name provided \n");
        fclose(decInfo->fptr_stego_file);
        return e_failure;
    }
    
    //skip the 54 bytes of header file
    fseek(decInfo->fptr_stego_file,54,SEEK_SET);

    return e_success;
}

// For decoding length (used for magic string length, extension length,  file size, etc.)
Status decode_integers(FILE *fptr_stego, uint *integers)
{
    unsigned char str[32];
    *integers = 0;
    
    fread(str, 1, 32, fptr_stego);
    
    for(int i = 0; i < 32; i++)
    {
        *integers |= ((str[i] & 0x01) << i);
    }
    return e_success;
}
Status decode_magic_string(FILE *fptr_stego_file, char *magic_string, int len)
{
    // For each character in the magic string
    for (int i = 0; i < len; i++)
    {
        unsigned char byte;
        char decoded_char = 0;
        
        // Read 8 bytes to get 8 bits of one character
        for (int bit = 0; bit < 8; bit++)
        {
            if (fread(&byte, 1, 1, fptr_stego_file) != 1)
            {
                printf("ERROR: Unable to read byte\n");
                return e_failure;
            }
            // Get LSB and add it to decoded_char
            decoded_char |= ((byte & 1) << bit);
        }
        magic_string[i] = decoded_char;
    }
    magic_string[len] = '\0';

    printf("INFO: Length of magic string: %d\n", len);

    // Get user input
    char user_input[100];
    printf("Enter magic string: ");
    scanf("%s", user_input);

    // Compare strings
    if (strcmp(magic_string, user_input) == 0)
    {
        printf("INFO: Magic string matched\n");
        return e_success;
    }

    printf("INFO: Magic string not matched\n");
    return e_failure;
}

Status do_decoding(DecodeInfo *decInfo, int argc, char *argv[])
{
    //read and validate the arguments
    if(read_and_validate_decode_args(argc,argv,decInfo)==e_failure)
    {
        printf("ERROR : Unable to read and validate the arguments\n");
        return e_failure;
    }
    //open the files
    if(open_file(decInfo)==e_failure)
    {
        printf("ERROR : Unable to open the files\n");
        return e_failure;
    }
    
    //decode the length of the magic string
    int magic_string_len;
    decode_integers(decInfo->fptr_stego_file, &magic_string_len);
    

    if (magic_string_len <= 0 || magic_string_len > 100)  // Assume max length is 100
    {
        printf("ERROR: Invalid magic string length %d\n", magic_string_len);
        return e_failure;
    }
    //decode the magic string
    char *magic_string = malloc(magic_string_len + 1);
    if (!magic_string)
    {
        printf("ERROR: Memory allocation failed for magic string\n");
        return e_failure;
    }
   // Decode the magic string before using it
    if (decode_magic_string(decInfo->fptr_stego_file, magic_string, magic_string_len) == e_failure)
    {
        printf("ERROR: Unable to decode the magic string\n");
        free(magic_string);
        return e_failure;
    }

    printf("String matched Successfully\n");

   //decode the extension length and string

    
    char extn[MAX_FILE_SUFFIX+1];//extra space for null terminator
    if(decode_secret_file_extn(decInfo->fptr_stego_file,extn)==e_failure)
    {
        printf("ERROR : Unable to decode the extension\n");
        return e_failure;
    }

    

    // Close the current output file
    if (decInfo->fptr_decoded_image != NULL)
    {
        fclose(decInfo->fptr_decoded_image);
        decInfo->fptr_decoded_image = NULL;
    }

    // Create new output filename with the correct extension
    char *base_name = (argc > 3) ? argv[3] : "decoded_secret";
    char *new_filename = malloc(strlen(base_name) + strlen(extn) + 2);  // +2 for '.' and '\0'
    if (new_filename == NULL)
    {
        printf("ERROR: Memory allocation failed\n");
        return e_failure;
    }
    sprintf(new_filename, "%s.%s", base_name, extn);
    decInfo->decoded_image_fname = new_filename;

    // Open the new output file
    decInfo->fptr_decoded_image = fopen(decInfo->decoded_image_fname, "wb");
    if (decInfo->fptr_decoded_image == NULL)
    {
        printf("ERROR: Unable to create output file %s\n", decInfo->decoded_image_fname);
        free(new_filename);
        return e_failure;
    }
    printf("INFO: Created output file: %s\n", decInfo->decoded_image_fname);
    // Decode secret file size
    int secret_file_size;
    if (decode_secret_file_size(decInfo->fptr_stego_file, &secret_file_size) == e_failure)
    {
        printf("ERROR: Unable to decode the secret file size\n");
        return e_failure;
    }
    


    // Pass file_size to decode_secret_file_data
    if (decode_secret_file_data(decInfo, secret_file_size) == e_failure)
    {
        printf("ERROR: Unable to decode secret file data\n");
        return e_failure;
    }
    fclose(decInfo->fptr_decoded_image);
    fclose(decInfo->fptr_stego_file);
    free(magic_string);
    return e_success;
   
}
Status decode_secret_file_extn(FILE *fptr_stego_file, char *extn)
{
    
    
    int extn_length;
    if (decode_integers(fptr_stego_file, &extn_length) == e_failure)
    {
        printf("ERROR: Unable to decode the extension length\n");
        return e_failure;
    }
    printf("Successfully read the extension length: %d\n", extn_length);


    // Validate extension length
    if (extn_length <= 0 || extn_length > MAX_FILE_SUFFIX - 1)  // Leave room for null terminator
    {
        printf("ERROR: Invalid extension length: %d\n", extn_length);
        return e_failure;
    }

    // Read extension character by character
    unsigned char str[8];
    for (int i = 0; i < extn_length; i++)
    {
        extn[i] = 0;
        if (fread(str, 1, 8, fptr_stego_file) != 8)
        {
            printf("ERROR: Failed to read extension data\n");
            return e_failure;
        }
        
        // Decode each character
        for (int j = 0; j < 8; j++)
        {
            extn[i] |= ((str[j] & 0x01) << j);
        }
    }
    extn[extn_length] = '\0'; // Ensure null-termination
    printf("Extension: %s\n", extn);
    
    return e_success;
}
//decode secret file size
Status decode_secret_file_size(FILE *fptr_stego, int *file_size)
{
    // Initialize file_size
    *file_size = 0;
    
    // Read the size using decode_integers
    uint size_value;
    if (decode_integers(fptr_stego, &size_value) != e_success)
    {
        printf("ERROR: Failed to read secret file size\n");
        return e_failure;
    }

    // Validate the size - allow small files but prevent unreasonably large ones
    if (size_value == 0 || size_value > 1000000)  // 1MB max, adjust as needed
    {
        printf("ERROR: Invalid secret file size: %u\n", size_value);
        return e_failure;
    }

    *file_size = (int)size_value;
    printf("INFO: Secret file size decoded: %d bytes\n", *file_size);
    
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo, int file_size)
{
    if (decInfo->fptr_decoded_image == NULL)
    {
        printf("ERROR: Output file pointer is NULL\n");
        return e_failure;
    }

    // Read and decode data byte by byte
    unsigned char str[8];
    char ch;
    int bytes_decoded = 0;
    
    while (bytes_decoded < file_size && fread(str, 1, 8, decInfo->fptr_stego_file) == 8)
    {
        ch = 0;
        for (int i = 0; i < 8; i++)
        {
            ch |= ((str[i] & 0x01) << i);
        }
        if (fwrite(&ch, 1, 1, decInfo->fptr_decoded_image) != 1)
        {
            printf("ERROR: Failed to write decoded data\n");
            return e_failure;
        }
        bytes_decoded++;
    }

    printf("INFO: Successfully decoded %d bytes of secret data\n", bytes_decoded);
    return e_success;
}
