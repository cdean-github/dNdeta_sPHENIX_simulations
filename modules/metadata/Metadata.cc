#include "Metadata.h"

#include <cmath>    // for NAN
#include <utility>  // for pair

void Metadata::identify(std::ostream &out) const
{
  out << "identify yourself: I am an Metadata Object" << std::endl;
  auto iter = m_StringRunProperties.begin();
  while (iter != m_StringRunProperties.end())
  {
    out << iter->first << ": " << iter->second << std::endl;
    ++iter;
  }
  auto iter = m_IntRunProperties.begin();
  while (iter != m_IntRunProperties.end())
  {
    out << iter->first << ": " << iter->second << std::endl;
    ++iter;
  }
  auto iterf = m_FloatRunProperties.begin();
  while (iterf != m_FloatRunProperties.end())
  {
    out << iterf->first << ": " << iterf->second << std::endl;
    ++iterf;
  }
  return;
}

int Metadata::isValid() const
{
  return ((RunNumber) ? 1 : 0);  // return 1 if runnumber not zero
}

void Metadata::set_floatval(const std::string &name, const float fval)
{
  m_FloatRunProperties[name] = fval;
}

float Metadata::get_floatval(const std::string &name) const
{
  std::map<std::string, float>::const_iterator iter = m_FloatRunProperties.find(name);
  if (iter != m_FloatRunProperties.end())
  {
    return iter->second;
  }
  return NAN;
}

void Metadata::set_intval(const std::string &name, const int ival)
{
  m_IntRunProperties[name] = ival;
}

int Metadata::get_intval(const std::string &name) const
{
  std::map<std::string, int>::const_iterator iter = m_IntRunProperties.find(name);
  if (iter != m_IntRunProperties.end())
  {
    return iter->second;
  }
  return -999999;
}

void Metadata::set_stringval(const std::string &name, const string sval)
{
  m_StringRunProperties[name] = sval;
}

int Metadata::get_stringval(const std::string &name) const
{
  std::map<std::string, std::string>::const_iterator iter = m_StringRunProperties.find(name);
  if (iter != m_StringRunProperties.end())
  {
    return iter->second;
  }
  return "Empty";
}
