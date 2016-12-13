#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <exception>
#include <iomanip>
#include <cstring>

namespace BOT_SyntaxParser
{
	struct Grammar
	{
		// S, N, T
		std::string startSymbol;
		std::unordered_set<std::string> nonTerminalSymbols;
		std::unordered_set<std::string> terminalSymbols;

		// NonTerminalSymbol -> { Production-Rule }
		// Production-Rule = [TerminalSymbol | NonTerminalSymbol]*
		std::unordered_map<
			std::string,
			std::vector<std::vector<std::string>>
		> productionRules;

		Grammar (std::istream &is)
		{
			static const std::string productionSymbol ("->");
			static const std::string whiteChar (" ");
			static const std::string pipeLine ("|");

			char buffer[1024];
			while (is.getline (buffer, 1024))
			{
				std::string productionRule (buffer);
				if (productionRule.empty ())
					continue;

				// Fix for Linux :-(
				if (productionRule.back () == '\r')
					productionRule.pop_back ();
				if (productionRule.empty ()) continue;

				// Split nonTerminal
				std::string nonTerminal = SplitStr (productionRule,
													productionSymbol);

				// Trim nonTerminal
				while (true)
				{
					auto seg = SplitStr (nonTerminal, whiteChar);
					if (!seg.empty ())
					{
						nonTerminal = std::move (seg);
						break;
					}
				}

				// Split productionRule
				while (!productionRule.empty ())
				{
					auto statement = SplitStr (productionRule, pipeLine);
					std::vector<std::string> symbolsOfStatement;
					while (!statement.empty ())
					{
						auto symbol = SplitStr (statement, whiteChar);
						if (!symbol.empty ())
							symbolsOfStatement.emplace_back (std::move (symbol));
					}
					productionRules[nonTerminal]
						.emplace_back (std::move (symbolsOfStatement));
				}

				// Set Start Symbol
				if (startSymbol.empty ())
					startSymbol = std::move (nonTerminal);
			}

			GetSymbols ();
		}

	protected:
		void GetSymbols ()
		{
			nonTerminalSymbols.clear ();
			terminalSymbols.clear ();

			auto isNT = [this] (const std::string &symbol)
			{
				return nonTerminalSymbols.find (symbol)
					!= nonTerminalSymbols.end ();
			};

			// NonTerminalSymbols
			for (const auto &rule : productionRules)
				nonTerminalSymbols.emplace (rule.first);

			// TerminalSymbols
			for (const auto &rule : productionRules)
				for (const auto &statement : rule.second)
					for (const auto &symbol : statement)
						if (!isNT (symbol))
							terminalSymbols.emplace (symbol);
		}

		std::string SplitStr (std::string &inputStr,
							  const std::string &delim)
		{
			size_t pos = 0;
			if ((pos = inputStr.find (delim, 0)) != std::string::npos)
			{
				auto ret = inputStr.substr (0, pos);
				inputStr.erase (0, pos + delim.size ());
				return std::move (ret);
			}

			// the Last one
			auto ret = std::move (inputStr);
			inputStr = "";
			return std::move (ret);
		}
	};

	class LR1Grammar : public Grammar
	{
	public:
		// lhs -> { productionRule } with dot-Position, lookAhead
		struct Item
		{
			std::string lhs;
			std::vector<std::string> productionRule;
			size_t dotPos;
			std::string lookAhead;

			bool operator == (const Item &op) const
			{
				return lhs == op.lhs &&
					productionRule == op.productionRule &&
					dotPos == op.dotPos &&
					lookAhead == op.lookAhead;
			}
		};

		struct ItemHasher
		{
			size_t operator() (const Item &obj) const
			{
				static const std::hash<std::string> hasher;
				return (hasher (obj.lhs) << 16) +
					hasher (obj.productionRule.front ()) +
					(obj.dotPos << 8);
			}
		};

		// Item Set
		using ItemSet = std::unordered_set<Item, ItemHasher>;

		// Item Sets
		std::vector<ItemSet> itemSets;

		using TablePair = std::pair<ItemSet, std::string>;

		enum class ActionType
		{
			reduce,
			shift_goTo
		};

		struct Action
		{
			ActionType type;

			// for Reduce / Accept
			std::pair<
				std::string,
				std::vector<std::string>
			> productionRule;

			// for Shift / Goto
			size_t state;
		};

		struct PairHasher
		{
			size_t operator() (
				const std::pair<size_t, std::string> &obj) const
			{
				// For msvc's ordering result :-)
				// Using Conflict to Aggregate entries with the Same State ^_^
				return obj.first;
			}
		};

		// ItemSet Index * (Lookahead / Lhs) -> Action
		std::unordered_map<
			std::pair<size_t, std::string>,
			Action, PairHasher
		> table;

		LR1Grammar (std::istream &is)
			: Grammar (is)
		{}

		void Augmenting ()
		{
			auto start = startSymbol + "'";
			productionRules[start].emplace_back (
				std::vector<std::string> { startSymbol });
			startSymbol = start;
			GetSymbols ();
		}

		void GetLR1Table ()
		{
			static const std::string epsilon ("epsilon");
			static const std::string dollar ("$");

			using FSet = std::unordered_map<
				std::string,
				std::unordered_set<std::string>
			>;
			FSet firstSet;

			auto placeToSet = [] (FSet &set,
								  const std::string &key,
								  const std::string &value)
			{
				if (set[key].find (value) != set[key].end ())
					return false;

				set[key].emplace (value);
				return true;
			};

			auto allToSet = [&placeToSet] (FSet &set,
										   const std::string &src,
										   const std::string &dst)
			{
				bool isChanged = false;
				for (const auto &symbol : set[src])
					// always skip epsilon
					if (symbol != epsilon)
						isChanged = isChanged || placeToSet (
							set, dst, symbol);
				return isChanged;
			};

			auto isT = [this] (const std::string &symbol)
			{
				return terminalSymbols.find (symbol)
					!= terminalSymbols.end ();
			};

			auto hasEpsilon = [] (
				const std::unordered_set<std::string> &set)
			{
				return set.find (epsilon) != set.end ();
			};

			// Get First Set for Terminals
			for (const auto &symbol : terminalSymbols)
				placeToSet (firstSet, symbol, symbol);

			// Get First Set for NonTerminals
			while (true)
			{
				auto isChanged = false;
				for (const auto &rule : productionRules)
				{
					const auto &key = rule.first;
					for (const auto &statement : rule.second)
					{
						size_t i = 0;
						for (; i < statement.size (); i++)
						{
							isChanged = isChanged || allToSet (
								firstSet, statement[i], key);

							// break if hasn't epsilon
							if (!hasEpsilon (firstSet[statement[i]]))
								break;
						}

						// if all has epsilon
						if (i == statement.size ())
							isChanged = isChanged || placeToSet (
								firstSet, key, epsilon);
					}
				}
				if (!isChanged) break;
			}

			auto getClosure = [&isT, &firstSet, this] (ItemSet &itemSet)
			{
				auto isUpdated = true;

				// Repeat until NO Changes
				while (isUpdated)
				{
					isUpdated = false;

					// Foreach Item in the ItemSet
					for (const auto &item : itemSet)
					{
						// Reduction Item
						if (item.dotPos == item.productionRule.size ())
							continue;

						const auto &leader = item.productionRule[item.dotPos];

						// Shift Item
						if (isT (leader))
							continue;

						// Reduction Expecting Item

						// Get New Look Ahead Set
						std::unordered_set<std::string> lookAheads;
						if (item.dotPos != item.productionRule.size () - 1)
						{
							// leader is not the last one
							// Get from the firstSet of the one after leader
							for (const auto lookAhead :
								 firstSet[item.productionRule[item.dotPos + 1]])
							{
								lookAheads.emplace (lookAhead);
							}
						}
						else
						{
							// leader is the last one
							// Get from this item
							lookAheads.emplace (item.lookAhead);
						}

						// Foreach productionRule of leader's productionRules
						for (const auto &productionRule : productionRules[leader])
						{
							// Foreach Look Ahead in New Look Ahead Set
							for (const auto &lookAhead : lookAheads)
							{
								Item newItem {
									leader, productionRule, 0, lookAhead
								};

								if (itemSet.find (newItem) == itemSet.end ())
								{
									itemSet.emplace (newItem);
									isUpdated = true;
								}
							}
						}
					}
				}
			};

			auto getGo = [this, &getClosure] (const ItemSet &itemSet,
											  const std::string &symbolToGo)
			{
				ItemSet newItemSet;
				for (const auto &item : itemSet)
				{
					// Reduction Item
					if (item.dotPos == item.productionRule.size ())
						continue;

					const auto &leader = item.productionRule[item.dotPos];

					// If leader is symbolToGo, Add it
					if (leader == symbolToGo)
					{
						auto newItem = item;
						newItem.dotPos++;
						newItemSet.emplace (newItem);
					}
				}

				// Get Closure of New Item Set
				getClosure (newItemSet);

				return newItemSet;
			};

			// Add Starting Item into the initial Item Set
			auto initItemSet = ItemSet { Item {
				startSymbol, productionRules[startSymbol].front (), 0, dollar
			} };

			// Get Closure for the initial Item Set
			getClosure (initItemSet);

			// Move the initial Item Set to the Item Sets
			itemSets.emplace_back (std::move (initItemSet));

			auto go = [this, &getGo] (const ItemSet &itemSet,
									  const std::string &symbol)
			{
				auto newItemSet = getGo (itemSet, symbol);

				// go (I, X) is Empty
				if (newItemSet.empty ())
					return -1;

				auto i = 0;
				for (; i < itemSets.size (); ++i)
				{
					const auto &preItemSet = itemSets[i];
					if (preItemSet == newItemSet)
						break;
				}

				// go (I, X) is not in Item Sets
				if (i == itemSets.size ())
					itemSets.emplace_back (std::move (newItemSet));

				// i is the Index of newItemSet
				return i;
			};

			std::pair<
				std::string,
				std::vector<std::string>
			> emptyProductionRule;

			// Foreach Item Set in Item Sets
			// Repeat until NO Change
			for (auto index = 0; index < itemSets.size (); ++index)
			{
				// Foreach Symbol
				for (const auto &symbol : nonTerminalSymbols)
				{
					auto goRet = go (itemSets[index], symbol);

					// Add shift / goto to Table
					if (goRet > 0)
						table.emplace (std::make_pair (index, symbol), Action {
							ActionType::shift_goTo, emptyProductionRule, (size_t) goRet
					});
				}
				for (const auto &symbol : terminalSymbols)
				{
					auto goRet = go (itemSets[index], symbol);

					// Add shift / goto to Table
					if (goRet > 0)
						table.emplace (std::make_pair (index, symbol), Action {
							ActionType::shift_goTo, emptyProductionRule, (size_t) goRet
					});
				}

				// Add reduce to Table
				for (const auto &item : itemSets[index])
				{
					if (item.dotPos == item.productionRule.size ())
						table.emplace (std::make_pair (index, item.lookAhead), Action {
							ActionType::reduce, std::make_pair (item.lhs, item.productionRule), 0
					});
				}
			}
		}
	};

	struct LexParser
	{
		std::vector<std::string> tokens;

		LexParser (std::istream &is,
				   const std::unordered_set<std::string> &puncSet)
		{
			while (true)
			{
				std::string seg;
				char ch;

				if (!is.get (ch))
					// Eof
					break;
				seg += ch;
				int nch = is.peek ();

				// Blank
				// Fix for Linux :-(
				if (ch == ' ' || ch == '\r')
					continue;

				// Number
				if (ch == '.' || isdigit (ch))
				{
					auto state = 0;
					if (ch == '.') state = 1;

					while (true)
					{
						auto isEnd = false;
						char nch = is.peek ();
						switch (state)
						{
							// (digit)+
						case 0:
							if (isdigit (nch))
								;
							else if (nch == '.')
								state = 2;
							else
								isEnd = true;
							break;
							// .
						case 1:
							if (isdigit (nch))
								state = 2;
							else
								isEnd = true;
							break;
							// (digit)* . (digit)+ | (digit)+ . (digit)*
						case 2:
							if (isdigit (nch))
								;
							else
								isEnd = true;
							break;
						}
						if (isEnd) break;

						// Get Char
						seg += (char) is.get ();
					}

					if (state == 0 || state == 2)
						tokens.emplace_back ("num");
					continue;
				}

				// Punc
				if (puncSet.find (seg) != puncSet.end ())
					tokens.emplace_back (seg);
			}
		}
	};

	class LR1Parser
	{
	public:
		LR1Parser (std::istream &is,
				   std::ostream &os)
			: _grammar (is)
		{
			_grammar.Augmenting ();
			PrintGrammar (os);

			_grammar.GetLR1Table ();
			PrintLR1ItemSets (os);
			PrintLR1Table (os);
		}

		void Parse (std::istream &is,
					std::ostream &os)
		{
			char buffer[1024];
			while (is.getline (buffer, 1024))
			{
				if (strlen (buffer) == 0 || buffer[0] == '\r')
					continue;

				os << "\nParse $" << buffer << "$ :\n";
				std::stringstream iStream (buffer);
				_Parse (iStream, os);
			}
		}

	private:
		LR1Grammar _grammar;

		void _Parse (std::istream &is,
					 std::ostream &os)
		{
			static const std::string dollar ("$");

			auto isNT = [this] (const std::string &symbol)
			{
				return _grammar.nonTerminalSymbols.find (symbol)
					!= _grammar.nonTerminalSymbols.end ();
			};

			auto isT = [this] (const std::string &symbol)
			{
				return _grammar.terminalSymbols.find (symbol)
					!= _grammar.terminalSymbols.end ();
			};

			std::vector<size_t> stateStack;
			std::vector<std::string> symbolStack;

			const auto &table = _grammar.table;
			std::vector<std::string> tokenFlow
				= LexParser (is, _grammar.terminalSymbols).tokens;

			// Init input & stack
			tokenFlow.emplace_back (dollar);
			stateStack.emplace_back (0);

			// Output
			std::vector<std::tuple<
				std::string, std::string, std::string
				>> output;
			size_t width1 = 5;  // 5 is preserved for title
			size_t width2 = 5;

			std::string stackStr;
			std::string inputStr;
			std::string actionStr;

			// Run
			try
			{
				for (auto pToken = tokenFlow.begin ();
					 pToken != tokenFlow.end ();)
				{
					auto state = stateStack.back ();
					const auto &token = *pToken;

					// Stack
					stackStr = std::to_string (stateStack.front ()) + " ";
					for (auto i = 0; i < symbolStack.size (); ++i)
					{
						stackStr += symbolStack[i] + " ";
						stackStr += std::to_string (stateStack[i + 1]) + " ";
					}
					if (width1 < stackStr.size ())
						width1 = stackStr.size ();

					// Input
					inputStr.clear ();
					for (auto p = pToken; p != tokenFlow.end (); ++p)
						inputStr += *p;
					if (width2 < inputStr.size ())
						width2 = inputStr.size ();

					// Action
					LR1Grammar::Action action;
					try
					{
						action = table.at (std::make_pair (state, token));
					}
					catch (...)
					{
						throw std::runtime_error (
							"No Action for (" + std::to_string (state) +
							", " + token + ")");
					}

					switch (action.type)
					{
					case LR1Grammar::ActionType::shift_goTo:
						// Output
						actionStr = "shift " + std::to_string (action.state);

						// Action
						stateStack.emplace_back (action.state);
						symbolStack.emplace_back (token);
						++pToken;
						break;

					case LR1Grammar::ActionType::reduce:
						if (action.productionRule.first == _grammar.startSymbol)
						{
							// Output
							actionStr = "accept";

							// Action
							pToken = tokenFlow.end ();
						}
						else
						{
							auto lhsReducedTo = action.productionRule.first;

							// Output
							actionStr = "reduce by " + lhsReducedTo + " -> ";
							for (int i = (int) action.productionRule.second.size () - 1; i >= 0; --i)
							{
								// Output
								actionStr += action.productionRule.second[i] + " ";
								if (symbolStack.back () != action.productionRule.second[i])
									throw std::runtime_error ("Top of Symbol Stack '" +
															  symbolStack.back () +
															  "' does not match '" +
															  action.productionRule.second[i]);
								// Action
								symbolStack.pop_back ();
								stateStack.pop_back ();
							}

							try
							{
								action = table.at (std::make_pair (
									stateStack.back (), lhsReducedTo));
							}
							catch (...)
							{
								throw std::runtime_error ("No Goto for (" +
														  std::to_string (stateStack.back ()) +
														  ", " +
														  lhsReducedTo +
														  ")");
							}

							if (action.type != LR1Grammar::ActionType::shift_goTo)
								throw std::runtime_error ("Bad Goto for (" +
														  std::to_string (stateStack.back ()) +
														  ", " +
														  lhsReducedTo +
														  ")");
							// Action
							stateStack.emplace_back (action.state);
							symbolStack.emplace_back (lhsReducedTo);
						}
						break;

					default:
						throw std::runtime_error ("WTF unknown action type in LR1 Table");
						break;
					}

					// Output
					output.emplace_back (stackStr, inputStr, actionStr);
				}
			}
			catch (const std::exception &ex)
			{
				actionStr = ex.what ();

				// Output
				output.emplace_back (stackStr, inputStr, actionStr);
			}

			os << std::setw (width1) << std::left
				<< "Stack" << " | "
				<< std::setw (width2) << std::left
				<< "Input" << " | "
				<< "Action" << std::endl;

			for (const auto &row : output)
			{
				os << std::setw (width1) << std::left
					<< std::get<0> (row) << " | "
					<< std::setw (width2) << std::right
					<< std::get<1> (row) << " | "
					<< std::get<2> (row) << std::endl;
			}
		}

		void PrintGrammar (std::ostream &os)
		{
			os << "\nGrammar:\n";

			os << "Starting Symbol:\n\t" << _grammar.startSymbol;

			os << "\nNon Terminal Symbols:\n\t";
			for (const auto &symbol : _grammar.nonTerminalSymbols)
				os << symbol << " ";

			os << "\nTerminal Symbols:\n\t";
			for (const auto &symbol : _grammar.terminalSymbols)
				os << symbol << " ";

			os << "\nProduction Rules:\n";
			for (const auto &rule : _grammar.productionRules)
			{
				os << "\t" << rule.first << " -> ";
				for (size_t i = 0; i < rule.second.size (); i++)
				{
					const auto &statement = rule.second[i];
					for (size_t j = 0; j < statement.size (); j++)
					{
						os << statement[j];
						if (j != statement.size () - 1)
							os << " ";
					}
					if (i != rule.second.size () - 1)
						os << " | ";
					else
						os << std::endl;
				}
			}
		}

		void PrintLR1ItemSets (std::ostream &os)
		{
			size_t index = 0;
			os << "\nLR1 Item Sets:\n";
			for (const auto &itemSet : _grammar.itemSets)
			{
				os << "Item Set " << index++ << ":\n";
				for (const auto &item : itemSet)
				{
					os << "    " << item.lhs << " -> ";
					for (size_t i = 0; i < item.productionRule.size (); i++)
					{
						if (i == item.dotPos)
							os << "~ ";
						os << item.productionRule[i] << " ";
					}
					if (item.dotPos == item.productionRule.size ())
						os << "~ ";

					os << "| " << item.lookAhead << std::endl;
				}
			}
		}

		void PrintLR1Table (std::ostream &os)
		{
			auto isNT = [this] (const std::string &symbol)
			{
				return _grammar.nonTerminalSymbols.find (symbol)
					!= _grammar.nonTerminalSymbols.end ();
			};

			os << "\nLR1 Table:\n";
			for (const auto &entry : _grammar.table)
			{
				os << "<" << std::setw (2) << std::right
					<< entry.first.first << ", "
					<< std::setw (5) << std::left
					<< entry.first.second << ">  =>  ";

				const auto &action = entry.second;
				switch (action.type)
				{
				case LR1Grammar::ActionType::reduce:
					if (action.productionRule.first == _grammar.startSymbol)
						os << "acc";
					else
					{
						os << "reduce by " << action.productionRule.first << " -> ";
						for (auto i = 0; i < action.productionRule.second.size (); ++i)
						{
							os << action.productionRule.second[i];
							if (i != action.productionRule.second.size ())
								os << " ";
							else
								os << std::endl;
						}
					}
					break;

				case LR1Grammar::ActionType::shift_goTo:
					if (isNT (entry.first.second))
						os << "goto " << action.state;
					else
						os << "shift " << action.state;
					break;

				default:
					throw std::runtime_error ("WTF unknown action type in LR1 Table");
					break;
				}
				os << std::endl;
			}
		}
	};
}

int main (int argc, char *argv[])
{
	try
	{
		using namespace BOT_SyntaxParser;

		if (argc < 3)
			throw std::runtime_error ("Too few Arguments");

		std::ifstream ifsGrammar (argv[1]);
		std::ofstream ofsGrammar (argv[1] + std::string (".output.txt"));
		std::ifstream ifsInput (argv[2]);
		std::ofstream ofsOutput (argv[2] + std::string (".output.txt"));

		if (!ifsGrammar.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[1]);
		if (!ofsGrammar.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[1] + ".output.txt");
		if (!ifsInput.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[2]);
		if (!ofsOutput.is_open ())
			throw std::runtime_error (std::string ("Unable to Open") +
									  argv[2] + ".output.txt");

		// Generate parser from ifsGrammar into ofsGrammar
		LR1Parser parser (ifsGrammar, ofsGrammar);

		// Parse ifs into ofs :-)
		parser.Parse (ifsInput, ofsOutput);
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what () << std::endl;
		return 1;
	}
	return 0;
}
