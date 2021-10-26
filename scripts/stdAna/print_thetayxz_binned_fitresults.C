
#define FOR(i, size) for (int i = 0; i < size; ++i)

void setAxisLabels(THStack* h, int n, const char** labels);


void print_thetayxz_binned_fitresults(const char* fname, const char *outpref)
{
    gStyle->SetHistFillStyle(0);
    gStyle->SetMarkerSize(1.);
    gStyle->SetPadGridY(1);

    auto f = TFile::Open(fname, "read");

    auto tree = (TTree*)f->Get("elifetime");

    double tau_e[3][4][4];
    double tau_e_error[3][4][4];
    double qcqa[3][4][4];
    double qcqa_error[3][4][4];

    tree->SetBranchStatus("*", 0);
    tree->SetBranchStatus("tau_e*", 1);
    tree->SetBranchStatus("qcqa*", 1);

    tree->SetBranchAddress("tau_e", tau_e);
    tree->SetBranchAddress("tau_e_error", tau_e_error);
    tree->SetBranchAddress("qcqa", qcqa);
    tree->SetBranchAddress("qcqa_error", qcqa_error);

    tree->GetEntry(0);

    TH1F* hists_tau[4];
    TH1F* hists_qcqa[4];
    FOR(iseg, 4) {
	hists_tau[iseg] = new TH1F(Form("hTau_%d", iseg),
				   Form("Plane %d; Direction bin;#tau_{e} [ms]", iplane),
				   4, 0, 4);
	hists_qcqa[iseg] = new TH1F(Form("hQcqa_%d", iseg),
				   Form("Segment %d; Direction bin; Q_{C}/Q_{A}", iplane),
				   4, 0, 4);
	FOR(ibin, 4) {
	    hists_tau[iseg]->SetBinContent(ibin+1, tau_e[2][ibin][iseg]);
	    hists_tau[iseg]->SetBinError(ibin+1, tau_e_error[2][ibin][iseg]);
	    hists_qcqa[iseg]->SetBinContent(ibin+1, qcqa[2][ibin][iseg]);
	    hists_qcqa[iseg]->SetBinError(ibin+1, qcqa_error[2][ibin][iseg]);

	    cout<<"Segment "<<iseg<<" bin: "<<ibin<<" : "
		<<tau_e[2][ibin][iseg]<<" pm "<<tau_e_error[2][ibin][iseg]<<endl;
	}
    }


    int colors[4] = {kRed, kBlue, kGreen+1, kOrange};

    auto hs_tau = new THStack("hs", "#tau_{e};;#tau_{e} [ms]");
    auto hs_qcqa = new THStack("hs", "Q_{C}/Q_{A};; Q_{C}/Q_{A}");
    FOR(iseg, 4) {
	hs_tau->Add(hists_tau[iseg]);
	hists_tau[iseg]->SetMarkerColor(colors[iseg]);
	hists_tau[iseg]->SetLineColor(colors[iseg]);

    	hs_qcqa->Add(hists_qcqa[iseg]);
	hists_qcqa[iseg]->SetMarkerColor(colors[iseg]);
	hists_qcqa[iseg]->SetLineColor(colors[iseg]);
    }

    const char* axis_labels[4] = {"Vertical", "Towards APA", "Parallel to APA", "Rest"};

    auto c = new TCanvas("c", "", 1);
    c->SetBottomMargin(0.2);

    hs_tau->SetMaximum(3.2);
    hs_tau->SetMinimum(2.1);
    hs_qcqa->SetMinimum(0.36);

    hs_tau->Draw("PEX0 nostack");
    setAxisLabels(hs_tau, 4, axis_labels);
    c->BuildLegend();
    c->SaveAs(Form("%stau.pdf", outpref));

    hs_qcqa->Draw("PEX0 nostack");
    setAxisLabels(hs_qcqa, 4, axis_labels);
    c->BuildLegend();
    c->SaveAs(Form("%sqcqa.pdf", outpref));
}

void setAxisLabels(THStack* h, int n, const char** labels)
{
    auto ax = h->GetXaxis();
    ax->SetLabelSize(0.08);
    ax->SetTickLength(0.);
    for (int i = 1; i <= n; ++i) {
	ax->SetBinLabel(i, labels[i-1]);
    }
}
