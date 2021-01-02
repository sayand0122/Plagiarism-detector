#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>   // a collection of functions designed to be used on ranges of elements.
#include <algorithm> //used to point at the memory addresses of STL container
#include <iterator>  // A type representing a directory stream
#include <dirent.h>  // associative containers that store elements in a mapped fashions
#include <map>
#include <sstream>
#include <iomanip>
using namespace std;

// Import text files with which we want to check from the SourceFile folder.
char const *SourceFile = "C:\\Users\\Sayan\\Desktop\\Codes\\Projects\\plagiarism-detector\\SourceFile";

// Import text files which we want to check from the Target folder.
char const *target_folder = "C:\\Users\\Sayan\\Desktop\\Codes\\Projects\\plagiarism-detector\\TargetFiles";

// Also import the stop words which have no value in the text.
char const *removedwords_file = "E:\\Plagiarism Detector\\removedwords.txt";

// initializing the maximum value of score variable
int score_accuracy = 1;

//Maximum number of tests
int number_of_tests = 3;

// calculating the dot product of the vectors.
float dot_product(vector<int> a, vector<int> b)
{
    float sum = 0;
    for (int i = 0; i < a.size(); i++)
        sum += a[i] * b[i];
    return sum;
}

float sum(vector<int> v)
{
    float sumv = 0;
    for (auto &n : v)
        sumv += n;
    return sumv;
}

float get_multiplier(string word)
{
    return word.length() * word.length();
}

// Function for Creating cosine score for tokens.
float cosine_score(vector<int> bvector, vector<int> tvector)
{
    return dot_product(bvector, tvector) /
           (sqrt(dot_product(bvector, bvector)) *
            sqrt(dot_product(tvector, tvector)));
}

// Comparing the file that do they ends with same length and with same ending.
bool endswith(string const &fullString, string const &ending)
{
    if (fullString.length() >= ending.length())
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    else
        return false;
}

// Removing the punctuation marks like ",", ";", etc.
void cleanString(string &str)
{
    size_t i = 0;
    size_t len = str.length();
    while (i < len)
    {
        if (!isalnum(str[i]) && str[i] != ' ')
        {
            str.erase(i, 1);
            len--;
        }
        else
            i++;
    }
}

// Getting the file from the given Path.
string getfile(string filepath)
{
    ifstream mFile(filepath);
    string output;
    string temp;

    while (mFile >> temp)
    {
        output += string(" ") + temp;
    }

    cleanString(output);
    return output;
}

// Find out the frequency of the words. we will compare both files base file and the target file and the find the frequency of each word in each file
map<string, int> get_frequency(vector<string> tokens)
{
    map<string, int> freqs;
    for (auto const &x : tokens)
        ++freqs[x];
    vector<string> unique_tokens;
    vector<int> freq_token;
    for (auto const &p : freqs)
    {
        unique_tokens.push_back(p.first);
        freq_token.push_back(p.second);
    }

    return freqs;
}

// Implementation of string to vector conversion. We will convert the string to the vector so that we can we can do tests on that vectors.
vector<string> string_to_token(string str)
{
    istringstream mstream(str);
    return vector<string>(istream_iterator<string>{mstream}, istream_iterator<string>{});
}

// Calculating the NGran score using NGram testing
float ngram_score(vector<string> base, vector<string> target, int n)
{
    vector<vector<string>> bngrams;
    vector<vector<string>> tngrams;
    vector<string> temp;

    for (int i = 0; i <= base.size() - n; i++)
    {
        temp.clear();
        for (int j = i; j < i + n; j++)
            temp.push_back(base[j]);
        bngrams.push_back(temp);
    }

    for (int i = 0; i <= target.size() - n; i++)
    {
        temp.clear();
        for (int j = i; j < i + n; j++)
            temp.push_back(target[j]);
        tngrams.push_back(temp);
    }

    int shared = 0;
    int total = tngrams.size();

    for (auto const &tngram : tngrams)
        for (auto const &bngram : bngrams)
            if (tngram == bngram)
            {
                shared += 1;
                break;
            }

    return 1.0 * shared / total;
}

// Here we are implementing the tokenized test. where we will compare the frequency of each word present in both the files.
float tokenize_test(vector<string> b_tokens, vector<string> t_tokens)
{
    ifstream infile(removedwords_file);
    string stopword;
    while (infile >> stopword)
    {
        t_tokens.erase(remove(t_tokens.begin(), t_tokens.end(), stopword), t_tokens.end());
    }

    auto t_freqs = get_frequency(t_tokens);
    auto b_freqs = get_frequency(b_tokens);

    int shared = 0;
    int total = 0;

    for (auto const &word : t_freqs)
    {
        auto search = b_freqs.find(word.first);
        if (search != b_freqs.end())
        {
            shared += min(word.second, search->second) * get_multiplier(word.first);
            total += word.second * get_multiplier(word.first);
        }
        else
        {
            total += word.second * get_multiplier(word.first);
        }
    }
    float score = 10.0 * shared / total;

    return score;
}

// Here we are implementing the NGram test where we will compare the tokens in each files.
float ngram_test(vector<string> b_tokens, vector<string> t_tokens)
{
    vector<int> tests{3, 5, 7};
    vector<int> weights{3, 5, 7};

    vector<float> ngresults;

    ngresults.push_back(ngram_score(b_tokens, t_tokens, 3));
    ngresults.push_back(ngram_score(b_tokens, t_tokens, 5));
    ngresults.push_back(ngram_score(b_tokens, t_tokens, 7));

    float score = 10 * pow((ngresults[0] * weights[0] + ngresults[1] * weights[1] + ngresults[2] * weights[2]) / sum(weights), 0.4);
    return score;
}

// Implementation of cosine_test
float cosine_test(vector<string> b_tokens, vector<string> t_tokens)
{
    ifstream infile(removedwords_file);
    string stopword;
    while (infile >> stopword)
    {
        t_tokens.erase(remove(t_tokens.begin(), t_tokens.end(), stopword), t_tokens.end());
        b_tokens.erase(remove(b_tokens.begin(), b_tokens.end(), stopword), b_tokens.end());
    }

    vector<string> all_tokens;
    all_tokens.reserve(t_tokens.size() + b_tokens.size());
    all_tokens.insert(all_tokens.end(), t_tokens.begin(), t_tokens.end());
    all_tokens.insert(all_tokens.end(), b_tokens.begin(), b_tokens.end());
    sort(all_tokens.begin(), all_tokens.end());
    all_tokens.erase(unique(all_tokens.begin(), all_tokens.end()), all_tokens.end());

    auto t_freqs = get_frequency(t_tokens);
    auto b_freqs = get_frequency(b_tokens);

    vector<int> b_vector;
    vector<int> t_vector;

    for (auto &token : all_tokens)
    {
        auto search = b_freqs.find(token);
        if (search != b_freqs.end())
        {
            b_vector.push_back(search->second);
        }
        else
        {
            b_vector.push_back(0);
        }

        search = t_freqs.find(token);
        if (search != t_freqs.end())
        {
            t_vector.push_back(search->second);
        }
        else
        {
            t_vector.push_back(0);
        }
    }

    float score = 10.0 * cosine_score(b_vector, t_vector);

    return score;
}

// Getting Verdict and printing the output.
void get_verdict(vector<float> t, vector<string> m)
{
    vector<int> weights(t.size(), 0);

    /**************************
        test1 - Tokenized test
        test2 - NGram test
        test3 - Cosine test
    ***************************/

    weights[0] = 3;
    weights[1] = 4;
    weights[2] = 3;

    float final_score = 10.0 * (t[0] * weights[0] + t[1] * weights[1] + t[2] * weights[2]) / sum(weights);
    string verdict;

    // Getting the value of verdict which will tell how much the file is plagiarized.
    if (final_score < 10)
        verdict = "Not Plagiarised";
    else if (final_score < 50)
        verdict = "Slightly Plagiarised";
    else if (final_score < 80)
        verdict = "Fairly Plagiarised";
    else
        verdict = "Highly Plagiarised";

    m.erase(remove(m.begin(), m.end(), ""), m.end());
    sort(m.begin(), m.end());
    m.erase(unique(m.begin(), m.end()), m.end());

    cout << "********************************************" << endl;
    cout << "\tFinal score: " << final_score << endl;
    cout << "\tVerdict: " << verdict << endl;
    if (verdict != "Not Plagiarised")
    {
        cout << "\tMatch found in:";
        if (m.size() == 0)
            cout << "\t-nil-" << endl;
        for (auto const &file : m)
            cout << "\t" << file << endl
                 << endl;
    }

    cout << "********************************************" << endl;
}

// main function
int main()
{
    DIR *dir;
    DIR *dirB;
    struct dirent *dir_object;

    string target_file;
    string base_file;

    string target;
    string base;

    float temp;

    // Getting the file from the given directory.
    if ((dir = opendir(target_folder)) != NULL)
    {
        while ((dir_object = readdir(dir)) != NULL)
            if (endswith(string(dir_object->d_name), ".txt"))
            {
                printf("\nPlagiarism scores for %s\n \n", dir_object->d_name);
                target_file = target_folder + string("/") + dir_object->d_name;

                target = getfile(target_file);

                vector<float> test(number_of_tests, 0.0);
                vector<string> match(number_of_tests, "");

                if ((dirB = opendir(SourceFile)) != NULL)
                {
                    while ((dir_object = readdir(dirB)) != NULL)
                        if (endswith(string(dir_object->d_name), ".txt"))
                        {
                            base_file = SourceFile + string("/") + dir_object->d_name;

                            base = getfile(base_file);

                            auto b_tokens = string_to_token(base);
                            auto t_tokens = string_to_token(target);

                            temp = tokenize_test(b_tokens, t_tokens);
                            if (test[0] < temp)
                            {
                                test[0] = temp;
                                match[0] = dir_object->d_name;
                            }
                            temp = ngram_test(b_tokens, t_tokens);
                            if (test[1] < temp)
                            {
                                test[1] = temp;
                                match[1] = dir_object->d_name;
                            }
                            temp = cosine_test(b_tokens, t_tokens);
                            if (test[2] < temp)
                            {
                                test[2] = temp;
                                match[2] = dir_object->d_name;
                            }
                        }
                    closedir(dirB);
                }

                cout << "Test 1 score: " << fixed << setprecision(score_accuracy) << test[0] * 10 << "%" << endl
                     << endl;
                cout << "Test 2 score: " << fixed << setprecision(score_accuracy) << test[1] * 10 << "%" << endl
                     << endl;
                cout << "Test 3 score: " << fixed << setprecision(score_accuracy) << test[2] * 10 << "%" << endl
                     << endl;

                get_verdict(test, match);

                cout << endl;
            }

        closedir(dir);
    }
}