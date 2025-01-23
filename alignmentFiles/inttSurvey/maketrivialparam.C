void maketrivialparam()
{
    // open localAlignmentParasFile.txt, read only
    // ifstream infile("localAlignmentParamsFile.txt");
    ifstream infile("/sphenix/user/frawley/production/macros/TrackingProduction/july29_repo_localAlignmentParamsFile.txt");
    if (!infile.is_open())
    {
        cout << "Error: cannot open localAlignmentParamsFile.txt" << endl;
        return;
    }

    // create a new file
    ofstream outfile("localAlignmentParamsFile_trivial.txt");
    if (!outfile.is_open())
    {
        cout << "Error: cannot open localAlignmentParamsFile_trivial.txt" << endl;
        return;
    }

    // read the ifstream line by line
    // the format of the file is:
    // 16 -0.0008566 0.0004851 -0.0001525 0.0051194 0.0274591 0.0197684
    string line;
    while (getline(infile, line))
    {
        // split the line by space
        istringstream iss(line);
        vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};

        cout << "tokens.size() = " << tokens.size() << endl;
        for (int i = 0; i < tokens.size(); i++)
        {
            cout << "tokens[" << i << "] = " << tokens[i] << endl;
        }
        // change all numbers to 0 except the first one and write to the new file
        for (int i = 0; i < tokens.size(); i++)
        {
            if (i == 0)
            {
                outfile << tokens[i] << " ";
            }
            else
            {
                outfile << "0 ";
            }
            // break line
            if (i == tokens.size() - 1)
            {
                outfile << endl;
            }
        }
    }

    // close the files
    infile.close();
    outfile.close();
}