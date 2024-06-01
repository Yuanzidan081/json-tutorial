# 1. 这句话进行了什么操作：`c->stack = (char *)realloc(c->stack, c->size);`

这行代码执行了对指针`c->stack`所指向的内存块的重新分配操作。具体来说，它的作用是重新分配内存，以便存储`c->size`个字节的数据，并将新分配的内存地址赋给`c->stack`。

在这里，realloc 函数用于重新分配之前动态分配的内存块的大小。如果`c->stack`之前指向的内存块大小不足以存储`c->size`个字节的数据，`realloc`会重新分配一块足够大的内存，并将原有内存中的数据拷贝到新分配的内存中。如果原内存块足够大，则它可能会被扩展，也可能会被缩小。

需要注意的是，`realloc` 可能会返回一个新的地址，因此在实际应用中，我们通常会将其返回的地址赋给原指针，以确保指针指向新分配的内存块。同时，为了避免内存泄漏，通常在调用 `realloc` 之后，应该检查其返回值是否为 NULL，以确保内存分配操作成功。

总的来说，这行代码的作用是重新分配 `c->stack` 指向的内存块，以便存储 `c->size` 个字节的数据，并将新分配的内存地址赋给 `c->stack`。

# 2. 下面这个代码ret返回的不应该是一个指针的拷贝吗，ret不会变成野指针吗？返回的新的拷贝的值指向的内容还存在吗？
```
static void *lept_context_push(lept_context *c, size_t size)
{
    void *ret;
    assert(size > 0);
    if (c->top + size >= c->size)
    {
        if (c->size == 0)
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1; /* c->size * 1.5 */
        /*c->stack 在初始化时为 NULL，realloc(NULL, size) 的行为是等价于 malloc(size) 的*/
        c->stack = (char *)realloc(c->stack, c->size); /* 重新对内存进行分配 */
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}
```

在这段代码中，ret 并不是一个指针的拷贝，而是指向原始堆栈内存的指针。当 c->stack 通过 realloc 进行重新分配内存后，ret 仍然指向原始内存块的起始位置，因此它不会成为野指针。

在 lept_context_push 函数中，ret 被赋值为 c->stack + c->top，这意味着 ret 指向了堆栈内存中的一个特定位置，而 c->top 则被增加了 size，以便指向堆栈中的下一个可用位置。这样，ret 返回的是原始堆栈内存中的一个位置指针，而不是一个新的拷贝。

因此，当函数调用结束并返回 ret 时，它所指向的内容仍然存在，并且可以被调用者进一步使用。这是因为 ret 指向的是原始堆栈内存中的一个位置，而在函数调用结束后，这块内存并没有被释放或者发生改变。