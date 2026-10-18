#pragma once

struct Node {
    Node* next;
    Node* prev;
};

class ListIter {
  private:
    Node* m_cur;

  public:
    ListIter(Node* node) : m_cur(node) {}

    Node* next() {
        if (m_cur) {
            auto ret = m_cur;
            m_cur = m_cur->next;
            return ret;
        }
        return nullptr;
    }
};

class List {
 private:
    Node* m_head;

 public:
    List() = default;

    bool is_empty() { return m_head == nullptr; }
    Node* get_head() const { return m_head; }
    ListIter iter() { return ListIter(m_head); }

    void remove_node(const Node& node) {
        auto prev_node = node.prev;
        auto next_node = node.next;

        if (prev_node) {
            prev_node->next = next_node;
        }

        if (next_node) {
            next_node->prev = prev_node;
        }

        if (&node == m_head) {
            m_head = next_node;
        }
    }

    void insert(Node& new_node) {
        if (m_head == nullptr) {
            m_head = &new_node;
            return;
        }

        auto itr = this->iter();
        for (auto node = itr.next(); node != nullptr; node = itr.next()) {
            if (node->next == nullptr || node->next > (&new_node)) {
                auto next = node->next;
                node->next = &new_node;
                new_node.prev = node;
                new_node.next = next;
                if (next) {
                    next->prev = &new_node;
                }
            }
        }
    }
};
