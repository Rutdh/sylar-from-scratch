#include "crt.h"
#include "node.h"
#include "token.h"
#include "scanner.h"
#include "content.h"
#include "parser.h"
#include "scalar.h"
#include "sequence.h"
#include "map.h"

namespace YAML
{
	// the ordering!
	bool ltnode::operator ()(const Node *pNode1, const Node *pNode2) const
	{
		return *pNode1 < *pNode2;
	}

	Node::Node(): m_pContent(0), m_alias(false)
	{
	}

	Node::~Node()
	{
		Clear();
	}

	void Node::Clear()
	{
		delete m_pContent;
		m_pContent = 0;
		m_alias = false;
	}

	void Node::Parse(Scanner *pScanner, const ParserState& state)
	{
		Clear();

		ParseHeader(pScanner, state);

		// is this an alias? if so, it can have no content
		if(m_alias)
			return;

		// now split based on what kind of node we should be
		switch(pScanner->PeekToken().type) {
			case TT_SCALAR:
				m_pContent = new Scalar;
				m_pContent->Parse(pScanner, state);
				break;
			case TT_FLOW_SEQ_START:
			case TT_BLOCK_SEQ_START:
			case TT_BLOCK_ENTRY:
				m_pContent = new Sequence;
				m_pContent->Parse(pScanner, state);
				break;
			case TT_FLOW_MAP_START:
			case TT_BLOCK_MAP_START:
				m_pContent = new Map;
				m_pContent->Parse(pScanner, state);
				break;
		}
	}

	// ParseHeader
	// . Grabs any tag, alias, or anchor tokens and deals with them.
	void Node::ParseHeader(Scanner *pScanner, const ParserState& state)
	{
		while(1) {
			if(pScanner->IsEmpty())
				return;

			switch(pScanner->PeekToken().type) {
				case TT_TAG: ParseTag(pScanner, state); break;
				case TT_ANCHOR: ParseAnchor(pScanner, state); break;
				case TT_ALIAS: ParseAlias(pScanner, state); break;
				default: return;
			}
		}
	}

	void Node::ParseTag(Scanner *pScanner, const ParserState& state)
	{
		Token& token = pScanner->PeekToken();
		if(m_tag != "")
			throw ParserException(token.line, token.column, ErrorMsg::MULTIPLE_TAGS);

		m_tag = state.TranslateTag(token.value);

		for(unsigned i=0;i<token.params.size();i++)
			m_tag += token.params[i];
		pScanner->PopToken();
	}
	
	void Node::ParseAnchor(Scanner *pScanner, const ParserState& state)
	{
		Token& token = pScanner->PeekToken();
		if(m_anchor != "")
			throw ParserException(token.line, token.column, ErrorMsg::MULTIPLE_ANCHORS);

		m_anchor = token.value;
		m_alias = false;
		pScanner->PopToken();
	}

	void Node::ParseAlias(Scanner *pScanner, const ParserState& state)
	{
		Token& token = pScanner->PeekToken();
		if(m_anchor != "")
			throw ParserException(token.line, token.column, ErrorMsg::MULTIPLE_ALIASES);
		if(m_tag != "")
			throw ParserException(token.line, token.column, ErrorMsg::ALIAS_CONTENT);

		m_anchor = token.value;
		m_alias = true;
		pScanner->PopToken();
	}

	void Node::Write(std::ostream& out, int indent, bool startedLine, bool onlyOneCharOnLine) const
	{
		// write anchor/alias
		if(m_anchor != "") {
			if(m_alias)
				out << "*";
			else
				out << "&";
			out << m_anchor << " ";
			startedLine = true;
			onlyOneCharOnLine = false;
		}

		// write tag
		if(m_tag != "") {
			out << "!<" << m_tag << "> ";
			startedLine = true;
			onlyOneCharOnLine = false;
		}

		if(!m_pContent) {
			out << std::endl;
		} else {
			m_pContent->Write(out, indent, startedLine, onlyOneCharOnLine);
		}
	}

	// begin
	// Returns an iterator to the beginning of this (sequence or map).
	Node::Iterator Node::begin() const
	{
		if(!m_pContent)
			return Iterator();

		std::vector <Node *>::const_iterator seqIter;
		if(m_pContent->GetBegin(seqIter))
			return Iterator(seqIter);

		std::map <Node *, Node *, ltnode>::const_iterator mapIter;
		if(m_pContent->GetBegin(mapIter))
			return Iterator(mapIter);

		return Iterator();
	}

	// end
	// . Returns an iterator to the end of this (sequence or map).
	Node::Iterator Node::end() const
	{
		if(!m_pContent)
			return Iterator();

		std::vector <Node *>::const_iterator seqIter;
		if(m_pContent->GetEnd(seqIter))
			return Iterator(seqIter);

		std::map <Node *, Node *, ltnode>::const_iterator mapIter;
		if(m_pContent->GetEnd(mapIter))
			return Iterator(mapIter);

		return Iterator();
	}

	// size
	// . Returns the size of this node, if it's a sequence node.
	// . Otherwise, returns zero.
	unsigned Node::size() const
	{
		if(!m_pContent)
			return 0;

		return m_pContent->GetSize();
	}

	const Node& Node::operator [] (unsigned u) const
	{
		if(!m_pContent)
			throw BadDereference();

		Node *pNode = m_pContent->GetNode(u);
		if(pNode)
			return *pNode;

		return GetValue(u);
	}

	const Node& Node::operator [] (int i) const
	{
		if(!m_pContent)
			throw BadDereference();

		Node *pNode = m_pContent->GetNode(i);
		if(pNode)
			return *pNode;

		return GetValue(i);
	}

	///////////////////////////////////////////////////////
	// Extraction

	void operator >> (const Node& node, std::string& s)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(s);
	}

	void operator >> (const Node& node, int& i)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(i);
	}

	void operator >> (const Node& node, unsigned& u)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(u);
	}

	void operator >> (const Node& node, long& l)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(l);
	}

	void operator >> (const Node& node, float& f)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(f);
	}

	void operator >> (const Node& node, double& d)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(d);
	}

	void operator >> (const Node& node, char& c)
	{
		if(!node.m_pContent)
			throw;

		node.m_pContent->Read(c);
	}

	std::ostream& operator << (std::ostream& out, const Node& node)
	{
		node.Write(out, 0, false, false);
		return out;
	}

	int Node::Compare(const Node& rhs) const
	{
		// Step 1: no content is the smallest
		if(!m_pContent) {
			if(rhs.m_pContent)
				return -1;
			else
				return 0;
		}
		if(!rhs.m_pContent)
			return 1;

		return m_pContent->Compare(rhs.m_pContent);
	}

	bool operator < (const Node& n1, const Node& n2)
	{
		return n1.Compare(n2) < 0;
	}
}