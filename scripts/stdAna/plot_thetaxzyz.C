#include "FileHandler.h"
#include "HistHandler.h"

#include "anatree.h"


void plot_thetaxzyz(const char* outpref, const char* fname, size_t batch, size_t nruns, size_t startrun = 0,
		    const char* version = "", const char* source = "")
{
    // Histograms
    vector<TH1*> hists;
    auto hTrue = new TH2F("hTrue", "True muon trajectory;#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
			  360, -180, 180,
			  360, -180, 180);
    hists.push_back(hTrue);

    auto hReco = new TH2F("hReco", "Reconstructed track;#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
			  360, -180, 180,
			  360, -180, 180);
    hists.push_back(hReco);

    auto hRecoDownward = new TH2F("hRecoDownward", "#splitline{Reconstructed track}{(corrected downward)}"
				  ";#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
				  360, -180, 180,
				  360, -180, 180);
    hists.push_back(hRecoDownward);

    TH2F* hRecoHitsDownward[3] = {new TH2F("hRecoHitsDownward_plane0", "#splitline{Reconstructed track hits Plane 0}{(corrected downward)}"
					   ";#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
					   360, -180, 180,
					   360, -180, 180),
				  new TH2F("hRecoHitsDownward_plane1", "#splitline{Reconstructed track hits Plane 1}{(corrected downward)}"
					   ";#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
					   360, -180, 180,
					   360, -180, 180),
				  new TH2F("hRecoHitsDownward_plane2", "#splitline{Reconstructed track hits Plane 2}{(corrected downward)}"
					   ";#theta_{XZ} [#circ];#theta_{YZ} [#circ]",
					   360, -180, 180,
					   360, -180, 180) };
    for (int iplane = 0; iplane < 3; ++iplane)
	hists.push_back(hRecoHitsDownward[iplane]);

    auto hRecoTrueDownward_xz = new TH1F("hRecoTrueDownward_xz", "#splitline{Reconstructed track vs True - #theta_{xz}}{(corrected downward)}"
				  ";#Delta_{rec-true}(#theta_{XZ}) [#circ];",
				  360, -180, 180);
    hists.push_back(hRecoTrueDownward_xz);
    auto hRecoTrueDownward_yz = new TH1F("hRecoTrueDownward_yz", "#splitline{Reconstructed track vs True - #theta_{yz}}{(corrected downward)}"
				  ";#Delta_{rec-true}(#theta_{YZ}) [#circ];",
				  360, -180, 180);
    hists.push_back(hRecoTrueDownward_yz);
    auto hRecoTrueDownward = new TH1F("hRecoTrueDownward", "#splitline{Reconstructed track vs True}{(corrected downward)}"
				      ";cos#theta_{rec-true};",
				      200, 0, 1);
    hists.push_back(hRecoTrueDownward);


    //***** Input tree *****
    auto tree = new TChain("analysistree/anatree");
    size_t size = FileHandler::attachFiles(tree, fname, batch, nruns, startrun, version, source);
    anatree* evt = new anatree(tree);

    if (size == 0)
	return 0;

    vector<TString> allowed = {
	"geant_list_size",
	"TrackId",
	"Mother",
	"inTPCActive",
	"pathlen",
	"theta_xz", // likely direction at the start
	"theta_yz",
	"StartPx_tpcAV",
	"StartPy_tpcAV",
	"StartPz_tpcAV",
	"ntracks_pandoraTrack",
	"trkthetaxz_pandoraTrack",
	"trkthetayz_pandoraTrack",
	"trkstartdcosx_pandoraTrack",
	"trkstartdcosy_pandoraTrack",
	"trkstartdcosz_pandoraTrack",
	"trklen_pandoraTrack",
	"trkidtruth_pandoraTrack",
	"trkpdgtruth_pandoraTrack",
	"ntrkhits_pandoraTrack"
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


	// Process reco tracks
	for (int i = 0; i < evt->ntracks_pandoraTrack; ++i) {
	    if (i > MAX_TRACKS) {
		cout<<"Maximum number of reco tracks exceeded."<<endl;
		break;
	    }
	    if ( evt->trklen_pandoraTrack[i] < 200. ) continue;
	    double xz = evt->trkthetaxz_pandoraTrack[i]*toDeg;
	    double yz = evt->trkthetayz_pandoraTrack[i]*toDeg;

	    hReco->Fill(xz, yz);

	    if (yz > 0.) {
		yz -= 180.;
		xz -= (xz > 0.) ? 180. : -180.;
	    }
	    hRecoDownward->Fill(xz, yz);
	    // fill hit counts by plane
	    for (int iplane = 0; iplane < 3; ++iplane) {
		if (evt->ntrkhits_pandoraTrack[i][iplane] > 0)
		    hRecoHitsDownward[iplane]->Fill(xz, yz, evt->ntrkhits_pandoraTrack[i][iplane]);
	    }

	    // angular residuals
	    int geantid = TMath::Abs(evt->trkidtruth_pandoraTrack[i][0]);
	    int pdg = evt->trkpdgtruth_pandoraTrack[i][0];
	    // find true track
	    size_t idx = 0;
	    for (; idx < evt->geant_list_size; ++idx)
		if ( evt->TrackId[idx] == geantid ) break;
	    TVector3 true_dir(evt->StartPx_tpcAV[idx], evt->StartPy_tpcAV[idx], evt->StartPz_tpcAV[idx]);
	    true_dir.SetMag(1.);
	    double true_xz = TMath::ATan2(true_dir.X(), true_dir.Z()) * toDeg;
	    double true_yz = TMath::ATan2(true_dir.Y(), true_dir.Z()) * toDeg;

	    // cout << "TrackID: " << geantid <<", pdg: " << pdg << ", idx: " << idx
	    // 	 << ", true xz: " << true_xz
	    // 	 << ", true yz: " << true_yz
	    // 	 << ", reco xz: " << xz
	    // 	 << ", reco yz: " << yz << endl;

	    hRecoTrueDownward_xz->Fill(xz - true_xz);
	    hRecoTrueDownward_yz->Fill(yz - true_yz);

	    // direction difference
	    TVector3 rec_dir(evt->trkstartdcosx_pandoraTrack[i],
			     evt->trkstartdcosy_pandoraTrack[i],
			     evt->trkstartdcosz_pandoraTrack[i]);
	    double cos = rec_dir.Dot(true_dir);
	    hRecoTrueDownward->Fill(cos);
	}



    }// tree entry loop
    cout<<"|"<<endl;



    TString outfname = outpref;

    // auto c = new TCanvas("c","", 2);
    // hTrue->Draw("colz");
    // c->SaveAs(outfname + "theta_true.pdf");

    // hReco->Draw("colz");
    // c->SaveAs(outfname + "theta_reco.pdf");
    // hRecoDownward->Draw("colz");
    // c->SaveAs(outfname + "theta_reco_downward.pdf");

    // hRecoTrueDownward_xz->Draw();
    // c->SaveAs(outfname + "theta_reco_vs_true_xz_downward.pdf");
    // hRecoTrueDownward_yz->Draw();
    // c->SaveAs(outfname + "theta_reco_vs_true_yz_downward.pdf");

    // hRecoTrueDownward->Draw();
    // c->SaveAs(outfname + "theta_reco_vs_true_downward.pdf");

    // save hists into a root file
    auto outf = TFile::Open(outfname + "theta_hists.root", "recreate");

    for (auto h : hists)
	h->Write();

    outf->Close();
}
