#include <iostream>

class View
{
public:
  View();
  ~View();
  void start();

  static void setCurrentTPS(int num);
  static void setQueueMsgNum(unsigned long long num);
  static void setErrorNum(unsigned long long num);
  static void setMsgReceived(unsigned long long num);
  static void setMsgSend(unsigned long long num);
  static void setTransactionFinished(unsigned long long num);
  static void setLoadStatus(std::string status);
  static void printDynamic();
  static void printDynamicNormal();
  static void printLoadStatus();
  static void printLoadStatusNormal();
  void printPermanent();
  void printPermanentNormal();
  void print();
  void printNormal();
  void stop();
private:
  void initPermString();

  // members
  static int current_tps_;
  static unsigned long long queueMsg_;
  static unsigned long long errorMsg_;
  static unsigned long long msgReceived_;
  static unsigned long long msgSend_;
  static unsigned long long transaction_;
  static std::string loadStatus_;

  int rate_;
  std::string perm_connectionInfo_;
  std::string perm_taskInfo_;
  std::string perm_expectedTPS_;
  std::string dyna_tps_;
  std::string dyna_transactionsInfo__;
  std::string dyna_msgSend_;
  std::string dyna_msgReceived_;
  std::string dyna_msgQueued_;

  bool stopped_;
};
