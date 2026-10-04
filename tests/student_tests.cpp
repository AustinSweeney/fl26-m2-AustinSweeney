#include "aiws/chunker.hpp"
#include "aiws/context_builder.hpp"
#include "aiws/processing_core.hpp"
#include "aiws/retrieval_engine.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include <string>


// Student-written M2 tests
//
// Add your own tests to this file. Your tests are part of the submitted work
// and are evaluated under Student-Written Testing & Validation.
//
// Do not modify tests/public_tests.cpp.
//
// Your tests should exercise important M2 behavior beyond the supplied public
// tests. Consider default compatibility, custom strategies, runtime dispatch,
// invalid configuration, ownership/move behavior, and component interactions.

namespace tests
{
	int failures = 0;
	
	void check(bool condition, const std::string& message)
	{
		if (!condition)
		{
			++failures;
			std::cerr << "failure: " << message << "\n";
		}
	}
}


class TestRetrieval : public aiws::RetrievalStrategy
{
public:
	std::vector<aiws::SearchResult> search(
		const std::string&,
		int,
		const std::vector<aiws::Chunk>& chunks,
		const aiws::CorpusIndex&) const override
	{
		if (chunks.empty())
		{
			return {};
		}
		aiws::SearchResult result;
		result.chunk_id = chunks.front().id;
		result.score = 999.0;

		return { result };
	}
};

int main()
{
	using namespace aiws;
	using namespace tests;
	{
		ProcessingCore core;
		Workspace workspace;
		workspace.add_document(Document("doc1", "test document", "hello world"));

		core.rebuild(workspace);

		check(core.chunk_count() == 1, "default core should create chunk");

		check(!core.search("hello", 1).empty(), "default search should find an instance of hello");


	}

	{
		ProcessingCore core(std::make_unique<Chunker>(),
			std::make_unique<TestRetrieval>(),
			std::make_unique<ContextBuilder>());

		Workspace workspace;

		workspace.add_document(Document("doc1", "test document", "hello world"));

		core.rebuild(workspace);

		auto result = core.search("anything", 1);

		check(!result.empty() && result.front().score == 999.0, "custom retrival should be used");

	}

	bool threw = false;

	try
	{
		ProcessingCore core(
			nullptr,
			std::make_unique<RetrievalEngine>(),
			std::make_unique<ContextBuilder>());
	}

	catch (const std::invalid_argument&)
	{
		threw = true;
	}

	check(threw, "null should be rejected");

	if (failures == 0)
	{
		std::cout << "All tests passed" << std::endl;
		return 0;
	}
	return 1;

}

