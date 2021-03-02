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

const double WireAngle[3] = {-35.7, 35.7, 0.};
const double PlaneReverse[4] = {1., -1., 1., -1.};
const double PlaneXboundary[5] = {-725., -363., 0., 363., 725.};

const double toDeg = TMath::RadToDeg();
const double toRad = TMath::DegToRad();

const TVector3 YDirection[3] = { TVector3(0, TMath::Cos(WireAngle[0] * toRad), TMath::Sin(WireAngle[0] * toRad)),
				 TVector3(0, TMath::Cos(WireAngle[1] * toRad), TMath::Sin(WireAngle[1] * toRad)),
				 TVector3(0, 1., 0.) };

void plot_thetayxz(const char* outpref, const char* fname, size_t batch, size_t nruns, size_t startrun = 0,
		    const char* version = "", const char* source = "")
{
    FOR(i, 3) {
	cout<<"Plane"<<i<<": ";
	YDirection[i].Print();
    }

    // Histograms
    vector<TH1*> hists;
    TH2* hTrue[3];
    FOR(i, 3) {
	hTrue[i] = new TH2F(Form("hTrue_%d", i), Form("True muon trajectory plane %d coords"
						      ";#theta_{XZ} [#circ];#theta_{Y} [#circ]", i),
			    360, -180, 180,
			    180, 0., 180.);
	hists.push_back(hTrue[i]);
    }

    TH2* hReco[3];
    FOR(i, 3) {
	hReco[i] = new TH2F(Form("hReco_%d", i), Form("Reco muon track plane %d coords"
						      ";#theta_{XZ} [#circ];#theta_{Y} [#circ]", i),
			    360, -180, 180,
			    180, 0., 180.);
	hists.push_back(hReco[i]);
    }

    TH2* hRecoDown[3];
    FOR(i, 3) {
	hRecoDown[i] = new TH2F(Form("hRecoDown_%d", i), Form("#splitline{Reco muon track plane %d coords}"
							      "{corrected downward}"
							      ";#theta_{XZ} [#circ];#theta_{Y} [#circ]", i),
			    360, -180, 180,
			    180, 0., 180.);
	hists.push_back(hRecoDown[i]);
    }

    TH2* hRecoHits[3];
    FOR(i, 3) {
	hRecoHits[i] = new TH2F(Form("hRecoHits_%d", i), Form("#splitline{Reco muon track hits plane %d coords}"
							      "{corrected downward}"
							      ";#theta_{XZ} [#circ];#theta_{Y} [#circ]", i),
				360, -180, 180,
				180, 0., 180.);
	hists.push_back(hRecoHits[i]);
    }


    TH1* hRecoTrue_xz[3];
    FOR(i, 3) {
	hRecoTrue_xz[i] = new TH1F(Form("hRecoTrue_xz_%d", i),
				   Form("#splitline{Reconstructed track vs True - #theta_{xz}}"
					"{Plane %d coords (corrected downward)}"
					";#Delta_{rec-true}(#theta_{XZ}) [#circ];", i),
				   1440, -180, 180);
	hists.push_back(hRecoTrue_xz[i]);
    }


    TH1* hRecoTrue_y[3];
    FOR(i, 3) {
	hRecoTrue_y[i] = new TH1F(Form("hRecoTrue_y_%d", i), Form("#splitline{Reconstructed track vs True - #theta_{y}}"
								    "{Plane %d coords (corrected downward)}"
								    ";#Delta_{rec-true}(#theta_{Y}) [#circ];", i),
				   1440, -180, 180);
	hists.push_back(hRecoTrue_y[i]);
    }

    auto hRecoTrue = new TH1F("hRecoTrue", "#splitline{Reconstructed track vs True}{(corrected downward)}"
			      ";#theta_{rec-true};",
			      720, 0, 180);
    hists.push_back(hRecoTrue);

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
	"StartPx_tpcAV",
	"StartPy_tpcAV",
	"StartPz_tpcAV",
	"ntracks_pandoraTrack",
	"trkthetaxz_pandoraTrack",
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
	    TVector3 dir(evt->StartPx_tpcAV[i], evt->StartPy_tpcAV[i], evt->StartPz_tpcAV[i]);
	    dir.SetMag(1.);
	    FOR(iplane, 3) {
		TVector3 tmpdir = dir;
		tmpdir.RotateX(-WireAngle[iplane]*toRad);
		hTrue[iplane]->Fill(TMath::ATan2(tmpdir.X(), tmpdir.Z()) * toDeg,
				    TMath::ACos(tmpdir.Y()) * toDeg );
		// cout<<"Plane "<<iplane<<": "
		//     <<dir.Dot(YDirection[iplane]) <<", "
		//     <<TMath::ACos(dir.Dot(YDirection[iplane])) <<", "
		//     <<TMath::ACos(dir.Dot(YDirection[iplane])) * toDeg <<", ";
		//     dir.Print();
	    }
	    break;
	}


	// Process reco tracks
	for (int i = 0; i < evt->ntracks_pandoraTrack; ++i) {
	    if (i > MAX_TRACKS) {
		cout<<"Maximum number of reco tracks exceeded."<<endl;
		break;
	    }
	    if ( evt->trklen_pandoraTrack[i] < 200. ) continue;

	    TVector3 dir(evt->trkstartdcosx_pandoraTrack[i],
			 evt->trkstartdcosy_pandoraTrack[i],
			 evt->trkstartdcosz_pandoraTrack[i]);

	    int geantid = TMath::Abs(evt->trkidtruth_pandoraTrack[i][0]);
	    int pdg = evt->trkpdgtruth_pandoraTrack[i][0];

	    // find corresponding true trajectory
	    size_t idx = 0;
	    for (; idx < evt->geant_list_size; ++idx)
		if ( evt->TrackId[idx] == geantid ) break;
	    TVector3 true_dir(evt->StartPx_tpcAV[idx], evt->StartPy_tpcAV[idx], evt->StartPz_tpcAV[idx]);
	    true_dir.SetMag(1.);

	    // loop over all 3 planes
	    FOR(iplane, 3) {
		TVector3 tmpdir = dir;
		tmpdir.RotateX(-WireAngle[iplane]*toRad);
		TVector3 tmpdir_true = true_dir;
		tmpdir_true.RotateX(-WireAngle[iplane]*toRad);

		double xz = TMath::ATan2(tmpdir.X(), tmpdir.Z())*toDeg;
		double y = TMath::ACos(tmpdir.Y()) * toDeg;
		hReco[iplane]->Fill(xz, y);

		double true_xz = TMath::ATan2(tmpdir_true.X(), tmpdir_true.Z()) * toDeg;
		double true_y = TMath::ACos(tmpdir_true.Y()) * toDeg;

		// correct upward going muon to downward going muon
		if (dir.Y() > 0.) {
		    y = 180. - y;
		    xz -= (xz > 0.) ? 180. : -180.;
		}

		hRecoDown[iplane]->Fill(xz, y);
		// fill hit counts by plane
		if (evt->ntrkhits_pandoraTrack[i][iplane] > 0)
		    hRecoHits[iplane]->Fill(xz, y, evt->ntrkhits_pandoraTrack[i][iplane]);

		// angular residuals
		// cout << "TrackID: " << geantid <<", pdg: " << pdg << ", idx: " << idx
		// 	 << ", true xz: " << true_xz
		// 	 << ", true y: " << true_y
		// 	 << ", reco xz: " << xz
		// 	 << ", reco y: " << y << endl;

		hRecoTrue_xz[iplane]->Fill(xz - true_xz);
		hRecoTrue_y[iplane]->Fill(y - true_y);

	    }

	    // direction difference
	    if (dir.Y() > 0.) dir = -dir;

	    double cos = dir.Dot(true_dir);
	    hRecoTrue->Fill(TMath::ACos(cos)*toDeg);
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
    // hRecoTrueDownward_y->Draw();
    // c->SaveAs(outfname + "theta_reco_vs_true_y_downward.pdf");

    // hRecoTrueDownward->Draw();
    // c->SaveAs(outfname + "theta_reco_vs_true_downward.pdf");

    // save hists into a root file
    auto outf = TFile::Open(outfname + "thetayxz_hists.root", "recreate");

    for (auto h : hists)
	h->Write();

    outf->Close();
}
