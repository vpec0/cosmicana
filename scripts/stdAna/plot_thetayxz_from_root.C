

enum {
    kHorizontal,
    kVertical
};

void drawBand(TPad* c, TH1* h, double low, double hi, int orientation = kHorizontal, int color = kRed);

void plot_thetayxz_from_root(const char* fname, const char* outpref)
{
    gStyle->SetTitleFont(43, "");
    gStyle->SetTitleSize(20, "");
    gStyle->SetTitleAlign(23);
    gStyle->SetNumberContours(256);

    gROOT->ForceStyle();


    auto f = TFile::Open(fname);

    vector<TH1*> hists;

    auto c = new TCanvas("c","");
    c->SetLogz();

    TIter next(f->GetListOfKeys());
    while (auto key = (TKey*)next()) {
	auto keyclass = gROOT->GetClass(key->GetClassName());
	//cout << keyclass->GetName() << endl;
	// if (keyclass->InheritsFrom("TH1"))
	//     cout << "  Inherits from TH!" << endl;
	if (!keyclass->InheritsFrom("TH1")) continue;
	auto h = (TH1*)key->ReadObj();

	if (strstr( h->GetName(), "RecoHits" ))
	    ((TH2*)h)->SetMaximum(2e5);

	if (keyclass->InheritsFrom("TH2")) {
	    ((TH2*)h)->Draw("colz");
	} else {
	    c->SetLogy();
	    h->Draw();
	}

	c->SaveAs(Form("%s%s.pdf", outpref, key->GetName()));
	c->SetLogy(0);

	// if (keyclass->InheritsFrom("TH2")) {
	//     // vertical tracks
	//     drawBand(c, h, -95., -85., kHorizontal);
	//     drawBand(c, h, 85., 95., kHorizontal);
	//     // tracks perpendicular to APA
	//     drawBand(c, h, -95., -85., kVertical);
	//     drawBand(c, h, 85., 95., kVertical);
	//     // tracks paralel with induciton wires
	//     drawBand(c, h, 54.3 - 5, 54.3 + 5, kHorizontal, kBlue);
	//     drawBand(c, h, 125.7 - 5, 125.7 + 5, kHorizontal, kBlue);
	//     drawBand(c, h, -54.3 - 5, -54.3 + 5, kHorizontal, kBlue);
	//     drawBand(c, h, -125.7 - 5, -125.7 + 5, kHorizontal, kBlue);

	//     c->SaveAs(Form("%s%s_wBands.pdf", outpref, key->GetName()));
	// }

	// low res versions of 2D hists
	if (keyclass->InheritsFrom("TH2")) {
	    ((TH2*)h)->Rebin2D(5,5);

	    ((TH2*)h)->Draw("colz");
	    c->SaveAs(Form("%s%s_lowres.pdf", outpref, key->GetName()));
	    // vertical tracks
	    // drawBand(c, h, -95., -85., kHorizontal);
	    // drawBand(c, h, 85., 95., kHorizontal);
	    // // tracks perpendicular to APA
	    // drawBand(c, h, -95., -85., kVertical);
	    // drawBand(c, h, 85., 95., kVertical);
	    // // tracks paralel with induciton wires
	    // drawBand(c, h, 54.3 - 5, 54.3 + 5, kHorizontal, kBlue);
	    // drawBand(c, h, 125.7 - 5, 125.7 + 5, kHorizontal, kBlue);
	    // drawBand(c, h, -54.3 - 5, -54.3 + 5, kHorizontal, kBlue);
	    // drawBand(c, h, -125.7 - 5, -125.7 + 5, kHorizontal, kBlue);

	    // c->SaveAs(Form("%s%s_lowres_wBands.pdf", outpref, key->GetName()));
	}

	if (!strstr( h->GetName(), "RecoHits" )) continue;

	// // create projections of the RecoHits histograms
	// auto px = ((TH2*)h)->ProjectionX(Form("%s_pXZ", h->GetName()));
	// auto py = ((TH2*)h)->ProjectionY(Form("%s_pYZ", h->GetName()));

	// px->Draw("hist min0");
	// c->SaveAs(Form("%s%s_pXZ.pdf", outpref, key->GetName()));
	// py->Draw("hist min0");
	// c->SaveAs(Form("%s%s_pYZ.pdf", outpref, key->GetName()));
    }

}


void drawBand(TPad* c, TH1* h, double low, double hi, int orientation, int color)
{
    double x1 = (orientation == kHorizontal) ? h->GetXaxis()->GetXmin() : low;
    double x2 = (orientation == kHorizontal) ? h->GetXaxis()->GetXmax() : low;
    double x3 = (orientation == kHorizontal) ? h->GetXaxis()->GetXmin() : hi;
    double x4 = (orientation == kHorizontal) ? h->GetXaxis()->GetXmax() : hi;
    double y1 = (orientation != kHorizontal) ? h->GetYaxis()->GetXmin() : low;
    double y2 = (orientation != kHorizontal) ? h->GetYaxis()->GetXmax() : low;
    double y3 = (orientation != kHorizontal) ? h->GetYaxis()->GetXmin() : hi;
    double y4 = (orientation != kHorizontal) ? h->GetYaxis()->GetXmax() : hi;

    auto l1 = new TLine(x1, y1, x2, y2);
    l1->SetLineColor(color);
    l1->Draw();
    auto l2 = new TLine(x3, y3, x4, y4);
    l2->SetLineColor(color);
    l2->Draw();

}
