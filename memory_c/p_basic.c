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

// Double pointer
// Lets make an matrix of integers using double pointer

// void allocate_array(int ***arr, size_t rows, size_t cols) {
//     *arr = malloc(rows * sizeof(int *));
//     for (size_t i = 0; i < rows; i++) {
//         (*arr)[i] = malloc(cols * sizeof(int));
//     }
// }

// void fill_array(int **arr, size_t rows, size_t cols) {
//     for (size_t i = 0; i < rows; i++) {
//         for (size_t j = 0; j < cols; j++) {
//             arr[i][j] = (int)i * j;
//         }
//     }
// }

// void print_array(int **arr, size_t rows, size_t cols) {
//     for (size_t i = 0; i < rows; i++) {
//         for (size_t j = 0; j < cols; j++) {
//             printf("%d ", arr[i][j]);
//         }
//         printf("\n");
//     }
// }

// int main() {
//     size_t rows, cols;

//     printf("Enter rows: ");
//     scanf("%zu", &rows);

//     printf("Enter cols: ");
//     scanf("%zu", &cols);

//     int **arr = NULL;

//     allocate_array(&arr, rows, cols);
//     fill_array(arr, rows, cols);
//     print_array(arr, rows, cols);

//     for (size_t i = 0; i < rows; i++)
//         free(arr[i]);

//     free(arr);

//     return 0;
// }

// Lets make a matrix multiplication using double pointer

// void allocate_array(int **arr,size_t size) {
//     *arr =(int *) malloc(size * sizeof(int));
// }

// void fill_array(int **arr,size_t size) {
//     for(size_t i = 0; i < size; i++) (*arr)[i] = (int)i;
// }

// void print_array(int **arr,size_t size) {
//     for(size_t i = 0; i < size; i++) printf("%d ",(*arr)[i]);
// }

// int main(void) {
//     size_t elements;

//     printf("Enter the number of elements: ");
//     scanf("%zu", &elements);

//     int *arr = NULL;

//     allocate_array(&arr,elements);

//     fill_array(&arr,elements);

//     print_array(&arr,elements);

//     free(arr);

//     return 0;
// }