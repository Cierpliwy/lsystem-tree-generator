#include "LSystemTextModel.h"
#include <iostream>
#include <sstream>

LSystemTextModel::LSystemTextModel()
{
}

void LSystemTextModel::process(LSystem &l, int recursion) {
    QString tmp_buffer;
    for(int i = 0; i <=recursion; i++){
        tmp_buffer = l.generate(i).c_str();
        for(int i = 0; i < tmp_buffer.size(); i++ ){
            QChar c = tmp_buffer.at(i);
            std::map<char, Command>::const_iterator it = l.getDefinitionMap().find(c.toAscii());
            if(it != l.getDefinitionMap().end()){
                buffer_.append(it->second.name.c_str());
                buffer_.append(" ");
                for(unsigned int i = 0; i < it->second.args; ++i){
                    std::stringstream ss;
                    ss << it->second.argv[i] << " ";
                    buffer_.append(ss.str().c_str());
                }
                buffer_.append(" ");
            }
        }
        std::cout << tmp_buffer.toStdString() << " " <<  buffer_.toStdString() <<std::endl;
        buffer_.clear();
    }
}
