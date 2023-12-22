/**
 * qjsc 生成的二进制数组文件转换为bin文件
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int qjsc_helloworld_size = 96;

const unsigned char qjsc_helloworld[96] = {
 0x02, 0x04, 0x0e, 0x63, 0x6f, 0x6e, 0x73, 0x6f,
 0x6c, 0x65, 0x06, 0x6c, 0x6f, 0x67, 0x16, 0x48,
 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72,
 0x6c, 0x64, 0x32, 0x6d, 0x79, 0x50, 0x72, 0x6a,
 0x2f, 0x6a, 0x73, 0x53, 0x72, 0x63, 0x2f, 0x68,
 0x65, 0x6c, 0x6c, 0x6f, 0x77, 0x6f, 0x72, 0x6c,
 0x64, 0x2e, 0x6a, 0x73, 0x0e, 0x00, 0x06, 0x00,
 0xa0, 0x01, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00,
 0x14, 0x01, 0xa2, 0x01, 0x00, 0x00, 0x00, 0x38,
 0xe1, 0x00, 0x00, 0x00, 0x42, 0xe2, 0x00, 0x00,
 0x00, 0x04, 0xe3, 0x00, 0x00, 0x00, 0x24, 0x01,
 0x00, 0xcd, 0x28, 0xc8, 0x03, 0x01, 0x01, 0x04,
};

int main(int argc,char* argv[])
{
    printf("des | src \n");
    if (argc != 3)
    {
        printf("argc is not equal 2 %d",argc);
        return -1;
    }
    char* des = strdup(argv[1]);
    char* src = strdup(argv[2]);

    printf("src is %s des is %s \n",src, des);

    FILE* fileS = fopen(src, "r");

    if (fileS == NULL)
    {
        printf("file %s not exist", src);
        goto done;
    }

    char buff[1024 * 5] = {0};
    int n = fread(buff, sizeof(char), sizeof(buff), fileS);

    char* subfile = strstr(buff, "= {");
    if (subfile == NULL)
    {
        printf("not find key word \n");
        goto done;
    }
    printf("%s\n", subfile);
    printf("find key = {\n");

    int arraySize = 0;
    unsigned char bd[1024 * 5] = {0};

    while((subfile = strstr(subfile, "0x")) != NULL)
    {
        int s = 0;
        int g = 0;
        if (subfile[2] >= '0' && subfile[2] <= '9')
        {
            s = subfile[2] - '0';
        }
        else
        {
            s = subfile[2] - 'a' + 10;
        }

        if (subfile[3] >= '0' && subfile[3] <= '9')
        {
            g = subfile[3] - '0';
        }
        else
        {
            g = subfile[3] - 'a' + 10;
        }

        char d = s * 16 + g ;
        bd[arraySize] = d;
        arraySize++;
        subfile += 4;
    }

    printf("arraySize %d", arraySize);

    for(int i = 0;i < arraySize;i++)
    {
        printf("0x%02x  ",bd[i]);
    }
    printf("\n");

    FILE* fileD = NULL;
    if (arraySize != 0)
    {
        fileD = fopen(des, "wb");
        fwrite(bd, arraySize, arraySize, fileD);
        fclose(fileD);
        fileD = NULL;
    }

done:
    if (fileD)
    {
        fclose(fileD);
    }
    if (fileS)
    {
        fclose(fileS);
    }

    free(src);
    free(des);
    return 0;

}