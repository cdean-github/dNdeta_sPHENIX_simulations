// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef METADATA_H
#define METADATA_H

#include <phool/PHObject.h>

#include <cmath>
#include <iostream>
#include <string>  // for string

///
class Metadata : public PHObject
{
 public:
  /// dtor
  ~Metadata() override {}

  PHObject *CloneMe() const override;

  /// Clear Event
  void Reset() override;

  /** identify Function from PHObject
      @param os Output Stream 
   */
  void identify(std::ostream &os = std::cout) const override;

  /// isValid returns non zero if object contains valid data
  int isValid() const override;

  virtual void set_floatval(const std::string & /*name*/, const float /*fval*/) { return; }
  virtual float get_floatval(const std::string & /*name*/) const { return NAN; }

  virtual void set_intval(const std::string & /*name*/, const int /*ival*/) { return; }
  virtual int get_intval(const std::string & /*name*/) const { return -999999; }

  virtual void set_stringval(const std::string & /*name*/, const string /*ival*/) { return; }
  virtual int get_stringval(const std::string & /*name*/) const { return "Empty"; }

  /// switches off the pesky virtual warning messages
  void NoWarning(const int i = 1);

 private:
  void warning(const std::string &func) const;

  ClassDefOverride(Metadata, 1)
};

#endif
