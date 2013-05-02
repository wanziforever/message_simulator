#include "perf_config_manager.hh"
#include "utils.hh"
#include "log.hh"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include "DOMTreeErrorReporter.hh"

using namespace xercesc;


static const XMLCh *g_xmlch_performance = NULL;       
static const XMLCh *g_xmlch_field = NULL;             
static const XMLCh *g_xmlch_range = NULL;             
static const XMLCh *g_xmlch_attribute_name = NULL;    
static const XMLCh *g_xmlch_attribute_prefix = NULL;  
static const XMLCh *g_xmlch_attribute_suffix = NULL;  
static const XMLCh *g_xmlch_attribute_start = NULL;   
static const XMLCh *g_xmlch_attribute_end = NULL;     
static const XMLCh *g_xmlch_attribute_delta = NULL;

#define INITIALIZE_XMLCH_TAG                                            \
  g_xmlch_performance = XMLString::transcode("performance");            \
  g_xmlch_field = XMLString::transcode("field");                         \
  g_xmlch_range = XMLString::transcode("range");                         \
  g_xmlch_attribute_name = XMLString::transcode("name");                 \
  g_xmlch_attribute_prefix = XMLString::transcode("prefix");             \
  g_xmlch_attribute_suffix = XMLString::transcode("suffix");             \
  g_xmlch_attribute_start = XMLString::transcode("start");               \
  g_xmlch_attribute_end = XMLString::transcode("end");                   \
  g_xmlch_attribute_delta = XMLString::transcode("delta");
  

static XercesDOMParser::ValSchemes gValScheme = XercesDOMParser::Val_Auto;


extern DOMNode* getNextElementNode(DOMNode *cur);

std::string PerfField::toString()
{
  return "name: " + name_ + ", prefix: " + prefix_ + ", suffix: " +
    suffix_ + ", start: " + Utils::longlongToString(start_) +
    ", end: " + Utils::longlongToString(end_) + ", delta: " +
    Utils::intToString(delta_);
}

// get attribute by attribute name for give DOM NODE
extern std::string getAttributeValue(const DOMNode *n, const XMLCh *name);

std::string PerformanceConf::toString()
{
  std::string ret;
  for (std::vector<PerfField*>::iterator ite = fields.begin();
       ite != fields.end(); ite++) {
    ret += (*ite)->toString() + std::string("\n");
  }
  return ret;
}

bool PerfConfigManager::initParser(std::string perfPath)
{
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::initParser enter %s", perfPath.c_str());
  if (!Utils::fileExist(perfPath)) {
    debugLog(NGB_PERF_CONFIG,
             "PerfConfigManager::updateMsgWithPerfConfigData "
             "performance configration data not exist");
    return false;
  }
  
  if(parser_) {
    return true;
  }
  // initialize the XML4C2 system
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::initParser going to init XMLPlatformUtils");
  try {
    XMLPlatformUtils::Initialize();
  }
  catch(const XMLException &toCatch) {
    std::cerr << "Error during Xerces-c Initialization.\n"
              << " Exception message:"
              << StrX(toCatch.getMessage()) << std::endl;
    return false;
  }
  
  INITIALIZE_XMLCH_TAG;
  // Create our parser, then attach an error handler to the parser.
  // The parser will call back to methods of the ErrorHandler if it
  // discovers errors during the course of parsing the XML docuemnt.
  parser_ = new XercesDOMParser;
  errorReporter_ = new DOMTreeErrorReporter();
  parser_->setErrorHandler(errorReporter_);
  parser_->setValidationScheme(gValScheme);

  try {
    parser_->parse(perfPath.c_str());
  }
  catch(...) {
    debugLog(NGB_ERROR, "DOM error occured\n");
  }
  debugLog(NGB_PERF_CONFIG, "PerfConfigManager::initParser exit ");
  return true;
}

bool PerfConfigManager::updateMsgWithPerfConfigData(std::string perfPath)
{
  return true;
}

bool PerfConfigManager::checkPerfConfig(std::string path)
{
  std::string perfName = getPerfConfigName(path);
  return Utils::fileExist(perfName);
  
}

std::string PerfConfigManager::getPerfConfigName(std::string path)
{
  std::string perfName;
  perfName = path + ".perf";
  return perfName;
}

bool PerfConfigManager::parsePerfConfig(PerformanceConf *pconf)
{
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::parsePerfConfig enter");
  DOMDocument *doc = parser_->getDocument();
  DOMNodeList *avpNodeList = doc->getElementsByTagName(g_xmlch_performance);

  if (avpNodeList->getLength() < 1) {
    debugLog(NGB_PERF_CONFIG, "PerfConfigManager::parsePerfConfig "
             "no \"performance\" tag provisioned.");
    return false;
  }
  // if there is more than one performance tag provisioned,
  // just use the first one
  DOMNode *n = avpNodeList->item(0);
  parseFields(n, pconf);
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::parsePerfConfig exit");
}

bool PerfConfigManager::parseFields(DOMNode *n, PerformanceConf *pconf)
{
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::parseFields enter");
  DOMNode *m = n->getFirstChild();
  for (m = getNextElementNode(m); m != NULL; m = getNextElementNode(m)) {
    if (!XMLString::equals(m->getNodeName(), g_xmlch_field))
      continue;
    parseOneField(m, pconf);
  }
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::parseFields exit");
  return 0;
}

bool PerfConfigManager::parseOneField(DOMNode *n, PerformanceConf *pconf)
{
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::parseOneField enter");
  PerfField *field = new PerfField;
  field->setName(getAttributeValue(n, g_xmlch_attribute_name));
  field->setPrefix(getAttributeValue(n, g_xmlch_attribute_prefix));
  field->setSuffix(getAttributeValue(n, g_xmlch_attribute_suffix));
  parseRange(n, field);
  pconf->fields.push_back(field);
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::parseOneField exit");
  return 0;
}

bool PerfConfigManager::parseRange(DOMNode *n, PerfField *field)
{
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::parseRange enter");
  DOMNode *m = n->getFirstChild();
  // only get the first range tag if there is more than one
  if ((m = getNextElementNode(m)) == NULL) {
    debugLog(NGB_PERF_CONFIG,
             "PerfConfigManager::parseRange "
             "there is no range tag provisioned for field %s",
             field->getName().c_str());
    return false;
  }
  std::string tmpstr;
  tmpstr = getAttributeValue(m, g_xmlch_attribute_start);
  field->setStart(Utils::stringToLongLong(tmpstr));
  tmpstr = getAttributeValue(m, g_xmlch_attribute_end);
  field->setEnd(Utils::stringToLongLong(tmpstr));
  tmpstr = getAttributeValue(m, g_xmlch_attribute_delta);
  field->setDelta(Utils::stringToInt(tmpstr));
  debugLog(NGB_PERF_CONFIG,
           "PerfConfigManager::parseRange exit");
  return 0;
}
