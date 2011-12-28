#include "LSystem.h"
using std::cout;
using std::string;
using std::vector;
using std::map;

LSystem::LSystem(){
    //ctor
}

LSystem::~LSystem(){
    //dtor
}

//Generuje ciąg L-systemu dla podanej generacji.
const string& LSystem::generate(int recursion_depth) {
    //ustaw ostatnia wygenerowana glebokosc rekurencji
    int curr_depth = recursions_.size();

    // Jezeli wymagamy glebszej rekurencji
    if(curr_depth <= recursion_depth ) {
        //jezeli wczesniej nie byla generowana zadna rekurencja, zwroc wyraz poczatkowy
        if (curr_depth == 0 && recursion_depth == 0) {
             recursions_.push_back(axiom_);
             return axiom_;
        }else if (curr_depth == 0){
             recursions_.push_back(axiom_);
             curr_depth = recursions_.size();
        }

        string next_recursion;
        map<char, string>::iterator it;
        for(int recursion = curr_depth; recursion <= recursion_depth; ++recursion ){
            string curr_recursion = recursions_.back();
            next_recursion = "";
            for(unsigned int i = 0; i < curr_recursion.size(); ++i){
                 it = rules_.find(curr_recursion[i]);
                 if(it != rules_.end()){
                     next_recursion += (*it).second;
                 } else {
                     next_recursion += curr_recursion[i];
                 }
            }
            recursions_.push_back(next_recursion);
        }
        return recursions_.back();
    }else {
        //jezeli dana rekurencja byla juz wygenerowana
        vector<string>::iterator it = recursions_.begin();
        for(int i = 0; i < recursion_depth; ++i){
            ++it;
        }
        return (*it);
    }


}
