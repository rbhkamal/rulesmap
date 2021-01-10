//-----------------------------------------------------------------------------
// Copyright (c) 2021 Ribhi Kamal - rbhkamal@gmail.com
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <functional>
#include <regex>

#include <trie.h>

void TestCorrectness()
{
    enum class RuleId
    {
        Unknown,
        Home,
        OtherUsers,
        Tmp,
        RootUser,
        Bins,
        Libs,
        Java,
        Default,
    };

    
    std::vector<std::pair<const char *, RuleId>> orderedRules =
    {
        {"/"         , RuleId::Default},
        {"/home/user", RuleId::Home},
        {"/tmp"      , RuleId::Tmp},
        {"/home/*/"  , RuleId::OtherUsers},
        {"/root"     , RuleId::RootUser},
        {"/*java"    , RuleId::Java}, // Added before bin to give it a higher priority
        {"/*/bin"    , RuleId::Bins},
        {"/*/lib"    , RuleId::Libs},
        {"/*/lib64"  , RuleId::Libs}
    };
    
    // Create and Populate the trie with some rules and associate data
    trie<char, RuleId> path2RuleTrie;

    for (auto &tRule : orderedRules)
    {
        path2RuleTrie.Insert(tRule.first , tRule.second);
    }

    // The path that we are testing and which rule ID it should map to when matched against
    // the rules above
    std::vector<std::pair<const char *, RuleId>> tests =
    {
        {"/home/user/Desktop/test.sh", RuleId::Home},
        {"/home/otheruser/Desktop/test.sh",RuleId::OtherUsers},
        {"/test.sh",RuleId::Default},
        {"/tmp/test.sh",RuleId::Tmp},
        {"/local/usr/bin/test.sh",RuleId::Bins},
        {"/local/usr/lib/test.so",RuleId::Libs},
        {"/root/test.sh",RuleId::RootUser},
        {"/opt/jre_123/bin/java",RuleId::Java},
        {"/opt/java/testing",RuleId::Default},
        {"blah blah blah",RuleId::Unknown}
    };

    // Perform lookups
    for (auto &test : tests)
    {
        RuleId *matchRuleId = nullptr;

        bool testPassed = false;
        if (path2RuleTrie.Match(test.first, &matchRuleId))
        {
            if (test.second == *matchRuleId)
            {
                testPassed = true;
            }
            else
            {
                std::cout << "Test path " << test.first << " expected " << static_cast<int>(test.second) << " but received " << static_cast<int>(*matchRuleId) << std::endl;
            }
        }
        else
        {
            // This is only OK if the test indicates that there should be no match
            if (test.second == RuleId::Unknown)
            {
                testPassed = true;
            }
            else
            {
                std::cout << "Test path " << test.first << " expected a match but none were found" << std::endl;
            }
            
        }

        std::wcout << std::left  <<  std::setw(50) << test.first
                   << std::right << (testPassed? "passed" : "failed")
                   << std::endl;
                   
    }
}

std::chrono::duration <double, std::nano> TimeIt1k(const std::function<void()>& func) noexcept
{
    std::chrono::steady_clock::time_point tEnd, tStart;

    tStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        func();
    }
    tEnd = std::chrono::high_resolution_clock::now();

    return std::chrono::duration <double, std::nano> ((tEnd - tStart)/1000);
}



// An example from the internet which is likely what everyone will use for
// wilcard support.
bool matchInternetImpl(const char *rule, const char * second) 
{ 
    // If we reach at the end of both strings, we are done 
    if (*rule == '\0' && *second == '\0') 
        return true; 
  
    // Make sure that the characters after '*' are present 
    // in second string. This function assumes that the first 
    // string will not contain two consecutive '*' 
    if (*rule == '*' && *(rule+1) != '\0' && *second == '\0') 
        return false; 
  
    // If the first string contains '?', or current characters 
    // of both strings match 
    if (*rule == '?' || *rule == *second) 
        return matchInternetImpl(rule+1, second+1); 
  
    // If there is *, then there are two possibilities 
    // a) We consider current character of second string 
    // b) We ignore current character of second string. 
    if (*rule == '*') 
        return matchInternetImpl(rule+1, second) || matchInternetImpl(rule, second+1); 
    return false; 
}


bool matchInternetWrapperImpl(const std::string& rule, const char * second)
{
    return matchInternetImpl(rule.c_str(), second);
}

bool regexImpl(const std::regex& rule, const char * lookupPath) 
{
    return std::regex_match (lookupPath, rule);
}

template <typename RuleT>
int SequentialSearch(
    const std::vector<std::pair<RuleT, int>>& rulesVector,
    const std::string& lookupPath,
    const std::function<bool (const RuleT& rule, const char* path)>& matchImpl) noexcept
{
    for (const auto &rule : rulesVector)
    {
        if (matchImpl(rule.first, lookupPath.c_str()))
        {
            return rule.second;
        }
    }

    std::cerr << "Sequential search failed for " << typeid(RuleT).name() << " with path " << lookupPath << std::endl;
    std::abort();

    return -1;
}

void TestPerformance()
{
    trie<char, int> rulesTrie;
    std::vector<std::pair<std::string, int>> rulesVector;
    std::vector<std::pair<std::regex, int>> rulesAsRegExVector;

    std::cout << "Generating rules" << std::endl;
    std::string longPath;
    for (int i = 0; i < 1000; ++i)
    {
        longPath.append("/a");
    }

    // Rules are stored from most specific (longest) to least specific (shortest)
    std::string generatedRule = longPath;
    int id = 0;
    while (!generatedRule.empty())
    {
        std::string ruleWithWildCard = generatedRule + "*ing";
        std::regex ruleWithRegEx(generatedRule + ".*ing");

        rulesTrie.Insert(ruleWithWildCard.c_str(), id);
        rulesVector.emplace_back(ruleWithWildCard, id);
        rulesAsRegExVector.emplace_back(std::move(ruleWithRegEx), id);

        auto lastSlash = generatedRule.find_last_of('/');
        if(lastSlash != std::string::npos)
        {
            // Discard the slash and everything to the right of it
            generatedRule = generatedRule.substr(0, lastSlash);

            ++id;
        }
        else
        {
            break;
        }
    }

    /*
     The rules now look like this (rule -> rule id):
     /a/a/a/a/a/a/a/a/a/a --> 1
     /a/a/a/a/a/a/a/a/a --> 2
     /a/a/a/a/a/a/a/a --> 3
      ....
     /a/a/a --> 998
     /a/ ->  999
    */
     

    // Perform lookups
    
    std::wcout << std::left  << std::setw(20) << "Vector Time (Ms)"
                   << std::left << std::setw(20) << "RegEx Time (Ms)"
                   << std::left << std::setw(20) << "Trie Time (Ms)"
                   << std::left << std::setw(20) << "Path Length"
                   << std::endl;

    std::string testPath = longPath + "/testing";
   

    // Create the functions that we want to test
    // Trie Tests
    auto trieTime = [&]{int *tOutputRulePtr; rulesTrie.Match(testPath.c_str(), &tOutputRulePtr); };
    // Wild Card Tests (sample internet solution)
    auto internetWildcardTime = [&]{  SequentialSearch<std::string>(rulesVector, testPath, std::bind(matchInternetWrapperImpl, std::placeholders::_1, std::placeholders::_2));};
    // Regex
    auto regexTime = [&]{  SequentialSearch<std::regex>(rulesAsRegExVector, testPath, std::bind(regexImpl, std::placeholders::_1, std::placeholders::_2));  };

    while (!longPath.empty())
    {
        std::wcout << std::left  << std::setw(20) << std::chrono::duration <double, std::micro> (TimeIt1k(internetWildcardTime)).count()
                   << std::left << std::setw(20) << std::chrono::duration <double, std::micro> (TimeIt1k(regexTime)).count()
                   << std::left << std::setw(20) << std::chrono::duration <double, std::micro> (TimeIt1k(trieTime)).count()
                   << std::left << std::setw(20) << longPath.size()
                   << std::endl;

        auto lastSlash = longPath.find_last_of('/');
        if(lastSlash != std::string::npos)
        {
            // Discard the slash and everything to the right of it
            longPath = longPath.substr(0, lastSlash);

            testPath = longPath + "/testing";
        }
        else
        {
            break;
        }
    }
}


int main()
{
    std::wcout << "Correctness test:" << std::endl;
    TestCorrectness();
    std::wcout << "-----------------" << std::endl;
    std::wcout << "Peformance test:" << std::endl;
    TestPerformance();
    std::wcout << "-----------------" << std::endl;
}