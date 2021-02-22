
#define FOR(i, size) for (int i = 0; i < size; i++)

void plotLandauFitGausSigmaVsX(const char* fname, int batch, int part, int plane, const char* outpref) {
    auto f = TFile::Open(fname, "read");

    auto c = new TCanvas("c","", 2);

    TString dirname = Form("%d/part%d", batch, part);
    auto dir = f->GetDirectory(dirname);
    dir->ls();

    TString histname = Form("Dqdx_vs_x_plane%d_profile", plane);
    cout<<histname<<endl;

    auto dqdx_vs_x = (TGraphErrors*)dir->Get(histname);
    int n = dqdx_vs_x->GetN();

    dir = dir->GetDirectory("plane0/segment0");
    auto list = dir->GetListOfKeys();

    vector<double> x,y,yerr;

    FOR(i, list->GetEntries()) {
	double xx = dqdx_vs_x->GetX()[i];
	auto key = (TKey*)list->At(i);
	auto h = (TH1*)key->ReadObj();

	auto fit = h->GetFunction("langaus");
	double yy = fit->GetParameter(3);
	double yyerr = fit->GetParError(3);

	x.push_back(xx);
	y.push_back(yy);
	yerr.push_back(yyerr);
    }


    auto gr = new TGraphErrors(x.size(), x.data(), y.data(), 0,  yerr.data());
    gr->SetName("gr");
    gr->Draw("AP");

    gr->Fit("pol1");
}
