

void
plotDqDxHist(const char* fname, const char* outpref)
{
    TString hname = "dqdx/Dqdx_vs_x";

    auto c = new TCanvas("c","");

    auto f = TFile::Open(fname, "read");
    for (int i = 0; i < 3; i++) {
	auto h = (TH2*)f->Get(hname + Form("_plane%d", i));
	h->Draw("colz");

	auto xaxis = h->GetXaxis();
	auto yaxis = h->GetYaxis();

	yaxis->SetRangeUser(200.,400.);

	c->SaveAs(Form("%sdqdx_vs_x_plane%d.png", outpref,  i));
    }

    c->Close();
}
