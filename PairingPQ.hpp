// Project identifier: 43DE0E0C4C76BFAA6D8C2F5AEAE0518A9C42CF4E

#ifndef PAIRINGPQ_H
#define PAIRINGPQ_H

#include <deque>
#include <utility>

#include "Eecs281PQ.hpp"

// A specialized version of the priority queue ADT implemented as a pairing
// heap.
template<typename TYPE, typename COMP_FUNCTOR = std::less<TYPE>>
class PairingPQ : public Eecs281PQ<TYPE, COMP_FUNCTOR> {
    // This is a way to refer to the base class object.
    using BaseClass = Eecs281PQ<TYPE, COMP_FUNCTOR>;

public:
    // Each node within the pairing heap
    class Node {
    public:
        // Description: Custom constructor that creates a node containing
        //              the given value.
        explicit Node(const TYPE &val)
            : elt { val } {}

        // Description: Allows access to the element at that Node's position.
        //              There are two versions, getElt() and a dereference
        //              operator, use whichever one seems more natural to you.
        // Runtime: O(1) - this has been provided for you.
        const TYPE &getElt() const { return elt; }
        const TYPE &operator*() const { return elt; }

        // The following line allows you to access any private data
        // members of this Node class from within the PairingPQ class.
        // (ie: myNode.elt is a legal statement in PairingPQ's add_node()
        // function).
        friend PairingPQ;

    private:
        TYPE elt;
        Node *child = nullptr;
        Node *sibling = nullptr;
        
        Node *parent = nullptr;
    };


    // Description: Construct an empty pairing heap with an optional
    //              comparison functor.
    // Runtime: O(1)
    explicit PairingPQ(COMP_FUNCTOR comp = COMP_FUNCTOR())
        : BaseClass { comp } {
        nodeCount = 0;
        root = nullptr;
    }


    // Description: Construct a pairing heap out of an iterator range with an
    //              optional comparison functor.
    // Runtime: O(n) where n is number of elements in range.
    template<typename InputIterator>
    PairingPQ(InputIterator start, InputIterator end, COMP_FUNCTOR comp = COMP_FUNCTOR())
        : BaseClass { comp } {
        nodeCount = 0;
        root = nullptr;
        for (auto i = start; i != end; ++i) {
            addNode(*i);
        }
    }


    // Description: Copy constructor.
    // Runtime: O(n)
    PairingPQ(const PairingPQ &other)
        : BaseClass { other.compare } {
        nodeCount = 0;
        root = nullptr;
        std::deque<TYPE> elts;
        makeDeque(other.root, elts);
        for (const auto &elmt : elts) {
            addNode(elmt);
        }
    }


    // Description: Copy assignment operator.
    // Runtime: O(n)
    PairingPQ &operator=(const PairingPQ &rhs) {
        if (this != &rhs) {
            PairingPQ temp(rhs);
            swap(temp);
        }
        return *this;
    }


    // Description: Destructor
    // Runtime: O(n)
    ~PairingPQ() {
        destroy(root);
        nodeCount = 0;
        root = nullptr;
    }

    // Description: Move constructor and assignment operators don't need any
    //              code, the members will be reused automatically.
    PairingPQ(PairingPQ &&) noexcept = default;
    PairingPQ &operator=(PairingPQ &&) noexcept = default;


    // Description: Assumes that all elements inside the pairing heap are out
    //              of order and 'rebuilds' the pairing heap by fixing the
    //              pairing heap invariant.  You CANNOT delete 'old' nodes
    //              and create new ones!
    // Runtime: O(n)
    virtual void updatePriorities() {
        if (!root) return;
        std::deque<Node*> nodes;
        makeDeque(root, nodes);
        root = nullptr;
        for (Node* node : nodes) {
            node->child = nullptr;
            node->sibling = nullptr;
            node->parent = nullptr;
        }
        for (Node* node : nodes) {
            root = meld(root, node);
        }
    }

    // Description: Add a new element to the pairing heap. This is already
    //              done. You should implement push functionality entirely
    //              in the addNode() function, and this function calls
    //              addNode().
    // Runtime: O(1)
    virtual void push(const TYPE &val) { addNode(val); }  // push()


    // Description: Remove the most extreme (defined by 'compare') element
    //              from the pairing heap.
    // Note: We will not run tests on your code that would require it to pop
    // an element when the pairing heap is empty. Though you are welcome to
    // if you are familiar with them, you do not need to use exceptions in
    // this project.
    // Runtime: Amortized O(log(n))
    virtual void pop() {
        Node* oldRoot = root;
        if (!root->child) {
            root = nullptr;
        }
        else {
            Node* current = root->child;
            current->parent = nullptr;

            Node* nextSibling = current->sibling;
            current->sibling = nullptr;
            while (nextSibling) {
                Node* next = nextSibling->sibling;
                nextSibling->sibling = nullptr;

                if (this->compare(nextSibling->elt, current->elt)) {
                    nextSibling->parent = nullptr;
                    current->parent = nextSibling;
                    current->sibling = nextSibling->child;
                    if (current->sibling) current->sibling->parent = nextSibling;
                    nextSibling->child = current;
                    current = nextSibling;
                }
                else {
                    nextSibling->parent = current;
                    nextSibling->sibling = current->child;
                    if (nextSibling->sibling) nextSibling->sibling->parent = current;
                    current->child = nextSibling;
                }
                nextSibling = next;
            }
            root = current;
            root->parent = nullptr;
        }
        delete oldRoot;
        nodeCount -= 1;
    }

    // Description: Return the most extreme (defined by 'compare') element of
    //              the pairing heap. This should be a reference for speed.
    //              It MUST be const because we cannot allow it to be
    //              modified, as that might make it no longer be the most
    //              extreme element.
    // Runtime: O(1)
    virtual const TYPE &top() const {
        return root->elt;
    }

    // Description: Get the number of elements in the pairing heap.
    // Runtime: O(1)
    [[nodiscard]] virtual size_t size() const {
        return nodeCount;
    }

    // Description: Return true if the pairing heap is empty.
    // Runtime: O(1)
    [[nodiscard]] virtual bool empty() const {
        return (nodeCount == 0);
    }

    // Description: Updates the priority of an element already in the pairing
    //              heap by replacing the element refered to by the Node with
    //              new_value.  Must maintain pairing heap invariants.
    //
    // PRECONDITION: The new priority, given by 'new_value' must be more
    //              extreme (as defined by comp) than the old priority.
    //
    // Runtime: As discussed in reading material.
    void updateElt(Node* node, const TYPE &new_value) {
        node->elt = new_value;
        if (node != root) {
            if (node->sibling) {
                node->sibling->parent = node->parent;
            }
            if (node->parent->child == node) {
                node->parent->child = node->sibling;
            }
            else {
                Node* prevSibling = node->parent->child;
                while (prevSibling->sibling != node) {
                    prevSibling = prevSibling->sibling;
                }
                prevSibling->sibling = node->sibling;
            }

            node->sibling = nullptr;
            node->parent = nullptr;

            root = meld(root, node);
        }
    }

    // Description: Add a new element to the pairing heap. Returns a Node*
    //              corresponding to the newly added element.
    // Runtime: O(1)
    // NOTE: Whenever you create a node, and thus return a Node *, you must
    //       be sure to never move or copy/delete that node in the future,
    //       until it is eliminated by the user calling pop(). Remember this
    //       when you implement updateElt() and updatePriorities().
    Node* addNode(const TYPE &val) {
        Node* newNode = new Node(val);
        root = meld(root, newNode);
        nodeCount++;
        return newNode;
    }


private:
    // TODO: Add any additional member variables or member functions you
    // require here.

    Node* root = nullptr;
    std::size_t nodeCount = 0;

    Node* meld(Node* first, Node* second) {
        if (!first) return second;
        if (!second) return first;

        if (this->compare(second->elt, first->elt)) {
            second->parent = nullptr;
            first->parent = second;
            first->sibling = second->child;
            if (first->sibling) {
                first->sibling->parent = second;
            }
            second->child = first;
            return second;
        }
        else {
            second->parent = first;
            second->sibling = first->child;
            if (second->sibling) {
                second->sibling->parent = first;
            }
            first->child = second;
            return first;
        }
    }

    void destroy(Node* node) {
        if (node) {
            destroy(node->child);
            destroy(node->sibling);
            delete node;
        }
    }

    void makeDeque(Node* node, std::deque<Node*>& nodes) {
        if (node) {
            nodes.push_back(node);
            makeDeque(node->sibling, nodes);
            makeDeque(node->child, nodes);
        }
    }

    void swap(PairingPQ &other) noexcept {
        using std::swap;
        swap(root, other.root);
        swap(nodeCount, other.nodeCount);
        swap(this->compare, other.compare);
    }

    // NOTE: For member variables, you are only allowed to add a "root
    //       pointer" and a "count" of the number of nodes. Anything else
    //       (such as a deque) should be declared inside of member functions
    //       as needed.
};


#endif  // PAIRINGPQ_H
