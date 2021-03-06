/*  egret.cpp: entry point into EGRET engine

    Copyright (C) 2016  Eric Larson and Anna Kirk
    elarson@seattleu.edu

    This file is part of EGRET.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "NFA.h"
#include "ParseTree.h"
#include "Scanner.h"
#include "Stats.h"
#include "TestGenerator.h"
#include "error.h"

using namespace std;

static bool debug_mode = false;
static bool stat_mode = false;

vector <string>
run_engine(string regex, string base_substring, bool debug = false, bool stat = false)
{
  vector <string> test_strings;

  // process arguments
  debug_mode = debug;
  stat_mode = stat;
  
  // clear warnings
  clearWarnings();

  try {

    // check base_substring
    if (base_substring.length() < 2) {
      throw EgretException("ERROR: Base substring must have at least two letters");
    }
    for (unsigned int i = 0; i < base_substring.length(); i++) {
      if (!isalpha(base_substring[i])) {
        throw EgretException("ERROR: Base substring can only contain letters");
      }
    }

    // initialize scanner with regex
    Scanner scanner;
    scanner.init(regex);
  
    // build parse tree
    ParseTree tree;
    tree.build(scanner);

    // build NFA
    NFA nfa;
    nfa.build(tree);

    // generate tests
    TestGenerator gen(nfa, base_substring, tree.get_punct_marks());
    test_strings = gen.gen_test_strings();
    
    // print debug info
    if (debug_mode) {
      cout << "RegEx: " << regex << endl;
      scanner.print();
      tree.print();
      nfa.print();
    }

    // print stats
    Stats stats;
    if (stat_mode) {
      scanner.add_stats(stats);
      tree.add_stats(stats);
      nfa.add_stats(stats);
      gen.add_stats(stats);
      stats.print();
    }
  }
  catch (EgretException const &e) {
    vector <string> result;
    result.push_back(e.getError());
    return result;
  }

  // Add warnings to front of list.
  string warnings = getWarnings();
  if (warnings == "") warnings = "SUCCESS";

  test_strings.insert(test_strings.begin(), warnings);

  return test_strings;
}
