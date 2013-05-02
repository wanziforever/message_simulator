#ifndef NGB_PERF_CONFIG_MANAGER_H__
#define NGB_PERF_CONFIG_MANAGER_H__

#include <iostream>
#include <vector>
#include <xercesc/util/XercesDefs.hpp>

class PerfField
{
public:
  PerfField() : name_(""), prefix_(""), suffix_(""),
                start_(0), end_(0), delta_(0) {}
  ~PerfField() {}

  // getter menthod
  std::string getName() { return name_; }
  std::string getPrefix() { return prefix_; }
  std::string getSuffix() { return suffix_; }
  unsigned long long getStart() { return start_; }
  unsigned long long getEnd() { return end_; }
  int getDelta() { return delta_; }

  void setName(std::string name) { name_ = name; }
  void setPrefix(std::string prefix) { prefix_ = prefix; }
  void setSuffix(std::string suffix) { suffix_ = suffix; }
  void setStart(long long start) { start_ = start; }
  void setEnd(long long end) { end_ = end; }
  void setDelta(int delta) { delta_ = delta; }
  std::string toString();
  
private:

  // members
  std::string name_;
  std::string prefix_;
  std::string suffix_;
  unsigned long long start_;
  unsigned long long end_;
  int delta_;
};

class PerformanceConf
{
public:
  bool hasPerformanceData() { return fields.size(); }
  std::string toString();

  // members
  std::vector<PerfField*> fields;
};
  

XERCES_CPP_NAMESPACE_BEGIN
class DOMNode;
class XercesDOMParser;
XERCES_CPP_NAMESPACE_END
class DOMTreeErrorReporter;

class Message;
class PerfConfigManager
{
public:
  PerfConfigManager(): parser_(0) {}
  ~PerfConfigManager() {}
  bool updateMsgWithPerfConfigData(std::string filePath);
  bool parsePerfConfig(PerformanceConf *pconf);
  bool initParser(std::string name);
  std::string getPerfConfigName(std::string path);
private:
  bool checkPerfConfig(std::string filePath);
  bool parseFields(xercesc::DOMNode *n, PerformanceConf *pconf);
  bool parseOneField(xercesc::DOMNode *n, PerformanceConf *pconf);
  bool parseRange(xercesc::DOMNode *n, PerfField *field);
  // members
  xercesc::XercesDOMParser *parser_;
  DOMTreeErrorReporter *errorReporter_;
};

#endif
