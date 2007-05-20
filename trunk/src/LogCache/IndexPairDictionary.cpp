#include "StdAfx.h"
#include ".\indexpairdictionary.h"

#include "DiffIntegerInStream.h"
#include "DiffIntegerOutStream.h"

///////////////////////////////////////////////////////////////
// begin namespace LogCache
///////////////////////////////////////////////////////////////

namespace LogCache
{

///////////////////////////////////////////////////////////////
// CIndexPairDictionary::CHashFunction
///////////////////////////////////////////////////////////////

// simple construction

CIndexPairDictionary::CHashFunction::CHashFunction 
	( CIndexPairDictionary* aDictionary)
	: data (&aDictionary->data)
{
}

///////////////////////////////////////////////////////////////
// CIndexPairDictionary
///////////////////////////////////////////////////////////////

// construction / destruction

CIndexPairDictionary::CIndexPairDictionary(void)
	: hashIndex (CHashFunction (this))
{
}

CIndexPairDictionary::~CIndexPairDictionary(void)
{
}

// dictionary operations

void CIndexPairDictionary::reserve (index_t min_capacity)
{
	data.reserve (min_capacity);
	hashIndex.reserve (min_capacity);
}

index_t CIndexPairDictionary::Find (const std::pair<index_t, index_t>& value) const
{
	return hashIndex.find (value);
}

index_t CIndexPairDictionary::Insert (const std::pair<index_t, index_t>& value)
{
	assert (Find (value) == NO_INDEX);

	index_t result = (index_t)data.size();
	hashIndex.insert (value, (index_t)result);
	data.push_back (value);

	return result;
}

index_t CIndexPairDictionary::AutoInsert (const std::pair<index_t, index_t>& value)
{
	index_t result = Find (value);
	if (result == NO_INDEX)
		result = Insert (value);

	return result;
}

void CIndexPairDictionary::Clear()
{
	data.clear();
	hashIndex.clear();
}

// stream I/O

IHierarchicalInStream& operator>> ( IHierarchicalInStream& stream
								  , CIndexPairDictionary& dictionary)
{
	// read the first elements of all pairs

	CDiffIntegerInStream* firstStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CIndexPairDictionary::FIRST_STREAM_ID));

	index_t count = firstStream->GetValue();
	dictionary.data.resize (count);

	for (index_t i = 0; i < count; ++i)
		dictionary.data[i].first = firstStream->GetValue();

	// read the second elements

	CDiffIntegerInStream* secondStream 
		= dynamic_cast<CDiffIntegerInStream*>
			(stream.GetSubStream (CIndexPairDictionary::SECOND_STREAM_ID));

	for (index_t i = 0; i < count; ++i)
		dictionary.data[i].second = secondStream->GetValue();

	// build the hash (ommit the empty string at index 0)

	dictionary.hashIndex 
		= quick_hash<CIndexPairDictionary::CHashFunction>
			(CIndexPairDictionary::CHashFunction (&dictionary));
	dictionary.hashIndex.reserve (dictionary.data.size());

	for (index_t i = 0; i < count; ++i)
		dictionary.hashIndex.insert (dictionary.data[i], i);

	// ready

	return stream;
}

IHierarchicalOutStream& operator<< ( IHierarchicalOutStream& stream
								   , const CIndexPairDictionary& dictionary)
{
	size_t size = dictionary.data.size();

	// write string data

	CDiffIntegerOutStream* firstStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CIndexPairDictionary::FIRST_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));

	firstStream->Add ((int)size);
	for (size_t i = 0; i != size; ++i)
		firstStream->Add (dictionary.data[i].first);

	// write offsets

	CDiffIntegerOutStream* secondStream 
		= dynamic_cast<CDiffIntegerOutStream*>
			(stream.OpenSubStream ( CIndexPairDictionary::SECOND_STREAM_ID
								  , DIFF_INTEGER_STREAM_TYPE_ID));

	for (size_t i = 0; i != size; ++i)
		secondStream->Add (dictionary.data[i].second);

	// ready

	return stream;
}

///////////////////////////////////////////////////////////////
// end namespace LogCache
///////////////////////////////////////////////////////////////

}
