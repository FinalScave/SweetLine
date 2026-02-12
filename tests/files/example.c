/* C 语言高亮示例 */
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define MAX_SIZE 1024
#define SQUARE(x) ((x) * (x))

#ifdef DEBUG
#pragma message("Debug mode")
#endif

// 结构体定义
struct Point {
    double x;
    double y;
};

enum Color { RED, GREEN, BLUE };

union Data {
    int i;
    float f;
    char c;
};

typedef unsigned long size_t;
typedef struct Point Point;

// 多参数函数声明
double distance(double x1, double y1, double x2, double y2);

static int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

double distance(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return dx * dy;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main(void) {
    const char *msg = "Hello, World!\n";
    char ch = 'A';
    int hex = 0xFF;
    int bin = 0b1010;
    float pi = 3.14f;
    bool flag = true;
    int arr[MAX_SIZE];

    struct Point p = {1.0, 2.0};
    enum Color color = RED;
    union Data data;
    int result = SQUARE(5);

    // 结构体成员访问
    p.x = 3.0;
    p.y = 4.0;
    double dist = distance(0.0, 0.0, p.x, p.y);

    // 指针与数组
    int *ptr = &result;
    arr[0] = *ptr;

    if (flag && result > 0) {
        printf("Result: %d, dist: %f\n", result, dist);
    }

    for (int i = 0; i < MAX_SIZE; i++) {
        if (i == 10) break;
    }

    switch (color) {
        case RED:
            break;
        case GREEN:
        case BLUE:
        default:
            break;
    }

    /* 多行注释
       可以跨多行 */
    return NULL == false ? 0 : 1;
}
