#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <sys/stat.h>
#include <fcntl.h>


static int latency_target_fd = -1;
static int32_t latency_target_value = 0;
static void set_latency_target(void)
{
	struct stat s;
	int err;

	errno = 0;
	err = stat("/dev/cpu_dma_latency", &s);
	if (err == -1) {
		std::cout<<errno<< "WARN: stat /dev/cpu_dma_latency failed"<<std::endl;
		return;
	}

	errno = 0;
	latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
	if (latency_target_fd == -1) {
		std::cout<<errno<< "WARN: open /dev/cpu_dma_latency fail"<<std::endl;
		return;
	}

	errno = 0;
	err = write(latency_target_fd, &latency_target_value, 4);
	if (err < 1) {
		std::cout<<errno<<"# error setting cpu_dma_latency to "<< latency_target_value <<std::endl;
		close(latency_target_fd);
		return;
	}
	printf("# /dev/cpu_dma_latency set to %dus\n", latency_target_value);
}
void print_time(){
  using namespace std::chrono;

    // get current time
    auto now = system_clock::now();

    // get number of milliseconds for the current second
    // (remainder after division into seconds)
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    
    auto mus = duration_cast<microseconds>(now.time_since_epoch()) % 1000;

    // convert to std::time_t in order to convert to std::tm (broken time)
    auto timer = system_clock::to_time_t(now);

    // convert to broken time
    std::tm bt = *std::localtime(&timer);

    std::ostringstream oss;

    oss << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count()<<'.'<<std::setfill('0') << std::setw(3) << mus.count();
    
   std::cout<<'['<<oss.str()<<']';
}
void handle(int SignNo){
  print_time();
  if(SignNo == SIGUSR1){
    printf("capture SIGUSR1\n");
  } else{
    printf("capture sin no is %d\n",SignNo);
  }
}
void timer_sign_set(){
 struct sigevent evp;
 struct itimerspec ts;
 timer_t timer;
 int ret;

 evp.sigev_value.sival_ptr = &timer;
 evp.sigev_notify = SIGEV_SIGNAL;
 evp.sigev_signo = SIGALRM;
 signal( evp.sigev_signo, handle);

 ret = timer_create(CLOCK_REALTIME, &evp, &timer);
 if( ret )
  perror("timer_create");
 ts.it_interval.tv_sec = 0;
 ts.it_interval.tv_nsec = 1000000;
 ts.it_value.tv_sec = 1;
 ts.it_value.tv_nsec = 0;

 ret = timer_settime(timer, 0, &ts, NULL);
 if( ret )
  perror("timer_settime");
  
}

int main(){
  set_latency_target();
  timer_sign_set();
  while(true){
    sleep(5);
  }
}


