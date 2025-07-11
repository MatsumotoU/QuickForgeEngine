#pragma once
#include <cstddef>

struct IntrusiveListNode {
	IntrusiveListNode* prev;
	IntrusiveListNode* next;

	// コンストラクタ
	IntrusiveListNode() : prev(nullptr), next(nullptr) {}

	// コピーコンストラクタと代入演算子を削除
	IntrusiveListNode(const IntrusiveListNode&) = delete;
	IntrusiveListNode& operator=(const IntrusiveListNode&) = delete;
};

// リストに入れたい基底クラス
class IntrusiveListElement {
public:
	IntrusiveListNode node;
	IntrusiveListElement() = default;
	virtual ~IntrusiveListElement() = default;
};

// リスト本体
class IntrusiveList {
private:
	IntrusiveListNode head; // リストの先頭ノード
	IntrusiveListNode tail; // リストの末尾ノード
	size_t m_size;

public:
	IntrusiveList() : m_size(0) {
		head.next = &tail;
		tail.prev = &head;
	}
	void push_back(IntrusiveListElement* element) {
		element->node.prev = tail.prev;
		element->node.next = &tail;
		tail.prev->next = &element->node;
		tail.prev = &element->node;
		m_size++;
	}
	void pop_front() {
		if (m_size == 0) return;
		IntrusiveListNode* frontNode = head.next;
		head.next = frontNode->next;
		head.next->prev = &head;
		m_size--;
	}
	size_t size() const {
		return m_size;
	}
	bool empty() const {
		return m_size == 0;
	}
};