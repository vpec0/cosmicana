#include "FileHandler.h"
#include "HistHandler.h"

#include "anatree.h"


void plot_thetaxzyz(const char* fname, size_t batch, size_t nruns, size_t startrun = 0,
		    const char* version = "", const char* source = "")
{
    // Histograms

    auto hTrue = new TH2F("hTrue", "True muon trajectory;#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
			  72, -180, 180,
			  72, -180, 180);

    auto hReco = new TH2F("hReco", "Reconstructed track;#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
			  72, -180, 180,
			  72, -180, 180);

    auto hRecoDownward = new TH2F("hRecoDownward", "#splitline{Reconstructed track}{(corrected downward)}"
				  ";#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
				  72, -180, 180,
				  72, -180, 180);


    //***** Input tree *****
    auto tree = new TChain("analysistree/anatree");
    size_t size = FileHandler::attachFiles(tree, fname, batch, nruns, startrun, version, source);
    anatree* evt = new anatree(tree);

    if (size == 0)
	return 0;

    vector<TString> allowed = {
	"geant_list_size",
	"Mother",
	"inTPCActive",
	"pathlen",
	"theta_xz",
	"theta_yz",
	"ntracks_pandoraTrack",
	"trkthetaxz_pandoraTrack",
	"trkthetayz_pandoraTrack",
	"trklen_pandoraTrack"
    };

    tree->SetBranchStatus("*", 0);
    AnaTree::AllowBranches(tree, allowed);
    tree->SetMakeClass(1);


    const double toDeg = 180./TMath::Pi();

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

	for (int i = 0; i < evt->geant_list_size; ++i) {
	    if ( evt->Mother[i] != 0 ) continue; // looking for primary

	    if ( !evt->inTPCActive[i] ) break; // must hit TPC
	    if ( evt->pathlen[i] < 200. ) break; // true path length in TPC > 2 m

	    hTrue->Fill(evt->theta_xz[i]*toDeg, evt->theta_yz[i]*toDeg);
	    break;
	}


	for (int i = 0; i < evt->ntracks_pandoraTrack; ++i) {
	    if ( evt->trklen_pandoraTrack[i] < 200. ) continue;
	    double xz = evt->trkthetaxz_pandoraTrack[i]*toDeg;
	    double yz = evt->trkthetayz_pandoraTrack[i]*toDeg;

	    hReco->Fill(xz, yz);

	    if (yz > 0.) {
		yz -= 180.;
		xz -= (xz > 0.) ? 180. : -180.;
	    }
	    hRecoDownward->Fill(xz, yz);
	    break;
	}



    }// tree entry loop
    cout<<"|"<<endl;



    auto c = new TCanvas("c","", 2);
    hTrue->Draw("colz");
    c->SaveAs("test_theta_true.pdf");

    hReco->Draw("colz");
    c->SaveAs("test_theta_reco.pdf");
    hRecoDownward->Draw("colz");
    c->SaveAs("test_theta_reco_downward.pdf");
}
