#include "view.hh"
#include "config_manager.hh"
#include "utils.hh"
#include <unistd.h>
#include <curses.h>

int View::current_tps_ = 0;
unsigned long long View::queueMsg_ = 0;
unsigned long long View::msgReceived_ = 0;
unsigned long long View::msgSend_ = 0;
unsigned long long View::transaction_ = 0;
std::string View::loadStatus_;
extern bool SYSTEM_STOP;
int startX = 10;
int startY = 10;

View::View()
{
  rate_ = 1000000;
  initPermString();
}

View::~View()
{
  endwin();
}

void View::initPermString()
{
  std::string dest = ConfigManager::getDestAddress();
  int destPort = ConfigManager::getDestPort();
  // can change to the local ip address in future
  std::string local = "local";
  int localPort = ConfigManager::getLocalPort();

  perm_connectionInfo_ = local + ":" + Utils::intToString(localPort) +
    " ----------> " + dest + ":" + Utils::intToString(destPort);
  perm_taskInfo_ = ConfigManager::getTask();
  perm_expectedTPS_ = Utils::intToString(ConfigManager::getExpectedTPS());
}


void View::printPermanentNormal()
{
  std::cout << perm_connectionInfo_ << std::endl
            << "task: " << perm_taskInfo_ << std::endl
            << "expected TPS: " << perm_expectedTPS_ << std::endl;
}

void View::printPermanent()
{
  clear();
  refresh();
  mvprintw(startX + 0, startY + 0, "%s", perm_connectionInfo_.c_str());
  mvprintw(startX + 1, startY + 0, "task: %s", perm_taskInfo_.c_str());
  mvprintw(startX + 1, startY + 30, "expected TPS: %s", perm_expectedTPS_.c_str());
  mvprintw(startX + 2, startY + 0, "---------------------------------------------------");
}

void View::printDynamicNormal()
{
  std::cout << "TPS: " << current_tps_ << std::endl
            << "Transaction: " << transaction_ << std::endl
            << "send: " << msgSend_ << std::endl
            << "received: " << msgReceived_ << std::endl
            << "queued: " << queueMsg_ << std::endl;
}

void View::printDynamic()
{
  mvprintw(startX + 3, startY + 0, "TPS: %d", current_tps_);
  mvprintw(startX + 3, startY + 30, "Transaction: %d", transaction_);
  mvprintw(startX + 4, startY + 0, "send: %d", msgSend_);
  mvprintw(startX + 4, startY + 15, "received: %d", msgReceived_);
  mvprintw(startX + 4, startY + 30, "queued: %d", queueMsg_);
}

void View::start()
{
  print();
  //printNormal();
}

void View::print()
{
  initscr();
  while (!SYSTEM_STOP) {
    printPermanent();
    printDynamic();
    refresh();
    usleep(rate_);
  }
  endwin();
}

void View::printNormal()
{
  while (!SYSTEM_STOP) {
    printPermanentNormal();
    printDynamicNormal();
    printLoadStatusNormal();
    usleep(rate_);
  }
}

void View::printLoadStatusNormal()
{
  std::cout << "STATUS: " << loadStatus_ << std::endl;
}

void View::printLoadStatus()
{
  mvprintw(0, 5, "STATUS: %s", loadStatus_.c_str());
}

void View::setCurrentTPS(int num)
{
  current_tps_ = num;
}

void View::setQueueMsgNum(unsigned long long num)
{
  queueMsg_ = num;
}

void View::setMsgReceived(unsigned long long num)
{
  msgReceived_ = num;
}

void View::setMsgSend(unsigned long long num)
{
  msgSend_ = num;
}

void View::setTransactionFinished(unsigned long long num)
{
  transaction_ = num;
}

void View::setLoadStatus(std::string status)
{
  loadStatus_ = status;
}
