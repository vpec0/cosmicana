OUTPUT="$1"
TOMERGE="${@:2}"



root -l -b  <<EOF
TString tomerge = "$TOMERGE";
auto flist  = tomerge.Tokenize(" ");

auto tree = new TChain("elifetime");

for (auto fname: *flist) {
    tree->Add(((TObjString*)fname)->String());
}
auto outf = TFile::Open("$OUTPUT","recreate");
tree->CloneTree(-1,"fast"); outf->Write(); outf->Close();
EOF
