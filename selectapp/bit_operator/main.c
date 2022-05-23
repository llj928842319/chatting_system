#include <stdio.h>
#include <stdlib.h>

int main() {

    int fd1 = 0;
    int fd2 = 0;
    
    // 逻辑 大于
    if(fd1 > 0) {

    }

    // 赋值
    if(fd1 = 0) {

    }

    // 按位 异或 XOR
    if(fd1 ^ 1) {

    }

    // 按位 非
    if(!fd1) {

    }

    // 逻辑 或
    if(fd1 || 0) {

    }

    // 按位 或
    if(fd1 | 0) {

    }

    

    // 逻辑运算 / 布尔运算
    if(fd1 && 0) {
        printf("fd1 && 0 is true\n");
    } else {
        printf("fd1 && 0 is false\n");
    }

    // 逻辑运算 / 布尔运算
    if(fd1 == 0) {
        printf("fd1 == 0 is true\n");
    } else {
        printf("fd1 == 0 is false\n");
    }

    fd1 = 14;
    // 位运算 （按位 与）
    // 0b1011
    if(fd1 & 2) { // 0b0010
        printf("fd1 & 0 is true\n");
    } else {
        printf("fd1 & 0 is false\n");
    }
    return 0;
}