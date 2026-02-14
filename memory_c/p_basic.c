#include<stdio.h>
#include<stdlib.h>

// basics
// Functions swap each other values
// void swap(int * a,int * b) {
//     int vala = *a;
//     int valb = *b;
//     *b=vala;
//     *a=valb;
// }

// int main() {
//     // int a = 12;
//     // int* pa = &a;
//     // *pa = 13;
//     // printf("%d",a);

//     int a = 10;
//     int b = 20;
//     swap(&a,&b);

//     printf("a : %d,b : %d",a,b);

//     return 0;
// }



// Pointer arthimetic in arrays

// void printArray(int *arr,size_t size) {
//     for(size_t i = 0; i < size; i++) {
//         printf("%d\n",*(arr + i));
//     }
// }

// void printString(char *s) {
//     while(*s != '\0') {
//         printf("%c\n",*s);
//         s++;
//     }
// }

// int main() {
//     int a[] = {1,2,3,4,5};
//     int *ptra = a;
//     size_t size = sizeof(a)/sizeof(a[0]);
//     printArray(ptra,size);

//     char sa[] = "abcdefgh";
//     char* s = sa;
//     printString(s);

//     return 0;
// }


// int main() {
//     double *b = (double *)malloc(sizeof(double));
//     if(b == NULL) return 1;
//     *b = 5.788;
//     printf("%f",*b);
//     free(b);
//     return 0;
// }

// int main() {
//     size_t n;

//     printf("Enter the number of intgers:");
//     scanf("%zu",&n);

//     int *arr = (int *)malloc(n * sizeof(int));

//     for(size_t i = 0 ; i < n; i++) {
//         int num;
//         scanf("%d",&num);
//         arr[i] = num;
//     }

//     for(size_t i = 0; i < n; i++) {
//         arr[i] *= 2;
//         printf("%d\n",arr[i]); 
//     }

//     free(arr);
//     return 0;
// }


// string copy using pointers
// my naive one
// void copyString(const char* src,char* dest) {
//     while(*src != '\0') {
//         *dest++ = *src++;
//     }
// }

// int main() {
//     char s[] = "abcd";
//     char i[] = "";
//     char * ip = i;
//     const char *sp = s;
//     copyString(sp,ip);
//     return 0;
// }

// good one 

// void copyString(const char* src, char* dest) {
//     while ((*dest++ = *src++) != '\0');
// }

// int main() {
//     char s[] = "abcd";
//     char i[5];

//     copyString(s, i);

//     printf("%s\n", i);

//     return 0;
// }

