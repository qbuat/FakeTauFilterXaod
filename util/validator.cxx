// ROOT include(s):
#include <TChain.h>
#include <TFile.h>
#include <TError.h>
#include <TSystem.h>
#include <TH1I.h>

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTau/TauJetContainer.h"

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


  FilterValidation val1("val1");
  auto * h1 = new TH1I("matched", "matched", 2, 0, 2);

  Long64_t entries = event.getEntries();
  for (Long64_t entry = 0; entry < entries; entry++) {
    if ((entry%200)==0)
      ::Info(APP_NAME, "Start processing event %d", (int)entry);

    event.getEntry(entry);

    const xAOD::TruthParticleContainer *truthParticles = 0;
    CHECK(event.retrieve(truthParticles, "TruthParticles"));

    const xAOD::TauJetContainer *taus = 0;
    CHECK(event.retrieve(taus, "TauJets"));
    
    ::Info(APP_NAME, "--------------------------------------");
    CHECK(filter.execute(truthParticles));

    for (const auto tau: *taus) {

      if (tau->pt() < 30000.) 
	continue;
      
      if (fabs(tau->eta() > 2.5) )
	  continue;
      ::Info(APP_NAME, "consider taus of event %d", (int)entry);
      auto * truthfake = filter.matchedFake(tau);
      
      if (truthfake != NULL) {
	h1->Fill(1);
	val1.fill_histograms(tau, truthfake);
      } else
	h1->Fill(0);

    }
	  
  }
  TFile fout("validation.root", "RECREATE");
  for (auto it: val1.Histograms())
    it.second->Write();
  for (auto it: val1.Maps())
    it.second->Write();
  h1->Write();
  fout.Close();
}
