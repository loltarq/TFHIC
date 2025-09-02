void readCSV() {
    // Open the CSV file
    std::ifstream file("./../blastwave/data/data_TableIV.csv");
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    // value, bin to bin uncertainity, systematic uncertainty
    double beta_t[3];
    double Tkin[3];
    double n_profile[3];
    double chisquare_dof;

    int centrality_class;
    std::string centrality;

    // Create a ROOT file and a TTree to hold the data
    TFile *f = new TFile("data_TableIV.root", "RECREATE");
    TTree *tree = new TTree("tree", "Table IV Data");

    // Branches in the TTree
    tree->Branch("Class", &centrality_class);
    tree->Branch("Centrality", &centrality);
    tree->Branch("beta_t", &beta_t[0]);
    tree->Branch("beta_t_btb", &beta_t[1]);
    tree->Branch("beta_t_sys", &beta_t[2]);
    tree->Branch("Tkin", &Tkin[0]);
    tree->Branch("Tkin_btb", &Tkin[1]);
    tree->Branch("Tkin_sys", &Tkin[2]);
    tree->Branch("n_profile", &n_profile[0]);
    tree->Branch("n_profile_btb", &n_profile[1]);
    tree->Branch("n_profile_sys", &n_profile[2]);
    tree->Branch("chi^2/dof", &chisquare_dof);

    std::string line;
    // Read each line of the CSV file
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;

        // Read the columns (assuming comma-separated)
        std::getline(ss, cell, ',');
        centrality_class = std::stoi(cell);

        std::getline(ss, cell, ',');
        centrality = cell;

        for (int i = 0; i < 3; i++)
        {
            std::getline(ss, cell, ',');
            beta_t[i] = std::stod(cell);
        }

        for (int i = 0; i < 3; i++)
        {
            std::getline(ss, cell, ',');
            Tkin[i] = std::stod(cell);
        }

        for (int i = 0; i < 3; i++)
        {
            std::getline(ss, cell, ',');
            n_profile[i] = std::stod(cell);
        }

        std::getline(ss, cell, ',');
        chisquare_dof = std::stod(cell);

        // Fill the tree with the data
        tree->Fill();
    }

    // Write the TTree to the file and close
    tree->Write();
    f->Close();

    std::cout << "done!" << std::endl;
}
