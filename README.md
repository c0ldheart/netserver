> 目的：
> 1. 熟悉C/C++编程语言和常用库的使用，高效优雅开发思路实践
> 2. 学习计算机网络知识，包括TCP协议，Linux下socket编程基础，高并发服务器架构设计
> 3. 学习操作系统知识，包括线程、系统调用等
> 
思路：
> 1. 面向对象编程提升编码效率和可读性
> 2. IO复用和多线程提升网络IO并发表现
> 
技术栈：
> 1. 现代C++编程，包括面向对象编程思想，智能指针，函数包装器，工程构建等
> 2. epoll IO复用、缓冲区、线程池
> 3. 主从Reactor

### 1. 朴素socket
#### 1. 服务端建立连用socket文件描述符
```cpp
#incldue <sys/socket.h> // 头文件
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```

- 第一个参数：IP地址类型，AF_INET表示使用IPv4，如果使用IPv6请使用AF_INET6。
- 第二个参数：数据传输方式，SOCK_STREAM表示流格式、面向连接，多用于TCP。SOCK_DGRAM表示数据报格式、无连接，多用于UDP。
- 第三个参数：协议，0表示根据前面的两个参数自动推导协议类型。设置为IPPROTO_TCP和IPPTOTO_UDP，分别表示TCP和UDP。
#### 2. 绑定服务器IP和端口，监听
```cpp
struct sockaddr_in server_address_listen;
bzero(&server_address_listen, sizeof(server_address_listen));
server_address_listen.sin_family = AF_INET;
server_address_listen.sin_addr.s_addr = inet_addr("127.0.0.1");
server_address_listen.sin_port = htons(1888);

bind(sockfd_listen, (struct sockaddr *)&server_address_listen, sizeof(server_address_listen));

listen(sockfd_listen, SOMAXCONN);
```

1. 声明专用socket地址结构体
2. bzero初始化为0
3. 对结构体内参数绑定，注意**inet**库转IP地址字符串、**htons**转端口
4. 专用socket地址转换到通用sockaddr，再绑定到之前定义的socktfd
5. listen, SOMAXCONN默认最大监听队列长度128
#### 3. accept建立连接
```cpp
struct sockaddr_in client_address;
socklen_t client_address_len = sizeof(client_address);
bzero(&client_address, client_address_len);

int sockfd_accept = accept(sockfd_listen, (struct sockaddr *)&client_address, &client_address_len);
```
### 2. 基于epoll
**三个系统调用：**
```cpp
//int epfd = epoll_create(1024);  //参数表示监听事件的大小，如超过内核会自动调整，已经被舍弃，无实际意义，传入一个大于0的数即可
int epfd = epoll_create1(0);       //参数是一个flag，一般设为0，详细参考man epoll
```
```cpp
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);    //添加事件到epoll
epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);    //修改epoll红黑树上的事件
epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);   //删除事件


typedef union epoll_data {
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event {
  uint32_t events;	/* Epoll events */
  epoll_data_t data;	/* User data variable */
} __EPOLL_PACKED;
```
其中sockfd表示我们要添加的IO文件描述符，ev是一个epoll_event结构体，其中的events表示事件，如EPOLLIN等
```cpp
int nfds = epoll_wait(epfd, events, maxevents, timeout);
```
其中events是一个epoll_event结构体数组，maxevents是可供返回的最大事件大小，一般是events的大小，timeout表示最大等待时间，设置为-1表示一直等待。
#### 给服务器改写成epoll
在创建了服务器socket fd后，先将这个listen_fd添加到epoll，只要这个**listen_fd上发生可读事件**，表示有一个新的客户端连接。然后accept这个客户端并将客户端的accept_socket fd添加到epoll，epoll会多监听客户端socket fd是否有事件发生，如果发生则处理事件。
在listen后，创建epfd，定义epoll_event类型的events（用于存所有有变化的fd) 和 ev(用于listen fd和临时处理有变化的fd）
```cpp
int epfd = epoll_create1(0); // 建立epoll fd
struct epoll_event events[MAX_EVENTS], ev;
ev.events = EPOLLIN; // ET模式
ev.data.fd = sockfd_listen; // 该IO口为服务器socket fd
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd_listen, &ev); //将服务器listen socket fd添加到epoll
```
### 3. 面向对象封装
略
### 4. epoll进阶：Channel
原来的events里，`data`用了union的 `int fd`，也就是只知道文件描述符，不知道更多信息。
我们要用void* ptr，指向Channel对象，Channel类可以封装更多信息，如该fd属于哪种类型（ftp、http）。
```cpp
class Channel{
private:
    Epoll *ep;  // 所属Epoll对象（之前封装的）
    int fd;     // socketfd 描述符
    uint32_t events;  // 表示希望监听这个文件描述符的哪些事件类型，因为不同事件的处理方式不一样
    uint32_t revents; // 表示在epoll返回该Channel时文件描述符正在发生的事件
    bool inEpoll;  // 表示当前Channel是否已经在epoll红黑树中，为了注册Channel的时候方便区分使用EPOLL_CTL_ADD还是EPOLL_CTL_MOD
};
```
在创建socket（listen或accept）后，new一个Channel对象，`Channel *servChannel = new Channel(ep, socket_listen->get_fd());`这个对象存了ep对象的指针和sockfd，接下来需要手动把fd注册到这个ep：`clntChannel->enableReading();`
```cpp
void Channel::enableReading(){
    events = EPOLLIN | EPOLLET;
    ep->updateChannel(this);
}

void Epoll::updateChannel(Channel* channel)
{
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if (!channel->getInEpoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->setInEpoll();
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
    }
}
```
之后改写`ep.poll()`的活动events的类型是`vector<Channel*>`，利用`int chfd = activeChannels[i]->getFd();`获得文件描述符。
### 5. Reactor模式
![image.png](https://cdn.nlark.com/yuque/0/2023/png/21765548/1679465908521-da29eff8-a536-41d1-a14f-943669b99209.png#averageHue=%23f8f0e2&clientId=u028b706c-46b7-4&from=paste&height=417&id=uf67187ce&name=image.png&originHeight=834&originWidth=1427&originalType=binary&ratio=2&rotation=0&showTitle=true&size=276780&status=done&style=none&taskId=u42cedf9a-1dfe-49cc-975e-cd7441497a0&title=%E5%8D%95Reactor%E5%8D%95%E7%BA%BF%E7%A8%8B%E6%A8%A1%E5%BC%8F&width=713.5 "单Reactor单线程模式")
接下来将项目改写为单Reactor单线程和事件循环模式：
将服务器抽象成`Server`类，存初始化构造函数（最初的监听处理）和处理请求的函数体，类中有一个Reactor(现在还是单Reactor），Reactor里的核心是**事件循环**`EventLoop`类，其成员变量是一个ep对象，其实就是不断`ep.poll()`返回vector<Channel*>并调用**回调函数**。
Channel类的成员不再是ep，而是封装了ep的EventLoop对象，改写所有构造函数。
**回调函数**是这次改写新增的功能，根据创建Channel时注册的回调函数（不同描述符和事件类型的函数不同），执行不同的处理（连接or读写）。
**回调函数**前置知识：对象包装器和绑定器
> std::function是函数包装器，用于存储、复制和调用**可调用目标**，包括普通函数、成员函数、类对象（重载了operator()的类的对象）、Lambda表达式等。是对C++现有的可调用实体的一种类型安全的包裹（相比而言，**函数指针**这种可调用实体，是**类型不安全**的）。std::function因为有着保存函数并可以延迟执行的特性，因此非常适合作为回调函数来使用
> [std::bind用来将可调用对象与起参数一起进行绑定，绑定的结果使用std::function进行保存，并在我们需要调用的时候调用](https://cloud.tencent.com/developer/article/1886364)[2](https://cloud.tencent.com/developer/article/1886364)[。std::bind主要有以下两个作用：将可调用对象和参数绑定成为一个仿函数；只绑定部分参数，减少可调用对象传入的参数](https://blog.csdn.net/qq_38410730/article/details/103637778)[3](https://blog.csdn.net/qq_38410730/article/details/103637778)。

#### 回调函数的注册：
函数体写在`Server`类中，分别是和 `handleReadEvent`，就是之前写的那两个功能。
```cpp
void handleReadEvent(int);
void newConnection(Socket *serv_sock);
```
对于不同场景的Channel，我们先利用std::bind绑定函数体和参数，如新建用的Channel(fd)绑定的函数和参数是`Server::newConnection`和`sockfd_listen`，收发Channel(fd)事件绑定`Server::handleReadEvent`和`channle.get_fd()`
```cpp
 std::function<void()> call_back_func_new_connection = std::bind(&Server::newConnection, this, serv_sock);

std::function<void()> call_back_func_read = std::bind(&Server::handleReadEvent, this, clnt_sock->getFd());
```
注意由于绑定的函数是类成员函数，所以语法上第一个参数是**类成员函数指针**，第二个参数是**实例对象指针this**，第三个参数才是函数参数。
#### 架构理解：
EventLoop包含ep，EventLoop不断loop()，即调用其ep.poll()，将事件列表vector<Channel*>返回，Channel有所属ep和fd信息，另外还有本身注册的回调函数，loop在handle处理Channel时直接调用同一个接口即可（**利用function和bind而不是虚函数实现类似接口功能和多态**？）。
EventLoop中的ep和ep的Channel来自Server类，首先Server初始化监听的Channel并注册连接回调函数，此后当Channel建立连接时，这个**连接回调函数**会新建Channel和注册对应的**read回调函数**，并加到loop(ep)。
以上功能通过`Server *server = new Server(loop);`把功能加到loop上，彼此是分离非耦合的。
```cpp
void Server::newConnection(Socket* socket_listen)
{
    InetAddress* address_client = new InetAddress(); // 会发生内存泄露！没有delete
    int sockfd_client = socket_listen->accept(address_client);
    Socket* socket_client = new Socket(sockfd_client);
    socket_client->setnonblocking();
    Channel* client_Channel = new Channel(loop, socket_client->get_fd()); // 新建Channel
    std::function<void()> call_back_func_read = std::bind(&Server::handleReadEvent, this, socket_client->get_fd());
    client_Channel->set_callback(call_back_func_read);
    client_Channel->enableReading(); // 添加到loop(ep)
    printf("new client fd %d accepted(establised)! IP: %s, Port: %d\n", sockfd_client, inet_ntoa(address_client->addr.sin_addr), ntohs(address_client->addr.sin_port));
}
```
### 6. 添加Acceptor
上一个版本，反复提到的「首先Server初始化监听的Channel并注册连接回调函数」写在了Server构造函数中，这不够抽象。接下来按照Reactor模式中的Acceptor模块设计，为建立连接**操作**抽象出Acceptor类（从Server类中分离出来），**不再有listen用的各种变量**，Server类留下具体函数功能体，应用时用`acceptor = new Acceptor(loop);`把这个功能模块插到loop中。
`Acceptor`类成员变量是服务器监听用的InetAdress->Socket->Channel，此外还有主EventLoop的指针，用于添加监听Channel到loop；成员函数`newConnectionCallback`和相应地赋值函数，也就是为成员Channel注册执行函数。
Server类构造函数中，首先调用Accptor类构造函数为loop新建监听Channel，同时还要传回调函数体（bind Server中的函数）到Accptor类的监听Channel回调函数。
总的来说，函数功能体依旧来自Server，但监听对象来自Acceptor。
### 7. 添加Connection
在上一版的`Server`类中，尽管已经抽出了`Acceptor`用于初始化监听Channel，但是处理建立连接的变量（InetAdress,Socket, Channel)和功能仍堆在函数中，且这些连接难以管理（建立后只丢到了ep中）。现在进一步将一个TCP连接抽象为`Connection`类，它存放`Acceptor`类似的成员和函数（不同点是类内部定义了客户端消息处理函数，接受绑定函数是`Server`中实现的的delete函数），然后在`Server`类中用一个`Map[sockfd_t, Connection*]`即用存所有Channel连接。
`Connection`对象为`Channel`成员注册了echo回调函数，~~另外Connection本身还注册了delete函数，用于清理记录和回收资源。~~
#### 总结当前版本大致流程：

1. 初始化：new一个Acceptor给Loop，具体细节操作是new一个带有new Connection回调函数的Channel给Loop。
2. 建立连接：Loop调用初始化的最初的监听Channel的回调函数，触发new Connection：新建一个带有echo回调的Channel给Loop。
3. 会话：客户端Channel执行其特有的echo回调函数。
### 7.1 缓冲区的封装
在现在的echo函数中，我们通过非阻塞式IO，不断读**服务端指定的buf大小**的数据，直到读完**客户端buf大小（**非阻塞式IO情况下read返回-1且error=11)。这样做的缺点是：用户真正输入的消息可能小于客户端buf，客户端buf形如：「消息+\0+空值空值空值...」，服务端读取时也会把**空值读完**，造成了浪费。
因此我们封装服务端buf，用一个临时服务端buf不断读read，存到服务端buf中，当读到\0后不再读存了, 另外，**读完后才输出全部内容**（而**不是之前读一次服务端buf就输出一次**）。
```cpp
void Buffer::append(const char* _str, int _size){
    for(int i = 0; i < _size; ++i){
        if(_str[i] == '\0') {
            is_over = true;
            break;
        }
        buf.push_back(_str[i]);
    }
}
```
### 8. 线程池
原来的Channel触发回调都是在Loop上阻塞执行，一个Channel处理完才处理下一个。现在基于生产者消费者模型创建一个线程池工作。
首先创建一个任务队列（无上限，有下限0）。每当Reactor创建Channel并发到ep后，还要把这个Channel的echo回调函数（已绑定fd参数）添加到任务队列（注意是互斥操作，先要在作用于上互斥锁`std::unique_lock<std::mutex> lock(tasks_mtx);`）。
```cpp
// 添加任务到任务队列中
void ThreadPool::add(std::function<void()> func)
{
    {
        // 同上，在{}作用域内对std::mutex加锁
        std::unique_lock<std::mutex> lock(tasks_mtx);
        // 如果线程池已经停止，则抛出异常
        if (stop)
            throw std::runtime_error("ThreadPoll already stop, can't add task any more");
        tasks.emplace(func); // 将函数对象添加到任务队列中
    }
    cv.notify_one(); // 通知一个等待条件变量的线程来执行任务
}
```
条件变量`cv`表示任务队列任务非空。线程等待（阻塞）函数第二个参数是判断条件，这里任务队列非空就唤醒。
```cpp
while (true) { // 无限循环
    std::function<void()> task; // 定义一个函数对象task
    {
        // unique_lock和lock_guard类似，但是可以手动控制加锁和解锁
        // std::unique_lock<std::mutex>构造函数需要一个互斥锁对象
        // 这里的tasks_mtx是一个std::mutex对象，用于保护任务队列
        std::unique_lock<std::mutex> lock(tasks_mtx); // 对tasks_mtx加锁
        // cv.wait()会等待条件变量，直到满足指定条件，唤醒该线程并解锁锁定的互斥锁
        // 这里的条件是任务队列不为空或线程池停止
        // 如果条件不满足，则线程将一直等待
        cv.wait(lock, [this]() { // 等待条件变量
            return stop || !tasks.empty(); // 条件为任务队列不为空或线程池停止
        });
        // 如果线程池已经停止并且任务队列为空，则退出线程
        if (stop && tasks.empty())
            return;
        // 从任务队列头取出一个任务
        task = tasks.front();
        tasks.pop(); // 从队列中移除该任务
    }
    task(); // 执行任务
}
```
以上代码定义了一个线程执行的函数，用lambda表达式的方式将这个线程函数绑定到线程，并把这样的n个线程加到线程池，完成初始化。
修改Channel类的handle_event函数，这个函数**不再是立即执行回调函数**，而是把这个Channel绑定的回调函数丢给任务队列，让线程们去消费。
```cpp
void Channel::handle_event() {
    loop->addThread(this->callback);
    // this->callback();
}
```
### 9 主从Reactor多线程
![image.png](https://cdn.nlark.com/yuque/0/2023/png/21765548/1679550272404-aff6d1a6-add7-4a8b-9583-8d112fd2d376.png#averageHue=%23f8efe0&clientId=u0b6a97a5-70f6-4&from=paste&height=631&id=uf4e68745&name=image.png&originHeight=1262&originWidth=1772&originalType=binary&ratio=2&rotation=0&showTitle=false&size=429608&status=done&style=none&taskId=ubb82f3ad-b996-40d2-8d37-4b1b4ae7c6b&title=&width=886)
架构的升级：当前版本是单Reactor多线程模式
Todo list:
服务器版本的迭代是从C语言风格逐渐到C++风格，从单线程到多线程，从阻塞式IO到非阻塞式IO，从任务驱动到事件驱动。已然变成屎山，需要重构。

- [ ] 任务队列的拷贝用右值移动、完美转发
- [ ] 线程池执行函数返回值
- [ ] 智能指针
- [ ] 内存泄露检测、性能测试
