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
#pragma once

#include <vector>
#include <cassert>

template <typename CharT, typename DataT>
class trie
{
public:
	trie() = default;

	template<typename ...ArgsT>
	trie(ArgsT... args)
	: mHasData(true)
	, mData{std::forward<ArgsT>(args)}
	{};

	void SetData(const DataT &aData)
	{
		mHasData = true;
		mData = aData;
	};

	void Insert(const CharT *aPath, const DataT &aData, unsigned int aSkipsAllowed = 0) noexcept
	{
		switch (*aPath)
		{
		case '*':
			Insert(aPath + 1, aData, -1);
			break;
		case '?':
			Insert(aPath + 1, aData, 1);
			break;
		
		default:
			auto &tNewNode = AddChild(*aPath, aSkipsAllowed);

			if (*aPath == '\0')
			{
				// This means a rule is overrideing data. (Duplicate rule)
				assert(!tNewNode.mHasData);

				tNewNode.SetData(aData);
			}
			else
			{
				tNewNode.Insert(aPath + 1, aData, 0);
			}
		}
	};

	
	bool Match(const CharT *aPath, DataT **aDataPtr, bool aAllowPartialMatch = true) const
	{
		bool tMatchFound = false;
		const trie * tNullCharNode = nullptr;

		for (const NodeEntry &tEntry : mNodes)
		{
			unsigned int tSkipCount = 0;
			unsigned int tAllowedSkips = tEntry.skips;
			do
			{
				if (tEntry.key == aPath[tSkipCount])
				{
					// We found a path. Check if we reached the end of the string
					if (aPath[tSkipCount] =='\0')
					{
						if (tEntry.node.mHasData)
						{
							*aDataPtr = &(tEntry.node.mData);
						}

						return tEntry.node.mHasData;
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
				if (aAllowPartialMatch && tEntry.key =='\0')
				{
					tNullCharNode = &(tEntry.node);
				}
			} while (aPath[tSkipCount] !='\0' &&
				     ++tSkipCount <= tAllowedSkips);
		}

		if (aAllowPartialMatch && tNullCharNode && tNullCharNode->mHasData)
		{
			// If none our children provided an exact match and we currently
			// have data at the current node, then use it as the fallback match
			// if the user set aAllowPartialMatch to true. 
			// This would cause a rule for c:\windows to match c:\windows\system32\cmd.exe
			// if no specific rule for cmd.exe was found.

			// use it as the fallback match
			*aDataPtr = &(tNullCharNode->mData);
			return true;
		}
		
		return false;
	};

private:

	trie& AddChild(const CharT &aKey, unsigned int &aSkips) noexcept
	{
		// Find a child node if one already exists
		for (auto &tEntry : mNodes)
		{
			if (tEntry.key == aKey && tEntry.skips == aSkips)
			{
				return tEntry.node;
			}
		}

		// Add the child if none already exist	
		mNodes.emplace_back(aSkips, aKey);

		return mNodes.back().node;
	};


	struct NodeEntry
	{
		NodeEntry(unsigned int aSkips, CharT aKey)
			:	skips(aSkips),
				key(aKey)
		{};

		unsigned int skips;
		CharT		 key;
		trie node;
	};

	bool		mHasData{false};
	mutable DataT		mData;
	std::vector<NodeEntry>	mNodes;
};