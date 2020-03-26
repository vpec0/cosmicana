/**
 * APA positions:
 * $ cat dune10kt_v1_nowires.gdml |grep -A3 "<physvol>" |grep -A1 'posAPAFrameYSideNeg' |grep "x="
 *  x="-726.7681"
 *  x="0"
 *  x="726.7681"
 *
 * CPA positions:
 *  $ cat dune10kt_v1_nowires.gdml |grep -A3 "<physvol>" |grep -A1 'posCathode' |grep "x="
 *   x="-363.38405"
 *   x="363.38405"
 *
 * TPC: assuming 1st starts at Z=0
 * size in Z = 232.39
 *
 **/
namespace CosmicMuonEvent {

    const unsigned int MAX_TRACKS = 1024;
    const unsigned int MAX_APAS = 150;

    const double APA_X_POSITIONS[] = {-726.7681, 0., 726.7681};
    const double CPA_X_POSITIONS[] = {-363.38405, 363.38405};
    const double TPC_Z_SIZE = 232.39;


#define VAR_LIST				\
    TRUTH_LIST;					\
    PANDORA_LIST;				\
    PM_LIST					\

#define TRUTH_LIST				\
    INT(run);					\
    INT(event);					\
    DOUBLE(StartPointx_tpcAV);			\
    DOUBLE(StartPointy_tpcAV);			\
    DOUBLE(StartPointz_tpcAV);			\
    DOUBLE(EndPointx_tpcAV);			\
    DOUBLE(EndPointy_tpcAV);			\
    DOUBLE(EndPointz_tpcAV);			\
    DOUBLE(StartE_tpcAV);			\
    DOUBLE(Eloss);				\
    DOUBLE(StartPx_tpcAV);			\
    DOUBLE(StartPy_tpcAV);			\
    DOUBLE(StartPz_tpcAV);			\
    DOUBLE(StartP_tpcAV);			\
    DOUBLE(pathlen);				\
    INT(nApa);					\
    INT_ARRAY(Apa,nApa);			\
    INT(nCpa);					\
    INT_ARRAY(Cpa,nCpa)				\

#define PANDORA_LIST						\
    INT(nRecoPandora);						\
    DOUBLE_ARRAY(trkstartx_pandoraTrack, nRecoPandora);		\
    DOUBLE_ARRAY(trkstarty_pandoraTrack, nRecoPandora);		\
    DOUBLE_ARRAY(trkstartz_pandoraTrack, nRecoPandora);		\
    DOUBLE_ARRAY(trkendx_pandoraTrack, nRecoPandora);		\
    DOUBLE_ARRAY(trkendy_pandoraTrack, nRecoPandora);		\
    DOUBLE_ARRAY(trkendz_pandoraTrack, nRecoPandora);		\
    DOUBLE_ARRAY(trkstartdcosx_pandoraTrack, nRecoPandora);	\
    DOUBLE_ARRAY(trkstartdcosy_pandoraTrack, nRecoPandora);	\
    DOUBLE_ARRAY(trkstartdcosz_pandoraTrack, nRecoPandora);	\
    DOUBLE_ARRAY(trklen_pandoraTrack, nRecoPandora)		\

#define PM_LIST							\
    INT(nRecoPm);						\
    DOUBLE_ARRAY(trkstartx_pmtrack, nRecoPm);			\
    DOUBLE_ARRAY(trkstarty_pmtrack, nRecoPm);			\
    DOUBLE_ARRAY(trkstartz_pmtrack, nRecoPm);			\
    DOUBLE_ARRAY(trkendx_pmtrack, nRecoPm);			\
    DOUBLE_ARRAY(trkendy_pmtrack, nRecoPm);			\
    DOUBLE_ARRAY(trkendz_pmtrack, nRecoPm);			\
    DOUBLE_ARRAY(trkstartdcosx_pmtrack, nRecoPm);		\
    DOUBLE_ARRAY(trkstartdcosy_pmtrack, nRecoPm);		\
    DOUBLE_ARRAY(trkstartdcosz_pmtrack, nRecoPm);		\
    DOUBLE_ARRAY(trklen_pmtrack, nRecoPm)			\


#define INT(var)	Int_t var
#define DOUBLE(var)	Double_t var
#define DOUBLE_ARRAY(var, size) Double_t var[MAX_TRACKS]
#define INT_ARRAY(var, size) Int_t var[MAX_APAS]

    struct Event_t {
	VAR_LIST;
    };

#undef INT
#undef DOUBLE
#undef DOUBLE_ARRAY
#undef INT_ARRAY


#define INT(var) tree->Branch(#var, &event.var, #var"/I")
#define DOUBLE(var) tree->Branch(#var, &event.var, #var"/D")
#define DOUBLE_ARRAY(var,size) tree->Branch(#var, event.var, #var"["#size"]/D")
#define INT_ARRAY(var,size) tree->Branch(#var, event.var, #var"["#size"]/I")

    TTree* createTree(const char* tree_name, Event_t& event) {
	TTree* tree = new TTree(tree_name, tree_name);

	VAR_LIST;

	return tree;
    }



#undef INT
#undef DOUBLE
#undef DOUBLE_ARRAY
#undef INT_ARRAY

#define INT(var) tree->SetBranchAddress(#var, &event.var)
#define DOUBLE(var) tree->SetBranchAddress(#var, &event.var)
#define DOUBLE_ARRAY(var, size) tree->SetBranchAddress(#var, event.var)
#define INT_ARRAY(var, size) tree->SetBranchAddress(#var, event.var)

    TTree* registerTree(TTree* tree, Event_t& event) {

	VAR_LIST;

	return tree;
    }
}
