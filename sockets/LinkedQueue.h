#pragma once

namespace Network
{
	template <typename T>
	class LinkedQueue
	{
		struct Node
		{
			Node* next;
			T data;

			Node(const T& data) : data(data), next(nullptr) {}
		};

	private:
		Node* head;
		Node* tail;

	public:
		LinkedQueue();
		~LinkedQueue();

		LinkedQueue(const LinkedQueue& other) = delete;

		void Enqueue(const T& data);
		T* Dequeue();
		void Print()const;
	};

	template <typename T>
	LinkedQueue<T>::LinkedQueue()
		:head(nullptr), tail(nullptr)
	{
	}

	template <typename T>
	LinkedQueue<T>::~LinkedQueue()
	{
		if (head == nullptr)
		{
			return;
		}

		Node* temp = head;

		while (temp != nullptr)
		{
			Node* temp2 = temp->next;
			delete temp;
			temp = temp2;
		}
	}

	template <typename T>
	void LinkedQueue<T>::Enqueue(const T& data)
	{
		static_assert(std::is_standard_layout<T>::value, "Data is too complex to insert into the queue");

		Node* newNode = new Node(data);

		// List is empty set the head to point to a new Node
		if (tail == nullptr)
		{
			head = newNode;
		}
		else
		{
			// If list is not empty we point what the tail is pointing to and points its next to the new node.
			tail->next = newNode;
		}

		// Adjust the tail to point to the end of the list, the new Node.
		tail = newNode;

		std::cout << "Added a new node!" << std::endl;
	}

	template <typename T>
	T* LinkedQueue<T>::Dequeue()
	{
		// List is empty
		if (head == nullptr)
		{
			std::cout << "Dequeue: Cannot remove from an empty list!" << std::endl;
			return nullptr;
		}

		T* result = &head->data;
		Node* temp = head;
		head = head->next;
		delete temp;

		if (head == nullptr)
		{
			tail = nullptr;
		}

		std::cout << "Dequeued a node!" << std::endl;

		return result;
	}

	template <typename T>
	void LinkedQueue<T>::Print() const
	{
		if (head == nullptr)
		{
			return;
		}

		Node* temp = head;

		std::cout << "The list contains: ";

		while (temp != nullptr)
		{
			std::cout << temp->data;
			if (temp->next != nullptr)
			{
				std::cout << ", ";
			}
			temp = temp->next;
		}

		std::cout << std::endl;
	}
}