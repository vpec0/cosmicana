#ifndef HISTHANDLER_H
#define HISTHANDLER_H

class HistHandler
{
public:
    HistHandler(){};
    ~HistHandler(){};

    static void doXlog(TH1* h)
    // redo scales for x-log hists
    {
	TAxis* axis = h->GetXaxis();

	double start = TMath::Log10(axis->GetXmin());
	double stop = TMath::Log10(axis->GetXmax());
	double range = stop - start;
	int nbins = axis->GetNbins();
	double binwidth = range / nbins;

	double *bins = new double[nbins+1];
	for (int i = 0; i < (nbins+1); ++i) {
	    bins[i] = TMath::Power(10, start + i*binwidth);
	}

	axis->Set(nbins, bins);

	delete[] bins;
    };


};

#endif
