/*
Copyright 2019 Ribhi Kamal - rbhkamal@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions: The above copyright notice and 
this permission notice shall be included in all copies or substantial portions 
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef _RECURSIVE_PATH_TRIE_H
#define _RECURSIVE_PATH_TRIE_H

#include <vector>
#include <cassert>

template <typename Data_T>
class PathTrieNode
{
public:
	PathTrieNode() :
		m_HasData(false)
	{};

	PathTrieNode(int && a) :
		m_HasData(false)
	{};

	PathTrieNode(const Data_T &aData) :
		m_HasData(true),
		m_Data(aData)
	{};

	PathTrieNode &
	addChild(const wchar_t &aKey, unsigned int &aSkips)
	{
		PathTrieNode * tNode = NULL;

		// Find a child node if one already exists
		for (auto &tEntry : m_Nodes)
		{
			if (tEntry.key == aKey && tEntry.skips == aSkips)
			{
				tNode = &tEntry.node;
				break;
			}
		}

		// Add the child if none already exist
		if (tNode == NULL)
		{
			m_Nodes.emplace_back(aSkips, aKey);

			tNode = &(m_Nodes.back().node);
		}

		return *tNode;
	};

	void SetData(const Data_T &aData)
	{
		m_HasData = true;
		m_Data = aData;
	};

	void AddPath(const wchar_t *aPath, const Data_T &aData, unsigned int aSkipsAllowed = 0)
	{
		switch (*aPath)
		{
		case '*':
			AddPath(aPath + 1, aData, -1);
			break;
		case '?':
			AddPath(aPath + 1, aData, 1);
			break;
		
		default:
			auto &tNewNode = addChild(*aPath, aSkipsAllowed);

			if (*aPath == '\0')
			{
				// This means a rule is overrideing data. (Duplicate rule)
				assert(!tNewNode.m_HasData);

				tNewNode.SetData(aData);
			}
			else
			{
				tNewNode.AddPath(aPath + 1, aData, 0);
			}
		}
	};

	
	bool Match(const wchar_t *aPath, Data_T **aDataPtr, bool aAllowPartialMatch = true)// const
	{
		bool tMatchFound = false;
		PathTrieNode * tNullCharNode = NULL;

		for (NodeEntry &tEntry : m_Nodes)
		{
			unsigned int tSkipCount = 0;
			unsigned int tAllowedSkips = tEntry.skips;
			do
			{
				if (tEntry.key == aPath[tSkipCount])
				{
					// We found a path. Check if we reached the end of the string
					if (aPath[tSkipCount] == L'\0')
					{
						if (tEntry.node.m_HasData)
						{
							*aDataPtr = &(tEntry.node.m_Data);
						}

						return tEntry.node.m_HasData;
					}
					else
					{
						if (tEntry.node.Match(aPath + tSkipCount + 1, aDataPtr, aAllowPartialMatch))
						{
							// If we found a match, return now and skip iterating
							// over the rest of the nodes
							return true;
						}
					}
				}

				// We utilize this loop to do double work, search for a match
				// for for locating the null terminator entry (if any) to be
				// used below. We need to find that entry because it is the one
				// that contains the data.
				if (aAllowPartialMatch && tEntry.key == L'\0')
				{
					tNullCharNode = &(tEntry.node);
				}
			} while (aPath[tSkipCount] != L'\0' &&
				     ++tSkipCount <= tAllowedSkips);
		}

		if (aAllowPartialMatch && tNullCharNode && tNullCharNode->m_HasData)
		{
			// If none our children provided an exact match and we currently
			// have data at the current node, then use it as the fallback match
			// if the user set aAllowPartialMatch to true. 
			// This would cause a rule for c:\windows to match c:\windows\system32\cmd.exe
			// if no specific rule for cmd.exe was found.

			// use it as the fallback match
			*aDataPtr = &(tNullCharNode->m_Data);
			return true;
		}
		
		return false;
	};

	struct NodeEntry
	{
		NodeEntry(unsigned int aSkips, wchar_t aKey)
			:	skips(aSkips),
				key(aKey)
		{};

		unsigned int skips;
		wchar_t		 key;
		PathTrieNode node;
	};

	typedef std::vector<NodeEntry> Nodes;

	bool		m_HasData;
	Data_T		m_Data;
	Nodes		m_Nodes;
};


template <typename Data_T>
class PathTrie
{
public:
	PathTrie()
	{};
	virtual ~PathTrie()
	{};

	
	bool Match(const wchar_t *aPath, Data_T **aDataPtr)
	{
		return m_Root.Match(aPath, aDataPtr);
	};
	

	void AddPath(const wchar_t *aPath, const Data_T &aData)
	{
		return m_Root.AddPath(aPath, aData);
	};


private:


	PathTrieNode<Data_T> m_Root;
};

#endif // _RECURSIVE_PATH_TRIE_H