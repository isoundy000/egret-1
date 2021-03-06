/*  RegexLoop.cpp: represents a regex repeat quantifier

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
#include <set>
#include <string>
#include "RegexLoop.h"
#include "StringPath.h"
using namespace std;

StringPath
RegexLoop::get_substring()
{
  // Adds the loop substring a sufficient number of times if the lower
  // bound is greater than 1.
  StringPath substring;
  for (int j = 1; j < repeat_lower; j++) {
    substring.add_path(curr_substring);
  }

  return substring;
}

void
RegexLoop::process_min_iter_string(StringPath *min_iter_string)
{
  if (repeat_lower != 0) {
    min_iter_string->add_path(get_substring());
  }
  else {
    int max_keep = min_iter_string->path.size() - path_substring.path.size();
    int num_to_remove = min_iter_string->path.size() - max_keep;
    
    for(int i = 0; i < num_to_remove; i++) {
      if(!min_iter_string->path.empty()) {
	min_iter_string->path.pop_back();
      }
    }
  }
}

void
RegexLoop::process_begin_loop(StringPath prefix, bool processed)
{
  curr_prefix.clear();
  curr_prefix = prefix;
  if (!processed) path_prefix = prefix;
}

void
RegexLoop::process_end_loop(StringPath prefix, bool processed)
{
  curr_substring.clear();
  vector <StringPath>::iterator path_iter;

  for (unsigned int i = curr_prefix.path.size(); i != prefix.path.size(); i++) {
    curr_substring.path.push_back(prefix.path[i]);
  }
  if (!processed) path_substring = curr_substring;
}

set <StringPath, spcompare>
RegexLoop::gen_evil_strings(StringPath path_string)
{
  set <StringPath, spcompare> evil_strings;
  StringPath path_suffix;
  int min = path_prefix.path.size() + path_substring.path.size();
  int max = path_string.path.size();
  for(int i = min; i < max; i++) {
    path_suffix.path.push_back(path_string.path[i]);
  }
  StringPath one_less_string = path_prefix;
  one_less_string.add_path(path_suffix);
  StringPath one_more_string = path_prefix;
  one_more_string.add_path(path_substring);
  one_more_string.add_path(path_substring);
  one_more_string.add_path(path_suffix);

  if (repeat_upper != -1) {

    // For cases like {n}, add strings for one less (n-1) and one more (n+1).
    if (repeat_lower == repeat_upper) {
      evil_strings.insert(one_less_string);
      evil_strings.insert(one_more_string);
    }
    else {
      // Handle one less on lower bound (note if lower bound is zero, the path
      // has one iteration so one less iteration will get us to zero iterations)
      evil_strings.insert(one_less_string);

      // Add enough path elements to get to the upper bound (note if lower bound
      // is zero, the path has one iteration so the starting point is bumped to one).
      // The variable path_elements is initialized to path_substring since path_suffix
      // has one substring less than lower bound.
      int base_iterations = repeat_lower;
      if (base_iterations == 0) base_iterations = 1;
      StringPath path_elements = path_substring;
      for (int i = base_iterations; i < repeat_upper; i++) {
        path_elements.add_path(path_substring);
      }

      // Add the upper bound string.
      StringPath upper_bound_string = path_prefix;
      upper_bound_string.add_path(path_elements);
      upper_bound_string.add_path(path_suffix);
      evil_strings.insert(upper_bound_string);

      // Add the string with one more iteration past the upper bound.
      StringPath past_bound_string = path_prefix;
      past_bound_string.add_path(path_elements);
      past_bound_string.add_path(path_substring);
      past_bound_string.add_path(path_suffix);
      evil_strings.insert(past_bound_string);
    } 
  }

  else {    // repeat_upper == -1 (no limit)
    // If lower bound is 0 or 1, add one less (zero) and add one more (two).  Want
    // to have one case that has repeated (two) elements.
    if (repeat_lower == 0 || repeat_lower == 1) {
      evil_strings.insert(one_less_string);
      evil_strings.insert(one_more_string);
    }
    // Otherwise, only add the string with one less iteration than the lower bound.
    else {
      evil_strings.insert(one_less_string);
    }
  }

  return evil_strings;
}

void
RegexLoop::print()
{
  if (repeat_lower == 0 && repeat_upper == -1)
    cout << "*";
  else if (repeat_lower == 1 && repeat_upper == -1)
    cout << "+";
  else if (repeat_lower == 0 && repeat_upper == 1)
    cout << "?";
  else if (repeat_upper == -1)
    cout << "{" << repeat_lower << ",}";
  else if (repeat_lower == repeat_upper)
    cout << "{" << repeat_lower << "}";
  else 
    cout << "{" << repeat_lower << "," << repeat_upper << "}";
}
