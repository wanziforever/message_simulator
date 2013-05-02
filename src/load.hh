#include <iostream>

class Task;
class Load
{
public:
  Load() {};
  ~Load() {}

  void start(Task *task, int expTps);
private:
  void maintainQueueSize();
  void* performanceRunThreadFunc();
  void setViewData();
  // member
  Task *task_;
  int expTps_;
};
