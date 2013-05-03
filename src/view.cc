#include "view.hh"
#include "config_manager.hh"
#include "utils.hh"
#include "log.hh"
#include <unistd.h>
#include <curses.h>

int View::current_tps_ = 0;
unsigned long long View::queueMsg_ = 0;
unsigned long long View::errorMsg_ = 0;
unsigned long long View::msgReceived_ = 0;
unsigned long long View::msgSend_ = 0;
unsigned long long View::transaction_ = 0;
std::string View::loadStatus_;
int startX = 10;
int startY = 10;
bool VIEW_DO_STOP = false;

View::View()
{
  rate_ = 1000000;
  stopped_ = false;
  initPermString();
}

View::~View()
{
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
            << "expected TPS: " << perm_expectedTPS_ << std::endl
            << "--------------------------------------------" << std::endl;
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
  std::cout << "TPS: " << current_tps_ << "        "
            << "Transaction: " << transaction_ << std::endl
            << "send: " << msgSend_ << "       "
            << "received: " << msgReceived_ << std::endl
            << "queued: " << queueMsg_ << "      "
            << "error: " << errorMsg_ << std::endl;
}

void View::printDynamic()
{
  mvprintw(startX + 3, startY + 0, "TPS: %d", current_tps_);
  mvprintw(startX + 3, startY + 25, "Transaction: %d", transaction_);
  mvprintw(startX + 4, startY + 0, "send: %d", msgSend_);
  mvprintw(startX + 4, startY + 25, "received: %d", msgReceived_);
  mvprintw(startX + 4, startY + 50, "queued: %d", queueMsg_);
  mvprintw(startX + 5, startY + 0, "error: %d", errorMsg_);
}

void View::start()
{
  print();
  //printNormal();
}

void View::print()
{
  initscr();
  while (!VIEW_DO_STOP) {
    debugLog(NGB_VIEW, "View::print in while");
    printPermanent();
    printDynamic();
    refresh();
    usleep(rate_);
  }
  stopped_ = true;
  
}

void View::stop()
{
  debugLog(NGB_VIEW, "View::stop called");
  VIEW_DO_STOP = true;
  
  debugLog(NGB_VIEW, "View::stop exit curses environment");
  //do {
  //  endwin();
  //} while(0);
  endwin();
  debugLog(NGB_VIEW, "view::stop print last load information");
  printNormal();
}

void View::printNormal()
{
  printPermanentNormal();
  printDynamicNormal();
  //printLoadStatusNormal();
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

void View::setErrorNum(unsigned long long num)
{
  errorMsg_ = num;
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
