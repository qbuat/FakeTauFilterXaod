// ROOT include(s):
#include <TChain.h>
#include <TFile.h>
#include <TError.h>
#include <TSystem.h>
#include <TH1I.h>

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauDefs.h"

// ROOT ACCESS Includes
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

// Local Stuff
#include "FakeTauFilterXaod/FakeTauFilterXaod.h"
#include "FakeTauFilterXaod/FilterValidation.h"
#include "FakeTauFilterXaod/Utils.h"


int main(int argc, char **argv) {


  // Get the name of the application:
  const char* APP_NAME = "validator";

  // Initialise the environment:
  RETURN_CHECK(APP_NAME, xAOD::Init(APP_NAME));

  static const char* FNAME = 
    "/afs/cern.ch/user/q/qbuat/work/public/"
    "mc15_13TeV/mc15_13TeV.341124.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_tautauhh."
    "merge.AOD.e3935_s2608_s2183_r6630_r6264/AOD.05569772._000004.pool.root.1";
 
  std::vector<std::string> filenames;
  if(argc < 2){
    filenames.push_back(std::string(FNAME));
  } else {
    filenames = Utils::splitNames(argv[1]);
  }

  // Create the TEvent object
  xAOD::TEvent event(xAOD::TEvent::kClassAccess);
  xAOD::TStore store;

  ::TChain chain1("CollectionTree");
  for(auto fname : filenames){
    chain1.Add(fname.c_str());
  }

  RETURN_CHECK(APP_NAME, event.readFrom(&chain1));

  FakeTauFilterXaod filter("FakeTauFilter");
  CHECK(filter.initialize());


  FilterValidation val_presel("val_presel");
  FilterValidation val_presel_truth("val_presel_truthselected");
  FilterValidation val_core_tracks("val_core_tracks");
  FilterValidation val_isol_tracks("val_isol_tracks");
  FilterValidation val_isol_tracks_truth("val_isol_tracks_truthselected");
  FilterValidation val_bdt("val_bdt");
  FilterValidation val_bdt_truth("val_bdt_truthselected");

  // std::map<std::string, FilterValidation> filters;
  // filters["val_presel"] = val_presel; //FilterValidation("val_presel");
  // filters["val_presel_truth"] = val_presel_truth; //FilterValidation("val_presel_truthselected");
  // filters["val_core_tracks"] = val_core_tracks; //FilterValidation("val_core_tracks");
  // filters["val_isol_tracks"] = val_isol_tracks; //FilterValidation("val_isol_tracks");
  // filters["val_isol_tracks_truth"] = val_isol_tracks_truth; //FilterValidation("val_isol_tracks_truthselected");
  // filters["val_bdt"] = val_bdt; //FilterValidation("val_bdt");
  // filters["val_bdt_truth"] = val_bdt_truth; //FilterValidation("val_bdt_truthselected");


  auto * h1 = new TH1I("matched", "matched", 1, 0.5, 1.5);
  auto * he = new TH1F("events", "events", 1, 0.5, 1.5);
  auto * hw = new TH1F("weighted_events", "weighted_events", 1, 0.5, 1.5);

  Long64_t entries = event.getEntries();
  for (Long64_t entry = 0; entry < entries; entry++) {
    if ((entry%200)==0)
      ::Info(APP_NAME, "Start processing event %d", (int)entry);

    event.getEntry(entry);

    const xAOD::EventInfo *ei = 0;
    CHECK(event.retrieve(ei, "EventInfo"));

    const xAOD::TruthParticleContainer *truthParticles = 0;
    CHECK(event.retrieve(truthParticles, "TruthParticles"));

    const xAOD::TauJetContainer *taus = 0;
    CHECK(event.retrieve(taus, "TauJets"));
    
    double weight = 1.0;
    bool isMC = false;
    if (ei->eventType(xAOD::EventInfo::IS_SIMULATION)) 
      isMC = true;

    if (isMC) { 
      const std::vector<float> weights = ei->mcEventWeights();
      if (weights.size() > 0) 
	weight = weights[0];
    }

    he->Fill(1.0);
    hw->Fill(1.0, weight);

    // ::Info(APP_NAME, "--------------------------------------");
    CHECK(filter.execute(truthParticles));
    for (const auto tau: *taus) {


      if (tau->pt() < 30000.) 
	continue;
      
      if (fabs(tau->eta() > 2.5) )
	  continue;

      auto * truthfake = filter.matchedFake(tau);
      
      h1->Fill((int)(truthfake != NULL));
      
      if (truthfake == NULL)
	continue;
      val_presel.fill_histograms(tau, truthfake);

      if (truthfake->is_good())
	val_presel_truth.fill_histograms(tau, truthfake);

      if (tau->nTracks() != 1 and tau->nTracks() != 3)
	continue;

      val_core_tracks.fill_histograms(tau, truthfake);

      if (tau->nWideTracks() != 0)
	continue;
      
      val_isol_tracks.fill_histograms(tau, truthfake);
      if (truthfake->is_good()) {
	val_isol_tracks_truth.fill_histograms(tau, truthfake);
      }

      // ::Info(APP_NAME, "Tracks = %d and Iso tracks = %d", truthfake->nTracks(), truthfake->nWideTracks());
      bool is_medium_off = tau->isTau(xAOD::TauJetParameters::JetBDTSigMedium);
      // ::Info(APP_NAME, "Tracks = %d and Iso tracks = %d", truthfake->nTracks(), truthfake->nWideTracks());
      if (not is_medium_off)
	continue;
      // if (not tau->isTau(xAOD::TauJetParameters::IsTauFlag::JetBDTSigMedium))
      // 	continue;

      val_bdt.fill_histograms(tau, truthfake);

      if (truthfake->is_good())
	val_bdt_truth.fill_histograms(tau, truthfake);
    }
	  
  }

  TFile fout("validation.root", "RECREATE");
  // for (auto filt: filters) {
  //   for (auto it: filt.second.Histograms())
  //     it.second->Write();
  //   for (auto it: filt.second.Maps())
  //     it.second->Write();
  // }

  for (auto it: val_presel.Histograms())
    it.second->Write();
  for (auto it: val_presel.Maps())
    it.second->Write();
  for (auto it: val_presel_truth.Histograms())
    it.second->Write();
  for (auto it: val_presel_truth.Maps())
    it.second->Write();
  for (auto it: val_core_tracks.Histograms())
    it.second->Write();
  for (auto it: val_core_tracks.Maps())
    it.second->Write();
  for (auto it: val_isol_tracks.Histograms())
    it.second->Write();
  for (auto it: val_isol_tracks.Maps())
    it.second->Write();
  for (auto it: val_isol_tracks_truth.Histograms())
    it.second->Write();
  for (auto it: val_isol_tracks_truth.Maps())
    it.second->Write();
  for (auto it: val_bdt.Histograms())
    it.second->Write();
  for (auto it: val_bdt.Maps())
    it.second->Write();
  for (auto it: val_bdt_truth.Histograms())
    it.second->Write();
  for (auto it: val_bdt_truth.Maps())
    it.second->Write();
  h1->Write();
  he->Write();
  hw->Write();
  fout.Close();
}
