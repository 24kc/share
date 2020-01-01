/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/**/    #if defined _MSC_VER && ! defined _CRT_SECURE_NO_WARNINGS          /**/
/**/    #define _CRT_SECURE_NO_WARNINGS                                    /**/
/**/    #endif                                                             /**/
/**/                                                                       /**/
/**/    #include <stdio.h>                                                 /**/
/**/                                                                       /**/
/**/    #ifdef __linux__                                                   /**/
/**/    const char *file_name = "1.txt";                                   /**/
/**/    #else                                                              /**/
/**/    const char *file_name = "C:\\Users\\86183\\Desktop\\ceshi.txt";    /**/
/**/    #endif                                                             /**/
/**/                                                                       /**/
/**/    int main()                                                         /**/
/**/    {                                                                  /**/
/**/            FILE* fp;                                                  /**/
/**/            fp = fopen(file_name, "r");                                /**/
/**/            if ( ! fp ) {                                              /**/
/**/                    printf("Can not open file\n");                     /**/
/**/                    return 0;                                          /**/
/**/            }                                                          /**/
/**/            int c=0,d=0;                                               /**/
/**/            char a[20], b[20];                                         /**/
/**/            fscanf(fp, "%d， %[^，]， %[^，]，%d", &c, a, b, &d);      /**/
/**/            printf("%d,%s,%s,%d\n",c,a,b,d);                           /**/
/**/            fclose(fp);                                                /**/
/**/    }                                                                  /**/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
