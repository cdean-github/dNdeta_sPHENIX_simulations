// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef METADATA_H
#define METADATA_H

#include <phool/PHObject.h>

#include <cmath>
#include <iostream>
#include <string>  // for string
#include <map>

///
class Metadata : public PHObject
{
 public:
  Metadata() = default;

  /// dtor
  virtual ~Metadata() = default;

  PHObject *CloneMe() const override;

  /// Clear Event
  void Reset() override;

  /** identify Function from PHObject
      @param os Output Stream 
   */
  void identify(std::ostream &os = std::cout) const override;

  /// isValid returns non zero if object contains valid data
  int isValid() const override;

  void set_floatval(const std::string & /*name*/, const float /*fval*/);
  float get_floatval(const std::string & /*name*/) const;

  void set_intval(const std::string & /*name*/, const int /*ival*/);
  int get_intval(const std::string & /*name*/) const;

  void set_stringval(const std::string & /*name*/, const std::string /*ival*/);
  std::string get_stringval(const std::string & /*name*/) const;

  /// switches off the pesky virtual warning messages
  void NoWarning(const int i = 1);

 private:
  void warning(const std::string &func) const;

  std::map<std::string, int> m_IntRunProperties;
  std::map<std::string, float> m_FloatRunProperties;
  std::map<std::string, std::string> m_StringRunProperties;

  //ClassDefOverride(Metadata, 1)
};

#endif
