#include "FileHandler.h"
#include "HistHandler.h"

#include "anatree.h"

#define FOR(i, size) for (int i = 0; i < size; ++i)

enum {
    kTrue = 0,
    kReco,
    kRecoDown,
    kHits
};

const double WireAngle[3] = {35.7, -35.7, 0.};
const double PlaneReverse[4] = {1., -1., 1., -1.};
const double PlaneXboundary[5] = {-725., -363., 0., 363., 725.};

const double toDeg = TMath::RadToDeg();
const double toRad = TMath::DegToRad();

const TVector3 YDirection[3] = { TVector3(0, TMath::Cos(WireAngle[0] * toRad), TMath::Sin(WireAngle[0] * toRad)),
				 TVector3(0, TMath::Cos(WireAngle[1] * toRad), TMath::Sin(WireAngle[1] * toRad)),
				 TVector3(0, 1., 0.) };


int whichQuadrant(double x);


void create_meandqdx_vs_thetaxyz(const char* outpref, const char* fname, size_t batch, size_t nruns, size_t startrun = 0,
				 const char* version = "", const char* source = "")
{
    FOR(i, 3) {
	cout<<"Plane"<<i<<": ";
	YDirection[i].Print();
    }

    // Histograms
    vector<TH1*> hists;
    TH3* hDqdx[3];
    TH3* hNhits[3];
    FOR(i, 3) {
	hDqdx[i] = new TH3D(Form("hDqdx_%d", i),
			    Form("#splitline{Reco muon track hits plane %d coords}"
				 "{Tracks corrected downward}"
				 ";X [cm];#theta_{XZ} [#circ];#theta_{Y} [#circ];<dQ/dx> [ADC-tick/cm]", i),
			    40, PlaneXboundary[0], PlaneXboundary[4],
			    36, -180, 180,
			    20, 80., 180.);
	hists.push_back(hDqdx[i]);
	hNhits[i] = new TH3D(Form("hNhits_%d", i),
			    Form("#splitline{Reco muon track hits plane %d coords}"
				 "{Tracks corrected downward}"
				 ";X [cm];#theta_{XZ} [#circ];#theta_{Y} [#circ];Number of hits", i),
			    40, PlaneXboundary[0], PlaneXboundary[4],
			    36, -180, 180,
			    20, 80., 180.);
	hists.push_back(hNhits[i]);
    }


    //***** Input tree *****
    auto tree = new TChain("analysistree/anatree");
    size_t size = FileHandler::attachFiles(tree, fname, batch, nruns, startrun, version, source);
    anatree* evt = new anatree(tree);

    if (size == 0)
	return 0;

    vector<TString> allowed = {
	// "geant_list_size",
	// "TrackId",
	// "Mother",
	// "inTPCActive",
	// "pathlen",
	// "theta_xz", // likely direction at the start
	// "StartPx_tpcAV",
	// "StartPy_tpcAV",
	// "StartPz_tpcAV",
	"ntracks_pandoraTrack",
	// "trkthetaxz_pandoraTrack",
	"trkstartdcosx_pandoraTrack",
	"trkstartdcosy_pandoraTrack",
	"trkstartdcosz_pandoraTrack",
	"trklen_pandoraTrack",
	// "trkidtruth_pandoraTrack",
	// "trkpdgtruth_pandoraTrack",
	"ntrkhits_pandoraTrack",
	"trkdqdx_pandoraTrack",
	"trkxyz_pandoraTrack"
    };

    tree->SetBranchStatus("*", 0);
    AnaTree::AllowBranches(tree, allowed);
    tree->SetMakeClass(1);

    cout<<"Starting a loop over the tree"<<endl;
    int entries_processed = 0;
    int fiftieth = size / 50;
    cout<<"Will loop over "<<size<<" entries."<<endl;
    //size = 50000;
    cout<<"|                                                  |\r|";
    size_t ientry = 0;
    while ( tree->GetEntry(ientry++) ) {
	// print progress
	if ( (ientry)%fiftieth == 0) {
	    cout<<"-";
	    cout.flush();
	}

	// Process reco tracks
	for (int itrack = 0; itrack < evt->ntracks_pandoraTrack; ++itrack) {
	    if (itrack > MAX_TRACKS) {
		cout<<"Maximum number of reco tracks exceeded."<<endl;
		break;
	    }
	    // cut on reco track length
	    if ( evt->trklen_pandoraTrack[itrack] < 200. ) continue;

	    // track direction, to be used when calculating zenith and azimuth in the wire-plane coordinate frame
	    TVector3 dir(evt->trkstartdcosx_pandoraTrack[itrack],
			 evt->trkstartdcosy_pandoraTrack[itrack],
			 evt->trkstartdcosz_pandoraTrack[itrack]);
	    // direction correction
	    if (dir.Y() > 0.) dir = -dir;


	    // track direction in 3 different views
	    double xz[3] = {}; // theta_xz
	    double y[3] = {}; // theta_y
	    FOR(iview, 3) {
		TVector3 tmpdir = dir;
		if (iview != 2)
		    tmpdir.RotateX(-WireAngle[iview]*toRad); // direction rotated to U/V view
		xz[iview] = TMath::ATan2(tmpdir.X(), tmpdir.Z())*toDeg;
		y[iview] = TMath::ACos(tmpdir.Y()) * toDeg;
	    }

	    // loop over all 3 planes
	    FOR(iplane, 3) {
		if (evt->ntrkhits_pandoraTrack[itrack][iplane] <= 0) continue;
		// loop over hits in this plane
		int nhits = evt->ntrkhits_pandoraTrack[itrack][iplane];
		if (nhits > MAX_TRACK_HITS)
		    nhits = MAX_TRACK_HITS;
		int iview = 2;
		FOR(ihit, nhits) {
		    float x = evt->trkxyz_pandoraTrack[itrack][iplane][ihit][0];
		    float dqdx = evt->trkdqdx_pandoraTrack[itrack][iplane][ihit];
		    int quadrant = whichQuadrant(x);
		    if (quadrant < 0 || quadrant > 3) continue;
		    // assign view for this plane based on the quadrant
		    if (iplane != 2)
			iview = (quadrant + iplane)%2;
		    else
			iview = 2;
		    hNhits[iplane]->Fill(x, xz[iview], y[iview]);
		    hDqdx[iplane]->Fill(x, xz[iview], y[iview], dqdx);
		}

	    }
	}
    }// tree entry loop
    cout<<"|"<<endl;


    // calculate mean dQ/dx in each bin
    FOR(iplane, 3)
	hDqdx[iplane]->Divide(hNhits[iplane]);
    // int nx = hDqdx[0]->GetNbinsX();
    // int ny = hDqdx[0]->GetNbinsY();
    // int nz = hDqdx[0]->GetNbinsZ();

    // FOR(iplane, 3) {
    // 	FOR(ix, nx) {
    // 	    FOR(iy, ny) {
    // 		FOR(iz, nz) {
    // 		    double nhits = hNhits[iplane]->GetBinContent(ix+1, iy+1, iz+1);
    // 		    double sumdqdx = hDqdx[iplane]->GetBinContent(ix+1, iy+1, iz+1);
    // 		    if (nhits)
    // 			hDqdx[iplane]->SetBinContent(ix+1, iy+1, iz+1,
    // 						     sumdqdx/nhits);
    // 		}
    // 	    }
    // 	}
    // }


    TString outfname = outpref;

    // save hists into a root file
    auto outf = TFile::Open(outfname + "meandqdx_vs_thetayxz_hists.root", "recreate");

    for (auto h : hists)
	h->Write();

    outf->Close();
}


int whichQuadrant(double x)
{
    int segment = -1;
    for (auto boundary: PlaneXboundary) {
	if (x > boundary)
	    segment++;
    }

    return segment;
}
