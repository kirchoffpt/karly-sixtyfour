
//linked list of chess pos

#include "postree.h"

postree::postree(chess_pos rootpos, int max_depth){

    max_depth = std::max(max_depth, 0);

    nodes.assign(max_depth+1, nullptr);
    for(auto &node : nodes){
        node = new chess_pos();
    }

    *nodes[0] = rootpos;

    //set next, prev pointers
    for(int i=1;i<(nodes.size()-1);i++){
        nodes[i]->next = nodes[i+1];
        nodes[i]->prev = nodes[i-1];
    }

    //set endpoint pointers
    if(nodes.size() > 1){
        nodes[0]->next = nodes[1];
        nodes[nodes.size()-1]->prev = nodes[nodes.size()-2];
    }

    //set ply
    for(int i=0;i<nodes.size();i++){
        nodes[i]->ply = i;
    }

}

postree::~postree(){
    for(auto &node : nodes){
        if(node != nullptr) delete node;
        node = nullptr;
    }
}

chess_pos* postree::root(){
    return nodes[0];
}

