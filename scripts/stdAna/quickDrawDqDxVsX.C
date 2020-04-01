{
    TFile *f = TFile::Open("plots/elifetime/batch_21_anahists.root");

    auto c = new TCanvas("c","",800,800);
    c->Divide(2,2);
    c->cd(1); Dqdx_vs_x->Draw("colz");
    c->cd(2); Dqdx_vs_x_plane0->Draw("colz");
    c->cd(3); Dqdx_vs_x_plane1->Draw("colz");
    c->cd(4); Dqdx_vs_x_plane2->Draw("colz");

    auto c2 = new TCanvas("c2","",800,800);
    c2->Divide(2,2);
    c2->cd(1); Dqdx_vs_t->Draw("colz");
    c2->cd(2); Dqdx_vs_t_plane0->Draw("colz");
    c2->cd(3); Dqdx_vs_t_plane1->Draw("colz");
    c2->cd(4); Dqdx_vs_t_plane2->Draw("colz");
}
