TH2* fillHist(TH2* h2, TH1* h1);
void setAxisLabels(TH1* h, int n, const char** labels);

void create_theta_bins(const char* fname, const char* outpref)
{
    gStyle->SetOptStat(0);
    gStyle->SetPadTopMargin(0.05);
    gStyle->SetPadBottomMargin(0.15);

    auto f = TFile::Open(fname);



    TH1F* h1D[3];
    for (int i = 0; i < 3; ++i)
	h1D[i] = new TH1F(Form("h1D_%d", i), Form("Plane %d", i), 5, 0., 5.);


    auto c = new TCanvas("c","");
    c->SetLogy(0);

    TH2* h2mask = 0;
    for (int iplane = 0; iplane < 3; ++iplane) {
	TString histname = Form("hRecoHitsDownward_plane%d", iplane);

	auto h = (TH2*)f->Get(histname);

	h2mask = fillHist(h, h1D[iplane]);
    }


    const char* axis_labels[5] = {"Vertical", "C-wire", "Towards APA", "C-wire wider", "Rest"};


    for (int iplane = 0; iplane < 3; ++iplane) {
	setAxisLabels(h1D[iplane], 5, axis_labels);
	h1D[iplane]->SetMarkerSize(1.8);
	h1D[iplane]->Draw("hist min0 text00");
	c->SaveAs(Form("%sangular_1D_plane%d.pdf", outpref, iplane));
    }

    gStyle->SetNumberContours(5);
    c->SetTopMargin(c->GetLeftMargin());
    c->SetBottomMargin(c->GetLeftMargin());
    h2mask->SetTitle("");
    h2mask->SetMaximum(5.5);
    h2mask->Draw("colz");
    c->SetLogy(0);
    c->Update();
    h2mask->GetZaxis()->SetNdivisions(5);
    h2mask->GetZaxis()->SetTickLength(0.);
    c->SaveAs(Form("%sangular_bin_mask.pdf", outpref));
}


TH2* fillHist(TH2* h2, TH1* h1)
{
    auto xax = h2->GetXaxis();
    auto yax = h2->GetYaxis();

    int nx = xax->GetNbins();
    int ny = yax->GetNbins();

    // create a masked histogram
    TH2* hout = (TH2*)h2->Clone("h2mask");
    hout->Reset();

    for (int ix = 0; ix < nx; ++ix) {
	double xz = xax->GetBinCenter(ix+1);
	for (int iy = 0; iy < ny; ++iy) {
	    double yz = yax->GetBinCenter(iy+1);
	    double content = h2->GetBinContent(ix+1, iy+1);
	    auto isxz = abs(abs(xz) - 90) < 20;
	    auto isyz = abs(abs(yz) - 90) < 10;
	    auto isyz2 = abs(abs(yz) - 90) < 20;
	    float bin = 4.5; // the rest
	    if (isxz && isyz)
		bin = 0.5;
	    else if (isyz)
		bin = 1.5;
	    else if (isxz)
		bin = 2.5;
	    else if (isyz2)
		bin = 3.5;

	    h1->Fill(bin, content);
	    hout->SetBinContent(ix+1, iy+1, bin);
	}
    }

    return hout;
}


void setAxisLabels(TH1* h, int n, const char** labels)
{
    auto ax = h->GetXaxis();
    ax->SetLabelSize(0.08);
    for (int i = 1; i <= n; ++i) {
	ax->SetBinLabel(i, labels[i-1]);
    }
}
