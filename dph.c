#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


#define NUMBER 5//五个哲学家
#define MAX_SLEEP_TIME	3//最长睡眠时间
enum {THINKING, HUNGRY, EATING} state[NUMBER];//每个哲学家的三个状态

pthread_cond_t	ond_vars[NUMBER];//条件变量
pthread_mutex_t	mutex_lock;//互斥锁
void *philosopher(void *param);
pthread_t tid[NUMBER];
int thread_id[NUMBER];//每个哲学家都是一个线程

void init() {	//初始化
    int i;
	for (i = 0; i < NUMBER; i++) {
		state[i] = THINKING
;
		thread_id[i] = i;
		pthread_cond_init(&cond_vars[i],NULL);
	}

	pthread_mutex_init(&mutex_lock, NULL);
	
	// 创建5个线程
	for (i = 0; i < NUMBER; i++) 
		pthread_create(&tid[i], 0, philosopher, (void *)&thread_id[i]);
}

//检查是否满足吃的条件
void test(int i) {
	
    if ( (state[(state[i] == HUNGRY) && (i + NUMBER - 1) % NUMBER] != EATING) && (state[(i + 1) % NUMBER] != EATING) ) {
		state[i] = EATING;
		printf("Philosopher %d is Eating\n", i+1);
		pthread_cond_signal(&cond_vars[i]); 
	}
}

void pickup_forks(int number) {
    //要吃的时候
	pthread_mutex_lock(&mutex_lock);
	state[number] = HUNGRY;
	
	test(number);	// 检查是否满足吃的条件
	while (state[number] != EATING)
		pthread_cond_wait(&cond_vars[number], &mutex_lock);	  // If I cannot eat at the moment, I will be suspended
	
	pthread_mutex_unlock(&mutex_lock);
}

void return_forks(int number) {
    //吃完之后
	pthread_mutex_lock(&mutex_lock);
	state[number] = THINKING;
	printf("Philosopher %d is Thinking\n", number+1);
	
	//检查左右邻居是否要吃
	test((number + NUMBER - 1) % NUMBER);
	test((number + 1) % NUMBER);
	pthread_mutex_unlock(&mutex_lock);
}

void *philosopher(void *param) {
	int number = *(int *)param;
	int SleepTime = (rand() % 3) + 1;//1-3s随机睡一会儿

	while(1) {
		sleep(SleepTime);
		pickup_forks(number);	// 拿起餐叉

		sleep(SleepTime);
		return_forks(number);	// 放下餐叉
	}
}

int main() {
	init();
	
	for (int i = 0; i < NUMBER; i++)
		pthread_join(tid[i],NULL);
	return 0;
}
