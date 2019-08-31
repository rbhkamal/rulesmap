#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <chrono>

#include "RecursivePathTrie.h"

void TestCorrectness()
{
    PathTrie<int> t1;

    std::vector<std::pair<const wchar_t *, int>> tRules =
    {
        {L"/"         , 0},
        {L"/home/user", 1},
        {L"/tmp"      , 2},
        {L"/home/*/"  , 3},
        {L"/root"     , 4},
        {L"/*/bin"    , 5},
        {L"/*/lib"    , 6}
    };

    std::vector<std::pair<const wchar_t *, int>> tTests =
    {
        {L"/home/user/Desktop/test.sh", 1},
        {L"/home/otheruser/Desktop/test.sh",3},
        {L"/test.sh",0},
        {L"/tmp/test.sh",2},
        {L"/local/usr/bin/test.sh",5},
        {L"/local/usr/lib/test.so",6},
        {L"/root/test.sh",4},
        {L"blah blah blah",-1}
    };

    // Populate the trie
    for (auto &tRule : tRules)
    {
        t1.AddPath(tRule.first , tRule.second);
    }

    // Perform lookups
    for (auto &tTest : tTests)
    {
        int *tRulePtr = nullptr;

        bool tPass = 
            t1.Match(tTest.first, &tRulePtr)? 
                tTest.second == *tRulePtr : 
                tTest.second == -1;

        std::wcout << std::left  <<  std::setw(50) << tTest.first
                   << std::right << (tPass? L"passed" : L"failed")
                   << std::endl;
                   
    }
}

void TestPerformance()
{
    PathTrie<std::wstring> t2;

    std::vector<const wchar_t *> tRules =
    {
        L"/"         , L"Default Rule",
        L"/home/user", L"User Profile Rule",
        L"/tmp"      , L"tmp folder rule",
        L"/home/*/"  , L"Other users profile",
        L"/root"     , L"Root user profile",
        L"/*/bin"    , L"Binaries",
        L"/*/lib"    , L"Libraries"
    };

    std::vector<const wchar_t *> tTests =
    {
        L"/home/user/Desktop/test.sh",
        L"/home/otheruser/Desktop/test.sh",
        L"/test.sh",
        L"/tmp/test.sh",
        L"/local/usr/bin/test.sh",
        L"/local/usr/lib/test.so",
        L"/root/test.sh",
        L"blah blah blah"
    };

    auto tEnd   = std::chrono::high_resolution_clock::now();
    auto tStart = std::chrono::high_resolution_clock::now();
    // Populate the trie
    for (int i = 0; i < tRules.size() - 1; i += 2)
    {
        t2.AddPath(tRules[i] , tRules[i+1]);
    }
    tEnd   = std::chrono::high_resolution_clock::now();

    std::wcout << L"Build Time: " 
             << std::chrono::duration <double, std::nano> (tEnd - tStart).count() << L"ns"
             << std::endl;

    // Perform lookups
    tStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < tTests.size(); ++i)
    {
        std::wstring *tRulePtr = nullptr;
        t2.Match(tTests[i], &tRulePtr);
    }
    tEnd   = std::chrono::high_resolution_clock::now();

    std::wcout << L"Lookup Time: " 
             << std::chrono::duration <double, std::nano> (tEnd - tStart).count()
             << L"ns"
             << std::endl;
             
}


int main(int argc, char **argv)
{
    std::wcout << L"Correctness test:" << std::endl;
    TestCorrectness();
    std::wcout << L"-----------------" << std::endl;
    std::wcout << L"Peformance test:" << std::endl;
    TestPerformance();
    std::wcout << L"-----------------" << std::endl;
}