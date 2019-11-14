////////////////////////////////////////////////////////////////////////
// Class:       CosmicAna
// Plugin Type: analyzer (art v3_02_06)
// File:        CosmicAna_module.cc
//
// Generated at Tue Jul  2 08:25:23 2019 by Viktor Pec using cetskelgen
// from cetlib version v3_07_02.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "larcore/Geometry/Geometry.h"
#include "larcorealg/Geometry/GeometryCore.h"
#include "larcoreobj/SimpleTypesAndConstants/geo_types.h"

#include "art_root_io/TFileService.h"

#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCTruth.h"

#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
class CosmicAna;


class CosmicAna : public art::EDAnalyzer {
public:
  explicit CosmicAna(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  CosmicAna(CosmicAna const&) = delete;
  CosmicAna(CosmicAna&&) = delete;
  CosmicAna& operator=(CosmicAna const&) = delete;
  CosmicAna& operator=(CosmicAna&&) = delete;

  // Required functions.
    virtual void analyze(art::Event const& e) override;

    virtual void beginJob() override;

public:
    static const size_t fMaxTrajPoints = 3000;

private:
    bool insideTPC(const TVector3& pos);
    void GetTPClimits();

private:
    // Declare member data here.
    // labels
    std::string fSimModuleLabel;


    int fEvent;
    int fRun;
    int fSubRun;

    TH1* fLengthHist;
    TH1* fDedxVsX;
    TH1* fDedxVsY;
    TH1* fDedxVsZ;


    TTree* fTree;

    struct Muon_t {
	double startPos[4]; /// where the muon entered the active TPC volume
	double endPos[4]; /// where the muon stopped or left the active TPC volume
	double startMom[4]; /// momentum at where the muon entered the active TPC volume
	double endMom[4]; /// momentum at where the muon stopped or left the active TPC volume
	unsigned short exited;

	unsigned int n_sp;
	double sp_pos[fMaxTrajPoints][3]; /// positions of individual
					  /// trajectory spacepoints
					  /// inside the active volume
					  /// of the TPC
	double sp_en[fMaxTrajPoints]; /// energy at individual spacepoints
	double dedx[fMaxTrajPoints];
	double dedx_pos[fMaxTrajPoints][3];

	std::string endProcess;
    } fMuon;

    geo::GeometryCore const* fGeometryService;

    double fXmin;
    double fXmax;
    double fYmin;
    double fYmax;
    double fZmin;
    double fZmax;
};


CosmicAna::CosmicAna(fhicl::ParameterSet const& p)
  : EDAnalyzer{p},
  // More initializers here.
    fSimModuleLabel   (p.get< std::string >("SimModuleLabel"))
{

    // Get a pointer to the geometry service provider.
    fGeometryService = lar::providerFrom<geo::Geometry>();

    GetTPClimits();

    // Call appropriate consumes<>() for any products to be retrieved by this module.
    // Since art 2.8, you can and should tell beforehand, here in the constructor,
    // all the data the module is going to read ("consumes") or might read
    // ("may_consume"). Diligence here will in the future help the framework
    // execute modules in parallel, making sure the order is correct.
    consumes<std::vector<simb::MCParticle>>(fSimModuleLabel);
    // consumes<std::vector<sim::SimChannel>>(fSimulationProducerLabel);
    consumes<art::Assns<simb::MCTruth, simb::MCParticle>>(fSimModuleLabel);
    // consumes<std::vector<recob::Hit>>(fHitProducerLabel);
    // consumes<std::vector<recob::Cluster>>(fClusterProducerLabel);
    // consumes<art::Assns<recob::Cluster, recob::Hit>>(fHitProducerLabel);
}

void CosmicAna::beginJob()
{
    art::ServiceHandle<art::TFileService const> tfs;

    fLengthHist = tfs->make<TH1D>("length", ";Trajectory length [cm]", 220, 0, 1100);

    fDedxVsX = tfs->make<TH2D>("dedxvsx", ";X [cm];dE/dx [MeV/cm]", 20, fXmin, fXmax, 100, 0., 5.);
    fDedxVsY = tfs->make<TH2D>("dedxvsy", ";Y [cm];dE/dx [MeV/cm]", 20, fYmin, fYmax, 100, 0., 5.);
    fDedxVsZ = tfs->make<TH2D>("dedxvsz", ";Z [cm];dE/dx [MeV/cm]", 23, fZmin, fZmax, 100, 0., 5.);


    fTree = tfs->make<TTree>("muon_tree", "Truth data for all cosmic muons entering the TPC");

    fTree -> Branch("Event" , &fEvent , "Event/I");
    fTree -> Branch("Run"   , &fRun   , "Run/I");
    fTree -> Branch("SubRun", &fSubRun, "SubRun/I");

    fTree -> Branch("startPos", fMuon.startPos, "startPos[4]/D");
    fTree -> Branch("endPos", fMuon.endPos, "endPos[4]/D");
    fTree -> Branch("startMom", fMuon.startMom, "startMom[4]/D");
    fTree -> Branch("endMom", fMuon.endMom, "endMom[4]/D");
    fTree -> Branch("exited", &fMuon.exited, "exited/s");

    fTree -> Branch("n_points", &fMuon.n_sp, "n_points/i");
    fTree -> Branch("sp_pos", fMuon.sp_pos, "sp_pos[n_points][3]/D");
    fTree -> Branch("sp_en", fMuon.sp_en, "sp_en[n_points]/D");
    fTree -> Branch("dedx", fMuon.dedx, "dedx[n_points]/D");
    fTree -> Branch("dedx_pos", fMuon.dedx_pos, "dedx_pos[n_points][3]/D");

    fTree -> Branch("endProcess", &fMuon.endProcess);
}


void CosmicAna::analyze(art::Event const& e)
{
  // Implementation of required member function here.
    fEvent = e.id().event();
    fRun = e.run();
    fSubRun = e.subRun();


    art::Handle< std::vector<simb::MCParticle> > particleHandle;
    // Then tell the event to fill the vector with all the objects of
    // that type produced by a particular producer.
    //
    // Note that if I don't test whether this is successful, and there
    // aren't any simb::MCParticle objects, then the first time we
    // access particleHandle, art will display a "ProductNotFound"
    // exception message and, depending on art settings, it may skip
    // all processing for the rest of this event (including any
    // subsequent analysis steps) or stop the execution.
    if (!e.getByLabel(fSimModuleLabel, particleHandle))
	{
	    // If we have no MCParticles at all in an event, then we're in
	    // big trouble. Throw an exception to force this module to
	    // stop. Try to include enough information for the user to
	    // figure out what's going on. Note that when we throw a
	    // cet::exception, the run and event number will automatically
	    // be displayed.
	    //
	    // __LINE__ and __FILE__ are values computed by the compiler.

	    throw cet::exception("AnalysisExample")
		<< " No simb::MCParticle objects in this event - "
		<< " Line " << __LINE__ << " in file " << __FILE__ << std::endl;
	}

    int total_muons = 0;
    int tpc_crossers = 0;
    for ( auto const& particle: (*particleHandle) ) {
	// check if this is a cosmic muon
	//   - muon which originates outside the TPC
	//   - and is a primary particle
	if ( TMath::Abs(particle.PdgCode()) != 13 ) continue;
	if ( insideTPC(particle.Position().Vect()) ) continue;
	if ( particle.Process() != "primary" ) continue;

	total_muons++;

	auto traj = particle.Trajectory();

	// find whether it has entered the TPC and where
	auto it = traj.begin(); // iterator is a pointer to a pair of 2 TLorentzVectors, pos, mom
	for (; it != traj.end(); it++)
	    if ( insideTPC(it->first.Vect()) ) break;

	if ( it == traj.end() ) // muon did not cross the TPC
	    continue;

	tpc_crossers++;

	// need to loop over all trajectory points, check if the point
	// is within the TPC, calculate distance between points,
	// energy loss and total travelled distance from entering the
	// TPC

        double lastEnergy = it->second.E(); // energy in GeV
	TVector3 lastPos = it->first.Vect(); // spatial dimensions in cm
	bool exited = false;
	double len = 0.;
	// std::cout<<total_muons<<"th muon is crossing and has "<<traj.size()<<" trajectory points"<<std::endl
	// 	 <<"Initial energy at entering the TPC: "<<lastEnergy<<std::endl
	// 	 <<"And entering at ";
	// lastPos.Print();

	it->first.GetXYZT(fMuon.startPos);
	it->second.GetXYZT(fMuon.startMom);

	fMuon.n_sp = 0;
	it++;
	for (; it != traj.end(); it++) {
	    // first check if we are still inside the TPC
	    TVector3 pos (it->first.Vect());
	    if (!insideTPC(pos)) break;

	    double de = lastEnergy - it->second.E(); // GeV
	    double dx = (lastPos - pos).Mag(); // cm

	    TVector3 dedx_pos(lastPos + (pos - lastPos)*0.5);
	    double dedx = de*1e3/dx;
	    fDedxVsX -> Fill (pos.X(),dedx);
	    fDedxVsY -> Fill (pos.Y(),dedx);
	    fDedxVsZ -> Fill (pos.Z(),dedx);


	    lastEnergy = it->second.E();
	    lastPos = pos;

	    len += dx;

	    pos.GetXYZ(fMuon.sp_pos[fMuon.n_sp]);
	    fMuon.sp_en[fMuon.n_sp] = it->second.E();
	    fMuon.dedx[fMuon.n_sp] = dedx;
	    dedx_pos.GetXYZ(fMuon.dedx_pos[fMuon.n_sp]);
	    fMuon.n_sp++;
	}
	if (it != traj.end()) {
	    fMuon.endProcess = "exit";
	    exited = true;
	} else {
	    fMuon.endProcess = particle.EndProcess();
	}
	fMuon.exited = exited;

	it--; // return to the last inside position of the trajectory
	it->first.GetXYZT(fMuon.endPos);
	it->second.GetXYZT(fMuon.endMom);

	fTree->Fill();

	fLengthHist->Fill(len);
    }

    std::cout<<"Event: "<<fEvent<<" Run: "<<fRun<<" SubRun: "<<fSubRun<<std::endl
	     <<"Total number of primary muons: "<<total_muons<<std::endl
	     <<"Of which only "<<tpc_crossers<<" entered the TPC"<<std::endl
	     <<"Out of "<<particleHandle->size()<<" simulated particles"<<std::endl;
}

bool CosmicAna::insideTPC(const TVector3& pos)
{
    // check if this point is inside active volume of the TPC
    if (pos.X() < fXmin || pos.X() > fXmax)
	return false;
    if (pos.Y() < fYmin || pos.Y() > fYmax)
	return false;
    if (pos.Z() < fZmin || pos.Z() > fZmax)
	return false;

    return true;
}

void CosmicAna::GetTPClimits()
{
    std::cout << "----> HERE!!!!: N TPCs: " << fGeometryService->NTPC() << std::endl;
    std::cout << fGeometryService->DetectorName() << std::endl;

    if (fGeometryService->DetectorName() == "protodune") {
	auto box1 = fGeometryService->TPC(1).ActiveBoundingBox(); // beam-side upstream tpc
	auto box2 = fGeometryService->TPC(2).ActiveBoundingBox(); // rack-side upstream tpc
	auto box3 = fGeometryService->TPC(9).ActiveBoundingBox(); // beam-side downstream tpc

	fXmin = box1.MinX(); fXmax = box2.MaxX();
	fYmin = box1.MinY(); fYmax = box2.MaxY();
	fZmin = box1.MinZ(); fZmax = box3.MaxZ();
    } else { // assume 10kt FD
	auto box1 = fGeometryService->TPC(1).ActiveBoundingBox(); // beam-right bottom upstream tpc
	auto box2 = fGeometryService->TPC(10).ActiveBoundingBox(); // mean-left top upstream tpc
	auto box3 = fGeometryService->TPC(289).ActiveBoundingBox(); // beam-right bottom downstream tpc

	fXmin = box1.MinX(); fXmax = box2.MaxX();
	fYmin = box1.MinY(); fYmax = box2.MaxY();
	fZmin = box1.MinZ(); fZmax = box3.MaxZ();
	// fXmin = -300; fXmax = 300;
	// fYmin = 0; fYmax = 600;
	// fZmin = 0; fZmax = 690;
    }

    std::cout << "min X: " << fXmin << "max X: " << fXmax << std::endl
	      << "min Y: " << fYmin << "max Y: " << fYmax << std::endl
	      << "min Z: " << fZmin << "max Z: " << fZmax << std::endl;


}

DEFINE_ART_MODULE(CosmicAna)
