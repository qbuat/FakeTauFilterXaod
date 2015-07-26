#include "FakeTauFilterXaod/FakeTauFilterXaod.h"
#include "TruthUtils/PIDCodes.h"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "xAODTruth/TruthVertex.h"

FakeTauFilterXaod::FakeTauFilterXaod(const std::string & name) : asg::AsgTool(name)

{

  declareProperty("FastJetConeSize", m_fastjet_cone_size=0.4);
  declareProperty("FastJetPtmin", m_fastjet_pt_min=25000.);
  declareProperty("FastJetEtamax", m_fastjet_eta_max=2.8);
  declareProperty("TrueTrackPt", m_true_track_pt=1000.);
  declareProperty("MinPtCore", m_pt_core_min=25000.);
  declareProperty("MinTracksCore", m_min_trk_core=1);
  declareProperty("MaxTracksCore", m_max_trk_core=4);
  declareProperty("MinTracksIso", m_min_trk_iso=0);
  declareProperty("MaxTracksIso", m_max_trk_iso=2);
  declareProperty("CoreDr", m_core_dr=0.2);
  declareProperty("IsoDr", m_iso_dr=0.4);
  declareProperty("NumberOfFakeTaus", m_n_truthfakes=2);

}

FakeTauFilterXaod::FakeTauFilterXaod(const FakeTauFilterXaod & other) : asg::AsgTool(other.name() + "_copy")

{

}

FakeTauFilterXaod::~FakeTauFilterXaod()
{
  for (unsigned int ip = 0; ip < m_TruthFakeTaus.size(); ip++) {
    delete m_TruthFakeTaus.at(ip); 
  }
}



StatusCode FakeTauFilterXaod::initialize() 
{

  return StatusCode::SUCCESS;

}


StatusCode FakeTauFilterXaod::execute(const xAOD::TruthParticleContainer * TruthParticles)
{
  // reset the filter result
  m_pass_filter = false;
  // reset the truthfaketaus
  m_TruthFakeTaus.clear();

  std::vector<IdentifiedPseudoJet> identified_pseudo_jets;
  for (const auto part: *TruthParticles) {

    if (not isGenStable(part))
      continue;

    // No muons
    if (part->pdgId() == MC::PID::MUON or part->pdgId() == MC::PID::ANTIMUON)
      continue;

    // No electron neutrinos 
    if (part->pdgId() == MC::PID::NU_E or part->pdgId() == MC::PID::NU_EBAR)
      continue;

    // No muon neutrinos 
    if (part->pdgId() == MC::PID::NU_MU or part->pdgId() == MC::PID::NU_MUBAR)
      continue;

    // No tau neutrinos 
    if (part->pdgId() == MC::PID::NU_TAU or part->pdgId() == MC::PID::NU_TAUBAR)
      continue;

    identified_pseudo_jets.push_back(IdentifiedPseudoJet(part));
  }


  // fastjet clustering
  const fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, m_fastjet_cone_size); 
  fastjet::ClusterSequence cseq(identified_pseudo_jets, jet_def);
  // auto jets = sorted_by_pt(cseq.inclusive_jets(m_fastjet_pt_min));
  auto jets = cseq.inclusive_jets(m_fastjet_pt_min);
  auto indices = cseq.particle_jet_indices(jets);

  int n_good_jets = 0;
  for (unsigned int ijet = 0; ijet < jets.size(); ijet++) {
    auto jet = jets[ijet];

    if (not is_good_jet(jet))
      continue;

    ATH_MSG_DEBUG("Consider jet "<< ijet 
		 << " out of "<< jets.size()
		 << " with pT = "<< jet.pt()
		 << " eta = " << jet.eta()
		 << " pseudorap = " << jet.pseudorapidity()
		 << " phi = " << jet.phi());

    int n_tracks_core = 0;
    int n_tracks_iso = 0;
    TLorentzVector jet_core;
    jet_core.SetPxPyPzE(0., 0., 0., 0.);
    // loop over all selected input particles
    for (unsigned int ip=0; ip < identified_pseudo_jets.size(); ip++) {
      auto part = identified_pseudo_jets[ip];
      // check that the true particle is in the considered jet.
      if (indices[ip] == (int)ijet) {
	// compute the core pT
	if (is_core_track(part, jet)) {
	  TLorentzVector jet_temp;
	  jet_temp.SetPtEtaPhiM(part.pt(), 
				part.pseudorapidity(),
				part.phi(),
				part.m());
	  jet_core += jet_temp;
	}
	// check that the selected particle 
	// makes a good track candidate 
	if (is_good_track(part)) {
	  ATH_MSG_DEBUG("\t particle "<< ip
		       << " with pT = "<< part.pt()
		       << " eta = " << part.eta()
		       << " phi = " << part.phi()
		       << " pdg id = " << part.pdg_id()
		       << " good track = "<< is_good_track(part));
	  ATH_MSG_DEBUG("\t\t particle "<< ip
		       << " is core = "<< is_core_track(part, jet)
		       << " is iso = " << is_iso_track(part, jet));
	  n_tracks_core += (int)is_core_track(part, jet);
	  n_tracks_iso += (int)is_iso_track(part, jet);
	}
      }
    }

    ATH_MSG_DEBUG("\t\t\t jet "<< ijet 
		 << " out of "<< jets.size()
		 << " with pT = "<< jet.pt()
		 << " and core pt "<<jet_core.Pt());

    if (jet_core.Pt() < m_pt_core_min)
      continue;

    TruthFakeTau* truthfaketau = new TruthFakeTau(jet_core);
    truthfaketau->set_ntracks(n_tracks_core);
    truthfaketau->set_nwidetracks(n_tracks_iso);
    m_TruthFakeTaus.push_back(truthfaketau);
    // ATH_MSG_DEBUG("TruthFakeTau with pt = " << truthfaketau.pt() 
    // 		 << ", eta = " << truthfaketau.pseudorapidity()
    // 		 << ", phi = " << truthfaketau.phi()
    // 		 << ", nTracks = " << truthfaketau.nTracks());


    // check that the jet passes the track counting requirements
    if (n_tracks_core >= m_min_trk_core and n_tracks_core <= m_max_trk_core)
      if (n_tracks_iso >= m_min_trk_iso and n_tracks_iso <= m_max_trk_iso) 
	n_good_jets += 1;
  }

  if (n_good_jets >= m_n_truthfakes)
    m_pass_filter = true;
  else
    m_pass_filter = false;



  return StatusCode::SUCCESS;

}


bool FakeTauFilterXaod::is_good_jet(const fastjet::PseudoJet & jet)
{
  // pt cut
  if (jet.pt() < m_fastjet_pt_min)
    return false;

  // eta cut
  if (fabs(jet.pseudorapidity()) > m_fastjet_eta_max)
    return false;

  return true;
}


bool FakeTauFilterXaod::is_good_track(const IdentifiedPseudoJet & track)
{
  // charge cut
  if (track.charge() == 0)
    return false;

  // pt cut
  if (track.pt() < m_true_track_pt)
    return false;

  return true;
}


bool FakeTauFilterXaod::is_core_track(const fastjet::PseudoJet & track, const fastjet::PseudoJet & jet)
{
  TVector3 jet_vec;
  TVector3 track_vec;
  jet_vec.SetPtEtaPhi(jet.pt(), jet.pseudorapidity(), jet.phi());
  track_vec.SetPtEtaPhi(track.pt(), track.pseudorapidity(), track.phi());
  ATH_MSG_DEBUG("\t\t\t Delta R = " << jet_vec.DeltaR(track_vec));
  if (jet_vec.DeltaR(track_vec) < m_core_dr) 
    return true;
  else
    return false;
}


bool FakeTauFilterXaod::is_iso_track(const fastjet::PseudoJet & track, const fastjet::PseudoJet & jet)
{
  TVector3 jet_vec;
  TVector3 track_vec;
  jet_vec.SetPtEtaPhi(jet.pt(), jet.pseudorapidity(), jet.phi());
  track_vec.SetPtEtaPhi(track.pt(), track.pseudorapidity(), track.phi());
  if (jet_vec.DeltaR(track_vec) < m_iso_dr and not is_core_track(track, jet))
    return true;
  else
    return false;
}


TruthFakeTau* FakeTauFilterXaod::matchedFake(const xAOD::IParticle * p)
{

  ATH_MSG_DEBUG("Tau with pt = "  << p->p4().Pt() 
		<< ", eta = "     << p->p4().Eta()
		<< ", phi = "     << p->p4().Phi());

  TruthFakeTau* matched = NULL;
  double deltaR = 999999;
  // for (auto part: m_TruthFakeTaus) {

  for (unsigned int ip = 0; ip < m_TruthFakeTaus.size(); ip++) {
    
    double deltaR_tmp = DeltaR(p, *(m_TruthFakeTaus.at(ip)));
    if (deltaR_tmp < deltaR) {
      deltaR = deltaR_tmp;
      matched = m_TruthFakeTaus.at(ip);
    }
  }
  if (deltaR < 0.2) {
    return matched;
  }  else {
    return NULL;
  }
}

double FakeTauFilterXaod::DeltaR(const xAOD::IParticle * part, const TruthFakeTau & truthFakeTau)
{
  TLorentzVector tFT_vec;
  tFT_vec.SetPtEtaPhiM(truthFakeTau.pt(),
		       truthFakeTau.pseudorapidity(),
		       truthFakeTau.phi(),
		       truthFakeTau.m());
  return part->p4().DeltaR(tFT_vec);
}


// HACK OF A Athena method
// http://acode-browser2.usatlas.bnl.gov/lxr-rel20/source/atlas/Generators/TruthUtils/TruthUtils/HepMCHelpers.h#0037
bool FakeTauFilterXaod::isGenStable(const xAOD::TruthParticle * part)

{

  /// @name Extra ATLAS-specific particle classifier functions
  //@{
  
  /// @brief Determine if the particle is stable at the generator (not det-sim) level,
  ///
  /// I.e. barcode < 200k and either status 1 or status % 1000 = 2 with a decay vertex
  /// barcode < -200k and/or status > 1000 (the latter would indicate a gen-stable decayed by the G4 sim.)
  if (part->barcode() >= 200000) return false; // This particle is from G4
  if (part->pdgId() == 21 && part->e() == 0) return false; //< Workaround for a gen bug?
  return ((part->status() % 1000 == 1) || //< Fully stable, even if marked that way by G4
	  (part->status() % 1000 == 2 && part->decayVtx() != NULL && part->decayVtx()->barcode() < -200000)); //< Gen-stable with G4 decay
     /// @todo Add a no-descendants-from-G4 check?

}
