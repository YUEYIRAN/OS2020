#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include<math.h>

#define NUM_THREADS 3
#define MAX_PRODUCT 20 
typedef struct product_s{//定义缓冲区数据结构
	pthread_mutex_t mutex;//互斥锁 
	short init;//用于判断是否初始化缓冲区		
	int rear;//生产者放入
	int front;//消费者读出
	int data[20]; 
}product;
product *p;
pthread_t tid[NUM_THREADS];

sem_t *sem_full;
sem_t *sem_empty;


//生成负指数分布的随机数 
double ProduceTime(double lambda){
	double z=0.0;
    do
    {
        z = ((double)rand() / RAND_MAX);
    }
    while((z==0) || (z == 1));
    return (-1/lambda * log(z));
}


void *producer(void *param){
	int lambda = *(int*)param;
	int random_time = ProduceTime(lambda);
	usleep(random_time);
	
	//生产一个1-100之间的随机数 
	int random_data = rand() % 63 + 1;	
	//等待消费者通知
	sem_wait(sem_empty);
	
	pthread_mutex_lock(&(p->mutex));
    p->data[p->rear] = random_data;
    printf("进程ID: %d, 线程ID: %lu, 随机数: %d\n", getpid(),
     (unsigned long)pthread_self(), p->data[p->rear]);
	p->rear = (p->rear + 1) % MAX_PRODUCT;	 
	pthread_mutex_unlock(&(p->mutex));

	//通知消费者进程
    sem_post(sem_full);
}

int main(int argc, char** argv)
{
    //两个信号量 
    sem_full = sem_open("/Full", O_CREAT, 0644, 0); 
    sem_empty = sem_open("/Empty", O_CREAT, 0644, 20);

    //创建共享内存并预创建空间
    int fd = shm_open("/sh_product", O_CREAT | O_RDWR, 0644);
    ftruncate(fd, sizeof(product));

    //把创建的共享内存文件映射到内存
    p = mmap(0, sizeof(product), PROT_WRITE, MAP_SHARED, fd, 0);
    p->init = 0;

    //验证有无初始化
    if (p->init != 1) {
        memset(p, 0, sizeof(product));
        p->init = 1;
    }
    
    pthread_mutex_init(&(p->mutex), NULL);
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    int lambda = atoi(argv[1]);

    for (int loop = 0; loop < 20; loop++) {
	    int i;
		for(i = 0; i < NUM_THREADS; i++)	
	        pthread_create(&tid[i], &attr, producer, &lambda);
	    
	    for(i = 0; i < NUM_THREADS; i++)
	        pthread_join(tid[i], NULL);
	}
    
    
    munmap(p, sizeof(product));
    shm_unlink("/sh_product"); 
    sem_close(sem_full);
    sem_close(sem_empty);

    return 0;
}