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

# DSAA(数据结构与算法)
本节讲解mugglec的dsaa(数据结构与算法)模块.   
该模块用于实现一些经典的基础数据结构与算法, 使用时需要注意:
* 所有的数据结构中节点存储的数据是指针
* 所有的数据结构返回数据时, 并不是直接返回存储在其中的指针, 而是返回数据所在的数据结构节点

## 数组列表
`array_list`是一个典型的数组列表, 列表中的节点在内存上是连续的.  

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
`muggle_array_list_size`用于查询获取数组列表中有多少数据

### 预先分配容量
`muggle_array_list_ensure_capacity`用于确保数组列表的容量大小, 当遇到要插入较多数据时, 可预先使用该函数分配容量, 从而避免插入过程中的自动扩容操作

## 链表
`linked_list`是一个双向链表的实现  

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
`muggle_linked_list_size`用于查询获取链表中有多少数据