#define STACK_DEBUG

#include <stdio.h>
#include "assert_my.h"
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <ctype.h>

typedef double StackElem_t;
//TODO проверка ошибки на переполнение типа
//TODO плохой указатель на структуру
#define POISON NAN
#define CAN_BEG_STRUCT (StackElem_t)0xDEADF00D
#define CAN_END_STRUCT (StackElem_t)0xF00D
#define CAN_BEG_DATA   (StackElem_t)0xDEADC0FFEE
#define CAN_END_DATA   (StackElem_t)0xC0FFEE

#define RESULT_FILE "result.txt"

struct stack_t {
    double  canary1;

    StackElem_t* data;
    ssize_t size;
    ssize_t capacity;
    ssize_t init_capacity;

    double  canary2;
};

enum stackErr { //TODO диапазон недопустимый в памяти
    STK_OK           = 0,
    STK_NULL_STACK   = 1,     // указатель на структуру == NULL
    STK_NULL_DATA    = 2,     // data == NULL
    STK_BAD_SIZE     = 3,     // size < 0
    STK_BAD_CAPACITY = 4,     // capacity <= 0
    STK_OVERFLOW     = 5,     // size > capacity
    STK_NOT_POISONED = 6,     // в свободной части лежит не POISON
    STK_UNDERFLOW    = 7,     // pop из пустого стека
    STK_NO_MEMORY    = 8,     // не удалось выделить память
    STK_NULL_ARG     = 9,     // NULL в аргументе функции
    STK_NO_NEW_DATA  = 10,    // не удаётся обновить массив
    STK_CANARY_ERROR = 11,    // ошибка совпадения канареек
    BAD_INIT_CAP     = 12,    // изначально неверная capacity
    INVALID_VALUE    = 13     // плохой аргумент в стек
};

enum stackNew {
    REALL_UP = 2,
    REALL_DOWN = 2
};

stackErr     stackinit    (stack_t* stk, ssize_t capacity);
stackErr     stackpush    (stack_t* stk, double value);
StackElem_t  stackpop     (stack_t* stk, stackErr* err);
StackElem_t* DataUpdate   (stack_t* stk, int new_capacity, stackErr* err);
stackErr     DestroyData  (stack_t* stk);
stackErr     CanaryCheck  (stack_t* stk);
stackErr     PrintStack   (FILE* fp, stack_t* stk, const char* func, const char* file, int line);
void         CheckStack   (FILE* fp, stackErr err \
ON_DBG(, const char* func, const char* file, int line, const char* date, const char* time));
StackElem_t stackpopSafe (FILE* fp, stack_t* stk \
ON_DBG(, const char* func, const char* file, int line, const char* date, const char* time));

int main(void) {
    FILE* fp = fopen(RESULT_FILE, "w");
    stackErr err = STK_OK;
    stack_t stack = {0};
    size_t capacity = 5;

    StackInit(&stack, capacity);

    StackPush(&stack, 1.1);
    StackPush(&stack, 1.2);
    StackPush(&stack, 1.3);
    StackPush(&stack, 1.4);
    StackPush(&stack, 1.5);
    StackPush(&stack, 1.6);
    StackPush(&stack, 1.7);
    StackPush(&stack, 1.8);

    double value = 0;
    value = StackPop(&stack); //TODO возвращать значение
    value = StackPop(&stack);

    StackDump;

    DestroyData(&stack);
    fclose(fp);
    return 0;
}

stackErr StackVerify(stack_t* stk) {
    if (!stk) return STK_NULL_STACK;
    if (!stk -> data) return STK_NULL_DATA;
    if (stk -> size < 0) return STK_BAD_SIZE;
    if (stk -> capacity <= 0) return STK_BAD_CAPACITY;
    if (stk -> size > stk -> capacity) return STK_OVERFLOW;

    for (int i = stk -> size; i < stk -> capacity - 2; i++) {
        if (!isnan(stk -> data[i + 1])) return STK_NOT_POISONED;
    }

    return CanaryCheck(stk);
}

stackErr stackinit (stack_t* stk, ssize_t capacity) {
    if (!stk) return STK_NULL_STACK;
    if (capacity <= 0) return BAD_INIT_CAP;
//TODO проверка на capacity
    stk -> capacity = capacity + 2;
    stk -> init_capacity = capacity + 2;

    stk->canary1 = CAN_BEG_STRUCT;
    stk->canary2 = CAN_END_STRUCT;

    stk -> data = (StackElem_t*)calloc(stk -> capacity, sizeof(StackElem_t));
    if (!stk -> data) return STK_NO_MEMORY;

    stk -> size = 0;

    for (int i = 0; i < stk -> capacity - 1; i++) {
        stk -> data[i] = POISON;
    }
    stk->data[0] = CAN_BEG_DATA;
    stk->data[stk->capacity - 1] = CAN_END_DATA;

    return CanaryCheck(stk);
}

stackErr stackpush (stack_t* stk, double value) {
    //if (StackVerify(stk)) return StackVerify(stk);
//canary check //TODO
    if (!isfinite(value)) return INVALID_VALUE;
    if (CanaryCheck(stk) != STK_OK) return STK_CANARY_ERROR;
    if (stk -> size + 1 < (stk -> capacity - 2)) {
        stk -> size += 1;
        stk -> data[stk -> size] = value;

    } else {
        stackErr err = STK_OK;
        stk -> data = DataUpdate(stk, (stk -> capacity - 2) * REALL_UP + 2, &err);

        if (err != STK_OK) return STK_NO_NEW_DATA;

        stk -> size += 1;
        stk -> data[stk -> size] = value;
    }

    return STK_OK;
}

StackElem_t stackpop (stack_t* stk, stackErr* err) {
    if (!err) return POISON;
    *err = STK_OK;

    if (!stk) *err = STK_NULL_STACK;
    if (stk -> size == 0) *err =  STK_UNDERFLOW;

    StackElem_t value = stk -> data[stk -> size];
    stk -> data[stk -> size] = POISON;

    if ((stk -> capacity != stk->init_capacity) && (stk -> size - 1 < stk -> capacity / REALL_DOWN)) {
        stackErr err1 = STK_OK;
        stk -> data = DataUpdate(stk, (stk -> capacity - 2) / REALL_DOWN + 2, &err1);

        if (err1 != STK_OK) *err = err1;
    }

    stk -> size -= 1;

    return value;
}

StackElem_t stackpopSafe (FILE* fp, stack_t* stk \
ON_DBG(, const char* func, const char* file, int line, const char* date, const char* time)) {
    stackErr err = STK_OK;

    StackElem_t value = stackpop(stk, &err);

    if (&err == NULL) \
        CheckStack(fp, STK_NULL_ARG ON_DBG(, __func__, __FILE__, __LINE__, __DATE__, __TIME__)); \
    if (err != STK_OK) \
        CheckStack(fp, err ON_DBG(, __func__, __FILE__, __LINE__, __DATE__, __TIME__));

    return value;
}

StackElem_t* DataUpdate (stack_t* stk, ssize_t new_capacity, stackErr* err) {
    StackElem_t* data_new = (StackElem_t*)realloc((void*)stk->data, new_capacity * sizeof(StackElem_t));

    if (data_new == NULL) {
        free(stk->data);
        *err = STK_NO_NEW_DATA;
        return NULL;
    }

    for (int i = stk->capacity; i < new_capacity; i++) {
        data_new[i] = POISON;
    }

    stk->capacity = new_capacity;
    stk->data[stk->capacity - 1] = CAN_END_DATA;

    return data_new;
}

stackErr DestroyData (stack_t* stk) {
    free(stk -> data);
    stk -> data = NULL;
    stk -> size = 0;
    stk -> capacity = 0;
    stk = NULL;

    return STK_OK;
}

stackErr PrintStack (FILE* fp, stack_t* stk, const char* func, const char* file, int line) {
    fprintf(fp, "DATA[%p] \"stack\"[%p] %s() at %s:%d\n",\
    (void*)stk->data, (void*)stk, func, file, line);
//TODO args in func
    fprintf(fp, "size - %zd capacity - %zd\n", stk->size, stk->capacity - 2);

    for (int i = 1; i < stk->capacity - 1; i++) {
        if (i <= stk->size) fprintf(fp, "*");
        else fprintf(fp, " ");

        fprintf(fp, "%d) %lf\n", i, stk->data[i]);
    }

    if (!CanaryCheck(stk)) fprintf(fp, "Canary Check is OK\n");

    else {
        fprintf(fp, "Canary Check is NOT OK\n \
        canary 1:   struct: %lg norm: %lg\n \
        canary 2:   struct: %lg norm: %lg\n \
        canary 3: data-beg: %lg norm: %lg\n \
        canary 4: data-end: %lg norm: %lg\n", stk->canary1, CAN_BEG_STRUCT, stk->canary1 , CAN_END_STRUCT,
        stk->data[0], CAN_BEG_DATA, stk->data[stk->capacity - 1], CAN_END_DATA);
    }
}

stackErr CanaryCheck (stack_t* stk) {
    if (stk->canary1 != CAN_BEG_STRUCT || \
        stk->canary2 != CAN_END_STRUCT || \
        stk->data[0] != CAN_BEG_DATA || \
        stk->data[stk->capacity - 1] != CAN_END_DATA) return STK_CANARY_ERROR;

    return STK_OK;
}

void CheckStack (FILE* fp, stackErr err \
ON_DBG(, const char* func, const char* file, int line, const char* date, const char* time)) {
    PrintInfo;
    switch(err)
    {
        case STK_NULL_STACK:
            fprintf(fp, "Null pointer to stack\n");
            break;
        case STK_NULL_DATA:
            fprintf(fp, "Null pointer to data\n");
            break;
        case STK_BAD_SIZE:
            fprintf(fp, "Bad size: size < 0\n");
            break;
        case STK_BAD_CAPACITY:
            fprintf(fp, "Bad capacity: capacity <= 0\n");
            break;
        case STK_OVERFLOW:
            fprintf(fp, "Stack overflow: size > capacity\n");
            break;
        case STK_NOT_POISONED:
            fprintf(fp, "Free part of data is not poisoned\n");
            break;
        case STK_UNDERFLOW:
            fprintf(fp, "Stack underflow: pop from empty stack\n");
            break;
        case STK_NO_MEMORY:
            fprintf(fp, "No memory fault\n");
            break;
        case STK_NULL_ARG:
            fprintf(fp, "NULL passed as function argument\n");
            break;
        case STK_NO_NEW_DATA:
            fprintf(fp, "Failed to update data array\n");
            break;
        case STK_CANARY_ERROR:
            fprintf(fp,"Canary check failed\n");
            break;
        case BAD_INIT_CAP:
            fprintf(fp, "Incorrect init capacity\n");
            break;
        case INVALID_VALUE:
            fprintf(fp, "Can't put this value in stack\n");
            break;
        default:
            fprintf(fp, "Unknown error code: %d\n", (int)err);
            break;
    }
}
