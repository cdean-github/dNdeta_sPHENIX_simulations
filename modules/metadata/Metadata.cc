#include "Metadata.h"

#include <cmath>    // for NAN
#include <utility>  // for pair
#include <phool/phool.h>

PHObject* Metadata::CloneMe() const
{
  std::cout << "Metadata::CloneMe() is not implemented in daugther class" << std::endl;
  return nullptr;
}

void Metadata::Reset()
{
  std::cout << PHWHERE << "ERROR Reset() not implemented by daughter class" << std::endl;
  return;
}


void Metadata::identify(std::ostream &out) const
{
  out << "Metadata information" << std::endl;
  auto iters = m_StringRunProperties.begin();
  while (iters != m_StringRunProperties.end())
  {
    out << iters->first << ": " << iters->second << std::endl;
    ++iters;
  }
  auto iteri = m_IntRunProperties.begin();
  while (iteri != m_IntRunProperties.end())
  {
    out << iteri->first << ": " << iteri->second << std::endl;
    ++iteri;
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
  std::cout << PHWHERE << "isValid not implemented by daughter class" << std::endl;
  return 0;
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

void Metadata::set_stringval(const std::string &name, const std::string sval)
{
  m_StringRunProperties[name] = sval;
}

std::string Metadata::get_stringval(const std::string &name) const
{
  std::map<std::string, std::string>::const_iterator iter = m_StringRunProperties.find(name);
  if (iter != m_StringRunProperties.end())
  {
    return iter->second;
  }
  return "Empty";
}
