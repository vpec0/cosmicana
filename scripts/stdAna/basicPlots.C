#define FOR(i, size) for (int i = 0; i < size; ++i)
typedef const char* STR;


void redoAspect (TH1*, TCanvas*);


void basicPlots(STR fname, STR outpref, int which = 0xff)
{

    auto f = TFile::Open(fname, "read");
    auto tree = (TTree*) f->Get("analysistree/anatree");
    auto c1 = new TCanvas("c1", "");


    if (which & 1 )
    //===== views of starts and end of all tracks =====
    {
	TString starts[] = {
	    "StartPointx:StartPointz", // top view
	    "StartPointy:StartPointz", // side view
	    "StartPointy:StartPointx", // back view
	};

	FOR(i, 3) {
	    tree->Draw(starts[i]);
	    auto h = (TH1*)gPad->FindObject("htemp");
	    redoAspect(h, c1);

	    // save the output
	    gPad->SaveAs(Form("%s_startpoint_%d.png", outpref, i));
	}

	TString ends[] = {
	    "EndPointx:EndPointz", // top view
	    "EndPointy:EndPointz", // side view
	    "EndPointy:EndPointx", // back view
	};


	FOR(i, 3) {
	    tree->Draw(ends[i]);
	    auto h = (TH1*)gPad->FindObject("htemp");
	    redoAspect(h, c1);

	    // save the output
	    gPad->SaveAs(Form("%s_endpoint_%d.png", outpref, i));
	}
    }

    if (which & (1<<1) )
    //===== views of starts and end of primary tracks (muons) =====
    {
	TString starts[] = {
	    "StartPointx:StartPointz", // top view
	    "StartPointy:StartPointz", // side view
	    "StartPointy:StartPointx", // back view
	};

	FOR(i, 3) {
	    tree->Draw(starts[i], "Mother == 0");
	    auto h = (TH1*)gPad->FindObject("htemp");
	    redoAspect(h, c1);

	    // save the output
	    gPad->SaveAs(Form("%s_startpoint_primary_%d.png", outpref, i));
	}

	TString ends[] = {
	    "EndPointx:EndPointz", // top view
	    "EndPointy:EndPointz", // side view
	    "EndPointy:EndPointx", // back view
	};


	FOR(i, 3) {
	    tree->Draw(ends[i], "Mother == 0");
	    auto h = (TH1*)gPad->FindObject("htemp");
	    redoAspect(h, c1);

	    // save the output
	    gPad->SaveAs(Form("%s_endpoint_primary_%d.png", outpref, i));
	}
    }

    if (which & (1<<2) )
    //===== muon energy distribution =====
    {
	tree->Draw("EndE", "Mother == 0");
	auto h = (TH1*)gPad->FindObject("htemp");
	h->SetFillColor(gStyle->GetHistFillColor());
	gPad->SetLogy();
	gPad->SaveAs(Form("%s_gen_energy_primary.pdf", outpref));
	gPad->SetLogy(0);
    }


    if (which & (1<<3) )
    //===== muon angular distribution - cos(theta) =====
    {
	tree->Draw("-Py/P", "Mother == 0");
	auto h = (TH1*)gPad->FindObject("htemp");
	h->SetTitle("Primary angular distribution - cos(#theta);cos(#theta)");
	h->SetFillColor(gStyle->GetHistFillColor());
	gPad->SetLogy(0);
	gPad->SaveAs(Form("%s_gen_theta_primary.pdf", outpref));
	gPad->SetLogy(0);
    }

    if (which & (1<<4) )
    //===== muon angular distribution - phi =====
    {
	TString cmd = "(TMath::ACos((Pz/sqrt(Px**2+Pz**2)))/TMath::Pi())";

	tree->Draw("(Px>0)?"+cmd+":2-"+cmd, "Mother == 0");
	auto h = (TH1*)gPad->FindObject("htemp");
	h->SetTitle("Primary angular distribution - #phi;#phi [#pi]");
	h->SetFillColor(gStyle->GetHistFillColor());
	gPad->SetLogy(0);
	gPad->SaveAs(Form("%s_gen_phi_primary.pdf", outpref));
	gPad->SetLogy(0);
    }

    //===== Muons which passed through detector

    TCut muon_select = "Sum$(Mother == 0 && inTPCActive)";


    if (which & (1<<5) )
    //===== views of starts and end of all tracks =====
    {
	TString starts[] = {
	    "StartPointx:StartPointz", // top view
	    "StartPointy:StartPointz", // side view
	    "StartPointy:StartPointx", // back view
	};

	FOR(i, 3) {
	    tree->Draw(starts[i], muon_select);
	    auto h = (TH1*)gPad->FindObject("htemp");
	    redoAspect(h, c1);

	    // save the output
	    gPad->SaveAs(Form("%s_muinactive_startpoint_%d.png", outpref, i));
	}

	TString ends[] = {
	    "EndPointx:EndPointz", // top view
	    "EndPointy:EndPointz", // side view
	    "EndPointy:EndPointx", // back view
	};


	FOR(i, 3) {
	    tree->Draw(ends[i], muon_select);
	    auto h = (TH1*)gPad->FindObject("htemp");
	    redoAspect(h, c1);

	    // save the output
	    gPad->SaveAs(Form("%s_muinactive_endpoint_%d.png", outpref, i));
	}
    }

    muon_select = "(Mother == 0 && inTPCActive)";

    if (which & (1<<6) )
    //===== views of starts and end of primary tracks (muons) =====
    {
	TString starts[] = {
	    "StartPointx:StartPointz", // top view
	    "StartPointy:StartPointz", // side view
	    "StartPointy:StartPointx", // back view
	};

	FOR(i, 3) {
	    tree->Draw(starts[i], muon_select);
	    auto h = (TH1*)gPad->FindObject("htemp");
	    redoAspect(h, c1);

	    // save the output
	    gPad->SaveAs(Form("%s_muinactive_startpoint_primary_%d.png", outpref, i));
	}

	TString ends[] = {
	    "EndPointx:EndPointz", // top view
	    "EndPointy:EndPointz", // side view
	    "EndPointy:EndPointx", // back view
	};


	FOR(i, 3) {
	    tree->Draw(ends[i], muon_select);
	    auto h = (TH1*)gPad->FindObject("htemp");
	    redoAspect(h, c1);

	    // save the output
	    gPad->SaveAs(Form("%s_muinactive_endpoint_primary_%d.png", outpref, i));
	}
    }

    if (which & (1<<7) )
    //===== muon energy distribution =====
    {
	tree->Draw("EndE", muon_select);
	auto h = (TH1*)gPad->FindObject("htemp");
	h->SetFillColor(gStyle->GetHistFillColor());
	gPad->SetLogy();
	gPad->SaveAs(Form("%s_muinactive_gen_energy_primary.pdf", outpref));
	gPad->SetLogy(0);
    }


    if (which & (1<<8) )
    //===== muon angular distribution - cos(theta) =====
    {
	tree->Draw("-Py/P", muon_select);
	auto h = (TH1*)gPad->FindObject("htemp");
	h->SetTitle("Primary angular distribution - cos(#theta);cos(#theta)");
	h->SetFillColor(gStyle->GetHistFillColor());
	gPad->SetLogy(0);
	gPad->SaveAs(Form("%s_muinactive_gen_theta_primary.pdf", outpref));
	gPad->SetLogy(0);
    }

    if (which & (1<<9) )
    //===== muon angular distribution - phi =====
    {
	TString cmd = "(TMath::ACos((Pz/sqrt(Px**2+Pz**2)))/TMath::Pi())";

	tree->Draw("(Px>0)?"+cmd+":2-"+cmd, muon_select);
	auto h = (TH1*)gPad->FindObject("htemp");
	h->SetTitle("Primary angular distribution - #phi;#phi [#pi]");
	h->SetFillColor(gStyle->GetHistFillColor());
	gPad->SetLogy(0);
	gPad->SaveAs(Form("%s_muinactive_gen_phi_primary.pdf", outpref));
	gPad->SetLogy(0);
    }

}


void redoAspect (TH1* h, TCanvas* c1)
{
	double xmin = h->GetXaxis()->GetXmin();
	double xmax = h->GetXaxis()->GetXmax();
	double ymin = h->GetYaxis()->GetXmin();
	double ymax = h->GetYaxis()->GetXmax();

	double aspect = (xmax - xmin)/(ymax - ymin);

	cout<<"Histogram size (w x h): " << (xmax - xmin) << " x " << (ymax - ymin);
	cout<<"Aspect ratio: " << aspect << endl;

	// correct current canvas size
	double ww = c1->GetWw();
	double wh = c1->GetWh();
	double windoww = c1->GetWindowWidth();
	double windowh = c1->GetWindowHeight();

	cout <<"World width, height: " << ww << " x "<<wh<<endl
	     <<"Window size (w x h): " << windoww << " x " << windowh << endl;

	c1->SetCanvasSize(aspect * wh, wh);
	c1->SetWindowSize(c1->GetWw() + (windoww - ww), windowh);
	gPad->SetCanvasSize(aspect * wh, wh);
}
