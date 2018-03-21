#ifndef FakeTauFilterXaod_FilterValidator_H
#define FakeTauFilterXaod_FilterValidator_H

#include <map>
#include <EventLoop/Algorithm.h>

#include "FakeTauFilterXaod/FilterValidation.h"
#include "FakeTauFilterXaod/FakeTauFilterXaod.h"

class FilterValidator : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;



  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:
  // Tree *myTree; //!
  // TH1 *myHist; //!

  std::map<std::string, FilterValidation*> m_book; //!
  FakeTauFilterXaod *m_filter; //!
  TH1F * he; //!
  TH1F* hw; //!
  TH1F* hnpassfilter;//!
  TH1F* hnpairpassfilter;//!
  TH1F* htaus; //!

  // this is a standard constructor
  FilterValidator ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // this is needed to distribute the algorithm to the workers
  //  ClassDef(FilterValidator, 1);
};

#endif
