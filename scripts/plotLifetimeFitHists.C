
void plotLifetimeFitHists(const char* fname, const char* outpref, float nominal_lifetime = 3.) {
    gStyle->SetStatW(0.3);
    gStyle->SetStatH(0.4);
    //gROOT->ForceStyle();

    auto f = TFile::Open(fname, "read");

    auto c = new TCanvas("c","", 2);

    vector<TH1*> hists;

    float rangelo = 9999.;
    float rangehi = -9999.;

    for (int iplane = 0; iplane < 3; iplane++) {
	for (int isegment = 0; isegment < 4; isegment++) {
	    auto h = (TH1*)f->Get(Form("h%d_%d", iplane, isegment));
	    hists.push_back(h);
	    auto maxbin = h->GetMaximumBin();
	    float max = h->GetBinCenter(maxbin);
	    auto stddev = h->GetStdDev();

	    if (rangelo > max - 4*stddev)
		rangelo = max - 4*stddev;
	    if (rangehi < max + 4*stddev)
		rangehi = max + 4*stddev;
	}
    }

    for (auto h: hists) {
	h->SetAxisRange(rangelo, rangehi);
	h->Draw();

	c->SaveAs(Form("%slifetime_fit_hist_%s.pdf", outpref, h->GetName()));
    }
}
