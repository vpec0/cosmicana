
void plotLifetimeFitHists(const char* fname, const char* outpref) {
    gStyle->SetStatW(0.3);
    gStyle->SetStatH(0.4);
    //gROOT->ForceStyle();

    auto f = TFile::Open(fname, "read");

    auto c = new TCanvas("c","", 2);

    for (int iplane = 0; iplane < 3; iplane++) {
	for (int isegment = 0; isegment < 4; isegment++) {
	    auto h = (TH1*)f->Get(Form("h%d_%d", iplane, isegment));

	    h->SetAxisRange(2.7, 3.);
	    h->Draw();

	    c->SaveAs(Form("%slifetime_fit_hist_%s.pdf", outpref, h->GetName()));
	}
    }


}
