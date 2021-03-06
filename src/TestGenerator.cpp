/*  TestGenerator.cpp: Generates paths and strings

    Copyright (C) 2016  Eric Larson and Anna Kirk
    elarson@seattleu.edu

    Some code in this file was derived from a RE->NFA converter
    developed by Eli Bendersky.

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

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>
#include "NFA.h"
#include "TestGenerator.h"
#include "Path.h"
#include "error.h"
#include <iostream>
#include "StringPath.h"
using namespace std;

vector <string>
TestGenerator::gen_test_strings()
{
  vector <string> ret_strings;
  vector <StringPath>::iterator it;
  paths = nfa.find_basis_paths();

  // gen initial strings
  gen_initial_strings();

  // gen evil backreference strings
  vector <int> backrefs_done;
  for(it = test_strings.begin(); it != test_strings.end(); it++) {
    vector <string> res = it -> gen_evil_backreference_strings(backrefs_done);
    vector <string>::iterator i;
    for(i = res.begin(); i != res.end(); i++) {
      if(!(std::find(ret_strings.begin(), ret_strings.end(), *i) != ret_strings.end()))
        ret_strings.insert(ret_strings.begin(), *i);
    }
  }
  
  // gen evil strings
  gen_evil_strings();
  // add evil strings to ret strings list
  for(it = test_strings.begin(); it != test_strings.end(); it++) {
    string s = it -> get_string();
    if(!(std::find(ret_strings.begin(), ret_strings.end(), s) != ret_strings.end()))
      ret_strings.insert(ret_strings.begin(), s);
  }
  return ret_strings;

}

void
TestGenerator::gen_initial_strings()
{
  bool all_start_with_caret = false;
  bool all_end_with_dollar = false;
  bool warn_anchor_middle = false;
  bool warn_caret_start = false;
  bool warn_dollar_end = false;
  bool warn_duplicate_character_set = false;
  
  vector <Path>::iterator path_iter;
  StringPath first_string;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {  // for each path
    // check for leading carets and trailing dollars
    bool start_with_caret = path_iter->has_leading_caret();
    bool end_with_dollar = path_iter->has_trailing_dollar();

    // go through each state in the path
    StringPath path_string;
    path_string.clear();
    path_string.add_path(path_iter->gen_initial_string(base_substring));
    add_to_test_strings(path_string);

    // for first path, record whether the path starts with ^ and/or ends with $
    if (first_string.path.empty()) {
      all_start_with_caret = start_with_caret;
      all_end_with_dollar = end_with_dollar;
      first_string = path_string;
    }

    // check for duplicate character sets
    if (path_iter->check_for_duplicate_character_sets()) {
      warn_duplicate_character_set = true;
    }

    // check for anchors in the middle
    string anchor_err = path_iter->check_anchor_middle();
    
    // process anchor warnings
    if (!warn_anchor_middle && anchor_err != "") {
      addWarning(anchor_err);
      warn_anchor_middle = true;
    }
    if (!warn_caret_start) {
      if (all_start_with_caret && !start_with_caret) {
        stringstream s;
        s << "ANCHOR WARNING: Some but not all strings start with a ^ anchor\n";
        s << "...String with ^ anchor:    " << first_string.get_string() << "\n";
        s << "...String with no ^ anchor: " << path_string.get_string();
        addWarning(s.str());
        warn_caret_start = true;
      }
      if (!all_start_with_caret && start_with_caret) {
        stringstream s;
        s << "ANCHOR WARNING: Some but not all strings start with a ^ anchor\n";
        s << "...String with ^ anchor:    " << path_string.get_string() << "\n";
        s << "...String with no ^ anchor: " << first_string.get_string();
        addWarning(s.str());
        warn_caret_start = true;
      }
    }
    if (!warn_dollar_end) {
      if (all_end_with_dollar && !end_with_dollar) {
        stringstream s;
        s << "ANCHOR WARNING: Some but not all strings end with a $ anchor\n";
        s << "...String with $ anchor:    " << first_string.get_string() << "\n";
        s << "...String with no $ anchor: " << path_string.get_string();
        addWarning(s.str());
        warn_dollar_end = true;
      }
      if (!all_end_with_dollar && end_with_dollar) {
        stringstream s;
        s << "ANCHOR WARNING: Some but not all strings end with a $ anchor\n";
        s << "...String with $ anchor:    " << path_string.get_string() << "\n";
        s << "...String with no $ anchor: " << first_string.get_string();
        addWarning(s.str());
        warn_dollar_end = true;
      }
    }
  }
  if(warn_duplicate_character_set) {
    stringstream s;
    s << "WARNING: Found duplicate character set";
    addWarning(s.str());
  }
}

void
TestGenerator::add_to_test_strings(StringPath s)
{
  test_strings.push_back(s);
}

void
TestGenerator::add_to_test_strings(set <StringPath, spcompare> strs)
{
  set <StringPath, spcompare>::iterator it;
  for (it = strs.begin(); it != strs.end(); it++) {
    StringPath ptr = *it;
    add_to_test_strings(ptr);
  }
}

void
TestGenerator::gen_evil_strings()
{
  vector <Path>::iterator path_iter;
  for (path_iter = paths.begin(); path_iter != paths.end(); path_iter++) {
    set <StringPath, spcompare> evil_strings = path_iter->gen_evil_strings(punct_marks);
    add_to_test_strings(evil_strings);
  }
}

void
TestGenerator::add_stats(Stats &stats)
{
  stats.add("PATHS", "Paths", paths.size());
  stats.add("PATHS", "Strings", test_strings.size());
}
