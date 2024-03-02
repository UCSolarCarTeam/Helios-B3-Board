#include "PQueue.hpp"
#include <cassert>
#include "CubeDefines.hpp"

void test_pqueue_send_receive() {
    PQueue<int, 5> pQueue;
    
    // Send items to the queue
    for (int i = 0; i < 5; ++i) {
        bool sent = pQueue.Send(i, Priority::NORMAL);
        // Assert that sending was successful
        assert(sent);
    }

    // Receive items from the queue
    for (int i = 0; i < 5; ++i) {
        int received;
        bool receivedSuccessfully = pQueue.Receive(received);
        // Assert that receiving was successful
        assert(receivedSuccessfully);
        // Assert that the received item is correct
        assert(received == i);
    }
}

void test_pqueue_full() {
    PQueue<int, 2> pQueue;

    // Fill up the queue
    pQueue.Send(1, Priority::NORMAL);
    pQueue.Send(2, Priority::NORMAL);

    // Attempt to send another item (queue should be full)
    bool sent = pQueue.Send(3, Priority::NORMAL);
    // Assert that sending fails because the queue is full
    assert(!sent);
}

void test_pqueue_empty() {
    PQueue<int, 5> pQueue;

    // Attempt to receive an item from an empty queue
    int received;
    bool receivedSuccessfully = pQueue.Receive(received);
    // Assert that receiving fails because the queue is empty
    assert(!receivedSuccessfully);
}

int main() {
    test_pqueue_send_receive();
    test_pqueue_full();
    test_pqueue_empty();

    return 0;
}
