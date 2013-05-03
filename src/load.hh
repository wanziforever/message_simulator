#include <iostream>

class Task;
class Load
{
public:
  Load() : task_(0), expTps_(0), stopped_(false) {};
  ~Load() {}

  void start(Task *task, int expTps);
  void stop();
private:
  void maintainQueueSize();
  void* performanceRunThreadFunc();
  void setViewData();
  // member
  Task *task_;
  int expTps_;
  bool stopped_;
};
