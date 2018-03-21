#ifndef FAKETAUFILTERXAOD_FAKETAUFILTERXAOD_H
#define FAKETAUFILTERXAOD_FAKETAUFILTERXAOD_H

#include "AsgTools/AsgTool.h"
#include "HEPUtils/FastJet.h"
/* #include "TruthUtils/PIDUtils.h" */
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODBase/IParticle.h"
#include "FakeTauFilterXaod/IFakeTauFilterXaod.h"
#include "TLorentzVector.h"

/// simple class to decorate a fastjet::PseudoJet
/// with charge and pdg_id info
class IdentifiedPseudoJet : public fastjet::PseudoJet {

 public:
 IdentifiedPseudoJet(const xAOD::TruthParticle * p)
    : fastjet::PseudoJet(p->p4().Px(), 
			 p->p4().Py(), 
			 p->p4().Pz(), 
			 p->p4().E())
    {
      m_pdgid = p->pdgId();
      m_charge = p->charge();
    }

  virtual ~IdentifiedPseudoJet() { }

  int pdg_id() const {return m_pdgid;}
  int charge() const {return m_charge;}
    
 private:
  int m_pdgid;
  int m_charge;
};

class TruthFakeTau : public fastjet::PseudoJet 
{
 public:
  TruthFakeTau(const TLorentzVector & vec)
    : fastjet::PseudoJet(vec.Px(), vec.Py(), vec.Pz(), vec.E())
    {}

  int nTracks() const {return m_nTracks;}
  int nWideTracks() const {return m_nWideTracks;}
  
  void set_ntracks(const int & n) {m_nTracks=n;}
  void set_nwidetracks(const int & n) {m_nWideTracks=n;}

  bool is_good() const 
  {
    // Those cuts are hardcoded, this is bad practice :-/
    return ((m_nTracks >= 0 and m_nTracks <= 4) and (m_nWideTracks >= 0 and m_nWideTracks <= 2));
  }

 private:
  int m_nTracks;
  int m_nWideTracks;
};

typedef std::vector<TruthFakeTau*> TruthFakeTaus;
typedef std::vector<std::pair<TruthFakeTau*, TruthFakeTau*>> DiTruthFakeTaus;
class FakeTauFilterXaod : virtual public IFakeTauFilterXaod, 
  public asg::AsgTool
{
  ASG_TOOL_CLASS(FakeTauFilterXaod, IFakeTauFilterXaod)

  public : 

  FakeTauFilterXaod(const std::string & name);
  FakeTauFilterXaod(const FakeTauFilterXaod & other);

  ~FakeTauFilterXaod();

  StatusCode initialize();
  StatusCode execute(const xAOD::TruthParticleContainer * TruthParticles);

  bool pass_filter() {return m_pass_filter;}

  TruthFakeTaus GetTruthFakeTaus() {return m_TruthFakeTaus;} 

  DiTruthFakeTaus GetDiTruthFakeTaus() {return m_DiTruthFakeTaus;}

  TruthFakeTau* matchedFake(const xAOD::IParticle * p);

 private :
  
  bool m_pass_filter;
  TruthFakeTaus m_TruthFakeTaus;
  DiTruthFakeTaus m_DiTruthFakeTaus;
  double m_fastjet_cone_size;
  double m_fastjet_pt_min;
  double m_fastjet_eta_max;
  double m_pt_core_min;
  double m_true_track_pt;
  int m_min_trk_core;
  int m_max_trk_core;
  int m_min_trk_iso;
  int m_max_trk_iso;
  double m_core_dr;
  double m_iso_dr;
  int m_n_truthfakes;

  double m_dr_tau_tau;
  void make_pairs();

  bool is_good_jet(const fastjet::PseudoJet & jet);
  bool is_good_track(const IdentifiedPseudoJet & track);
  bool is_core_track(const fastjet::PseudoJet & track, const fastjet::PseudoJet & jet);
  bool is_iso_track(const fastjet::PseudoJet & track, const fastjet::PseudoJet & jet);

  bool isGenStable(const xAOD::TruthParticle * part);

 public:
  double DeltaR(const xAOD::IParticle * part, const TruthFakeTau & truthFakeTau);




};
#endif
