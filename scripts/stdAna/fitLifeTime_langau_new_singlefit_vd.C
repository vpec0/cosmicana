
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


void
fitLifeTime_langau_new_singlefit_vd(const char* fname = "",
				    const int batchno = 0,
				    const int part = 0,
				    const int plane = 0,
				    const int segment = 0,
				    const int slice = 10,
				    const int rebinx = 2,
				    const int rebindqdx = 8,
				    const char* MPV = ""
				    )
{
    Fitter fitter_vd;

    fitter_vd.boundaries = (const double[]){ -326, 326 };
    fitter_vd.nboundaries = 2;

    const double drift_distance = fitter_vd.boundaries[1] - fitter_vd.boundaries[0];


    fitter_vd.gDoMPV = 0;
    TString mpv_opt(MPV);
    mpv_opt.ToUpper();
    if (mpv_opt == "MPV") // strings identical
	fitter_vd.gDoMPV = 1;

    // input file
    cout<<"Opening file "<<fname<<" for reading."<<endl;
    auto f = TFile::Open(fname, "read");


    auto c = new TCanvas("c", "");

    // Get histogram for single slice
    // TString dirname = Form("%d/part%d/plane%d/segment%d", batchno, part, plane, segment);
    // auto dir = f->GetDirectory(dirname);
    // TString histname = Form("Dqdx_vs_x_plane%d_%d", plane, slice);
    // TH1* h = (TH1*)dir->Get(histname);
    TString dirname = "dqdx";
    auto dir = f->GetDirectory(dirname);
    TString histname = Form("Dqdx_vs_x_plane%d", plane);
    auto h2 = (TH2*)dir->Get(histname);

    h2->Rebin2D(rebinx, 1);
    auto h = h2->ProjectionY("_py", slice+1, slice+1);

    h->Draw();
    c->SaveAs("langau_single_before_fit.pdf");

    // Fit slice with Langau

    double mpv = 0.;
    auto fitresult = fitter_vd.FitLangau(h, mpv);

    c->SaveAs("langau_single_fit.pdf");
}
