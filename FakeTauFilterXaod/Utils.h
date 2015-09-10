#include "xAODBase/IParticle.h"
#include "xAODTau/TauJet.h"
/* #include "xAODTau/TauDefs.h" */


#include <vector>
#include <string>

// Error checking macro
#define CHECK( ARG )\
  do {                                                                  \
  const bool result = ARG;\
  if(!result) {\
  ::Error(APP_NAME, "Failed to execute: \"%s\"",\
	  #ARG );    \
  return 1;\
  }\
  } while( false )


namespace Utils {
  bool comparePt(const xAOD::IParticle* t1, const xAOD::IParticle* t2) {
    return (t1->pt() > t2->pt() ? true: false);
  }

  bool compareBDT(const xAOD::TauJet* t1, const xAOD::TauJet* t2) {
    auto BDT_flag = xAOD::TauJetParameters::TauID::BDTJetScore;
    return (t1->discriminant(BDT_flag) > t2->discriminant(BDT_flag) ? true: false);
  }



  std::vector<std::string> splitNames(const std::string& files, std::string sep = ",") {
    std::vector<std::string> fileList;
    for (size_t i=0,n; i <= files.length(); i=n+1) {
      n = files.find_first_of(sep,i);
      if (n == std::string::npos) {
	n = files.length();
      }
      std::string tmp = files.substr(i,n-i);
      std::string ttmp;
      for(unsigned int j=0; j<tmp.size(); j++) {
	if(tmp[j]==' ' || tmp[j]=='\n') { continue; }
	ttmp += tmp[j];
      }
      fileList.push_back(ttmp);
    }
    return fileList;
  }

}
