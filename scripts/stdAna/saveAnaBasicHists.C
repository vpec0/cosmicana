/**
 * Plots histograms created by process_ana_basic.C
 *
 *
 **/

void saveAnaBasicHists(const char* fname, const char* outpref)
{
    gROOT->ForceStyle(1);
    gStyle->SetHistMinimumZero(1);
    gStyle->SetHistTopMargin(0.15);
    gStyle->SetPadTopMargin(0.15);
    gStyle->SetPadBottomMargin(0.15);
    gStyle->SetPadRightMargin(0.15);
    gStyle->SetPadLeftMargin(0.15);
    gStyle->SetTitleOffset(1.2, "xy");
    gStyle->SetTitleY(0.96);

    gStyle->SetPadGridX(1);
    gStyle->SetPadGridY(1);

    gStyle->SetOptStat(0);

#define LIST					\
    ITEM(    E_gen					)	\
	ITEM(   Phi_fnal_gen				)	\
	ITEM(	Phi_east_gen				)	\
	ITEM(	Theta_gen				)	\
	ITEM(	E_all					)	\
	ITEM(	E_primary				)	\
	ITEM(	E_secondary				)	\
	ITEM(	E_gen_logx				)	\
	ITEM(	E_all_logx				)	\
	ITEM(	E_primary_logx				)	\
	ITEM(	E_secondary_logx			)	\
	ITEM(	Phi	        			)	\
	ITEM(	Phi_fnal	       			)	\
	ITEM(	Phi_east	       			)	\
	ITEM(	Theta					)	\
	ITEM(	Path_primary				)	\
	ITEM(	Path_secondary				)	\
	ITEM(	Eloss					)	\
	ITEM(	Eloss_logx				)	\
	ITEM(	Elosspath				)	\
	ITEM(	Elosspath_logx				)	\
	ITEM(	EMichel_all				)	\
	ITEM(	EMichel_primary				)	\
	ITEM(	Epi0					)	\
	ITEM(	Npi0					)	\
	ITEM(	Ntracks					)	\
	ITEM(	Nshowers				)	\
	ITEM(	NHists )


#define ITEM(item) item,
    enum {
	LIST
    };

#undef ITEM
#define ITEM(item) #item,
    vector<TString> hnames = {
	LIST
    };

    vector<int> rebin(NHists, 0);
    rebin[E_gen] = 10;
    rebin[E_all] = 10;
    rebin[E_primary] = 10;
    rebin[Phi] = 2;
    rebin[Phi_east] = 2;
    rebin[Phi_fnal] = 2;
    rebin[Path_secondary] = 5;
    rebin[Eloss] = 5;
    rebin[Elosspath] = 1;
    rebin[EMichel_all] = 10;
    rebin[EMichel_primary] = 10;
    rebin[Epi0] = 10;

    vector<double> range_min(NHists, -999);
    range_min[Theta] = 0.;
    range_min[Theta_gen] = 0.;

    vector<double> range_max(NHists, -999);
    range_max[Path_secondary] = 4;
    range_max[Eloss] = 600;
    range_max[Elosspath] = 10;
    range_max[EMichel_all] = 60;
    range_max[EMichel_primary] = 60;
    range_max[Epi0] = 35;

    vector<int> logx(NHists, 0);
    for (int i = E_gen_logx; i <= E_secondary_logx; ++i)
	logx[i] = 1;
    logx[Eloss_logx] = 1;
    logx[Elosspath_logx] = 1;

    vector<int> logy(NHists, 1);
    logy[Phi] = 0;
    logy[Phi_fnal] = 0;
    logy[Phi_east] = 0;
    logy[Phi_fnal_gen] = 0;
    logy[Phi_east_gen] = 0;
    logy[EMichel_all] = 0;
    logy[EMichel_primary] = 0;

    auto f = TFile::Open(fname, "read");


    auto c = new TCanvas("c","", 2);

    int i = -1;
    for (auto hname : hnames) {
	i++;
	auto h = (TH1*)f->Get(hname);
	if (!h) continue;

	if (rebin[i] > 1)
	    h->Rebin(rebin[i]);

	if (range_min[i] != -999.)
	    h->SetAxisRange(range_min[i], h->GetBinLowEdge(h->GetNbinsX()+1));

	if (range_max[i] != -999.)
	    h->SetAxisRange(h->GetBinLowEdge(1), range_max[i]);


	if (logx[i]) {
	    h->Scale(1.,"width");
	    h->GetYaxis()->SetTitle("[GeV^{-1}]");
	}
	h->Draw("hist");
	c->SetLogx(logx[i]);
	c->SetLogy(logy[i]);

	c->SaveAs(Form("%s%s.pdf", outpref, hname.Data()));
    }
}
