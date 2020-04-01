

void fitLifeTime(const char fname, const char* outpref)
{
    enum {
	Dqdx_vs_x = 0,
	Dqdx_vs_x_plane0,
	Dqdx_vs_x_plane1,
	Dqdx_vs_x_plane2,
	Dqdx_vs_t,
	Dqdx_vs_t_plane0,
	Dqdx_vs_t_plane1,
	Dqdx_vs_t_plane2,
	NHists
    };


    vector<TString> histnames = {
	"Dqdx_vs_x",
	"Dqdx_vs_x_plane0",
	"Dqdx_vs_x_plane1",
	"Dqdx_vs_x_plane2",
	"Dqdx_vs_t",
	"Dqdx_vs_t_plane0",
	"Dqdx_vs_t_plane1",
	"Dqdx_vs_t_plane2",
    };

    auto f = TFile::Open(fname, "read");


    for (auto hname : histnames ) {
	auto h = (TH2*) f->Get(hname);

	sliceAndFit(h);
    }

}


void sliceAndFit(TH2* h)
{
    auto n = h->GetNbinsX();
    auto gr = new TGraph(n);

    for (int i = 0; i < n; ++i) {
	auto tmph = h->ProjectionY("tmph", i+1, i+1);
	auto maxbin = tmph->GetMaximumBin();
	if (maxbin <= 0)
	    continue;
	auto mpv = tmph->GetBinCenter(maxbin);
	auto x = h->GetXaxis()->GetBinCenter(i+1);
	gr->SetPoint(i, x, mpv);
    }

    gr->SetName(Form("%s_profile",h->GetName()));
    h->GetListOfFunctions()->Add(gr);

    fit(gr);
}

void fit(TGraph* gr)
{
    const double boundaries[] = {-726.7681, -363.38405 0., 363.38405, 726.7681};
    const double TPC_Z_SIZE = 232.39;

    const double kXtoT = 1./160.563; // converts cm to ms, calculated for field 0.5kV/cm and temperature 87K

    // loop over multiple tpc boundaries

    for (int i = 0; i < 4; ++i) {
	double low = boundaries[i] + 15; // allow for a margin around APC/CPA centre location
	double hi = boundaries[i+1] - 15;
	gr->Fit("expo", "Q+", "", low, hi);
    }

}
