#define DQDXPROCESSOR_CXX
#include "DqDxProcessor.h"

#ifndef FOR
#define FOR(i, size) for (int i = 0; i < size; ++i)
#endif

class ProcessorThetaYXZ : public DqDxProcessor
{
public:
    ProcessorThetaYXZ(const char* fname = "", const char* outpref = "",
		      size_t startRun = 0, size_t Nruns = 10) :
	DqDxProcessor(fname, outpref, 0, Nruns, startRun, "")
    {
	fAllowed.push_back("trkthetaxz_pandoraTrack");
	fAllowed.push_back("trkthetayz_pandoraTrack");
    }

public:
    int Initialize();
    int Process();
    int Finalize();

private:
    virtual int SelectEvent(anatree* evt);
    virtual int SelectTrack(anatree* evt, int itrack);
    virtual int SelectHit(anatree* evt, int itrack, int iplane, int ihit);


public:
    vector<TH3F*> mHists;


private:
    static const float pihalf;
};

const float ProcessorThetaYXZ::pihalf = TMath::Pi()*0.5;


void process_thetayxz_vs_x_new(const char* fname = "",
			       const char* outpref = "",
			       size_t startRun = 0, size_t Nruns = 10)
{
    auto processor = new ProcessorThetaYXZ(fname, outpref, startRun, Nruns);

    auto status = processor->Initialize();
    if (!status) return;

    processor->Process();

    //processor->mHist->Draw();

    processor->Finalize();

}


int ProcessorThetaYXZ::SelectEvent(anatree* evt)
{
    return 1;
}

int ProcessorThetaYXZ::SelectTrack(anatree* evt, int itrack)
{
    if (evt->trklen_pandoraTrack[itrack] < 200.)
	// track needs to have >= 5 m length
	return false;


    // make theta_xz cut
    // float theta_xz = evt->trkthetaxz_pandoraTrack[itrack];
    // static const float cut = pihalf*0.5;
    // if ( abs(theta_xz - pihalf) < cut ||
    // 	 abs(theta_xz + pihalf) < cut )
    // 	return false;

    //mHist->Fill(theta_xz/pi_half*0.5);

    return true;
}

int ProcessorThetaYXZ::SelectHit(anatree* evt, int itrack, int iplane, int ihit)
{
    return 1;
}

int ProcessorThetaYXZ::Initialize()
{
    // input file ant tree
    TTree* tree = findInputTree();

    // change the default allowed branches
    auto allowed = fAllowed.begin();
    while (allowed != fAllowed.end()) {
	if ( //!allowed->CompareTo("trkdqdx_pandoraTrack") ||
	     !allowed->CompareTo("trkdedx_pandoraTrack")   ) {
	    fAllowed.erase(allowed);
	}
	allowed++;
    }
    tree->SetBranchStatus("*", 0);
    AnaTree::AllowBranches(tree, fAllowed);
    tree->SetMakeClass(1);



    // Output histograms
    FOR(iplane, 3) {
	//if (iplane != 2) continue; // For now, do only collection plane
	mHists.push_back(new TH3F(Form("hit_ctyz_vs_txz_vs_x_plane%d", iplane),
				 Form("#theta_{yz} vs #theta_{xz} vs x, plane %d"
				      ";x [cm]; #theta_{xz}/#pi; cos #theta_{yz}", iplane),
				 160, -800, 800, 72, -1, 1, 100, -1, 1) );
    }
    // duplicate, but with angle theta_yz instead of cosine of it
    FOR(iplane, 3) {
	//if (iplane != 2) continue; // For now, do only collection plane
	mHists.push_back(new TH3F(Form("hit_tyz_vs_txz_vs_x_plane%d", iplane),
				 Form("#theta_{yz} vs #theta_{xz} vs x, plane %d"
				      ";x [cm]; #theta_{xz}/#pi;#theta_{yz}/#pi", iplane),
				 160, -800, 800, 72, -1, 1, 72, -1, 1) );
    }


    //***** Output file *****
    fOutFile = TFile::Open(Form("%sthetayxz_vs_x_anahists.root", fOutPref.Data()), "recreate");

    return fOutFile->IsOpen();
}

int ProcessorThetaYXZ::Finalize()
{
    fOutFile->cd();
    //***** Save hists *****
    for (auto hist: mHists) {
	for (auto h : mHists) {
	    h->Write(0, TObject::kOverwrite);
	}
    }

    fOutFile->Close();
    cout<<"Saved and closed output file "<<fOutFile->GetName()<<endl;


    return 1;
}

int ProcessorThetaYXZ::Process()
{
    //***** Process *****
    size_t size = fTree->GetEntries();
    auto tree = fTree;
    auto evt = fEvent;

    cout<<"Starting a loop over the tree"<<endl;
    int entries_processed = 0;
    int fiftieth = size / 50;
    cout<<"Will loop over "<<size<<" entries."<<endl;

    size_t events_passing_selection = 0;
    size_t tracks_passing_selection = 0;

    //size = 50000;
    cout<<"|                                                  |\r|";
    size_t ientry = 0;
    // cout<<">>> Trying to read tree entry: "<<endl;
    tree->GetEntry(0);
    // cout<<">>> before loop"<<endl;
    while ( tree->GetEntry(ientry++) ) {
	// cout<<">>> inside loop"<<endl;

	// print progress
	if ( (ientry)%fiftieth == 0) {
	    cout<<"-";
	    cout.flush();
	}

	entries_processed++;

	if (!SelectEvent(evt)) continue;

	int ntracks = evt->ntracks_pandoraTrack;

	// loop over stored tracks
	bool passed = false ;
	for (int itrack = 0; itrack < ntracks; ++itrack) {
	    // make a threshold on reco track length
	    if (!SelectTrack(evt, itrack)) continue;

	    bool track_passed = false;

	    Float_t tyz     = evt->trkthetayz_pandoraTrack[itrack]/TMath::Pi();
	    Float_t cosyz   = TMath::Cos(tyz*TMath::Pi() + pihalf);
	    Float_t txz     = evt->trkthetaxz_pandoraTrack[itrack]/TMath::Pi();
	    // loop over all planes
	    // loop over hits in the best plane
	    for (int iplane = 0; iplane < 3; ++iplane) {
		int nhits = evt->ntrkhits_pandoraTrack[itrack][iplane];
		if (nhits > MAX_TRACK_HITS)
		    nhits = MAX_TRACK_HITS;
		Float_t* dqdx_arr = evt-> trkdqdx_pandoraTrack[itrack][iplane];
		// Float_t* dedx_arr = evt->trkdedx_pandoraTrack[itrack][iplane];
		Float_t* xyz_arr = (Float_t*)evt->trkxyz_pandoraTrack[itrack][iplane];
		for (int i = 0; i < nhits; ++i) {
		    if (!SelectHit(evt, itrack, iplane, i)) continue;

		    if (!track_passed) track_passed = true;

		    double dqdx = dqdx_arr[i];
		    if ( dqdx != 0.) {
			double x = xyz_arr[i*3];

			mHists[iplane]->Fill(x, txz, cosyz);
			mHists[iplane+3]->Fill(x, txz, tyz);

			// int tpc = whichTPC(x) + 1;
			// double dx = ( -1 + 2*(tpc%2) )*(x - APA_X_POSITIONS[tpc/2]);

		    }
		}
	    }
	    if (track_passed) {
		++tracks_passing_selection;
		if (!passed) passed = true;

#ifdef DEBUG
		double start_x = evt->trkstartx_pandoraTrack[itrack];
		double end_x = evt->trkendx_pandoraTrack[itrack];
		cout<<""
		    <<"Entry: "<<ientry
		    <<", event: "<<evt->event
		    <<", track: "<<itrack
		    <<", track start x: "<<start_x
		    <<", track end x: "<<end_x
		    <<endl;
#endif // DEBUG
	    }

	}// track loop

	if (passed) {
	    ++events_passing_selection;

#ifdef DEBUG
	    fMVHists["dqdx"][3]->Draw("colz");
	    gPad->Modified();
	    gPad->Update();
	    getchar();
	    fMVHists["dqdx"][3]->Reset();
#endif // DEBUG
	}

    }// tree entry loop
    cout<<"|"<<endl;

    cout<<"Done."<<endl;
    cout<<"Processed "<<entries_processed<<" entries."<<endl;
    cout<<"Events passing selection: "<<events_passing_selection<<endl
	<<"Tracks passing selection: "<<tracks_passing_selection<<endl;

    return 1;
}
