- [memory](#memory)
	- [内存池](#内存池)
		- [优缺点](#优缺点)
		- [初始化](#初始化)
		- [销毁](#销毁)
		- [分配与回收内存](#分配与回收内存)
		- [禁止扩容](#禁止扩容)
	- [线程安全内存池](#线程安全内存池)
		- [优缺点](#优缺点-1)
		- [初始化](#初始化-1)
		- [销毁](#销毁-1)
		- [分配与回收内存](#分配与回收内存-1)
	- [环形内存池](#环形内存池)
		- [优缺点](#优缺点-2)
		- [初始化](#初始化-2)
		- [销毁](#销毁-2)
		- [分配与回收内存](#分配与回收内存-2)
	- [顺序读写内存池](#顺序读写内存池)
		- [优缺点](#优缺点-3)
		- [初始化](#初始化-3)
		- [销毁](#销毁-3)
		- [分配与回收内存](#分配与回收内存-3)
	- [字节缓冲区](#字节缓冲区)

# memory
本节讲解**mugglec**内存工具模块

## 内存池
**mugglec**实现了一个可扩展容量的内存池`muggle_memory_pool_t`

### 优缺点
* 优点
  * 分配与释放的速度都很快
  * 可选扩容
* 缺点
  * 非线程安全

### 初始化
`muggle_memory_pool_init`用于内存池的初始化
```
bool muggle_memory_pool_init(muggle_memory_pool_t* pool, unsigned int init_capacity, unsigned int block_size);
```
* pool: 指向一个内存池的指针
* init_capacity: 内存池的初始大小
* block_size: 内存池分配的数据块大小

### 销毁
`muggle_memory_pool_destroy`用于销毁内存池
```
void muggle_memory_pool_destroy(muggle_memory_pool_t* pool);
```
* pool: 指向一个内存池的指针

### 分配与回收内存
[alloc_free.c](alloc_free/alloc_free.c)
```
muggle_memory_pool_t pool;
muggle_memory_pool_init(&pool, 8, (unsigned int)sizeof(user_data_t));

user_data_t *arr[32];
for (int i = 0; i < (int)(sizeof(arr)/sizeof(arr[0])); i++)
{
	arr[i] = (user_data_t*)muggle_memory_pool_alloc(&pool);
	arr[i]->i32 = i;
	LOG_INFO("allocate %d", arr[i]->i32);
}

for (int i = 0; i < (int)(sizeof(arr)/sizeof(arr[0])); i++)
{
	LOG_INFO("free %d", arr[i]->i32);
	muggle_memory_pool_free(&pool, arr[i]);
}

muggle_memory_pool_destroy(&pool);
```
* 第1个for循环中, 从内存池分配数据并放入arr当中
  * **注意: 虽然内存池的大小我们初始化为8, 但在分配过程中, 会自动扩容**
* 第2个for循环中, 内存池将内存块回收
* 如果用户在销毁内存池时, 确定外部已分配的内存块不在被使用, 可以选择直接销毁内存池而不用逐个回收内存块

### 禁止扩容
有时用户并不希望内存池无限增长, 一方面是希望严格控制内存池的大小, 另一方面是因为内存池在扩容时需要进行较为耗时的操作: [constant_size.c](constant_size/constant_size.c)
```
muggle_memory_pool_set_flag(&pool, MUGGLE_MEMORY_POOL_CONSTANT_SIZE);
```

## 线程安全内存池
**mugglec** 还实现了一个线程安全的内存池 `muggle_ts_memory_pool_t`

### 优缺点
* 优点
  * 线程安全
* 缺点
  * 速度一般

### 初始化
```
int muggle_ts_memory_pool_init(
	muggle_ts_memory_pool_t *pool,
	muggle_atomic_int capacity,
	muggle_atomic_int data_size);
```
* pool: 一个指向线程安全内存池的指针
* capacity: 初始化线程安全内存池的大小; 需要注意的是, 不同于非线程安全的内存池, 线程安全内存池不支持扩容, 所以需要在初始化的时候就估算好合适的大小
* data_size: 内存池分配的数据块大小

### 销毁
```
void muggle_ts_memory_pool_destroy(muggle_ts_memory_pool_t *pool);
```
* pool: 一个指向线程安全内存池的指针

### 分配与回收内存
现在 让我们看一下使用线程安全内存池, 在不同的线程分配与回收内存块: [ts_alloc_free.c](./ts_alloc_free/ts_alloc_free.c)  
我们先在生产者线程当中分配内存块, 并通过channel发送给消费者
```
// producer
while (1)
{
	foo = muggle_ts_memory_pool_alloc(pool);
	if (foo)
	{
		break;
	}
	else
	{
		muggle_msleep(100);
	}
}
foo->idx = args->idx;
foo->val = i;

while (muggle_channel_write(chan, foo) != 0);
```
* 这里在`muggle_ts_memory_pool_alloc`外套了一层while循环, 是为了确保内存分配成功
* 在`muggle_channel_write`外套了一层while循环是为了当channel满时, 自动重试

接着, 我们在消费者线程当中接收并回收内存块
```
foo_t *foo = (foo_t*)muggle_channel_read(&chan);
if (foo)
{
	muggle_ts_memory_pool_free(foo);
	cnt++;
}
```
* `muggle_channel_read`从channel当中取出生产者发送过来的数据
* `muggle_ts_memory_pool_free`回收了内存块

## 环形内存池
除了上面提到的线程安全内存池, **mugglec** 还提供一个可选线程安全的内存池 `muggle_ring_memory_pool_t`

### 优缺点
* 优点
  * 可选线程安全
  * 释放速度快
  * 释放时无需指定池对象
* 缺点
  * 分配速度一般

### 初始化
```
int muggle_ring_memory_pool_init(
	muggle_ring_memory_pool_t *pool,
	muggle_sync_t capacity,
	muggle_sync_t data_size);
```
* pool: 一个指向内存池的指针
* capacity: 初始化 `muggle_ring_memory_pool_t` 的大小
* data_size: 内存池分配的数据块大小

### 销毁
```
void muggle_ring_memory_pool_destroy(muggle_ring_memory_pool_t *pool)
```
* pool: 一个指向内存池的指针

### 分配与回收内存
[ring_alloc_free](./ring_alloc_free/ring_alloc_free.c)  

我们在生产者中分配内存块, 并通过 Channel 发送给消费者
```
foo_t *foo = muggle_ring_memory_pool_alloc(&pool);
```
这里有一点需要注意, `muggle_ring_memory_pool_t` 释放数据内存时始终是线程安全的, 但是分配时, 线程安全为可选的  
* `muggle_ring_memory_pool_alloc`: 非线程安全分配
* `muggle_ring_memory_pool_threadsafe_alloc`: 线程安全分配

接着, 我们在消费者线程当中接收并回收内存块
```
foo_t *foo = (foo_t *)muggle_channel_read(chan);
if (foo) {
	muggle_ring_memory_pool_free(foo);
	cnt++;
}
```
这里我们可以发现, 使用 `muggle_ring_memory_pool_free` 释放内存时, 无需手动指定内存池对象

## 顺序读写内存池
**mugglec**中还提供了一个特殊的顺序读写内存池`muggle_sowr_memory_pool_t`, 它适用于单生产者单消费者的场景.  
一般用法是, 用户需要保证按照顺序释放内存. 比如内存块A比内存块B更早分配, 则释放A要在释放B之前.  
除了一般的用法, `muggle_sowr_memory_pool_t`还有一个有意思的特性, 就是当第n个分配的内存被释放, 则意味着[0, n-1]的内存都被释放了. 这个特性可被用于非重要消息的传递, 比如当线程间管道写满堵塞而导致写入失败时, 可以不用对分配出来的消息做释放处理, 只要后续有一个消息被正常回收, 则之前的所有消息都将被正常回收

### 优缺点
* 优点
  * 分配与释放的速度都很快
  * 使用场景明确, 配套的特性方便此场景下使用
  * 释放时无需指定池对象
* 缺点
  * 仅适用于一读一写的场景

### 初始化
```
int muggle_sowr_memory_pool_init(
	muggle_sowr_memory_pool_t *pool,
	muggle_atomic_int capacity,
	muggle_atomic_int data_size);
```
* pool: 一个指向sowr内存池的指针
* capacity: 初始化sowr内存池的大小; 需要注意的是, 不同于普通的内存池, sowr内存池不支持扩容, 所以需要在初始化的时候就估算好合适的大小
* data_size: 内存池分配的数据块大小

### 销毁
```
void muggle_sowr_memory_pool_destroy(muggle_sowr_memory_pool_t *pool);
```
* pool: 一个指向sowr内存池的指针

### 分配与回收内存
现在 让我们看一下使用sowr内存池的例子 [sowr_alloc_free.c](./sowr_alloc_free/sowr_alloc_free.c)  
我们先在生产者线程当中分配内存块, 并通过channel发送给消费者
```
for (int i = 0; i < 1024; i++)
{
	foo_t *foo = muggle_sowr_memory_pool_alloc(pool);
	if (foo == NULL)
	{
		continue;
	}
	foo->idx = args->idx;
	foo->val = i;

	muggle_channel_write(chan, foo);
}
```
* `muggle_sowr_memory_pool_alloc`用于分配内存
* `muggle_channel_write`将分配的内存写入channel当中; 注意, 这里假设我们可以容忍个别消息的丢失, 所以不对写入的结果做任何判断, 即使写入失败也不在生产者的线程对内存做释放处理, 而是依赖sowr内存池的特性, 后续只要有内存块被正常释放即可

接着, 我们在消费者线程当中接收并回收内存块
```
foo_t *foo = (foo_t*)muggle_channel_read(&chan);
if (foo)
{
	muggle_sowr_memory_pool_free(foo);
	cnt++;
}
```
* `muggle_channel_read`从channel当中取出生产者发送过来的数据
* `muggle_sowr_memory_pool_free`回收了内存块

## 字节缓冲区
**mugglec**提供了一个字节缓冲区`muggle_bytes_buffer_t`. 可以帮助用户缓冲字节流, 在后续操作中可以在零拷贝的情况下操作缓冲的字节数据.  
无论读/写, 字节缓冲区中都分为两种:
* 普通读/写
* 连续内存上的读/写

查询可读/可写的大小
```
int muggle_bytes_buffer_readable(muggle_bytes_buffer_t *bytes_buf);
int muggle_bytes_buffer_writable(muggle_bytes_buffer_t *bytes_buf);
```

通过查询可读/可写的大小, 后续可以进行读/写操作
```
bool muggle_bytes_buffer_read(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *dst);
bool muggle_bytes_buffer_write(muggle_bytes_buffer_t *bytes_buf, int num_bytes, void *src);
```
* num_bytes: 读/写的字节数量
* dst: 读取字节的目标地址
* src: 写入字节的源地址

有时用户希望操作的内存的连续的, 这样可以直接将结构体的指针指向字节缓冲区后直接使用.  

在字节缓冲区中寻找连续可写的内存
```
void* muggle_bytes_buffer_writer_fc(muggle_bytes_buffer_t *bytes_buf, int num_bytes);
```
* bytes_buf: 指向字节缓冲区的指针
* num_bytes: 寻找的内存大小
* 返回值: 若找到合适的内存, 返回内存的地址; 否则返回NULL

在获取到连续可写的缓冲区内存后, 用户可以直接往返回值的地址当中写入数据, 写入完毕后, 记得移动连续可写的指针
```
bool muggle_bytes_buffer_writer_move_n(muggle_bytes_buffer_t *bytes_buf, void *ptr, int num_bytes);
```
* bytes_buf: 指向字节缓冲区的指针
* ptr: 由 `muggle_bytes_buffer_writer_fc` 返回的指针
* num_bytes: 移动的字节数量
* **注意: 这里的移动起始点是连续内存的起始点, 所以有时当剩余可写大小为N, 找到连续内存大小n, 移动后剩余缓冲区大小 <= N-n**

读取连续内存也是相同的情况, 可以先通过`muggle_bytes_buffer_reader_fc`发现连续可读的内存, 接着直接将地址返回使用, 使用完之后通过`muggle_bytes_buffer_reader_move`移动缓冲区中的可读指针

例子 [example_bytes_buffer.c](./bytes_buffer/example_bytes_buffer.c) 展示了字节缓冲区的一些基本操作
