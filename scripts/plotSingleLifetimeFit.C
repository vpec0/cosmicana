void plotSingleLifetimeFit(const char* fname, const char* outpref, int batch,
			   int part = 0, int plane = 0,
			   int segment = 0, int slice = 30)
{
    gROOT->ForceStyle(1);

    auto f = TFile::Open(fname, "read");

    auto gr = (TGraphErrors*) f->Get(Form("%d/part%d/Dqdx_vs_x_plane%d_profile", batch, part, plane));

    gStyle->SetOptStat(0);
    gStyle->SetLineWidth(2);
    gStyle->SetLineColor(kRed);
    gStyle->SetMarkerSize(1);

    gr->SetTitle(Form("dQ/dx vs x plane %d;x [cm];dQ/dx [ADC/cm]", plane));

    auto c = new TCanvas("c","", 1);
    gr->Draw("apez");

    gr->SetMarkerSize(0.7);
    gr->SetLineWidth(3);
    gr->SetLineColor(kBlack);

    for (auto fit: *gr->GetListOfFunctions()) {
	if (strstr(fit->GetName(), "fit")) {
	    auto ff = (TF1*)fit;
	    ff->SetLineColor(kRed);
	    ff->SetLineWidth(2);
	    cout<<"Slope: "<<ff->GetParameter(1)<<endl
		<<"Error: "<<ff->GetParError(1)<<endl
		<<"Rel.Error: "<<ff->GetParError(1)/ff->GetParameter(1)<<endl
		<<"Chi2/ndf: "<<ff->GetChisquare()
		<<"/"<<ff->GetNDF()<<endl;
	}
    }


    c->Print(Form("%selifetime_examplefit_plane%d_%d_part%d.pdf", outpref, plane, batch, part));


    auto pad = new TCanvas("temp", "", 400,400);
    TString histname = Form("%d/part%d/plane%d/segment%d/Dqdx_vs_x_plane%d_%d",
			    batch, part, plane, segment, plane, slice);

    auto h = (TH1*)f->Get(histname);

    if (!h) {
	cerr<<"Can't find histogram "<<histname<<endl;
	f->ls();
	return;
    }

    cout<<"Found histogram "<<histname<<endl;
    h->Draw();
    pad->SaveAs(Form("%sexample_slice_%s.pdf", outpref, h->GetName()));



}
