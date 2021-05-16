#define FOR(i, size) for (int i = 0; i < size; ++i)

void plot_meandqdx_vs_thetaxyz(const char* fname, const char* outpref)
{
    gROOT->ForceStyle(1);
    gStyle->SetOptStat(0);
    gStyle->SetNdivisions(505, "yz");
    //gStyle->SetPadRightMargin(0.2);
    gStyle->SetTitleOffset(0.6, "yz");
    gStyle->SetTitleOffset(2, "x");

    auto f = TFile::Open(fname, "read");

    auto c = new TCanvas("c","", 800, 800);
    c->Divide(1,2);

    FOR(iplane, 3) {
	auto hdqdx = (TH3*)f->Get(Form("hDqdx_%d", iplane));
	auto hnhits = (TH3*)f->Get(Form("hNhits_%d", iplane));

	// for each bin in x plot 2D hist of mean dQ/dx in theta_y vs theta_xz
	int nx = hdqdx->GetNbinsX();
	FOR(ix, nx) {
	    hdqdx->GetXaxis()->SetRange(ix+1, ix+1);
	    hnhits->GetXaxis()->SetRange(ix+1, ix+1);
	    auto h2d = hdqdx->Project3D("zy");
	    auto h2dnhits = hnhits->Project3D("zy");

	    double xlow = hdqdx->GetXaxis()->GetBinLowEdge(ix+1);
	    double xup = hdqdx->GetXaxis()->GetBinUpEdge(ix+1);

	    h2d->SetName(Form("hdqdx_%d_%d", iplane, ix));
	    h2d->SetTitle(Form("Plane %d, %.1f < x < %.1f"
			       ";#theta_{XZ} [#circ];#theta_{Y} [#circ];<dQ/dx> [ADC-tick/cm]",
			       iplane, xlow, xup));

	    h2dnhits->SetName(Form("hnhits_%d_%d", iplane, ix));
	    h2dnhits->SetTitle(";#theta_{XZ} [#circ];#theta_{Y} [#circ];Number of hits");

	    h2d->SetMaximum(600);
	    c->cd(1);
	    h2d->Draw("colz");

	    c->cd(2);
	    h2dnhits->Draw("colz");


	    c->SaveAs(Form("%smeandqdx_vs_thetayxz_plane%d_xbin%d.pdf",
			   outpref, iplane, ix));
	}
    }

    f->Close();
}
