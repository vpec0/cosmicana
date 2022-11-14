#ifndef BASICANATREE_H
#define BASICANATREE_H


class BasicAnaTree {

public:
    static const int MAX_TRACKS = 1024;

public:
    BasicAnaTree(TTree* tree = 0);
    ~BasicAnaTree();

private:
    void create();

public:
    void clear();

public:
    TTree* fTree;

    Int_t run;
    Int_t event;

    // tree leaf variables
    // true quantities of primary muon
    Float_t E;
    Float_t costheta;
    Float_t phi;

    Float_t len_tpc;
    Float_t Edep_tpc;

    Float_t start[3];
    Float_t end[3];

    Int_t stopped;

    // reco quantities
    Int_t nTracks; // number of reconstructed tracks overall
    Int_t nTracksPrimary; // number of reconstructed tracks attributed to primary muon

    Float_t trk_len[MAX_TRACKS]; // length of reconstructed track attributed to primary muon
    Float_t trk_start[MAX_TRACKS][3]; // start position of reconstructed track attributed to primary muon
    Float_t trk_end[MAX_TRACKS][3]; // start position of reconstructed track attributed to primary muon

    Int_t dummy;
};
#endif

#ifdef BASICANATREE_CXX
BasicAnaTree::BasicAnaTree(TTree* tree = 0):
    fTree(0),
    run(0),
    event(0),
    E(0.),
    costheta(0.),
    phi(0.),
    len_tpc(0.),
    Edep_tpc(0.),
    start{0.},
    end{0.},
    stopped(0),
    nTracks(0),
    nTracksPrimary(0),
    trk_len{0.},
    trk_start{0.},
    trk_end{0.}
{
    if (tree == 0) {
	fTree = new TTree("basicanatree","");
	create();
    }
}

void BasicAnaTree::clear()
{
    memset(&run, 0, &dummy  - &run);
}

BasicAnaTree::~BasicAnaTree()
{
    delete fTree;
}

#define BRANCH(var, type) fTree->Branch(#var, &var, #var "/" type)
#define BRANCHARR(var, size, type) fTree->Branch(#var, var, #var size "/" type)
void BasicAnaTree::create()
{

    BRANCH(run,"I");
    BRANCH(event, "I");

    BRANCH(E, "F");
    BRANCH(costheta, "F");
    BRANCH(phi, "F");

    BRANCH(len_tpc, "F");
    BRANCH(Edep_tpc, "F");

    BRANCHARR(start, "[3]", "F");
    BRANCHARR(end, "[3]", "F");

    BRANCH(stopped, "I");

    BRANCH(nTracks, "I");
    BRANCH(nTracksPrimary, "I");

    BRANCHARR(trk_len, "[nTracksPrimary]", "F");
    BRANCHARR(trk_start, "[nTracksPrimary][3]", "F");
    BRANCHARR(trk_end, "[nTracksPrimary][3]", "F");

}


#endif
