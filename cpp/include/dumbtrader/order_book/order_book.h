#ifndef DUMBTRADER_ORDER_BOOK_ORDER_BOOK_H_
#define DUMBTRADER_ORDER_BOOK_ORDER_BOOK_H_

#include "dumbtrader/order_book/order.h"

#include <limits>
#include <memory>
#include <stack>

namespace dumbtrader::orderbook {

struct DataNode {
    int ord;
    float qty;
    float px;
    std::shared_ptr<DataNode> next;
};

struct SkipNode {
    int totalPopLvl;
    int totalOrd;
    float totalQty;
    float px;
    std::shared_ptr<SkipNode> next;
    std::shared_ptr<SkipNode> down; // use variant?
    std::shared_ptr<DataNode> data;
};

class OrderBook {
public:
    OrderBook() : 
        askRootDataNode_(std::make_shared<DataNode>(DataNode{0, 0, std::numeric_limits<float>::min(), nullptr})),
        bidRootDataNode_(std::make_shared<DataNode>(DataNode{0, 0, std::numeric_limits<float>::max(), nullptr})),
        askRootSkipNode_(std::make_shared<SkipNode>(SkipNode{0, 0, 0, std::numeric_limits<float>::min(), nullptr, nullptr, askRootDataNode_})),
        bidRootSkipNode_(std::make_shared<SkipNode>(SkipNode{0, 0, 0, std::numeric_limits<float>::max(), nullptr, nullptr, bidRootDataNode_}))
    {}

    template<OrderSide side>
    void update(float px, float qty, int ord);
private:
    std::shared_ptr<DataNode> askRootDataNode_;
    std::shared_ptr<DataNode> bidRootDataNode_;
    std::shared_ptr<SkipNode> askRootSkipNode_;
    std::shared_ptr<SkipNode> bidRootSkipNode_;
};

template<>
void OrderBook::update<OrderSide::ASK>(float px, float qty, int ord) {
    std::shared_ptr<SkipNode> &curSkipNode = askRootSkipNode_;
    std::stack<SkipNode*> path;
    
    while (true) {
        path.push(curSkipNode.get());
        if (curSkipNode->next && px >= curSkipNode->next->px) {
            curSkipNode = curSkipNode->next;
            continue;
        }
        if (curSkipNode->down) {
            curSkipNode = curSkipNode->down;
        } else {
            break;
        }
    }

    std::shared_ptr<DataNode> &curDataNode = curSkipNode->data;


    int lvlDelta = 1;
    int ordDelta = ord;
    float qtyDelta = qty;
    bool updateSkipPx = false;

    if (px < curDataNode->px) {
        // new node push front
        updateSkipPx = true;
        auto newDataNode = std::make_shared<DataNode>(DataNode{ord, qty, px, std::move(path.top()->data)});
        path.top()->data = std::move(newDataNode);
    }
    
    while (true) {
        if (px == curDataNode->px) {
            // update current
            lvlDelta = 0;
            ordDelta = ord - curDataNode->ord;
            qtyDelta = qty - curDataNode->qty;
            curDataNode->ord = ord;
            curDataNode->qty = qty;
            break;
        } 
        if (curDataNode->next) {
            if (px < curDataNode->next->px) {
                // new node insert
                auto newDataNode = std::make_shared<DataNode>(DataNode{ord, qty, px, std::move(curDataNode->next)});
                curDataNode->next = std::move(newDataNode);
                break;
            } else {
                // search next
                curDataNode = curDataNode->next;
            }
        } else {
            // new node push back
            auto newDataNode = std::make_shared<DataNode>(DataNode{ord, qty, px, nullptr});
            curDataNode->next = std::move(newDataNode);
            break;
        }
    }
    while(!path.empty()) {
        SkipNode* cur = path.top();
        path.pop();
        cur->totalPopLvl += lvlDelta;
        cur->totalQty += qtyDelta;
        cur->totalOrd += ordDelta;
        if (updateSkipPx) {
            cur->px = px;
        }
    }
}

} // namespace dumbtrader::orderbook

#endif // #define DUMBTRADER_ORDER_BOOK_ORDER_BOOK_H_
