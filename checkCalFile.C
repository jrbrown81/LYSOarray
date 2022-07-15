void checkCalFile(TString calFileName="calFile.csv")
{
// Read calibration file
	string line, word;
	Int_t col=0;
	ifstream calFile(calFileName);

	Int_t chn=-1;
	Int_t nCalChns=0;
	vector<Double_t> calCoef[1024];
	Int_t size=0;
	// for(int i=0;i<1024;i++) calCoef[i][1]=0;

	if(calFile.is_open()){
		cout << "Reading calibration from file: " << calFileName << endl;
		while(getline(calFile,line)){
			// cout << line << endl;
			nCalChns++;
			istringstream iss(line);
			while(getline(iss,word,',')) {
				if(col==0) {
					chn=(Int_t)stoi(word);
					if(calCoef[chn].size()!=0) cerr << "Warning! Channel " << chn << " is being read in multiple times." << endl;
					// cout << word;
				}
				else {
					calCoef[chn].push_back((Double_t)stof(word));
					// cout << " " << word;
				}
				// else calCoef[chn][col-1]=(Double_t)stof(word);
				col++;
			}
			// cout << endl;
			chn=-1;
			col=0;
		}
	} else cout << "Calibtaion file: " << calFileName << " not found!" << endl;
	// for(int i=0;i<1024;i++) {
	// 	if(calibrated[i]==1) {
	// 		cout << i << " ";
	// 		for(int j=0;j<3;j++) cout << calCoef[i][j] << " ";
	// 		cout << endl;
	// 	}
	// }

  TCanvas* calCoef_2c=new TCanvas("calCoef_2c","Calibration Coefficients 2D");
	calCoef_2c->Divide(2,3);
  TCanvas* calCoef_c=new TCanvas("calCoef_c","Calibration Coefficients");
	calCoef_c->Divide(2,3);
	TCanvas* calHist_c=new TCanvas("calHist_c","Calibration Coefficients Distributions");
	calHist_c->Divide(2,3);
  TCanvas* calCheck_c=new TCanvas("calCheck_c","Calibration Checks");
	calCheck_c->Divide(1,2);
  TH1I* nPol_h=new TH1I("nPol_h","Polynomial Order of calibration",10,0,10);
  TH1I* nPolVsChn_h=new TH1I("nPolVsChn_h","Polynomial Order of each channel",1024,0,1024);
  TH1F* calCoef_h[5];
  TH1F* calHist_h[5];
	calHist_h[0]=new TH1F(Form("calHist%i_h",0),Form("p%i",0),1000,-1000,1000);
	calHist_h[1]=new TH1F(Form("calHist%i_h",1),Form("p%i",1),1000,-100,100);
	calHist_h[2]=new TH1F(Form("calHist%i_h",2),Form("p%i",2),1000,-5,5);
	calHist_h[3]=new TH1F(Form("calHist%i_h",3),Form("p%i",3),1000,-0.5,0.5);
	calHist_h[4]=new TH1F(Form("calHist%i_h",4),Form("p%i",4),1000,-0.002,0.002);

	TH2F* calCoef_2h[5];
	calCoef_2h[0]=new TH2F(Form("calCoef%i_2h",0),Form("p%i",0),1024,0,1024,1000,-1000,1000);
	calCoef_2h[1]=new TH2F(Form("calCoef%i_2h",1),Form("p%i",1),1024,0,1024,1000,-100,100);
	calCoef_2h[2]=new TH2F(Form("calCoef%i_2h",2),Form("p%i",2),1024,0,1024,1000,-5,5);
	calCoef_2h[3]=new TH2F(Form("calCoef%i_2h",3),Form("p%i",3),1024,0,1024,1000,-0.5,0.5);
	calCoef_2h[4]=new TH2F(Form("calCoef%i_2h",4),Form("p%i",4),1024,0,1024,1000,-0.002,0.002);

  // TH1F* p_h[5];
	for(int i=0;i<5;i++) {
		calCoef_h[i]=new TH1F(Form("calCoef%i_h",i),Form("p%i",i),1024,0,1024);
		// p_h[i]=new TH1F(Form("p%i_h",i),Form("p%i",i),1024,0,1024);
	}

	// cout << "Chn	p0	p1	p2	p3	p4" << endl;
  for(int i=0;i<1024;i++) {

		size=calCoef[i].size();
		// if(size!=0) {
		// 	cout << i;
		// 	for(int j=0;j<size;j++) cout << "	" << calCoef[i][j];
		// 	cout << endl;
		// }

		if(size<=5) {
			for(int j=0;j<size;j++) {
				calCoef_h[j]->Fill(i,calCoef[i][j]);
				calHist_h[j]->Fill(calCoef[i][j]);
				calCoef_2h[j]->Fill(i,calCoef[i][j]);
			}
		}
		else {
			for(int j=0;j<5;j++) calCoef_h[j]->Fill(i,calCoef[i][j]);
			cerr << "Warning! " << size << " calibration coefficients found for channel " << i << endl;
		}

    nPolVsChn_h->Fill(i,size);
    nPol_h->Fill(size);
  }
	calCheck_c->cd(1);
	nPol_h->Draw();
	calCheck_c->cd(2);
	nPolVsChn_h->Draw("hist");

	for(int i=0;i<5;i++) {
		calCoef_c->cd(i+1);
		calCoef_h[i]->Draw("hist");
		calHist_c->cd(i+1);
		calHist_h[i]->Draw();
		calCoef_2c->cd(i+1);
		calCoef_2h[i]->Draw("");
	}


	cout << nCalChns << " calibration channels read." << endl;
}
