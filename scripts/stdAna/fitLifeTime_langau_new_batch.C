
//#include "common.icc"

#define FITTER_CXX
#include "Fitter.hh"

#ifndef FOR
#define FOR(i, size) for (int i = 0; i < size; ++i)
#endif

// TGraphErrors*	sliceAndFit(TH2* h);
// void		Fit(TGraphErrors* gr);
// int		getMpv(TH1* h, double& mpv, double& err);
// TFitResultPtr	FitLangau(TH1* h, double& mpv);
// Double_t	langaufun(Double_t *x, Double_t *par);

const double drift_distance = 362;

// spcify tree entries
#define TREE_LIST				\
    INT(batchNo);				\
    INT(batchPart);				\
    DOUBLE_ARRAY(tau_e);			\
    DOUBLE_ARRAY(tau_e_error);			\
    DOUBLE_ARRAY(slope);			\
    DOUBLE_ARRAY(slope_error);			\
    DOUBLE_ARRAY(qcqa);				\
    DOUBLE_ARRAY(qcqa_error);			\
    DOUBLE_ARRAY(chi2);				\
    DOUBLE_ARRAY(ndf);				\

#define INT(name) int name
// binned in [plane], [segment along x]
#define DOUBLE_ARRAY(name) double name[3][4]
struct TauEevent_t {
    TREE_LIST;
};
#undef INT
#undef DOUBLE_ARRAY

TTree* createNewTree(const char* treename);
void registerTree(TTree* tree, TauEevent_t* evt);


void
fitLifeTime_langau_new_batch(int batchno, int batchpart,
			     const char* outpref = "",
			     const char* base_dir = "plots/v08_34_00/elifetime/2m_track_cut",
			     const char* MPV = "",
			     const char* single_file = ""
			     )
{
    Fitter::gDoMPV = 0;
    if (!strcmp("MPV", MPV)) // strings identical
	Fitter::gDoMPV = 1;


    TString fname = Form("%s/batch_%d/new_batch_%d_part%d_anahists.root",
			 base_dir, batchno, batchno, batchpart);
    if (strcmp("", single_file)) // given non-empty string
	fname = single_file;

    cout<<"Opening file "<<fname<<" for reading."<<endl;
    auto f = TFile::Open(fname, "read");

    // open an output file; will store the profile graphs into a
    // separate directory (batchno/batchpart/); will update tree with
    // fit results
    TString outfname = outpref;
    if (Fitter::gDoMPV)
	outfname += "MPV_";
    outfname += "fits.root";
    cout<<"Will write data to "<<outfname<<endl;
    auto outf = TFile::Open(outfname, "update");
    const char* treename = "elifetime";
    auto tree = (TTree*)outf->Get(treename);

    TauEevent_t evt;

    if (!tree)
	tree = createNewTree(treename);

    registerTree(tree, &evt);

    // create directory structure
    TString dirname = Form("%d", batchno);
    if (!outf->GetDirectory(dirname))
	outf->mkdir(dirname);
    dirname = Form("%d/part%d", batchno, batchpart);
    if (!outf->GetDirectory(dirname))
	outf->mkdir(dirname);
    outf->cd(dirname);
    auto batch_part_dir = (TDirectory*)gDirectory;

    // loop over all planes and get corresponding histograms
    for (int iplane = 0; iplane < 3; ++iplane) {
	TString histname = Form("dqdx/Dqdx_vs_x_plane%d", iplane);
	auto h = (TH2D*) f->Get(histname);

	// rebin input histogram
	h->Rebin2D(5,5);

	// create plane directory
	TString plane_dir = Form("plane%d", iplane);
	if (!batch_part_dir->GetDirectory(plane_dir))
	    batch_part_dir->mkdir(plane_dir);
	batch_part_dir->cd(plane_dir);
	auto gr = Fitter::sliceAndFit(h);
	batch_part_dir->cd();

	// write out the histogram
	h->Write();
	gr->Write("", TObject::kOverwrite);

	// get fit exponential slope, convert to lifetime, store for each fit segment
	for (int ifit = 0; ifit < 4; ++ifit){
	    auto fit = gr->GetFunction(Form("fit%d", ifit));
	    double slope = (ifit%2*2 -1)*fit->GetParameter(1);
	    double slope_err = fit->GetParError(1);
	    double lt = 1./abs(slope) * Fitter::gXtoT;
	    double lt_err = slope_err * lt * lt / Fitter::gXtoT;

	    evt.tau_e[iplane][ifit] = lt;
	    evt.tau_e_error[iplane][ifit] = lt_err;

	    evt.slope[iplane][ifit] = slope/Fitter::gXtoT;
	    evt.slope_error[iplane][ifit] = slope_err/Fitter::gXtoT;


	    double qcqa = TMath::Exp(-slope*drift_distance);
	    double qcqa_err = drift_distance*qcqa * slope_err;

	    evt.qcqa[iplane][ifit] = qcqa;
	    evt.qcqa_error[iplane][ifit] = qcqa_err;

	    evt.chi2[iplane][ifit] = fit->GetChisquare();
	    evt.ndf[iplane][ifit] = fit->GetNDF();
	}
	delete h;
    }
    outf->cd();

    evt.batchNo = batchno;
    evt.batchPart = batchpart;
    tree->Fill();


    tree->Write(0, TObject::kOverwrite);
    outf->Close();
}

TTree* createNewTree(const char* treename)
{
    auto tree = new TTree(treename, "");

#define INT(name) tree->Branch(#name, 0, #name"/I");
#define DOUBLE_ARRAY(name) tree->Branch(#name, 0, #name"[3][4]/D");
    TREE_LIST;
#undef INT
#undef DOUBLE_ARRAY

    return tree;
}


void registerTree(TTree* tree, TauEevent_t* evt)
{
#define INT(name) tree->SetBranchAddress(#name, &evt->name);
#define DOUBLE_ARRAY(name) tree->SetBranchAddress(#name, evt->name);
    TREE_LIST;
#undef INT
#undef DOUBLE_ARRAY
}
