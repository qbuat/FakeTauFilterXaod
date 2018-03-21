################################################################################
# Package: FakeTauFilterXaod
################################################################################

# Declare the package name:
atlas_subdir( FakeTauFilterXaod )

# Declare the package's dependencies
atlas_depends_on_subdirs(PUBLIC
			Control/AthToolSupport/AsgTools
			PhysicsAnalysis/D3PDTools/EventLoop
                        PhysicsAnalysis/AnalysisCommon/PATInterfaces
                        PhysicsAnalysis/AnalysisCommon/PATCore
			Generators/TruthUtils
			Event/xAOD/xAODBase
			Event/xAOD/xAODEventInfo
			Event/xAOD/xAODTau
			Event/xAOD/xAODTracking
                        Event/xAOD/xAODJet
                        Event/xAOD/xAODCore
			Event/xAOD/xAODTruth
)



# Find the needed external(s):
find_package( FastJet )
find_package( HEPUtils )
find_package( ROOT COMPONENTS Core RIO Hist Tree )


# add the library itself:
atlas_add_library (FakeTauFilterXaodLib
   FakeTauFilterXaod/*.h Root/*.cxx ${FakeTauFilterXaodDictSource}
   PUBLIC_HEADERS FakeTauFilterXaod
   INCLUDE_DIRS
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${HEPUTILS_LIBRARIES} ${FASTJET_LIBRARIES} EventLoop AsgTools PATInterfaces TruthUtils xAODBase xAODEventInfo xAODTau xAODTracking xAODJet xAODCore xAODTruth
   PRIVATE_LINK_LIBRARIES )
