vector<string> ConfigParameters;
//string OutputFolder;
//string ParticleListFile;
//string ParticleConfigFile;
//string ResonanceDecayScheme;

vector<double> k_scan_values;
//correlation volume values

bool toGCE = false;

bool LoadConfigFile(string ConfigFile)
{

    ifstream inf(ConfigFile.c_str());

    if (!inf.good())
    {
        return false;
    }   
    else
    {
        string line;
        while(getline(inf, line))
        {
            if (line.size() == 0)
            {
                break;
            }

            if (line.find(",") == string::npos)
            {
                ConfigParameters.push_back(line);
            }
            else
            {
                istringstream iss(line);
                string substring;
                while(getline(iss,substring,','))
                {
                    k_scan_values.push_back(stod(substring));
                }
            }
        }

        inf.close();

        if (ConfigParameters.size() == 0 || k_scan_values.size() == 0)
        {
            cout << "Config vectors empty." << endl;
            return false;
        }
    }
    return true;
}

void LoadParticleRef(vector<int>& pdg1, vector<int>& pdg2, vector<string>& name1, vector<string>& name2, string input="", string path = "../conf/")
{
    ifstream inf((path + input).c_str());
    string line;

    if(inf.good())
    {
        while(getline(inf, line))
        {
                if(line.size() == 0)
                    break;

                istringstream iss(line);
                string substring;
                vector<string> substrings;
                while(getline(iss,substring,','))
                {
                    substrings.push_back(substring);
                }

                name1.push_back(substrings[0]);
                pdg1.push_back(stoi(substrings[1]));              
                name2.push_back(substrings[2]);            
                pdg2.push_back(stoi(substrings[3]));
        }
        inf.close();
    }
    else
        cout << "Can't open or find particle analysis config file" << endl << endl;

}


string dtos(double d) {
    std::stringstream ss;
    ss << d;

    return ss.str();
}

void GraphFromDatNew()
{

    string ConfigFile = "../conf/_AnalysisConfig.config";

    if (!LoadConfigFile(ConfigFile))
    {
        cout << "Couldn't load analysis config file - execution aborted." << endl;
        return;
    }
    cout << "Config file successfully loaded." << endl << endl;

    string InputFolder = ConfigParameters[0];
    string ParticleList = ConfigParameters[2];
    string Ensemble = ConfigParameters[4];
    ConfigParameters.clear();

    cout << "Config parameters - input dat folder, list of particles to be analyzed: " << endl;
    cout << InputFolder << endl;
    cout << ParticleList << endl;
    cout << Ensemble << endl;

    cout << "Correlation volume scan values: ";
    for (auto& item : k_scan_values)
        cout << item << " ";
    cout << endl << endl << endl;

    vector<int> pdgs1;
    vector<int> pdgs2;
    vector<string> names1;
    vector<string> names2;

    LoadParticleRef(pdgs1,pdgs2,names1,names2,ParticleList);


    std::vector<string> format;

    for (int i = 0; i<pdgs1.size(); i++)
    {
        std::string subformat1 = "%lg", subformat2 = "%*lg";
        std::string subformat = subformat1 + " " + subformat2;
        for (int j = 0; j < i; j++)
        {
            subformat += " " + subformat2;
        }
        subformat += " " + subformat1;
        format.push_back(subformat);
    }

    vector<string> filenames;

    TSystemDirectory dir(InputFolder.c_str(), InputFolder.c_str());
    TList *files = dir.GetListOfFiles();
    if (files)
    {
        TSystemFile *file;
        TIter next(files);
        while ((file=(TSystemFile*)next()))
        {
            string filename = string(file->GetName());
            if (!file->IsDirectory())
            {
                bool isConfigEnsemble = filename.find("_" + Ensemble) != string::npos;
                if (toGCE == true && isConfigEnsemble)
                {
                    filenames.push_back(filename);
                }
                else if (isConfigEnsemble)
                {
                    if (filename.find("_toGCE") == string::npos)
                        filenames.push_back(filename);
                }
            }

        }
    }
    delete files;

    TCanvas * canvas = new TCanvas("Canvas", "Canvas");

    int graphNum = k_scan_values.size();

    canvas -> Divide(1,1);

    for (int j=0; j<1; j++)
    {
        canvas -> cd(j+1);
        auto legend = new TLegend(0.55, 0.2, 0.9, 0.4);
        TMultiGraph * mg = new TMultiGraph();
        TGraph *g[graphNum];
        for (int k=0; k<graphNum; k++)
        {
            g[k] = new TGraph((InputFolder + filenames[k]).c_str(),format[j].c_str());
            //g[k] -> SetLineWidth(3);
            g[k] -> SetLineStyle(k);
            g[k] -> SetLineColor(k+2);
            std::string legendEntry = "Ratio @ k = ";
            std::string kstring;
            for (auto kvalue : k_scan_values)
            {
                if (filenames[k].find("_k"+dtos(kvalue)) != string::npos)
                    kstring = dtos(kvalue);
            }  
            legend -> AddEntry(g[k], (legendEntry + kstring).c_str(), "l");
            mg -> Add(g[k]);
        }
        mg->Draw("AC");
        TLine *l = new TLine(0,1,2000,1);
        l->SetLineStyle(10);
        l->Draw();
        mg->SetTitle((names1[j] + "/" + names2[j] + (toGCE ? " to GCE" : "")).c_str());
        mg->GetXaxis()->SetTitle("dNch/deta");
        if (toGCE)
            mg->GetYaxis()->SetTitle("Ratio to GCE limit");
        else
            mg->GetYaxis()->SetTitle("Ratio");
        mg->GetXaxis()->SetLimits(0.1,5000);
        gPad->SetLogx();

        legend -> SetHeader(Ensemble.c_str());
        //legend -> SetX1NDC(0.6);
        //legend -> SetX2NDC(1.);
        legend -> Draw();
        //gPad->BuildLegend();
        //mg->GetXaxis()->SetRangeUser(0.01,4000);
        //gPad->SetGrid();
        gPad->Modified();
        //gPad->Update();
    }
    
    canvas->SaveAs("Canvas.root");
    canvas->SaveAs("Canvas.png");

}
    