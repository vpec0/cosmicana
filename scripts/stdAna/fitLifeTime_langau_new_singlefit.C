
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

void
fitLifeTime_langau_new_singlefit(const char* fname = "",
			     const int batchno = 0,
			     const int part = 0,
			     const int plane = 0,
			     const int segment = 0,
			     const int slice = 10,
			     const char* MPV = ""
			     )
{
    Fitter::gDoMPV = 0;
    if (!strcmp("MPV", MPV)) // strings identical
	Fitter::gDoMPV = 1;

    // input file
    cout<<"Opening file "<<fname<<" for reading."<<endl;
    auto f = TFile::Open(fname, "read");

    // Get histogram for single slice
    TString dirname = Form("%d/part%d/plane%d/segment%d", batchno, part, plane, segment);
    auto dir = f->GetDirectory(dirname);
    TString histname = Form("Dqdx_vs_x_plane%d_%d", plane, slice);
    TH1* h = (TH1*)dir->Get(histname);

    // Fit slice with Langau
    auto c = new TCanvas("c", "");

    double mpv = 0.;
    auto fitresult = Fitter::FitLangau(h, mpv);

    c->SaveAs("langau_single_fit.pdf");
}
