#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>

using namespace xercesc;

int main()
{
  std::cout << "main start..." << std::endl;
  XMLPlatformUtils::Initialize();
  //char tmp[5] = {'c','o','m','m',0};
  //std::cout << "before tmp is: " << tmp << std::endl;
  //XMLCh *aa = XMLString::transcode(tmp);
  //std::cout << "after tmp is: " << tmp << std::endl;
  XMLCh *aa = XMLString::transcode("command");
  //std::cout << "result is; " << XMLString::transcode(aa) << std::endl;
  return 0;
}
