#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <math.h>

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

sem_t *sem_full;
sem_t *sem_empty;

void *consumer(void *param)
{
	int lambda = *(int*)param;
	int random_time = ProduceTime(lambda);
	usleep(random_time);
	
	//等待生产者通知
    sem_wait(sem_full);

    pthread_mutex_unlock(&(p->mutex));
    printf("pid: %d, tid: %lu, random_num: %d\n", getpid(), (unsigned long)pthread_self(), p->data[p->front]);
    p->data[p->front] = 0;
    p->front = (p->front + 1) % MAX_PRODUCT;
	pthread_mutex_unlock(&(p->mutex));
	
	//通知生产者生产
    sem_post(sem_empty);
}

int main(int argc, char** argv){
    //full
	sem_full = sem_open("/Full", O_EXCL, 0644, 0);
    if (sem_full == SEM_FAILED) {
        fprintf(stderr, "sem_open error\n");
        exit(1);
    }
	
	//empty
    sem_empty = sem_open("/Empty", O_EXCL, 0644, 20);
    if (sem_empty == SEM_FAILED) {
        fprintf(stderr, "sem_open error\n");
        exit(1);
    }
	
	//共享内存 
    int fd = shm_open("/sh_product", O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        fprintf(stderr, "shm_open error\n");
        exit(1);
    }

	//把共享内存文件映射到内存
    p = mmap(NULL, sizeof(product), PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "mmap error\n");
        exit(1);
    }

    if (p->init != 1) {
        memset(p, 0, sizeof(product));
        p->init = 1;
    }
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    int lambda = atoi(argv[1]);
    for (int loop = 0; loop < 20; loop++) {
	    int i;
		for(i = 0; i < NUM_THREADS; i++)	
	        pthread_create(&tid[i], &attr, consumer, &lambda);
	    
	    for(i = 0; i < NUM_THREADS; i++)
	        pthread_join(tid[i], NULL);
	}
    
    munmap(p, sizeof(product));
    shm_unlink("/sh_product");
    sem_close(sem_full);
    sem_close(sem_empty);

    return 0;
}