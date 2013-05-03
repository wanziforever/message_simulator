#include "load.hh"
#include "task.hh"
#include "perf_config_manager.hh"
#include "view.hh"
#include "log.hh"
#include <unistd.h>


#define MAX_QUEUE_SIZE 100

#ifdef UDP
#include "udp_agent.hh"
extern UdpAgent *g_udp_agent;
#elif defined TCP
#include "tcp_agent.hh"
extern TcpAgent *g_tcp_agent;
#endif
int tps = 0;
unsigned long long transaction = 0;

bool LOAD_DO_STOP = false;

typedef void* (Load::*Thread2Ptr)(void);
typedef void* (*PthreadPtr)(void*);

void Load::start(Task *task, int expTps)
{
  task_ = task;
  expTps_ = expTps;
  debugLog(NGB_LOAD, "Load::start enter, expected TPS is %d", expTps);
  debugLog(NGB_LOAD, "Load::start enter print task %s",
           task_->toString().c_str());
  pthread_t thread;
  pthread_attr_t thread_attr;

  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr,
                              PTHREAD_CREATE_DETACHED);
  
  Thread2Ptr t = &Load::performanceRunThreadFunc;
  PthreadPtr p = *(PthreadPtr*)&t;
  pthread_create(&thread, &thread_attr, p, this);

  return;
}

void Load::stop()
{
  debugLog(NGB_LOAD, "Load::stop called");
  LOAD_DO_STOP = true;
  while (!stopped_) {
    usleep(2000);
  }
  sleep(4);
  debugLog(NGB_LOAD, "Load::stop set the view data at last");
  setViewData();
  // better to delete the run_item resource, but not big problem
  // process will quit
  debugLog(NGB_LOAD, "Load::stop load has been stopped");
}

void* Load::performanceRunThreadFunc()
{
  unsigned long long expInterval = 0;
  unsigned long long interval = 0;
  expInterval = 1000000 / expTps_;
  while (!LOAD_DO_STOP) {
    debugLog(NGB_LOAD,
             "Load::performanceRunThreadFunc task data %s",
             task_->toString().c_str());
    interval = task_->processTask();
    if (interval == -1) {
      debugLog(NGB_LOAD,
               "Load::performanceRunThreadFunc fail to process task");
      View::setLoadStatus("load thread exit with error");
      return (void*)-1;
    }
    transaction++;
    if (interval < expInterval) {
      debugLog(NGB_LOAD,
               "Load::performanceRunThreadFunc delay %d usec "
               "for next transaction", expInterval-interval);
      usleep(expInterval - interval);
      tps = expTps_;
    } else {
      // maybe need to consider the average TPS
      tps = 1000000 / interval;
    }
    maintainQueueSize();
    setViewData();
  }
  stopped_ = true;
}

void Load::maintainQueueSize()
{
#ifdef UDP
  if (g_udp_agent->getQueueSize() >= MAX_QUEUE_SIZE) {
    g_udp_agent->clearQueue();
  }
#elif defined TCP
  if (g_tcp_agent->getQueueSize() >= MAX_QUEUE_SIZE) {
    g_tcp_agent->clearQueue();
  }
#endif
}

void Load::setViewData()
{
  View::setCurrentTPS(tps);
  View::setMsgReceived(task_->getMsgReceived());
  View::setMsgSend(task_->getMsgSend());
  View::setTransactionFinished(transaction);
#ifdef UDP
  View::setQueueMsgNum(g_udp_agent->getTotalCounter());
  View::setErrorNum(g_udp_agent->getErrorCounter());
#elif defined TCP
  View::setQueueMsgNum(g_tcp_agent->getTotalCounter());
  View::setErrorNum(g_tcp_agent->getErrorCounter());
#endif
}
