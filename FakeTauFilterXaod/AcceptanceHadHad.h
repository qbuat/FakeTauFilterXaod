#ifndef FakeTauFilterXaod_AcceptanceHadHad_H
#define FakeTauFilterXaod_AcceptanceHadHad_H

#include <EventLoop/Algorithm.h>
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "TH1F.h"

#include "FakeTauFilterXaod/HistogramsBook.h"
#include "FakeTauFilterXaod/FakeTauFilterXaod.h"

class AcceptanceHadHad : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;

  // Cuts 
  float tau1_pt;
  float tau2_pt;

  float min_dr_tautau;
  float max_dr_tautau;

  int n_jets;
  float jet1_pt;
  float jet2_pt;
  float jet_eta;
  bool do_vbf_sel;
  float delta_eta_jj;


  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
public:

  TH1F* m_cutflow; //!
  TH1F* m_truthpairs; //!

  HistogramsBook m_book; //!
  HistogramsBook m_book_os; //!
  HistogramsBook m_book_loose; //!
  
  FakeTauFilterXaod *m_filter; //!

 public:
  // this is a standard constructor
  AcceptanceHadHad ();

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

  virtual EL::StatusCode select_taus(xAOD::TauJetContainer *selected_taus, const xAOD::TauJetContainer *taus);
  virtual EL::StatusCode select_taus_fromtruth(xAOD::TauJetContainer *selected_taus, 
					       const xAOD::TauJetContainer *taus,
					       const DiTruthFakeTaus & truth_pairs);

  virtual EL::StatusCode select_jets(xAOD::JetContainer *selected_jets, const xAOD::JetContainer *jets, const xAOD::TauJet *tau1, const xAOD::TauJet *tau2);
  // this is needed to distribute the algorithm to the workers
  ClassDef(AcceptanceHadHad, 1);
};

#endif
