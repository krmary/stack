#ifdef STACK_DEBUG
    #define ON_DBG(...) __VA_ARGS__

    #define PrintInfo fprintf(fp, "file: %s, function: %s, line: %d, date: %s, time: %s\n", file, func, line, date, time)

    #define StackInit(stk, capacity) \
    { \
        err = stackinit(stk, capacity); \
        if (err != STK_OK) \
            CheckStack(fp, err, __func__, __FILE__, __LINE__, __DATE__, __TIME__); \
    }

    #define StackPush(stk, value) \
    { \
        err = stackpush(stk, value); \
        if (err != STK_OK) \
            CheckStack(fp, err, __func__, __FILE__, __LINE__, __DATE__, __TIME__); \
    }

    #define StackPop(stk) \
        stackpopSafe(fp, stk, __func__, __FILE__, __LINE__, __DATE__, __TIME__);
    //#define StackPop(stk) value //TODO чзх

    #define StackDump PrintStack(fp, &stack, __func__, __FILE__, __LINE__);

#else
    #define ON_DBG(...)

    #define PrintInfo
    #define StackPush(stk, value) \
    { \
        err = stackpush(stk, value); \
        if (err != STK_OK) \
            CheckStack(fp, err); \
    }

    #define StackPop(stk) \
        stackpopSafe(fp, stk);

    #define StackInit(stk, capacity) \
    { \
        err = stackinit(stk, capacity); \
        if (err != STK_OK) \
            CheckStack(fp, err); \
    }

    #define StackDump
#endif


// #define check(condition) \
//         {
//             if(!condition) \
//             {
//
//             }
//         }
// #ifdef STACK_DEBUG
// #define assert_my(condition, red_param, x) \
//         { \
//             if(! condition) \
//             { \
//                 fprintf(stderr, "HELLO\nerror:( : condition: " #condition ",\n\
// file: %s, function: %s, line: %d, date: %s, time: %s", __FILE__, __func__, __LINE__, __DATE__, __TIME__); \
//                 return red_param; \
//             } \
//         }
// #else
// #define assert_my(condition, red_param, x)
// #endif

// #define CollPrint(condition) \
//         {
//             fprintf(stderr, "HELLO\nerror:( : condition: " #condition ",\n\
// file: %s, function: %s, line: %d, date: %s, time: %s", __FILE__, __func__, __LINE__, __DATE__, __TIME__);
//         }

//int probnic(stack_t* stk, const char* func, const char* file, int line) {



