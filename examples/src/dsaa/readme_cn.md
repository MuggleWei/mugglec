- [DSAA(数据结构与算法)](#dsaa数据结构与算法)
	- [数组列表](#数组列表)
		- [初始化](#初始化)
		- [销毁](#销毁)
		- [插入/追加数据](#插入追加数据)
		- [添加与删除](#添加与删除)
		- [清空](#清空)
		- [销毁时清理](#销毁时清理)
		- [查找](#查找)
		- [大小](#大小)
		- [预先分配容量](#预先分配容量)
	- [链表](#链表)
		- [初始化](#初始化-1)
		- [销毁](#销毁-1)
		- [插入/追加数据](#插入追加数据-1)
		- [添加与删除](#添加与删除-1)
		- [清空](#清空-1)
		- [销毁时清理](#销毁时清理-1)
		- [查找](#查找-1)
		- [大小](#大小-1)
	- [队列](#队列)
		- [初始化](#初始化-2)
		- [销毁](#销毁-2)
		- [添加与删除](#添加与删除-2)
		- [清空](#清空-2)
		- [销毁时清理](#销毁时清理-2)
		- [大小](#大小-2)
	- [栈](#栈)
		- [初始化](#初始化-3)
		- [销毁](#销毁-3)
		- [进栈出栈](#进栈出栈)
		- [清空](#清空-3)
		- [销毁时清理](#销毁时清理-3)
		- [大小](#大小-3)
	- [堆](#堆)
		- [初始化](#初始化-4)
		- [销毁](#销毁-4)
		- [插入与提取](#插入与提取)
		- [插入与提取2](#插入与提取2)

# DSAA(数据结构与算法)
本节讲解mugglec的dsaa(数据结构与算法)模块.   
该模块用于实现一些经典的基础数据结构与算法, 使用时需要注意:
* 所有的数据结构中节点存储的数据是指针
* 所有的数据结构返回数据时, 并不是直接返回存储在其中的指针, 而是返回数据所在的数据结构节点

## 数组列表
`muggle_array_list_t`是一个典型的数组列表, 列表中的节点在内存上是连续的.  

### 初始化
`muggle_array_list_init`用于初始化数组列表
```
bool muggle_array_list_init(muggle_array_list_t *p_array_list, size_t capacity);
```
* p_array_list: 指向一个数组列表的指针
* capacity: 数组列表的初始化容量大小, 当插入的数据大于这个数值时, `array_list`会自动扩容

### 销毁
`muggle_array_list_destroy`用于销毁数组列表
```
void muggle_array_list_destroy(muggle_array_list_t *p_array_list, muggle_dsaa_data_free func_free, void *pool);
```
* p_array_list: 指向一个数组列表的指针
* func_free: 释放数据函数, 若为NULL, 销毁数组列表时不对列表中剩余的数据做释放操作
* pool: 传给释放函数的附加数据, 一般为NULL或者内存池, 当func_free为NULL时, 此字段无意义

### 插入/追加数据
在index之前插入数据
```
muggle_array_list_node_t* muggle_array_list_insert(muggle_array_list_t *p_array_list, int index, void *data);
```
* p_array_list: 指向一个数组列表的指针
* index: 插入数据的索引, 数据将被插入在这个索引的前一个位置. 当数组为空且索引为0或者-1时, 数据将被直接插入数组中
* data: 被插入的数据
* 返回值: 插入之后, 数据所在的节点的指针将被返回

**注意: 插入的返回值只可以被用来判断插入成功或失败, 不要保存这个值并在之后使用, 因为当数组列表扩容时, 数据所在的节点地址将会变化**  

在index之后插入数据
```
muggle_array_list_node_t* muggle_array_list_append(muggle_array_list_t *p_array_list, int index, void *data);
```
* append与insert的参数与返回值类似, 区别只在于, 数据将会被插入到index索引之后而不是之前.

### 添加与删除
现在我们简单的理用`array_list`来保存数据, 并在适当的时机将其删除 [array_list_add_remove.c](./array_list_add_remove/array_list_add_remove.c)
```
muggle_array_list_t array_list;
muggle_array_list_init(&array_list, 8);

for (int i = 0; i < 16; i++)
{
	user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
	data->value = i * 2;
	if (!muggle_array_list_append(&array_list, -1, data))
	{
		LOG_ERROR("failed append data: %d", data->value);
	}
}

for (int i = 0; i < muggle_array_list_size(&array_list); i++)
{
	muggle_array_list_node_t *node = muggle_array_list_index(&array_list, i);
	user_data_t *data = (user_data_t*)node->data;
	LOG_INFO("#%d: %d", i, data->value);
}

for (int i = muggle_array_list_size(&array_list)-1; i >= 0; i--)
{
	muggle_array_list_node_t *node = muggle_array_list_index(&array_list, i);
	free(node->data);
	muggle_array_list_remove(&array_list, i, NULL, NULL);
}

muggle_array_list_destroy(&array_list, NULL, NULL);
```
* 上面的代码里共有3个for循环
  * 第1个for循环中, 创建数据并追加到数据的末尾
  * 第2个for循环中遍历数组并打印
  * 第3个for循环中将数组每个节点中的数据释放并移除相应的节点

可以看到, 我们将数组列表的大小初始化为8, 之后往其中插入了16个数据, 插入的过程中, array_list将会自动扩容

### 清空
上一小节的代码中, 最后释放数据时需要写一个for循环来遍历每一个数组节点, 其实可以直接使用`muggle_array_list_clear`来达到清空数组列表的目的: [array_list_clear.c](./array_list_clear/array_list_clear.c)  
我们先添加自定义的数据释放函数, 当前就是在函数中简单的调用一下`free`
```
void do_free(void *pool, void *data)
{
	free(data);
}
```
然后删除释放数据的for循环代码, 改为下面的代码. 这样就达到了一次清空数组的目的
```
muggle_array_list_clear(&array_list, do_free, NULL);
```

### 销毁时清理
如果清空数组后不在使用, 那么可以直接在`muggle_array_list_destroy`指定释放数据的回调函数: [array_list_destroy.c](./array_list_destroy/array_list_destroy.c)  
我们删除上面的`muggle_array_list_clear`代码, 接着修改最后的destroy函数为
```
muggle_array_list_destroy(&array_list, do_free, NULL);
```
这样就达到了销毁时清理数据的目的

### 查找
`array_list`提供了用于查找的函数, 用户需要指定查找的起始索引以及对比函数: [array_list_find.c](./array_list_find/array_list_find.c)
```
int v = 1;
int idx = -1;
while((idx = muggle_array_list_find(&array_list, idx + 1, &v, cmp_int)) != -1)
{
	LOG_INFO("find value=%d at index %d", v, idx);
};
```
* 通过上面的循环, 找出所有数据为1的索引

### 大小
`muggle_array_list_size`用于查询数组列表中有多少数据

### 预先分配容量
`muggle_array_list_ensure_capacity`用于确保数组列表的容量大小, 当遇到要插入较多数据时, 可预先使用该函数分配容量, 从而避免插入过程中的自动扩容操作

## 链表
`muggle_linked_list_t`是一个双向链表的实现  

### 初始化
`muggle_linked_list_init`用于初始化链表
```
bool muggle_linked_list_init(muggle_linked_list_t *p_linked_list, size_t capacity);
```
* p_linked_list: 指向一个链表的指针
* capacity: 
  * 非0时, 表示用于链表节点内存分配的内存池的大小
  * 为0时, 表示不使用内存池来分配链表节点

### 销毁
`muggle_linked_list_destroy`用于销毁链表
```
void muggle_linked_list_destroy(muggle_linked_list_t *p_linked_list, muggle_dsaa_data_free func_free, void *pool);
```
* p_linked_list: 指向一个链表的指针
* func_free: 销毁时用于释放数据的函数, 指定为NULL, 代表销毁时不对链表中存储的数据做释放操作
* pool: 传给释放函数的附加数据, 一般为NULL或者内存池, 当func_free为NULL时, 此字段无意义

### 插入/追加数据
在node之前插入数据
```
muggle_linked_list_node_t* muggle_linked_list_insert(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, void *data);
```
* p_linked_list: 指向一个链表的指针
* node: 指定的节点, 当为NULL时, 表示插入在链表的头部
* data: 要插入的数据
* 返回值: 插入之后, 数据所在的节点的指针

**注意: 插入的返回值可以被保存并使用, 只要链表没销毁且节点没被删除, 则此返回的链表节点就是有效的**  

在node之后插入数据
```
muggle_linked_list_node_t* muggle_linked_list_append(muggle_linked_list_t *p_linked_list, muggle_linked_list_node_t *node, void *data);
```
* append与insert的参数与返回值类似, 区别只在于, 数据将会被插入到node之后而不是之前.
* 当node为NULL时, 表示插入在链表的尾部

### 添加与删除
现在我们简单的理用linked_list来保存数据, 并在适当的时机将其删除: [linked_list_add_remove.c](./linked_list_add_remove/linked_list_add_remove.c)
```
muggle_linked_list_t linked_list;
muggle_linked_list_init(&linked_list, 8);

int i = 0;
for (i = 0; i < 16; i++)
{
	user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
	data->value = i * 2;
	if (!muggle_linked_list_append(&linked_list, NULL, data))
	{
		LOG_ERROR("failed append data: %d", data->value);
	}
}

i = 0;
muggle_linked_list_node_t *node = muggle_linked_list_first(&linked_list);
for (; node; node = muggle_linked_list_next(&linked_list, node))
{
	user_data_t *data = (user_data_t*)node->data;
	LOG_INFO("#%d: %d", i++, data->value);
}

node = muggle_linked_list_first(&linked_list);
while (node)
{
	node = muggle_linked_list_remove(&linked_list, node, do_free, NULL);
}

muggle_linked_list_destroy(&linked_list, NULL, NULL);
```
* 第1个for循环, 将数据插入到链表中
* 第2个for循环, 遍历链表读取数据
* 最后一个while循环, 删除链表所有节点, 并指定do_free来释放链表中的数据

### 清空
当想要一次性清空整个链表, 可以直接调用`muggle_linked_list_clear`来实现: [linked_list_clear.c](./linked_list_clear/linked_list_clear.c)
```
muggle_linked_list_clear(&linked_list, do_free, NULL);
```
* 其中的第2个参数, 指定了在清空链表时用于释放其中数据的函数

### 销毁时清理
如果清空数组后不在使用, 那么可以直接在`muggle_linked_list_destroy`指定释放数据的回调函数: [linked_list_destroy.c](./linked_list_destroy/linked_list_destroy.c)  
我们删除上面的`muggle_linked_list_clear`代码, 接着修改最后的destroy函数为
```
muggle_linked_list_destroy(&linked_list, do_free, NULL);
```
这样就达到了销毁时清理数据的目的

### 查找
`linked_list`提供了用于查找的函数, 用户需要指定查找的起始节点(当为NULL时代表从头开始查找)以及对比函数: [linked_list_find.c](./linked_list_find/linked_list_find.c)
```
int v = 1;
muggle_linked_list_node_t *node = NULL;
while ((node = muggle_linked_list_find(&linked_list, node, &v, cmp_int)) != NULL)
{
	LOG_INFO("find value=%d at node address 0x%llx", v, (uintptr_t)node);
	if ((node = muggle_linked_list_next(&linked_list, node)) == NULL)
	{
		break;
	}
};
```
* 通过上面的循环, 找出`linked_list`中所有数据为1的节点

### 大小
`muggle_linked_list_size`用于查询链表中有多少数据

## 队列
`muggle_queue_t`是一个典型的队列实现, 通常用于先进先出(FIFO - first in first out)的场景

### 初始化
`muggle_queue_init`用于队列的初始化
```
bool muggle_queue_init(muggle_queue_t *p_queue, size_t capacity);
```
* p_queue: 指向一个队列的指针
* capacity:
  * 非0时, 表示用于队列节点内存分配的内存池的大小
  * 为0时, 表示不使用内存池来分配队列节点

### 销毁
`muggle_queue_destroy`用于销毁队列
```
void muggle_queue_destroy(muggle_queue_t *p_queue, muggle_dsaa_data_free func_free, void *pool);
```
* p_queue: 指向一个队列的指针
* func_free: 销毁时用于释放数据的函数, 指定为NULL, 代表销毁时不对队列中存储的数据做释放操作
* pool: 传给释放函数的附加数据, 一般为NULL或者内存池, 当func_free为NULL时, 此字段无意义

### 添加与删除
队列是一个操作受限的列表, 只允许在尾部插入数据, 并从头部取出数据, 也就是只能执行先进先出操作: [queue_enqueue_dequeue.c](./queue_enqueue_dequeue/queue_enqueue_dequeue.c)
```
muggle_queue_t queue;
muggle_queue_init(&queue, 8);

for (int i = 0; i < 16; i++)
{
	user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
	data->value = i * 2;
	if (!muggle_queue_enqueue(&queue, data))
	{
		LOG_ERROR("failed enqueue data: %d", data->value);
	}
}

int i = 0;
muggle_queue_node_t *node = NULL;
while (!muggle_queue_is_empty(&queue))
{
	node = muggle_queue_front(&queue);
	user_data_t *data = (user_data_t*)node->data;
	muggle_queue_dequeue(&queue, NULL, NULL);
	
	LOG_INFO("#%d: %d", i++, data->value);
	free(data);
}

muggle_queue_destroy(&queue, NULL, NULL);
```
* 第1个for循环将数据逐个放入队列的末尾
* 第2个for循环从队头逐个取出数据, 释放队头, 打印数据之后将其释放

### 清空
当想要一次性清空整个队列, 可以直接调用`muggle_queue_clear`来实现: [queue_clear.c](./queue_clear/queue_clear.c)
```
muggle_queue_clear(&queue, do_free, NULL);
```
* 其中的第2个参数, 指定了在清空队列时用于释放其中数据的函数

### 销毁时清理
如果清空数组后不在使用, 那么可以直接在`muggle_queue_destroy`指定释放数据的回调函数: [queue_destroy.c](./queue_destroy/queue_destroy.c)
```
muggle_queue_destroy(&queue, do_free, NULL);
```

### 大小
`muggle_queue_size`用于查询队列中有多少数据

## 栈
`muggle_stack_t`是一个栈的实现, 通常用于后进先出(LIFO - last in first out)的场景

### 初始化
`muggle_stack_init`用于初始化栈
```
bool muggle_stack_init(muggle_stack_t *p_stack, size_t capacity);
```
* p_stack: 指向一个栈的指针
* capacity:
  * 非0时, 表示用于栈节点内存分配的内存池的大小
  * 为0时, 表示不使用内存池来分配栈节点

### 销毁
`muggle_stack_destroy`用于销毁栈
```
void muggle_stack_destroy(muggle_stack_t *p_stack, muggle_dsaa_data_free func_free, void *pool);
```
* p_stack: 指向一个栈的指针
* func_free: 销毁时用于释放数据的函数, 指定为NULL, 代表销毁时不对栈中存储的数据做释放操作
* pool: 传给释放函数的附加数据, 一般为NULL或者内存池, 当func_free为NULL时, 此字段无意义

### 进栈出栈
栈和队列相似, 也是一个操作受限的列表, 不同的是, 越后被压入栈中的数据将在越先被返回: [stack_push_pop.c](./stack_push_pop/stack_push_pop.c)
```
muggle_stack_t stack;
muggle_stack_init(&stack, 8);

for (int i = 0; i < 16; i++)
{
	user_data_t *data = (user_data_t*)malloc(sizeof(user_data_t));
	data->value = i * 2;
	if (!muggle_stack_push(&stack, data))
	{
		LOG_ERROR("failed push data: %d", data->value);
	}
}

int i = 0;
muggle_stack_node_t *node = NULL;
while (!muggle_stack_is_empty(&stack))
{
	node = muggle_stack_top(&stack);
	user_data_t *data = (user_data_t*)node->data;
	muggle_stack_pop(&stack, NULL, NULL);
	LOG_INFO("#%d: %d", i++, data->value);
	free(data);
}

muggle_stack_destroy(&stack, NULL, NULL);
```
* 第1个for循环, 将数据压入栈中
* 第2个for循环, 从栈顶读取数据并出栈, 再打印之后释放数据

### 清空
当想要一次性清空整个栈, 可以直接调用muggle_stack_clear来实现: [stack_clear.c](./stack_clear/stack_clear.c)
```
muggle_stack_clear(&stack, do_free, NULL);
```
* 其中的第2个参数, 指定了在清空栈时用于释放其中数据的函数

### 销毁时清理
如果清空栈后不在使用, 那么可以直接在`muggle_stack_destroy`指定释放数据的回调函数: [stack_destroy.c](./stack_destroy/stack_destroy.c)
```
muggle_stack_destroy(&stack, do_free, NULL);
```

### 大小
`muggle_stack_size`用于查询栈中有多少数据

## 堆
`muggle_heap_t`是一个最小二叉堆的实现

### 初始化
`muggle_heap_init`用于初始化一个堆
```
bool muggle_heap_init(muggle_heap_t *p_heap, muggle_dsaa_data_cmp cmp, size_t capacity);
```
* p_heap: 指向一个堆的指针
* cmp: 用于比较插入数据优先级的函数, 越小的值(也就是cmp返回负数的值)优先级越高
* capacity:
  * 非0时, 表示用于堆节点内存分配的内存池的大小
  * 为0时, 表示不使用内存池来分配堆节点

### 销毁
`muggle_heap_destroy`用于销毁一个堆
```
void muggle_heap_destroy(muggle_heap_t *p_heap,
	muggle_dsaa_data_free key_func_free, void *key_pool,
	muggle_dsaa_data_free value_func_free, void *value_pool);
```
* p_heap: 指向一个堆的指针
* key_func_free: 用于释放堆节点中key值数据的函数
* key_pool: 传给key_func_free的附加参数, 一般为内存池指针或者NULL, 当key_func_free为NULL时, 此参数无意义
* value_func_free: 用于释放堆节点中value值数据的函数
* value_pool: 传给value_func_free的附加参数, 一般为内存池指针或者NULL, 当value_func_free为NULL时, 此参数无意义

### 插入与提取
可以通过`heap`可以轻松的实现一个优先队列的逻辑: [heap_insert_remove.c](./heap_insert_remove/heap_insert_remove.c)
```
srand((unsigned int)time(NULL));

muggle_heap_t heap;
muggle_heap_init(&heap, priority_cmp, 8);

for (int i = 0; i < 16; i++)
{
	task_t *task = (task_t*)malloc(sizeof(task_t));
	task->idx = i;
	task->priority = 1 + rand() % 8;
	LOG_INFO("generate task: idx=%d, priority=%d", task->idx, task->priority);
	if (!muggle_heap_insert(&heap, task, NULL))
	{
		LOG_ERROR("failed add task: %d", i);
	}
}

muggle_heap_node_t *node = NULL;
while ((node = muggle_heap_root(&heap)) != NULL)
{
	task_t *task = (task_t*)node->key;
	LOG_INFO("highest priority task: idx=%d, priority=%d", task->idx, task->priority);
	muggle_heap_remove(&heap, node, do_free, NULL, NULL, NULL);
}

muggle_heap_destroy(&heap, NULL, NULL, NULL, NULL);
```
* 在第1个for循环中, 将产生任务, 并随机赋予任务优先级
* priority_cmp决定优先级, 由于`muggle_heap_t`默认是一个最小二叉堆, 所以这里值越小优先级越高; 当然也可以通过在对比函数的最终结果取反, 来得到一个最大二叉堆
* 在第2个for循环中, 通过`muggle_heap_root`每次取得剩余优先级最高的任务, 打印出任务信息并将其从堆中删除

**注意: 这里堆的实现只会对优先级进行排序, 并不会对插入顺序进行排序**

### 插入与提取2
像上一小节的例子中, 每次使用完二叉堆的根节点之后都将其释放, 可以将提取与删除合并成为一步: [heap_insert_extract.c](./heap_insert_extract/heap_insert_extract.c)
```
muggle_heap_node_t node;
while (muggle_heap_extract(&heap, &node))
{
	task_t *task = (task_t*)node.key;
	LOG_INFO("highest priority task: idx=%d, priority=%d", task->idx, task->priority);
}
```
* 这里删除了`muggle_heap_root`与`muggle_heap_remove`代码, 取而代之的是`muggle_heap_extract`, 它将每次提取优先级最高的节点信息并同时从堆中将此节点删除

TODO: to be contineud......