#include <TROOT.h>
#include <TFile.h>
#include <TLatex.h>
#include <TSpectrum.h>
#include <TFitResult.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TGraph.h>
#include <Riostream.h>
#include "TRint.h"
#include "fitPeak2.C"
// #include "TF1.h"
// #include "TH1.h"
// #include "TLatex.h"

// TFitResultPtr fitPeak2(TH1F* histo, double fitMin, double fitMax, TString myOpt, TString opt, TString gopt);
// TFitResultPtr fitPeak2(TH1I* histo, double fitMin, double fitMax, TString myOpt, TString opt, TString gopt);

void LYSOcal(TString filename,Double_t fitMin=37, Double_t fitMax=47,
  Double_t thresh=0.001, Int_t nToFit=6, Int_t pol=4, TString myOpt="I", Int_t chn=-1)
{
  if(myOpt.Contains('Q')) gROOT->SetBatch(1);

  Int_t startChn, endChn;
  if(chn==-1) {
    startChn=541;
    endChn=1024;
  }
  else {
    startChn=chn;
    endChn=chn+1;
  }

  TFile* file = TFile::Open(filename);
  file->cd();

  TString str2=filename;
  str2.ReplaceAll("_out.root","_fits");
  gSystem->MakeDirectory(str2);
  gSystem->cd(str2);

  // TString str=filename;
  TString str="calibrationCoefficients";
  if(chn==-1) {
    str=Form("calibrationCoefficients_%i-%i.csv",startChn,endChn);
    // str.Append(".csv");
  }
  else str=Form("calibrationCoefficients_%i.csv",chn);
  ofstream out(str);

  TCanvas* lin_c=new TCanvas("lin_c","Linearity fits",0,0,800,800);
  lin_c->Divide(1,2);
  lin_c->cd(1);
  TH1I *histo;
  TFitResultPtr myFit;
  Double_t cent;
  Double_t sigma;
  TSpectrum* spec;
  Int_t nFound;
  Double_t* xPos;	// array of peaks found, ordered by amplitude

  vector<Double_t> ordered;	// vector of peaks found re-ordered by energy (largest first)
  // Double_t energyArray[4]={1275,511,307,202};//,0}; Na22 lines
  // Double_t energyArray[4]={661.66,511,306.78,201.83};//,0}; Na22 and Cs137 lines
  // Double_t energyArray[5]={661.66,511,122.06,59.5409,31.414};//,0}; Na22 and Cs137 lines
  // Double_t energyArray[4]={136.47,122.06,14.4,6.4};//,0}; 57Co 136 is much smaller than I thought. Am seeing escape peak at 122 - ~55 keV
  // Double_t energyArray[4]={122.06,67,14.4,6.4};//,0}; 57Co
  // Double_t energyArray[6]={1274.537,661.66,511,122.06,59.5409,31.414}; // for scatter detector
  Double_t energyArray[6]={1274.537,661.66,511,306.78,201.83,31.414};// for main arrays
  vector<Double_t> energyVec;
  if(nToFit==1) for(int i=1;i<nToFit+1;i++) energyVec.push_back(energyArray[i]);
  else for(int i=0;i<nToFit;i++) energyVec.push_back(energyArray[i]); // for Co56

  char answer;
  int	peakIndex=0;
  int peaksLeft=0;
  int nUsed=0;
  double min, max;
  TLine* tl=new TLine(0,0,1,1);
  TGraph *gr;
  TF1* linFunc;
  vector<Double_t> fitCoef;
  TH1F* p0_h=new TH1F("p0_h","p0 coefficient",1024,0,1024);
  TH1F* p1_h=new TH1F("p1_h","p1 coefficient",1024,0,1024);
  TH1F* p2_h=new TH1F("p2_h","p2 coefficient",1024,0,1024);
  TH1F* p3_h=new TH1F("p3_h","p3 coefficient",1024,0,1024);

  Bool_t readyToFit=0;
  Bool_t nextPeak=0;

  for(int chnID=startChn; chnID<endChn; chnID++) {
    if(answer=='q') break;
    if(!myOpt.Contains('Q')) {
      cout << "#########################################" << endl;
      cout << "\n Processing channel " << chnID << "...";
    }
    histo = (TH1I*)gDirectory->Get(Form("qdc%i_h",chnID));
    if(histo->GetEntries()>100) {
      myFit = fitPeak2(histo,fitMin,fitMax,myOpt,"","");
      cent=myFit->Parameter(3);
      sigma=myFit->Parameter(4);
      lin_c->cd(1);
    	gPad->SetLogy(1);
      histo->GetXaxis()->SetRangeUser(5,100);
    	histo->Draw();
    	spec=new TSpectrum();
    	// histo->GetXaxis()->UnZoom();
      nFound=spec->Search(histo,sigma,"",thresh);
      if(!myOpt.Contains('Q')) cout << endl << nFound << " peaks found..." << endl;

      vector<Double_t> qdcToUse;		// array of peaks to use for calibration and linearity correction
      vector<Double_t> EtoUse;

     	xPos=spec->GetPositionX();	// array of peaks found, ordered by amplitude
    	vector<Double_t> ordered;	// array of peaks found re-ordered by energy (largest first)
    	Int_t index[nFound];
    	peakIndex=0;
      peaksLeft=nFound;
    	nUsed=0;

    	TMath::Sort(nFound,xPos,index);

    	for(int i=0;i<nFound;i++) ordered.push_back(xPos[index[i]]);
    	if(myOpt.Contains("I")) {
    		if(!myOpt.Contains('Q')) {
          cout << "First guess at peaks to use for calibration: " << endl;
      		cout << "Index	Energy (keV)	Channel" << endl;
      		for(int i=0;i<nToFit;i++) cout << i << "	" << energyVec[i] << "		" << ordered[i] << endl;
        }
    	}

      histo->GetXaxis()->UnZoom();
    	// histo->GetXaxis()->SetRangeUser(0,ordered[0]*1.1);
    	// histo->GetXaxis()->SetRangeUser(ordered[nFound-1]*0.9,ordered[0]*1.1);
    	gPad->Update();
    	if(myOpt.Contains("V")) cout << endl << gPad->GetUymax() << " " << pow(10,gPad->GetUymax()) << endl;
    	min=pow(10,gPad->GetUymin());
    	max=pow(10,gPad->GetUymax());
    	gPad->Update();
    	tl=new TLine(0,0,1,1);
      tl->SetLineColor(2);
      tl->SetLineStyle(2);
      tl->SetLineWidth(2);

    	gPad->SetLogy(1);
    	histo->GetXaxis()->SetRangeUser(1,100);
    	// histo->GetXaxis()->SetRangeUser(ordered[nToFit-1]-ordered[0]*0.1,ordered[0]*1.1);
    	// histo->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
    	histo->DrawCopy();
      tl->SetLineColor(2);
    	for(int i=0;i<nToFit;i++) tl->DrawLine(ordered[i],min,ordered[i],max);

// Draw peaks and fit
      gr=new TGraph(ordered.size(),&ordered[0],&energyVec[0]);
    	lin_c->cd(2);
    	gr->GetXaxis()->SetLimits(ordered[nToFit-1]-ordered[0]*0.1,ordered[0]*1.1);
    	// gr->GetXaxis()->SetLimits(0,qdcToUse[0]*1.1);
    	gr->Draw("a*");
    	gr->GetXaxis()->SetRangeUser(1,100);
    	// gr->GetXaxis()->SetRangeUser(ordered[nToFit-1]-ordered[0]*0.1,ordered[0]*1.1);
    	// gr->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
      gr->GetYaxis()->SetRangeUser(0,energyVec[0]*1.1);
    	gr->SetTitle("Calibration");
    	gr->GetXaxis()->SetTitle("channel");
    	gr->GetYaxis()->SetTitle("energy (keV)");
      if(pol==1) linFunc = new TF1("linFunc","pol1",0,8192);
      if(pol==2) linFunc = new TF1("linFunc","pol2",0,8192);
      if(pol==3) linFunc = new TF1("linFunc","pol3",0,8192);
      if(pol==4) linFunc = new TF1("linFunc","pol4",0,8192);
      if(nToFit==1) linFunc->FixParameter(0,0);
      linFunc->SetParLimits(1,0,100);
    	gr->Fit("linFunc","qB");
    	gr->Draw("a*");
      gPad->Update();
/////////////////////

// interactively find the right peaks
      if(myOpt.Contains("I")) {
        cout << "Accept these peaks (y(es)/n(o)/s(kip)/q(uit)): ";
      	cin >> answer;
        if(answer=='q') break; // break out of loop over channels, i.e. abort!!!
        if(answer=='s') continue; // go to next channel
      	while(!readyToFit) {
      		if(answer=='y') {
            for(int i=0;i<nToFit;i++) {
              qdcToUse.push_back(ordered[i]);
              EtoUse.push_back(energyVec[i]);
              nUsed=nToFit;
            }
      			cout << "Using these peaks..." << endl;
            readyToFit=1;
      		} else if(answer=='n') {
            lin_c->Clear();
            lin_c->Divide(1,2);
            lin_c->cd(1);
            gPad->SetLogy(1);
            // histo->GetXaxis()->SetRangeUser(ordered[nFound-1]*0.9,ordered[0]*1.1);
            histo->GetXaxis()->UnZoom();
          	histo->Draw();

      			cout << "Which peaks to use..." << endl;

      			for(int i=0; i<nToFit; i++) {
      				// cout << "Use " << energyVec[i] << " keV? (y/n): ";
      				// cin >> answer;
              if(answer=='q') break;
              if(answer=='s') break;
              cout << energyVec[i] << " keV: " << endl;
              answer='y';
      				if(answer=='n') continue; // continue to next peak
      				else if(answer=='y') {
      					while(!nextPeak) {
                  if(peaksLeft<1) {
                    cout << "No peaks left. Exiting." << endl;
                    break; // break out of 'while(!readyToFit)'
                  }
                  tl->SetLineColor(1);
      						tl->DrawLine(ordered[peakIndex],min,ordered[peakIndex],max);
      						gPad->Update();
      						cout << "	Use channel: " << ordered[peakIndex] <<" ? (y/n/s): ";
      						cin >> answer;
                  if(answer=='q') break; // break out of "while(!nextPeak)"
                  if(answer=='s') break;
                  if(answer=='y') {
                    tl->SetLineColor(3);
                    tl->DrawLine(ordered[peakIndex],min,ordered[peakIndex],max);
                    peaksLeft--;
      							qdcToUse.push_back(ordered[peakIndex]);
      							EtoUse.push_back(energyVec[i]);
      							peakIndex++;
      							nUsed++;
      							break; // break out of 'while(!nextPeak)'
      						} else if(answer=='n') {
                    peaksLeft--;
      							peakIndex++;
      							continue; // try next peak
      						}
      					}
      				}
      			}
      			cout << endl << "New peaks to use for calibration: " << endl;
      			cout << "Energy (keV)	Channel" << endl;
      			for(int i=0;i<nUsed;i++) cout << EtoUse[i] << "		" << qdcToUse[i] << endl;
      			break; // break out of 'while(!readyToFit)'
      		} // end of 'if(answer=='n')', "Accept these peaks?"
      		else {
      			cout << "Try again (y/n): ";
      			cin >> answer;
            if(answer=='q') break;
            if(answer=='s') continue;
      		}
      	} // end of 'while(!readyToFit)'
        if(nUsed!=0) readyToFit=1;
      } // end of interctive bit
      else for(int i=0;i<nToFit;i++) {
        qdcToUse.push_back(ordered[i]);
        EtoUse.push_back(energyVec[i]);
        nUsed=nToFit;
        readyToFit=1;
      }

      lin_c->Clear();
      lin_c->Divide(1,2);

      if(readyToFit) {
      	lin_c->cd(1);
      	gPad->SetLogy(1);
      	histo->GetXaxis()->SetRangeUser(qdcToUse[nUsed-1]-qdcToUse[0]*0.1,qdcToUse[0]*1.1);
      	// histo->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
      	histo->DrawCopy();
        tl->SetLineColor(2);
      	for(int i=0;i<nUsed;i++) tl->DrawLine(qdcToUse[i],min,qdcToUse[i],max);

        gr=new TGraph(qdcToUse.size(),&qdcToUse[0],&EtoUse[0]);
      	lin_c->cd(2);
      	gr->GetXaxis()->SetLimits(qdcToUse[nUsed-1]-qdcToUse[0]*0.1,qdcToUse[0]*1.1);
      	// gr->GetXaxis()->SetLimits(0,qdcToUse[0]*1.1);
      	gr->Draw("a*");
      	gr->GetXaxis()->SetRangeUser(qdcToUse[nUsed-1]-qdcToUse[0]*0.1,qdcToUse[0]*1.1);
      	// gr->GetXaxis()->SetRangeUser(0,qdcToUse[0]*1.1);
        gr->GetYaxis()->SetRangeUser(0,EtoUse[0]*1.1);
      	gr->SetTitle("Calibration");
      	gr->GetXaxis()->SetTitle("channel");
      	gr->GetYaxis()->SetTitle("energy (keV)");
      	gr->Fit("linFunc","qB");
      	gr->Draw("a*");
        gPad->Update();

        for(int i=0; i<=pol; i++) fitCoef.push_back(linFunc->GetParameter(i));
        out << chnID;
        for(int i=0; i<=pol; i++) out << "," << fitCoef[i];
        out << endl;
        if(!myOpt.Contains('Q')) {
          cout << "Fit result:" << endl;
          cout << "ChannelID  p0  p1  p2..." << endl;
          cout << chnID;
          for(int i=0; i<=pol; i++) cout << "  " << fitCoef[i];
          cout << endl;
        }
        lin_c->SaveAs(Form("qdc_%i_linearity.png",chnID));
        p0_h->Fill(chnID,fitCoef[0]);
        p1_h->Fill(chnID,fitCoef[1]);
        if(pol>=2) p2_h->Fill(chnID,fitCoef[2]);
        if(pol>=3) p3_h->Fill(chnID,fitCoef[3]);
        ordered.clear();
        qdcToUse.clear();
        EtoUse.clear();
        fitCoef.clear();

      } // end of 'readyToFit'
    } else if(!myOpt.Contains('Q')) cout << "No histogram." << endl; // end of 'if(histo->GetEntries()>100)'
    readyToFit=0;
  } // end of loop over channels
  out.close();

  // gROOT->SetBatch(0);

  TCanvas* coef_c=new TCanvas("coef_c","Calibration Coeffiecnts");
  coef_c->Divide(2,2);
  coef_c->cd(1);
  p0_h->Draw("");
  coef_c->cd(2);
  p1_h->Draw("");
  coef_c->cd(3);
  p2_h->Draw("");
  coef_c->cd(4);
  p3_h->Draw("");
  if(chn==-1 && answer!='q') coef_c->SaveAs("calibrationCoefficients.pdf");

  gSystem->cd("../");
} // end of LYSOcal

void Usage()
{
	cout << endl << "LYSOcal(TString filename, Double_t fitMin, Double_t fitMax, Double_t thresh=0.0005, Int_t nToFit=4, Int_t pol=2, TString myOpt=\"Q\") \n\n"
		<< "Use 'fitMin' and 'fitMax' to set range to fit 511 keV peak in 'histo'. \n"
		<< "Use 'nToFit' to limit number of peaks to use for linearity correction. \n"
    << "'nToFit=3' will skip the 1275 keV peak. 'nToFit=1' uses only 511 keV and fixes the intercept at 0. \n"
    << "'thresh' can be used to adjust minimum amplitude of peaks that will be fitted.\n"
    << "'Use pol=2 for a quadriatic fit, or pol=1 for linear fit.\n'"
    << "Use 'myOpt=\"I\"' to interactively select the correct peaks." << endl;
}

void usage() { Usage();}
void Help() { Usage();}
void help() { Usage();}

// ////////////////////////////////////////////////////
// //                                                //
// // Code to fit a Gausian plus a linear background //
// // to a histogram, over the specified range and   //
// // return a pointer to the fit function.          //
// // Intended to be called my other macros.         //
// //                                                //
// // J. Brown                                       //
// // 05/01/17                                       //
// //                                                //
// ////////////////////////////////////////////////////
//
//
// TFitResultPtr fitPeak2(TH1F* histo, double fitMin, double fitMax, TString myOpt="", TString opt="QS", TString gopt="") {
//
// //	ofstream out("output.dat");
//
//    if(!opt.Contains("S")) opt+="S";
//    if(myOpt.Contains("Q")) opt+="Q";
//
//     double entries=(double)histo->GetEntries();
// // Centroid
//     double centroid=(fitMax+fitMin)/2;
//     Int_t minFitBin=(Int_t)histo->GetXaxis()->FindBin(fitMin);
//     Int_t maxFitBin=(Int_t)histo->GetXaxis()->FindBin(fitMax);
// // Constant
//     double integral=(double)histo->Integral(minFitBin,maxFitBin);
//     double maxConst=integral*2;
//     double minConst=integral/100;
// // Width
//     double range=fitMax-fitMin;
//     double fwhm=range/4;
//     double maxWidth=range;
//     double minWidth=range/100;
//
// // Slope
// 		double slope = (histo->GetBinContent(maxFitBin)-histo->GetBinContent(minFitBin))/(fitMax-fitMin);
//     double slopeMin = -2*abs(slope);
// 		double slopeMax = 2*abs(slope);;
// // Offset
//     double offset = histo->GetBinContent(maxFitBin) - slope*fitMax;
//     double offMin=offset*0.1;
//     // double offMin=offset*-0.5;
//     double offMax=offset*2;
//
// // Fit Function
//     TF1* myFunc = new TF1("myFunc","[0]+[1]*x+[2]*exp(-(x-[3])**2/(2*[4]**2))",0,4095);
//     myFunc->SetParNames("Offset","Slope","Constant","Mean","Sigma");
//     if(myOpt.Contains("B")) {
//       myFunc->FixParameter(0,0);
//       myFunc->FixParameter(1,0);
//     } else {
//       myFunc->SetParLimits(0,offMin,offMax);
//       myFunc->SetParLimits(1,slopeMin,slopeMax);
//     }
//     myFunc->SetParLimits(2,minConst,maxConst);
//     myFunc->SetParLimits(3,fitMin,fitMax);
//     myFunc->SetParLimits(4,minWidth,maxWidth);
//     myFunc->SetParameters(offset,slope,integral,centroid,fwhm);
//     myFunc->SetParameters(1,0,integral,centroid,fwhm);
//
//
//   if(!myOpt.Contains("Q")) {
//      cout << endl << "//////////////////////////////////////////////////////" << endl;
//      cout << "Fitting: " << histo->GetTitle() << endl;
//      cout << "//////////////////////////////////////////////////////" << endl;
//   }
//   if(myOpt.Contains("V")){
//     cout << endl << "//////////////////////////////////////////////////////" << endl;
//     cout << "//////////////// Fit Parameter Limits ////////////////" << endl;
//     cout << "Parameter	Min	 	Max       Guess" << endl;
//     cout << "Offset:		" << offMin << "		"	<< offMax << "     " << offset << endl;
//     cout << "Slope:      	" << slopeMin << "   	" << slopeMax	<< "		" << slope << endl;
//     cout << "Constant:       " << minConst << "           " << maxConst << "       " << integral << endl;
//     cout << "Mean:		" << fitMin << "		" << fitMax << "       " << centroid << endl;
//     cout << "Sigma:		" << minWidth << "	 	" << maxWidth << "        " << fwhm << endl;
//     cout << "//////////////////////////////////////////////////////" << endl << endl;;
//   }
//
//   TFitResultPtr fitPtr = histo->Fit(myFunc,opt,gopt,fitMin,fitMax);
//   centroid=fitPtr->Parameter(3);
//   fwhm=2.35482*fitPtr->Parameter(4);
//   if(!myOpt.Contains("Q")) cout << "Centroid: " << centroid << ", FWHM: " << fwhm << " (" << fwhm/centroid*100 << "%)" << endl;
//
// 	histo->GetXaxis()->SetRangeUser(fitMin*0.75,fitMax*1.25);
// 	histo->Draw();
//
//   TF1* f1=new TF1("f1","pol1",fitMin,fitMax);
// 	f1->SetParameters(myFunc->GetParameter(0),myFunc->GetParameter(1));
// 	f1->SetLineColor(3);
// 	f1->Draw("same");
//
// 	TF1* f2=new TF1("f2","gaus",fitMin,fitMax);
// 	f2->SetParameters(myFunc->GetParameter(2),myFunc->GetParameter(3),myFunc->GetParameter(4));
// 	f2->SetLineColor(6);
// 	f2->Draw("same");
//
//   if(!myOpt.Contains("N")) {
//       TLatex *tl=new TLatex;
//       tl->SetTextSize(0.05);
//       tl->SetTextColor(2);
//       tl->SetNDC();
//       tl->DrawLatex(0.6,0.7,Form("Centroid = %.3f",centroid));
//       tl->DrawLatex(0.6,0.65,Form("Sigma = %.3f",fwhm/2.35));
//       tl->DrawLatex(0.6,0.6,Form("FWHM = %.3f %%",fwhm/centroid*100));
//     }
//
//    return fitPtr;
// }
//
// TFitResultPtr fitPeak2(TH1I* histo, double fitMin, double fitMax, TString myOpt="", TString opt="QS", TString gopt="") {
//
//   if(!myOpt.Contains("Q")) cout << endl << "Received TH1I. Cloning to a TH1F for fitting." << endl;
//
//    TString str=histo->GetName();
//    str+="_th1f";
//    TH1F* hist_th1f=(TH1F*)histo->Clone(str);
//
//    return fitPeak2(hist_th1f,fitMin,fitMax,myOpt,opt,gopt);
// }
//
// // Don't use this as this macro will typically be called within other macros and hence lead to conficts
// // due to multiple help() functions.
// // void help() {
// //
// //    cout << endl << "Fit a Gaussian plus linear background to a defined range in a histogram." << endl
// //         << "Will accept either TH1F or TH1I." << endl
// //         << endl << "	fitPeak2(TH1I* histo, double fitMin, double fitMax, TString myOpt, TString opt, TString gopt)" << endl
// // 	<< endl << "where 'histo' is you histogram to fit and 'fitMin' and 'fitMax' define the range over which to fit," << endl
// //   << "and opt is an option string. Currently myOpt='N' prevents printing of fit results on the canvas." << endl;
// //
// // }
// //
// // void Help() {
// // 	help();
// // }
// // void usage() {
// // 	help();
// // }
// // void Usage() {
// // 	help();
// // }
