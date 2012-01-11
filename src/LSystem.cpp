#include "LSystem.h"
using std::cout;
using std::string;
using std::vector;
using std::map;
using std::make_pair;
using std::basic_string;

LSystem::LSystem(){
}

LSystem::~LSystem(){
}

const LSystem::RecursionElement& LSystem::generate(int recursion_depth) {

    int curr_depth = recursions_.size();

    if(curr_depth <= recursion_depth ) {

        vector<int> rec_dep;
        for(unsigned int i = 0; i < axiom_.size(); i++)
            rec_dep.push_back(0);

        if (curr_depth == 0 && recursion_depth == 0) {
            recursions_.push_back(make_pair(axiom_, rec_dep));
            return recursions_[0];
        } else if (curr_depth == 0) {
            recursions_.push_back(make_pair(axiom_, rec_dep));
            curr_depth = recursions_.size();
        }

        string next_recursion;
        vector<int> next_recursion_dep;

        map<char, string>::iterator it;

        for(int recursion = curr_depth; recursion <= recursion_depth; ++recursion ) {
            string curr_recursion = recursions_.back().first;
            vector<int> curr_recursion_dep = recursions_.back().second;

            next_recursion = "";
            next_recursion_dep.clear();

            for(unsigned int i = 0; i < curr_recursion.size(); ++i) {
                 it = rules_.find(curr_recursion[i]);
                 int tmp = curr_recursion_dep[i];

                 if(it != rules_.end()) {
                     next_recursion += (*it).second;
                     for(unsigned int i = 0; i < (*it).second.size(); i++)
                        next_recursion_dep.push_back(tmp+1);

                 } else {
                     next_recursion += curr_recursion[i];
                     next_recursion_dep.push_back(tmp);
                 }
            }
            recursions_.push_back(make_pair(next_recursion, next_recursion_dep));
        }
    }
    return recursions_[recursion_depth];
}

