void getDirection(double v1, double v2, double corr, double& result1, double& result2);

void plotCorrelations_planes(const char* fname, const char* outpref)
{
    auto f = TFile::Open(fname, "read");
    auto tree = f->Get<TTree>("elifetime");

    for (int segment = 0; segment < 4; segment++) {
	auto c1 = new TCanvas("c1",""); c1->Divide(2,2);
	for (int i=0; i<3; i++)
	    for (int j=i+1;j<3;j++) {
		c1->cd(i*2+j);
		tree->Draw(Form("tau_e[%d][%d]:tau_e[%d][%d]",i, segment, j, segment));
		auto gr = (TGraph*)gROOT->FindObject("Graph");
		gr->SetName(Form("gr%d", i*2+j));
		gr->SetMarkerSize(0.2);

		double corr = gr->GetCorrelationFactor();

		// Draw correlation coef.
		auto tt=new TText(0.7,0.92,Form("%f", corr));
		tt->SetNDC();
		tt->SetTextFont(44); tt->SetTextSize(15);

		tt->Draw("ndc");

		// Draw correlation axis
		double mean_x = gr->GetMean(1);
		double mean_y = gr->GetMean(2);
		double rms_x = gr->GetRMS(1);
		double rms_y = gr->GetRMS(2);
		double cov = gr->GetCovariance();

		double dir1, dir2;
		getDirection(rms_x*rms_x, rms_y*rms_y, cov, dir1, dir2);

		auto corr_func = new TF1(Form("corr_func%d%d%d", segment, i, j),
					 "(x - [0])*[2] + [1]",
					 0.,10.);
		corr_func->SetParameters(mean_x, mean_y,dir1);
		corr_func->SetLineColor(kRed);

		corr_func->Draw("lsame");

		auto corr_func2 = new TF1(*corr_func);
		corr_func2->SetParameter(2, dir2);
		corr_func2->SetLineColor(kBlue);
		corr_func2->Draw("lsame");
	    }
	c1->SaveAs(Form("%splane_correlations_segment%d.pdf", outpref, segment));
	delete c1;
    }
}

void getDirection(double v1, double v2, double cov, double& result1, double& result2)
{
    double dv = v1 - v2;
    double b = dv*dv + 4*cov*cov;
    b = sqrt(b);
    result1 = -(dv + b)/(2*cov);
    result2 = -(dv - b)/(2*cov);
}
